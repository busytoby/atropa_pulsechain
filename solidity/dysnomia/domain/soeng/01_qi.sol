// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../include/user.sol";
import "../../01_dysnomia_v2.sol";
import "../dan/interfaces/03b_qinginterface.sol";

contract QI is DYSNOMIA {
    string public constant Type = "QI";

    QINGINTERFACE public Zuo;
    SHIO public Eris;

    constructor(address ZuoQingAddress) DYSNOMIA("DYSNOMIA Qi", "QI", address(DYSNOMIA(ZuoQingAddress).Xiao())) {
        Zuo = QINGINTERFACE(ZuoQingAddress);
        Eris = Zuo.Cho().Void().Nu().Psi().Mu().Tau().Upsilon().GetRodByIdx(Zuo.Cho().Void().Nu().Psi().Rho().Lai.Xi).Shio;
        addOwner(tx.origin);

        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());

        _addMarketRate(AFFECTIONContract, 1 * 10 ** decimals());
        _addMarketRate(ZuoQingAddress, 1 * 10 ** decimals());
        _addMarketRate(address(Eris), 1 * 10 ** decimals());

        _mintToCap();
    }

    function ReactSoul(uint64 Soul) public returns (uint256 Qi) {
        User memory Alpha = Zuo.Cho().GetUserBySoul(Soul);
        return Eris.balanceOf(Alpha.On.Phi) / Alpha.Entropy;
    }

    function ReactWaat(uint256 Waat) public view returns (uint256 Qi) {
        QINGINTERFACE Qing = Zuo.GetQing(Waat);
        return Eris.balanceOf(address(Qing)) / Qing.Entropy(); 
    }
}