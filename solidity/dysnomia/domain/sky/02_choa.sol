// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "../operations/interfaces/01b_seiinterface.sol";
import "./interfaces/01b_chaninterface.sol";

contract CHOA is DYSNOMIA {
    string public constant Type = "CHOA";

    SEI public Sei;

    mapping(uint256 Maat => YUEINTERFACE) private _players;
    mapping(string Adjective => address[] Operators) private _operators;
    mapping(string Adjective => mapping(address Operator => address Contract)) private _operations;
    mapping(address UserToken => mapping(string Adjective => address Contract)) private _userOperations;

    constructor(address SeiAddress) DYSNOMIA("Dysnomia Chao", "CHAO", address(DYSNOMIA(SeiAddress).Xiao())) {
        Sei = SEI(SeiAddress);
        Sei.Chan().addOwner(address(this));
        Sei.Chan().Xie().Xia().Mai().addOwner(address(this));
        Sei.Chan().Xie().Xia().Mai().Qi().Zuo().VAT().addOwner(address(this));
        addOwner(tx.origin);
        addOwner(address(Sei.Chan().Xie().Xia().Mai().Qi().Zuo().VAT()));
        _mintToCap();
    }

    function Play(address UserTokenAddress) public returns (YUEINTERFACE Chi, LAU UserToken, QIN memory Player) {
        Sei.Chan().Xie().Xia().Mai().Qi().Zuo().Cho().Enter(UserTokenAddress);
        (Chi, UserToken, Player) = Sei.Chi();
        if(address(_players[Player.Maat]) == address(0x0)) Sei.Chan().YueMintToOrigin(Chi);
        _players[Player.Maat] = Chi;
    }

    function Chat(string memory MSG) public returns (uint256 Charge, uint256 UserQi, uint64 Omicron, uint64 Omega) {
        (YUEINTERFACE Yue, LAU UserToken, QIN memory Player) = Sei.Chi();
        (Charge, UserQi, Omega) = Sei.Chan().React(Player, "Chat");
        Sei.Chan().Xie().Xia().Mai().Qi().Zuo().VAT().QingChat(Player, UserToken, MSG);
        Omicron = Yue.React("Chat");
    }

    function GetExits() public view returns (string[] memory Exits) {
        (, , QIN memory Player) = Sei.Chi();
        uint256 Waat = Player.Location.Waat();
        uint256 ExitCount = Sei.Chan().Xie().Xia().Mai().Qi().Zuo().VAT().GetAliasCount(Waat);
        Exits = new string[](ExitCount);
        for(uint256 i = 0; i < ExitCount; i++) {
            Exits[i] = Sei.Chan().Xie().Xia().Mai().Qi().Zuo().VAT().GetAlias(Waat, i);
        }
    }

    function Move(string memory To) public {
        Sei.Chan().Xie().Xia().Mai().Move(To);       
    }

    function AddOperation(address UserToken, string memory Adjective, address OperationsContract) public {
        Sei.Chan().Xie().Xia().Mai().Qi().Zuo().Cho().VerifyUserTokenPermissions(UserToken);
        // verify qi deposit
        // verify presence of functions
        // test reflexes for known return value
        _userOperations[UserToken][Adjective] = OperationsContract;
        if(_operations[Adjective][UserToken] == address(0x0)) _operators[Adjective].push(UserToken);
        _operations[Adjective][UserToken] = OperationsContract;        
    }

    function React(address YUEAddress, uint256 Waat, string memory Adjective, uint256 Charge) public onlyOwners {
        // reactions return from YAI and pass to Operators
    }
}