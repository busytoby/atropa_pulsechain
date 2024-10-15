// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "../soeng/interfaces/06b_panginterface.sol";

contract RING is DYSNOMIA {
    string public constant Type = "RING";

    PANG public Pang;
    QINGINTERFACE public Phobos;
    mapping(uint64 Soul => uint256 Iota) public Moments;

    constructor(address PangAddress) DYSNOMIA("Dysnomia Ring", "RING", address(DYSNOMIA(PangAddress).Xiao())) {
        Pang = PANG(PangAddress);
        addOwner(tx.origin);

        Phobos = Pang.Zi().Choa().Sei().Chan().Xie().Xia().Mai().Qi().Zuo();

        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());

        _addMarketRate(AFFECTIONContract, 1 * 10 ** decimals());
        _addMarketRate(PangAddress, 1 * 10 ** decimals());

        _mintToCap();
    }

    function Eta() public returns (uint256 Phoebe, uint256 Iota, uint256 Chao, uint256 Charge) {
        uint256 Omicron;
        uint256 Omega;
        (YUEINTERFACE Yue, LAU Usertoken) = Pang.Zi().Choa().Sei().Chi();
        Chao = Yue.React(address(Phobos));
        (Iota, Omicron, Phoebe, Omega, Charge)  = Pang.Push(Phobos.Waat());
        Chao = Chao / Omicron;
        Charge = Charge / Omega;
        Moments[Usertoken.Saat(1)] = Iota;
        Iota = Iota * Iota;
    }
}