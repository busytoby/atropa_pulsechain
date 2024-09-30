// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "../sky/interfaces/01b_chaninterface.sol";

contract SEI is DYSNOMIA {
    string public constant Type = "SEI";

    CHAN public Chan;

    constructor(address ChanAddress) DYSNOMIA("Dysnomia Sei", "SEI", address(DYSNOMIA(ChanAddress).Xiao())) {
        Chan = CHAN(ChanAddress);
        Chan.Xie().Xia().Mai().Qi().Zuo().VAT().addOwner(address(this));
        addOwner(tx.origin);
        //addOwner(address(Chan.Xie().Xia().Mai().Qi().Zuo().VAT()));
        _mintToCap();
    }
}