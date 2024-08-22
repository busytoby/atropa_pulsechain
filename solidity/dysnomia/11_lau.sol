// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia.sol";
import "./interfaces/10b_voidinterface.sol";

contract LAU is DYSNOMIA {
    string public constant Type = "LAU";

    VOID public Eta;
    uint64[3] public Saat;
    Bao public On;
    address public CurrentArea;

    constructor(string memory name, string memory symbol, address VoidAddress) DYSNOMIA(name, symbol, address(DYSNOMIA(VoidAddress).Xiao())) {
        Eta = VOID(VoidAddress);
        (Saat, On) = Eta.Enter(name, symbol);
        On.Shio.addOwner(address(this));
        On.Shio.Rho().Rod.addOwner(address(this));
        _mintToCap();
    }

    // Potentially System OR User Function But Currently System Will Not Have Any Ownership
    function Withdraw(address what, uint256 amount) public onlyOwners {
        DYSNOMIA withdrawToken = DYSNOMIA(what);
        withdrawToken.transfer(msg.sender, amount);
        Eta.Log(string.concat("Withdraw Of ", String(amount), " ", withdrawToken.name(), " To ", Hex(msg.sender)));
    }

    function Void(bool really1, bool really2) public onlyOwners {
        if(really1 && really2) (Saat, On) = Eta.Enter();
        Eta.Log("Reset To Void");
        _mintToCap();
    }

    function Leave() public onlyOwners {
        CurrentArea = address(0x0);
        Eta.Log("Left Play");
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
        Eta.Log(string.concat("Alias[", Hex(name), "] set to ", value));
        _mintToCap();
    }

    function Alias(address name) public view onlyOwners returns (string memory) {
        return Eta.Alias(name);
    }

    function Alias(Bao memory Theta, string memory value) public onlyOwners {
         Eta.Alias(Theta, value);
        Eta.Log(string.concat("Alias[", Hex(Theta.Phi), "] set to ", value));
        _mintToCap();
    }

    function Alias(Bao memory Theta) public view onlyOwners returns (string memory) {
        return Eta.Alias(Theta);
    }
}