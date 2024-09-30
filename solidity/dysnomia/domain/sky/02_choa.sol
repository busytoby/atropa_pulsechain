// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "../operations/interfaces/01b_seiinterface.sol";
import "./interfaces/01b_chaninterface.sol";

contract CHOA is DYSNOMIA {
    string public constant Type = "CHOA";

    SEI public Sei;

    mapping(address => YUE) private _players;
    mapping(string Adjective => address[] Operators) private _operators;
    mapping(string Adjective => mapping(address Operator => address Contract)) private _operations;
    mapping(address UserToken => mapping(string Adjective => address Contract)) private _userOperations;

    constructor(address SeiAddress) DYSNOMIA("Dysnomia Chao", "CHAO", address(DYSNOMIA(SeiAddress).Xiao())) {
        Sei = SEI(SeiAddress);
        Sei.Chan().Xie().Xia().Mai().Qi().Zuo().VAT().addOwner(address(this));
        addOwner(tx.origin);
        addOwner(address(Sei.Chan().Xie().Xia().Mai().Qi().Zuo().VAT()));
        _mintToCap();
    }

    function Play(address UserToken) public {
        Sei.Chan().Xie().Xia().Mai().Qi().Zuo().Cho().VerifyUserTokenPermissions(UserToken);
        (YUE _chi, QIN memory Player) = Sei.Chi();
        _players[tx.origin] = _chi;
        // mai.Move to Zuo Qing with YAI if new player
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