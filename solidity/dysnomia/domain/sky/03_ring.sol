// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "../soeng/interfaces/06b_panginterface.sol";

contract RING is DYSNOMIA {
    string public constant Type = "RING";

    PANG public Pang;
    QINGINTERFACE public Phobos;

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

    function Eta() public returns (uint256 Phoebe) {
        (, , Phoebe, ,) = Pang.Push(Phobos.Waat());
    }
}