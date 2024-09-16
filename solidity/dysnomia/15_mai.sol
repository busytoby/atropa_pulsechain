// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia_v2.sol";
import "./interfaces/16b_zaointerface.sol";

struct QIN {
    uint256 Maat;
    QING Location;
    uint256 lastMove;
}

contract MAI is DYSNOMIA {
    string public constant Type = "MAI";

    QING public Zuo;
    mapping(uint64 Soul => QIN) private _players;

    constructor(address ZuoQingAddress) DYSNOMIA("Dysnomia MAI", "MAI", address(DYSNOMIA(ZuoQingAddress).Xiao())) {
        Zuo = QING(ZuoQingAddress);
        addOwner(tx.origin);
    }

    error NotPlaying(uint64 Soul);
    error OneMovePerBlock();
    function Move(string memory To) public onlyOwners {
        User memory Beta = Zuo.Cho().GetUser();
        if(Beta.Soul == 0) revert NotPlaying(Beta.Soul);
        if(_players[Beta.Soul].Maat == 0) _players[Beta.Soul].Maat = Zuo.Cho().Luo();
        if(_players[Beta.Soul].lastMove >= block.timestamp) revert OneMovePerBlock();
        uint256 From = _players[Beta.Soul].Location.Waat();
        address ToQing = Zuo.VAT().Alias(From, To);

        if(address(_players[Beta.Soul].Location) != address(0x0)) _players[Beta.Soul].Location.Leave();
        _players[Beta.Soul].Location = QING(ToQing);
        _players[Beta.Soul].Location.Join(Beta.On.Phi);
        _players[Beta.Soul].lastMove = block.timestamp;

    }
}