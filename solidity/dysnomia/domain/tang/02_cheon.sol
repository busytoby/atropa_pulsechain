// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "./interfaces/01b_seiinterface.sol";

contract CHEON is DYSNOMIA {
    string public constant Type = "CHEON";

    SEI public Sei;

    constructor(address SeiAddress) DYSNOMIA("Dysnomia Cheon", "CHEON", address(DYSNOMIA(SeiAddress).Xiao())) {
        Sei = SEI(SeiAddress);
        Sei.Chan().Xie().Xia().Mai().Qi().Zuo().Cho().addOwner(address(this));
        Sei.Chan().addOwner(address(this));
        addOwner(tx.origin);

        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());

        _addMarketRate(AFFECTIONContract, 1 * 10 ** decimals());
        _addMarketRate(SeiAddress, 1 * 10 ** decimals());

        _mintToCap();
    }

    error TrainingOnlyAvailableInZuoQing(address ZuoQing, address PlayerQing);
    function Train(address Qing) public returns (uint256 Charge, uint256 Hypobar, uint256 Epibar) {
        (YUEINTERFACE Chi, LAU UserToken) = Sei.Chi();

        QINGINTERFACE _qing = QINGINTERFACE(Qing);
        Charge = Sei.Chan().ReactYue(Chi, Qing);

        _mintToCap();
        uint256 Mai = Sei.Chan().Xie().Xia().Mai().React(UserToken.Saat(1), _qing.Waat());
        if(Mai > 1 * 10 ** decimals()) Mai = 1 * 10 ** decimals();
            if(balanceOf(address(this)) >= Mai)
                _transfer(address(this), address(Chi), Mai);

        Sei.Chan().YueMintToOrigin(Chi);
        (Hypobar, Epibar) = Chi.Bar(Qing);
    }
}