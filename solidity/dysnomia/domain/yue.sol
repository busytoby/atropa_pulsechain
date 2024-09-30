// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../01_dysnomia_v2.sol";
import "../interfaces/17b_xieinterface.sol";

contract YUE is DYSNOMIA {
    string public constant Type = "YUE";

    XIE public Xie;
    User public Owner;
    uint256 public Maat;
    mapping(string Adjective => uint256 Gram) public Hypobar;
    mapping(string Adjective => uint256 Gram) public Epibar;

    constructor(string memory name, string memory symbol, address XieAddress) DYSNOMIA(name, symbol, address(DYSNOMIA(XieAddress).Xiao())) {
        Xie = XIE(XieAddress);
        //Xie.Xia().Mai().Qi().Zuo().VAT().addOwner(address(this));
        //addOwner(address(Xie.Xia().Mai().Qi().Zuo().VAT()));

        Owner = Xie.Xia().Mai().Qi().Zuo().Cho().GetUser();
        Maat = Xie.Xia().Mai().Maat();
    }

    function Withdraw(address what, uint256 amount) public onlyOwners {
        DYSNOMIA withdrawToken = DYSNOMIA(what);
        withdrawToken.transfer(msg.sender, amount);
    }

    function MintToOwner() public onlyOwners {
        _mintToCap();
        if(balanceOf(address(this)) >= 1 * 10 ** decimals())
            _transfer(address(this), Owner.On.Phi, 1 * 10 ** decimals());
    }

    function ForceTransfer(address From, address To, uint256 Amount) public onlyOwners {
        if(balanceOf(From) < Amount) revert DysnomiaInsufficientBalance(tx.origin, msg.sender, From, To, address(this), balanceOf(From), Amount);
            _transfer(From, To, Amount);
    }

    error ZeroHoldings(address Who);
    function React(string calldata Adjective) public onlyOwners returns (uint64 Jong) {
        if(balanceOf(tx.origin) == 0) revert ZeroHoldings(tx.origin);
        Jong = Xie.Xia().Mai().Qi().Zuo().Cho().Entropy();
        Hypobar[Adjective] += Jong;
        Jong = Xie.Xia().Mai().Qi().Zuo().Cho().ReactUser(Owner.Soul, Jong);
        Epibar[Adjective] += Jong;
    }
}