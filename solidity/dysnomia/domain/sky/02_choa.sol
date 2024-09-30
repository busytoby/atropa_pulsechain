// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "../operations/interfaces/01b_seiinterface.sol";
import "./interfaces/01b_chaninterface.sol";

contract CHOA is DYSNOMIA {
    string public constant Type = "CHOA";

    CHAN public Chan;

    mapping(address => YUE) private _players;
    mapping(string Adjective => address[] Operators) private _operators;
    mapping(string Adjective => mapping(address Operator => address Contract)) private _operations;
    mapping(address UserToken => mapping(string Adjective => address Contract)) private _userOperations;

    constructor(address ChanAddress) DYSNOMIA("Dysnomia Chao", "CHAO", address(DYSNOMIA(ChanAddress).Xiao())) {
        Chan = CHAN(ChanAddress);
        Chan.Xie().Xia().Mai().Qi().Zuo().VAT().addOwner(address(this));
        addOwner(tx.origin);
        addOwner(address(Chan.Xie().Xia().Mai().Qi().Zuo().VAT()));
        _mintToCap();
    }

    function Play(address UserToken) public {
        Chan.Xie().Xia().Mai().Qi().Zuo().Cho().VerifyUserTokenPermissions(UserToken);
        // Register with YAI if new player
        // mai.Move to Zuo Qing with YAI if new player
    }

    function AddOperation(address UserToken, string memory Adjective, address OperationsContract) public {
        Chan.Xie().Xia().Mai().Qi().Zuo().Cho().VerifyUserTokenPermissions(UserToken);
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