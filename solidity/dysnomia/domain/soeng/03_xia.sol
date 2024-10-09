// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "./interfaces/02b_maiinterface.sol";

contract XIA is DYSNOMIA {
    string public constant Type = "XIA";

    MAI public Mai;
    SHIO public Fomalhaute;

    constructor(address MaiAddress) DYSNOMIA("Dysnomia Xia", "XIA", address(DYSNOMIA(MaiAddress).Xiao())) {
        Mai = MAI(MaiAddress);
        addOwner(tx.origin);

        Fomalhaute = Mai.Qi().Zuo().Cho().Void().Nu().Psi().Mu().Tau().Upsilon().GetRodByIdx(Mai.Qi().Zuo().Cho().Void().Nu().Psi().Mu().Tau().Xi()).Shio;

        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());

        _addMarketRate(AFFECTIONContract, 1 * 10 ** decimals());
        _addMarketRate(MaiAddress, 1 * 10 ** decimals());
        _addMarketRate(address(Fomalhaute), 1 * 10 ** decimals());

        _mintToCap();
    }

    function Charge(uint256 QingWaat) public returns (uint256) {
        _mintToCap();
        User memory Alpha = Mai.Qi().Zuo().Cho().GetUser();

        uint256 _b = Mai.Qi().ReactWaat(QingWaat);
        uint256 _e = Mai.React(Alpha.Soul, QingWaat);
        uint256 _m = Fomalhaute.balanceOf(Alpha.On.Phi);
        return Xiao.modExp(_b, _e, _m);
    }
}