// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "./interfaces/05b_ziinterface.sol";

contract PANG is DYSNOMIA {
    string public constant Type = "PANG";

    ZI public Zi;

    constructor(address ZiAddress) DYSNOMIA("Dysnomia Pang", "PANG", address(DYSNOMIA(ZiAddress).Xiao())) {
        Zi = ZI(ZiAddress);
        addOwner(tx.origin);

        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());

        _addMarketRate(AFFECTIONContract, 1 * 10 ** decimals());
        _addMarketRate(ZiAddress, 1 * 10 ** decimals());

        _mintToCap();
    }

    function Push(uint256 QingWaat) public returns (uint256 Iota, uint256 Omicron, uint256 Eta, uint256 Omega, uint256 Charge) {
        QINGINTERFACE Qing = Zi.Choa().Sei().Chan().Xie().Xia().Mai().Qi().Zuo().GetQing(QingWaat);
        _mintToCap();
        (Charge, Omicron, Omega) = Zi.Choa().Sei().Chan().Xie().Power(QingWaat);
        (Iota, , , Eta) = Zi.Spin(QingWaat);
        Omicron = Xiao.modExp(Omicron, Charge, Zi.Choa().Yuan(address(Qing)));
        Iota = Xiao.modExp(Iota, Qing.Entropy(), Zi.Choa().Yuan(address(Qing)));
    }
}