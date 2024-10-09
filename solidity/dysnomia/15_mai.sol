// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia_v2.sol";
import "./include/qin.sol";
import "./interfaces/14b_qiinterface.sol";

contract MAI is DYSNOMIA {
    string public constant Type = "MAI";

    QIINTERFACE public Qi;

    constructor(address QiAddress) DYSNOMIA("Dysnomia MAI", "MAI", address(DYSNOMIA(QiAddress).Xiao())) {
        Qi = QIINTERFACE(QiAddress);
        addOwner(tx.origin);
      
        _mintToCap();
    }

    function React(uint64 Soul, uint256 QingWaat) public returns (uint256 Mai) {
        User memory Alpha = Qi.Zuo().Cho().GetUserBySoul(Soul);
        QINGINTERFACE Qing = Qi.Zuo().GetQing(QingWaat);
        return Qing.balanceOf(Alpha.On.Phi) / Alpha.Entropy;
    }
}