// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "../../include/trait.sol";
import "./interfaces/01b_seiinterface.sol";
import "../sky/interfaces/02b_choainterface.sol";

contract CHEON is DYSNOMIA {
    string public constant Type = "CHEON";

    SEI public Sei;

    constructor(address SeiAddress) DYSNOMIA("Dysnomia Cheon", "CHEON", address(DYSNOMIA(SeiAddress).Xiao())) {
        Sei = SEI(SeiAddress);
        Sei.Chan().Xie().Xia().Mai().Qi().Zuo().Cho().addOwner(address(this));
        addOwner(tx.origin);
        _mintToCap();
    }

    error TrainingOnlyAvailableInZuoQing(address ZuoQing, address PlayerQing);
    function Train() public returns (TRAIT Trait, uint64 Power, uint256 Hypobar, uint256 Epibar) {
        (YUEINTERFACE Chi, LAU UserToken, QIN memory Player) = Sei.Chi();
        if(address(Player.Location) != address(Sei.Chan().Xie().Xia().Mai().Qi().Zuo())) 
            revert TrainingOnlyAvailableInZuoQing(address(Sei.Chan().Xie().Xia().Mai().Qi().Zuo()), address(Player.Location));
        uint256 Charge;
        uint256 Entropy;
        (Trait, Charge) = Sei.Chan().Xie().Tso();
        User memory _user = Sei.Chan().Xie().Xia().Mai().Qi().Zuo().Cho().GetUserBySoul(UserToken.Saat(1));
        Charge += Chi.React(Trait);
        Entropy = Sei.Chan().Chou();
        (uint64 Omicron, uint64 Omega) = Sei.Chan().Xie().Xia().Mai().React(_user, Charge);
        if(Omicron < Entropy) Omicron /= 2;
        Player.Location.ReactPlayer(_user, Omicron);
        Power = uint64(Charge % Omega);
        if(Omega < Entropy / 2) Power /= 4;
        Sei.Chan().YueMintToOrigin(Chi);
        (Hypobar, Epibar) = Chi.Bar(Trait);
    }
}