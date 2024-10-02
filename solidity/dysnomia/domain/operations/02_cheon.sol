// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "./interfaces/01b_seiinterface.sol";
import "../sky/interfaces/02b_choainterface.sol";


contract CHEON is DYSNOMIA {
    string public constant Type = "CHEON";

    SEI public Sei;
    CHOA public Choa;

    mapping(uint256 Maat => mapping(string Adjective => uint256 Power)) private _userPowers;

    constructor(address SeiAddress, address ChoaAddress) DYSNOMIA("Dysnomia Cheon", "CHEON", address(DYSNOMIA(ChoaAddress).Xiao())) {
        Sei = SEI(SeiAddress);
        Choa = CHOA(ChoaAddress);
        Choa.Chan().Xie().Xia().Mai().Qi().Zuo().VAT().addOwner(address(this));
        Choa.Chan().Xie().Xia().Mai().Qi().Zuo().Cho().addOwner(address(this));
        addOwner(tx.origin);
        _mintToCap();
    }

    error TrainingOnlyAvailableInZuoQing(address ZuoQing, address PlayerQing);
    function Train() public returns (string memory Adjective, uint64 Power, uint256 TotalPower) {
        (YUEINTERFACE Chi, LAU UserToken, QIN memory Player) = Sei.Chi();
        if(address(Player.Location) != address(Choa.Chan().Xie().Xia().Mai().Qi().Zuo())) 
            revert TrainingOnlyAvailableInZuoQing(address(Choa.Chan().Xie().Xia().Mai().Qi().Zuo()), address(Player.Location));
        uint256 Charge;
        (Adjective, Charge) = Choa.Chan().Xie().Tso();
        if(Charge > 0) {
            User memory _user = Choa.Chan().Xie().Xia().Mai().Qi().Zuo().Cho().GetUserBySoul(UserToken.Saat(1));
            Chi.React(Adjective);
            (uint64 Omicron, uint64 Omega) = Choa.Chan().Xie().Xia().Mai().React(_user, Charge);
            Player.Location.ReactPlayer(_user.Soul, Omicron);
            Power = uint64(Charge % Omega);
            _userPowers[Player.Maat][Adjective] += Power;
            return (Adjective, Power, _userPowers[Player.Maat][Adjective]);
        }
        return (Adjective, 0, 0);
    }
}