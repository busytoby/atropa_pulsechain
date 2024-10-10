// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "../sky/interfaces/02b_choainterface.sol";

contract ZI is DYSNOMIA {
    string public constant Type = "ZI";

    CHOA public Choa;
    CHOINTERFACE public Tethys;

    constructor(address ChoaAddress) DYSNOMIA("Dysnomia Xie", "XIE", address(DYSNOMIA(ChoaAddress).Xiao())) {
        Choa = CHOA(ChoaAddress);
        addOwner(tx.origin);

        Tethys = Choa.Sei().Chan().Xie().Xia().Mai().Qi().Zuo().Cho();

        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());

        _addMarketRate(AFFECTIONContract, 1 * 10 ** decimals());
        _addMarketRate(ChoaAddress, 1 * 10 ** decimals());
        _addMarketRate(address(Tethys), 1 * 10 ** decimals());

        _mintToCap();
    }

    function Spin(uint256 QingWaat) public returns (uint256 Iota, uint256 Omicron, uint256 Omega, uint256 Eta) {
        uint256 Charge;
        User memory Alpha = Choa.Sei().Chan().Xie().Xia().Mai().Qi().Zuo().Cho().GetUser();
        QINGINTERFACE Qing = Choa.Sei().Chan().Xie().Xia().Mai().Qi().Zuo().GetQing(QingWaat);
        _mintToCap();
        (Charge, Omicron, Omega) = Choa.Sei().Chan().Xie().Power(QingWaat);
        Iota = Xiao.modExp(Omicron, Charge, Choa.Yuan(address(Tethys)));
        Omicron = Xiao.modExp(Omega, Charge, Choa.Yuan(address(Tethys)));
        Omega = Tethys.balanceOf(Alpha.On.Phi) / Alpha.Entropy;
        Eta = Tethys.balanceOf(address(Qing)) / Qing.Entropy();
    }
}