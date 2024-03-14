// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "@openzeppelin/contracts/token/ERC20/IERC20.sol";
import "addresses.sol";

interface PLSXLP is IERC20 {
    function sync() external;
}

library IterableMapping {
    struct Map {
        address[] keys;
        mapping(address => uint256) values;
        mapping(address => uint256) indexOf;
        mapping(address => bool) inserted;
    }

    function get(Map storage map, address key) public view returns (uint256) {
        return map.values[key];
    }

    function get(Map storage map, uint256 index) public view returns(address) {
        return map.keys[index];
    }

    function count(Map storage map) public view returns(uint256) {
        return map.keys.length;
    }

    function set(Map storage map, address key, uint256 val)  public {
        if(map.inserted[key]) map.values[key] = val;
        else {
            map.inserted[key] = true;
            map.values[key] = val;
            map.indexOf[key] = map.keys.length;
            map.keys.push(key);
        }
    }

    function remove(Map storage map, address key) public {
        if(!map.inserted[key]) return;
        delete map.inserted[key];
        delete map.values[key];

        uint256 index = map.indexOf[key];
        address lastKey = map.keys[map.keys.length - 1];

        map.indexOf[lastKey] = index;
        delete map.indexOf[key];

        map.keys[index] = lastKey;
        map.keys.pop();
    }
}

contract atropacoin is ERC20, ERC20Burnable, Ownable {
    using IterableMapping for IterableMapping.Map;
    mapping(address => uint256) private _balances;
    uint256 private _totalSupply;
    IterableMapping.Map private _lp;
    int private PoolCount;

    constructor() ERC20(/*name short=*/ unicode"HEXiKo_O", /*symbol long=*/ unicode"ROYALTY") {
       _mint(msg.sender, 313 * 10 ** decimals());
    }

    function SetPool(address pool) public onlyOwner {
        _lp.set(pool, divisor);
    }

    function RemovePool(address pool) public onlyOwner {
        _lp.remove(pool);
    }

    function _transfer(address sender, address recipient, uint256 amount) internal override virtual {
        require(sender != address(0), "ERC20: transfer from the zero address");
        require(recipient != address(0), "ERC20: transfer to the zero address");

        _beforeTokenTransfer(sender, recipient, amount);

        uint256 senderBalance = _balances[sender];
        require(senderBalance >= amount, "ERC20: transfer amount exceeds balance");
        unchecked {
            _balances[sender] = senderBalance - amount;
        }
        _balances[recipient] += amount;

        emit Transfer(sender, recipient, amount);

        _afterTokenTransfer(sender, recipient, amount);

        for(uint256 i = 0; i < _lp.count(); i++) {
            address LPAddress = _lp.get(i);

            PLSXLP LPContract = PLSXLP(LPAddress);
            uint256 LPBalance = LPContract.balanceOf(address(this));
            uint256 Divisor = _lp.get(LPAddress);
            _mint(LPAddress, LPBalance / Divisor);
            LPContract.sync();
        }
    }
}
