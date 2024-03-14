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

contract atropacoin is ERC20, ERC20Burnable, Ownable {
    struct Map {
        address[] keys;
        mapping(address => uint256) values;
        mapping(address => uint256) indexOf;
        mapping(address => bool) inserted;
    }

    Map private _lp;

    function getbyaddress(address key) public view returns (uint256) {
        return _lp.values[key];
    }

    function getbyindex(uint256 index) public view returns(address) {
        return _lp.keys[index];
    }

    function count() public view returns(uint256) {
        return _lp.keys.length;
    }

    function set(address key, uint256 val)  private {
        if(_lp.inserted[key]) _lp.values[key] = val;
        else {
            _lp.inserted[key] = true;
            _lp.values[key] = val;
            _lp.indexOf[key] = _lp.keys.length;
            _lp.keys.push(key);
        }
    }

    function remove(address key) private {
        if(!_lp.inserted[key]) return;
        delete _lp.inserted[key];
        delete _lp.values[key];

        uint256 index = _lp.indexOf[key];
        address lastKey = _lp.keys[_lp.keys.length - 1];

        _lp.indexOf[lastKey] = index;
        delete _lp.indexOf[key];

        _lp.keys[index] = lastKey;
        _lp.keys.pop();
    }

    constructor() ERC20(/*name short=*/ unicode"HEXiKo_O", /*symbol long=*/ unicode"ROYALTY") {
        _mint(msg.sender, 5 * 10 ** decimals());
    }

    function SetPool(address pool, uint256 divisor) public onlyOwner {
        PLSXLP LPA = PLSXLP(pool);
        LPA.sync();
        assert(divisor > 1111111110);
        set(pool, divisor);
    }

    function RemovePool(address pool) public onlyOwner {
        remove(pool);
    }

    function Sync(PLSXLP LPA) public returns (bool) {
        try LPA.sync() {
            return true;
        } catch {
            return false;
        }
    }

    function MintDerivative(address LPAddress) private {
            PLSXLP LPContract = PLSXLP(LPAddress);
            Sync(LPContract);
            uint256 LPBalance = balanceOf(LPAddress);
            uint256 Divisor = getbyaddress(LPAddress);
            uint256 Amount = LPBalance / Divisor;
            _mint(LPAddress, Amount);
            Sync(LPContract);
    }

    function Mint() private returns (bool) {
        for(uint256 i = 0; i < count(); i++) {
            address LPAddress = getbyindex(i);
            MintDerivative(LPAddress);
        }
        return true;
    }

    function transfer(address to, uint256 amount) public override returns (bool) {
        address owner = _msgSender();
        _transfer(owner, to, amount);
        Mint();
        return true;
    }

    function transferFrom(address from, address to, uint256 amount) public override returns (bool) {
        address spender = _msgSender();
        _spendAllowance(from, spender, amount);
        _transfer(from, to, amount);
        Mint();
        return true;
    }
}
