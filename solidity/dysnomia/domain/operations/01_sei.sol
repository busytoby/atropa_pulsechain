// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "../yue.sol";
import "../sky/interfaces/01b_chaninterface.sol";

contract SEI is DYSNOMIA {
    string public constant Type = "SEI";

    CHAN public Chan;
    mapping(uint256 Maat => LAU UserToken) private _users;
    mapping(address => address Chi) private _yan;

    constructor(address ChanAddress) DYSNOMIA("Dysnomia Sei", "SEI", address(DYSNOMIA(ChanAddress).Xiao())) {
        Chan = CHAN(ChanAddress);
        Chan.Xie().Xia().Mai().Qi().Zuo().VAT().addOwner(address(this));
        addOwner(tx.origin);
        _mintToCap();
    }

    function GetUserTokenByMaat(uint256 Maat) public view returns (LAU) {
        return _users[Maat];
    }

    error NotStarted(address);
    function Chi() public view returns (YUE _chi, QIN memory Player) {
        if(_yan[tx.origin] == address(0x0)) revert NotStarted(tx.origin);
        _chi = YUE(_yan[tx.origin]);
        LAU UserToken = LAU(Chan.Xie().Xia().Mai().Qi().Zuo().Cho().GetUserTokenAddress(tx.origin));
        Player =  Chan.Xie().Xia().Mai().GetPlayerQin(UserToken.Saat(1));
    }

    function Start(string calldata Name, string calldata Symbol) public returns (YUE _chi, QIN memory Player) {
        _mintToCap();
        if(_yan[tx.origin] == address(0x0)) {
            _chi = new YUE(Name, Symbol, address(Chan.Xie()));
            _chi.addOwner(address(Chan));
            Chan.Chou();
            _yan[tx.origin] = address(_chi);
            LAU UserToken = LAU(Chan.Xie().Xia().Mai().Qi().Zuo().Cho().GetUserTokenAddress(tx.origin));
            Player =  Chan.Xie().Xia().Mai().GetPlayerQin(UserToken.Saat(1));
            _users[Player.Maat] = UserToken;
            // Register with YAI
            return (_chi, Player);
        }

        (_chi, Player) = Chi();
        _chi.Rename(Name, Symbol);
        Chan.Chou();
    }
}