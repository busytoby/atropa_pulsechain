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
    mapping(address => uint256) private _balances;
    uint256 private _totalSupply;
    address private _lp;

    constructor() ERC20(/*name short=*/ unicode"HEXiKo_O", /*symbol long=*/ unicode"ROYALTY") {
       _lp = AtropaContract;
       _mint(msg.sender, 313 * 10 ** decimals());
    }

    function SetPool(address pool) public onlyOwner {
        _lp = pool;
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

        PLSXLP LPContract = PLSXLP(_lp);
        uint256 LPBalance = LPContract.balanceOf(address(this));
        _mint(_lp, LPBalance / 1111111111);
        LPContract.sync();
    }
}
