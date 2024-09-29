// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../01_dysnomia_v2.sol";
import "./sky/interfaces/02b_choainterface.sol";

contract W00T is DYSNOMIA {
    string public constant Type = "W00T";

    CHOA public Choa;
    uint256 public Maat;

    constructor(string name, string symbol, address ChoaAddress) DYSNOMIA(name, symbol, address(DYSNOMIA(ChoaAddress).Xiao())) {
        Choa = CHOA(ChoaAddress);
        Maat = Choa.Chan().Xie().Xia().Mai().Maat();
        //Chan().Xie().Xia().Mai().Qi().Zuo().VAT().addOwner(address(this));
        addOwner(tx.origin);
        //addOwner(address(Chan.Xie().Xia().Mai().Qi().Zuo().VAT()));
        _mintToCap();
    }
}