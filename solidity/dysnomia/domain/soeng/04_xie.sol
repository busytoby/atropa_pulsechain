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

        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());

        _addMarketRate(AFFECTIONContract, 1 * 10 ** decimals());
        _addMarketRate(XiaAddress, 1 * 10 ** decimals());
        _addMarketRate(address(Fornax), 1 * 10 ** decimals());

        _mintToCap();
    }

    function Power(uint256 QingWaat) public returns (uint256 Charge, uint256 Omicron, uint256 Omega) {
        _mintToCap();
        User memory Alpha = Xia.Mai().Qi().Zuo().Cho().GetUser();
        QINGINTERFACE Qing = Xia.Mai().Qi().Zuo().GetQing(QingWaat);
        Charge = Xia.Charge(QingWaat);
        Omicron = Fornax.balanceOf(Alpha.On.Phi) / Alpha.Entropy;
        Omega = Fornax.balanceOf(address(Qing)) / Qing.Entropy();
    }
}