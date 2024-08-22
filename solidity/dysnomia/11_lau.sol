// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia.sol";
import "./interfaces/10b_voidinterface.sol";

contract LAU is DYSNOMIA {
    string public constant Type = "LAU";

    VOID public Eta;

    constructor(string memory name, string memory symbol, address VoidAddress) DYSNOMIA(name, symbol, address(DYSNOMIA(VoidAddress).Xiao())) {
        Eta = VOID(VoidAddress);

        Eta.Enter(name, symbol);
    }

    function Username() public view returns (string memory) {
        return Eta.GetAttribute("Username");
    }

    function Username(string memory newUsername) public onlyOwners {
        Eta.SetAttribute("Username", newUsername);
    }

    function Chat(string memory chatline) public onlyOwners {
        Eta.Chat(chatline);
    }
}