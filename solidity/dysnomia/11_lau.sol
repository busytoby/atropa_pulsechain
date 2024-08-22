// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia.sol";
import "./interfaces/10b_voidinterface.sol";

contract LAU is DYSNOMIA {
    using ToString for *;
    string public constant Type = "LAU";

    VOID public Eta;
    uint64[3] public Saat;
    Bao public On;

    constructor(string memory name, string memory symbol, address VoidAddress) DYSNOMIA(name, symbol, address(DYSNOMIA(VoidAddress).Xiao())) {
        Eta = VOID(VoidAddress);
        (Saat, On) = Eta.Enter(name, symbol);
        _mintToCap();
    }

    // Potentially System OR User Function But Currently System Will Not Have Any Ownership
    function Withdraw(address what, uint256 amount) public onlyOwners {
        DYSNOMIA withdrawToken = DYSNOMIA(what);
        withdrawToken.transfer(msg.sender, amount);
        Eta.Log(string.concat("Withdraw Of ", amount.String(), " ", withdrawToken.name(), " To ", msg.sender.String()));
    }

    function Void(bool really1, bool really2) public onlyOwners {
        if(really1 && really2) (Saat, On) = Eta.Enter();
        Eta.Log("Reset To Void");
        _mintToCap();
    }

    function Username() public view returns (string memory) {
        return Eta.GetAttribute("Username");
    }

    function Username(string memory newUsername) public onlyOwners {
        Eta.SetAttribute("Username", newUsername);
        Eta.Log(string.concat("Username set to ", newUsername));
        _mintToCap();
    }

    function Chat(string memory chatline) public onlyOwners {
        Eta.Chat(chatline);
        _mintToCap();
    }

    function Alias(address name, string memory value) public onlyOwners {
        Eta.Alias(name, value);
        Eta.Log(string.concat("Alias[", name.String(), "] set to ", value));
        _mintToCap();
    }

    function Alias(address name) public view onlyOwners returns (string memory) {
        return Eta.Alias(name);
    }

    function Alias(Bao memory Theta, string memory value) public onlyOwners {
         Eta.Alias(Theta, value);
        Eta.Log(string.concat("Alias[", Theta.Phi.String(), "] set to ", value));
        _mintToCap();
    }

    function Alias(Bao memory Theta) public view onlyOwners returns (string memory) {
        return Eta.Alias(Theta);
    }
}