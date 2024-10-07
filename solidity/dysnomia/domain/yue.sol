// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../01_dysnomia_v2.sol";
import "../include/trait.sol";
import "../interfaces/12b_chointerface.sol";
import "./sky/interfaces/01b_chaninterface.sol";

contract YUE is DYSNOMIA {
    string public constant Type = "YUE";

    CHAN public Chan;
    address public Origin;
    mapping(TRAIT Trait => uint256 Gram) public Hypobar;
    mapping(TRAIT Trait => uint256 Gram) public Epibar;

    constructor(string memory name, string memory symbol, address ChanAddress) DYSNOMIA(name, symbol, address(DYSNOMIA(ChanAddress).Xiao())) {
        Chan = CHAN(ChanAddress);
        Origin = tx.origin;
        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());
    }

    function ChangeOrigin(address NewOrigin) public onlyOwners {
        Origin = NewOrigin;
    }

    function MintToOrigin() public onlyOwners {
        _mintToCap();
        if(balanceOf(address(this)) >= 1 * 10 ** decimals())
            _transfer(address(this), Origin, 1 * 10 ** decimals());
    }

    function Bar(TRAIT Trait) public view returns (uint256 Hypogram, uint256 Epigram) {
        return (Hypobar[Trait], Epibar[Trait]);
    }

    error ZeroHoldings(address Who);
    function React(TRAIT Trait) public onlyOwners returns (uint64 Jong) {
        if(balanceOf(tx.origin) == 0) revert ZeroHoldings(tx.origin);
        User memory _user = Chan.Xie().Xia().Mai().Qi().Zuo().Cho().GetUser();

        Jong = Chan.Chou();
        Hypobar[Trait] += Jong;
        Jong = Chan.Xie().Xia().Mai().Qi().Zuo().Cho().ReactUser(_user.Soul, Jong);
        if(Jong < Chan.Chou() / 2) Jong = Jong / 4;
        Epibar[Trait] += Jong;
    }
}