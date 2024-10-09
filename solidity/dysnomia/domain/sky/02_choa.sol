// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "../../include/trait.sol";
import "../tang/interfaces/01b_seiinterface.sol";
import "./interfaces/01b_chaninterface.sol";

contract CHOA is DYSNOMIA {
    string public constant Type = "CHOA";

    SEI public Sei;

    mapping(uint64 Soul => YUEINTERFACE) private _players;
    mapping(TRAIT Trait => address[] Operators) private _operators;
    mapping(TRAIT Trait => mapping(address Operator => address Contract)) private _operations;
    mapping(address UserToken => mapping(TRAIT Trait => address Contract)) private _userOperations;

    constructor(address SeiAddress) DYSNOMIA("Dysnomia Chao", "CHOA", address(DYSNOMIA(SeiAddress).Xiao())) {
        Sei = SEI(SeiAddress);
        Sei.Chan().addOwner(address(this));
        addOwner(tx.origin);
        _mintToCap();
    }    

    function Play(address UserTokenAddress) public returns (YUEINTERFACE Chi, LAU UserToken) {
        Sei.Chan().Xie().Xia().Mai().Qi().Zuo().Cho().Enter(UserTokenAddress);
        (Chi, UserToken) = Sei.Chi();
        if(address(_players[UserToken.Saat(1)]) == address(0x0)) Sei.Chan().YueMintToOrigin(Chi);
        _players[UserToken.Saat(1)] = Chi;
    }

    function Chat(address Qing, string memory MSG) public returns (uint256 Charge) {
        (YUEINTERFACE Yue, LAU UserToken) = Sei.Chi();
        QINGINTERFACE _qing = QINGINTERFACE(Qing);
        _qing.Chat(UserToken, MSG);
        Charge = Sei.Chan().ReactYue(Yue, Qing);
        _mintToCap();
        uint256 Mai = Sei.Chan().Xie().Xia().Mai().React(UserToken.Saat(1), _qing.Waat());
        if(Mai > 1 * 10 ** decimals()) Mai = 1 * 10 ** decimals();
            if(balanceOf(address(this)) >= Mai)
                _transfer(address(this), address(Yue), Mai);
    }

    function AddOperation(address UserToken, TRAIT Trait, address OperationsContract) public {
        Sei.Chan().Xie().Xia().Mai().Qi().Zuo().Cho().VerifyUserTokenPermissions(UserToken);
        // verify qi deposit
        // verify presence of functions
        // test reflexes for known return value
        _userOperations[UserToken][Trait] = OperationsContract;
        if(_operations[Trait][UserToken] == address(0x0)) _operators[Trait].push(UserToken);
        _operations[Trait][UserToken] = OperationsContract;        
    }

    function React(address YUEAddress, uint256 Waat, TRAIT Trait, uint256 Charge) public onlyOwners {
        // reactions return from YAI and pass to Operators
    }
}