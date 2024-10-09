// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "./interfaces/03b_xiainterface.sol";

contract XIE is DYSNOMIA {
    string public constant Type = "XIE";

    XIA public Xia;
    SHIO public Fornax;

    constructor(address XiaAddress) DYSNOMIA("Dysnomia Xie", "XIE", address(DYSNOMIA(XiaAddress).Xiao())) {
        Xia = XIA(XiaAddress);
        addOwner(tx.origin);

        Fornax = SHIO(Xia.Mai().Qi().Zuo().Cho().Void().Nu().Psi().Mu().Tau().Upsilon().Eta().Psi());
        _mintToCap();
    }

    function Power(uint256 QingWaat) public returns (uint256 Charge, uint256 Omicron, uint256 Omega) {
        User memory Alpha = Xia.Mai().Qi().Zuo().Cho().GetUser();
        QINGINTERFACE Qing = Xia.Mai().Qi().Zuo().GetQing(QingWaat);
        Charge = Xia.Charge(QingWaat);
        Omicron = Fornax.balanceOf(Alpha.On.Phi) / Alpha.Entropy;
        Omega = Fornax.balanceOf(address(Qing)) / Qing.Entropy();
    }
}