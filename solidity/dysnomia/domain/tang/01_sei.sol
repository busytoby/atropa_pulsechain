// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "../yue.sol";
import "../sky/interfaces/01b_chaninterface.sol";

contract SEI is DYSNOMIA {
    string public constant Type = "SEI";

    CHAN public Chan;
    mapping(uint256 Maat => LAU UserToken) private _users;

    constructor(address ChanAddress) DYSNOMIA("Dysnomia Sei", "SEI", address(DYSNOMIA(ChanAddress).Xiao())) {
        Chan = CHAN(ChanAddress);
        Chan.Xie().Xia().Mai().Qi().Zuo().VAT().addOwner(address(this));
        addOwner(tx.origin);
        _mintToCap();
    }

    function GetUserTokenByMaat(uint256 Maat) public view returns (LAU) {
        return _users[Maat];
    }

    function ChiBySoul(uint64 Soul) public view returns (YUE Yue, LAU UserToken, QIN memory Player) {
        address UserTokenAddress = Chan.Xie().Xia().Mai().Qi().Zuo().Cho().GetAddressBySoul(Soul);
        if(Chan.Yan(UserTokenAddress) == address(0x0)) revert NotStarted(UserTokenAddress);
        Yue = YUE(Chan.Yan(UserTokenAddress));
        UserToken = LAU(Chan.Xie().Xia().Mai().Qi().Zuo().Cho().GetUserTokenAddress(UserTokenAddress));
        Player =  Chan.Xie().Xia().Mai().GetPlayerQin(Soul);
    }

    error NotStarted(address);
    function Chi() public view returns (YUE Yue, LAU UserToken, QIN memory Player) {        
        if(Chan.Yan(tx.origin) == address(0x0)) revert NotStarted(tx.origin);
        Yue = YUE(Chan.Yan(tx.origin));
        UserToken = LAU(Chan.Xie().Xia().Mai().Qi().Zuo().Cho().GetUserTokenAddress(tx.origin));
        Player =  Chan.Xie().Xia().Mai().GetPlayerQin(UserToken.Saat(1));
    }

    function Start(string calldata Name, string calldata Symbol) public returns (YUE Yue, LAU UserToken, QIN memory Player) {
        _mintToCap();
        if(Chan.Yan(tx.origin) == address(0x0)) {
            Yue = new YUE(Name, Symbol, address(Chan.Xie()), address(Chan));
            Yue.addOwner(address(Chan));
            Chan.Chou();
            Chan.AddYue(tx.origin, address(Yue));
            UserToken = LAU(Chan.Xie().Xia().Mai().Qi().Zuo().Cho().GetUserTokenAddress(tx.origin));
            Player =  Chan.Xie().Xia().Mai().GetPlayerQin(UserToken.Saat(1));
            _users[Player.Maat] = UserToken;
            Chan.Xie().Xia().Mai().MovePlayer(UserToken.Saat(1), address(Chan.Xie().Xia().Mai().Qi().Zuo()));
            return (Yue, UserToken, Player);
        }

        (Yue, UserToken, Player) = Chi();
        Yue.Rename(Name, Symbol);
        Chan.Chou();
    }
}