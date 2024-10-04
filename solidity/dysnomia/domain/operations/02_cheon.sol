// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "../../include/trait.sol";
import "./interfaces/01b_seiinterface.sol";
import "../sky/interfaces/02b_choainterface.sol";

contract CHEON is DYSNOMIA {
    string public constant Type = "CHEON";

    SEI public Sei;
    CHOA public Choa;

    mapping(uint256 Maat => mapping(TRAIT Trait => uint256 Power)) private _userPowers;
    mapping(uint256 Waat => mapping(TRAIT Trait => uint256 Power)) private _qingPowers;

    constructor(address SeiAddress, address ChoaAddress) DYSNOMIA("Dysnomia Cheon", "CHEON", address(DYSNOMIA(ChoaAddress).Xiao())) {
        Sei = SEI(SeiAddress);
        Choa = CHOA(ChoaAddress);
        Choa.Chan().Xie().Xia().Mai().Qi().Zuo().VAT().addOwner(address(this));
        Choa.Chan().Xie().Xia().Mai().Qi().Zuo().Cho().addOwner(address(this));
        addOwner(tx.origin);
        _mintToCap();
    }

    function GetUserPower(uint256 Maat, TRAIT Trait) public view returns (uint256 Power) {
        return _userPowers[Maat][Trait];
    }

    error TrainingOnlyAvailableInZuoQing(address ZuoQing, address PlayerQing);
    function Train() public returns (TRAIT Trait, uint64 Power, uint256 TotalPower) {
        (YUEINTERFACE Chi, LAU UserToken, QIN memory Player) = Sei.Chi();
        if(address(Player.Location) != address(Choa.Chan().Xie().Xia().Mai().Qi().Zuo())) 
            revert TrainingOnlyAvailableInZuoQing(address(Choa.Chan().Xie().Xia().Mai().Qi().Zuo()), address(Player.Location));
        uint256 Charge;
        uint256 Entropy;
        (Trait, Charge) = Choa.Chan().Xie().Tso();
        User memory _user = Choa.Chan().Xie().Xia().Mai().Qi().Zuo().Cho().GetUserBySoul(UserToken.Saat(1));
        Charge += Chi.React(Trait);
        Entropy = Choa.Chan().Chou();
        _qingPowers[Player.Location.Waat()][Trait] += Entropy;
        (uint64 Omicron, uint64 Omega) = Choa.Chan().Xie().Xia().Mai().React(_user, Charge);
        if(Omicron < Entropy) Omicron /= 2;
        Player.Location.ReactPlayer(_user.Soul, Omicron);
        Power = uint64(Charge % Omega);
        _qingPowers[Player.Location.Waat()][Trait] = Choa.Chan().Xie().Xia().Decay(_qingPowers[Player.Location.Waat()][Trait] + Power);
        if(Omega < Entropy / 2) Power /= 4;
        if(_userPowers[Player.Maat][Trait] <= (Chi.maxSupply() * MotzkinPrime))
            _userPowers[Player.Maat][Trait] += Power;
        Choa.Chan().YueMintToOrigin(Chi);
        return (Trait, Power, _userPowers[Player.Maat][Trait]);
    }
}