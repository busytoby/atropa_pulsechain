// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "../sky/interfaces/03b_ringinterface.sol";

contract META is DYSNOMIA {
    string public constant Type = "META";

    RING public Ring;

    constructor(address RingAddress) DYSNOMIA("Dysnomia Meta", "META", address(DYSNOMIA(RingAddress).Xiao())) {
        Ring = RING(RingAddress);
        addOwner(tx.origin);

        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());

        _addMarketRate(AFFECTIONContract, 1 * 10 ** decimals());
        _addMarketRate(RingAddress, 1 * 10 ** decimals());

        _mintToCap();
    }

    function Beat(uint256 QingWaat) public returns (uint256 Deimos, uint256 Yeo, uint256 Charge) {
        uint256 Omicron;
        uint256 Omega;
        uint256 Iota2;
        uint256 Dione;
        uint256 Charge2;
        QINGINTERFACE Qing = Ring.Pang().Zi().Choa().Sei().Chan().Xie().Xia().Mai().Qi().Zuo().GetQing(QingWaat);
        (uint256 Phoebe, uint256 Iota1, uint256 Chao, uint256 Charge1) = Ring.Eta();
        (Iota2, Omicron, Dione, Omega, Charge2) = Ring.Pang().Push(QingWaat);
        Deimos = Xiao.modExp(Dione, Phoebe, Ring.Pang().Zi().Choa().Yuan(address(Qing)));

        Yeo = Iota2 / Chao;
        Charge = Charge1 * Charge2 / Iota1;
    }
}