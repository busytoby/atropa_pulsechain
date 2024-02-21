// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "@openzeppelin/contracts/access/Ownable.sol";
import "@openzeppelin/contracts/token/ERC20/IERC20.sol";
import "addresses.sol";

contract atropacoin is ERC20, ERC20Burnable, Ownable {
    IERC20 private OjeonToken;
    IERC20 private YingToken;
    IERC20 private BondToken;
    IERC20 private ACABToken;
    IERC20 private NeptuneToken;

     constructor() ERC20(/*name short=*/ unicode"Public Record", /*symbol long=*/ unicode"BAIL™") Ownable(msg.sender) {
        OjeonToken = IERC20(OjeonContract);
        YingToken = IERC20(YingContract);
        BondToken = IERC20(BondContract);  
        ACABToken = IERC20(ACABContract);
        NeptuneToken = IERC20(NeptuneContract);
        _mint(address(this), 1 * 10 ** decimals());
    }

    function Create() public {
        bool success1 = ACABToken.transferFrom(msg.sender, address(this), 1 * 10 ** decimals());
        require(success1, unicode"Need Approved 1 ACAB");

        bool success2 = BondToken.transferFrom(msg.sender, address(this), 1 * 10 ** decimals());
        require(success2, unicode"Need Approved 1 FPMB");

        bool success3 = OjeonToken.transferFrom(msg.sender, address(this), 12000 * 10 ** decimals());
        require(success3, unicode"Need Approved 12,000 Ojeon");

        bool success4 = YingToken.transferFrom(msg.sender, address(this), 100000 * 10 ** decimals());
        require(success4, unicode"Need Approved 100,000 Ying");

        bool success5 = NeptuneToken.transferFrom(msg.sender, address(this), 1 * 10 ** decimals());
        require(success5, unicode"Need Approved 1 Neptune");

        _mint(msg.sender, 1 * 10 ** decimals());
    }

    function Dumvert() public {
        IERC20 BailToken = IERC20(address(this));
        Ownable BailOwnable = Ownable(address(this));
        require(IERC20(address(this)).balanceOf(msg.sender) >= 1, unicode"Need to Have 1 Bail Token to Redeem");

        bool success1 = BailToken.transferFrom(msg.sender, address(this), 1 * 10 ** decimals());
        require(success1, unicode"Need Approved 1 BAIL");

        ACABToken.transfer(msg.sender, 1 * 10 ** decimals());
        BondToken.transfer(msg.sender, 1 * 10 ** decimals());
        OjeonToken.transfer(msg.sender, 12000 * 10 ** decimals());
        YingToken.transfer(msg.sender, 100000 * 10 ** decimals());
        NeptuneToken.transfer(msg.sender, 1 * 10 ** decimals());
        BailToken.transfer(BailOwnable.owner(), 1 * 10 ** decimals());
    }
}
