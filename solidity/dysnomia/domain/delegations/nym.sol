// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../12_delegation.sol";

struct Acronym {
    string[7] ACRONYM;
    string PHRASE;
    uint16 VOTES;
}

struct UserVote {
    uint64 soul;
    uint16 vote;
}

contract Nym is DELEGATION {
    uint64 public RoundNumber = 0;
    uint16 public AcronymCount = 0;
    mapping(uint16 => Acronym) public Acronyms;
    UserVote[] public UserVotes;
    mapping(uint64 => uint64) public LastUserVote;

    constructor(address VoidAddress) DELEGATION(unicode"Champion", unicode"NYM", VoidAddress) {
        maxSupply = 11111111111111111111;
    }

    function Chat(string memory chatline) public override onlyOwners {
        if(Delegates[tx.origin].Soul == 0) revert UserNotEntered(tx.origin);
        User memory Alpha = Delegates[tx.origin];

        string memory Username = GetUsername(Alpha);
        On.Shio.Log(Saat[1], Void.Nu().Aura(), string.concat("<", Username, "> ", chatline));
        React(Alpha, Xiao.Random());
        
        _mintToCap();
    }

    function OperatorSendMSG(string memory chatline) public override onlyOwners {
        On.Shio.Log(Saat[1], Void.Nu().Aura(), string.concat(chatline));
        _mintToCap();
    }
}