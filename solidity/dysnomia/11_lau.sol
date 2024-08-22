// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia.sol";
import "./interfaces/10b_voidinterface.sol";

contract LAU is DYSNOMIA {
    string public constant Type = "LAU";

    VOID public Eta;
    uint64[3] public Saat;
    Bao public On;

    constructor(string memory name, string memory symbol, address VoidAddress) DYSNOMIA(name, symbol, address(DYSNOMIA(VoidAddress).Xiao())) {
        Eta = VOID(VoidAddress);
        (Saat, On) = Eta.Enter(name, symbol);
        _mintToCap();
    }

    function Void(bool really1, bool really2) public onlyOwners {
        if(really1 && really2) (Saat, On) = Eta.Enter();
        _mintToCap();
    }

    function Username() public view returns (string memory) {
        return Eta.GetAttribute("Username");
    }

    function Username(string memory newUsername) public onlyOwners {
        Eta.SetAttribute("Username", newUsername);
        _mintToCap();
    }

    function Chat(string memory chatline) public onlyOwners {
        Eta.Chat(chatline);
        _mintToCap();
    }

    function Alias(address name, string memory value) public onlyOwners {
        Eta.Alias(name, value);
        _mintToCap();
    }

    function Alias(address name) public view onlyOwners returns (string memory) {
        return Eta.Alias(name);
    }

    function Alias(Bao memory Theta, string memory value) public onlyOwners {
         Eta.Alias(Theta, value);
        _mintToCap();
    }

    function Alias(Bao memory Theta) public view onlyOwners returns (string memory) {
        return Eta.Alias(Theta);
    }
}