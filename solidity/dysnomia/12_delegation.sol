// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./include/user.sol";
import "./01_dysnomia_v2.sol";
import "./interfaces/11b_lauinterface.sol";
import "./interfaces/libstrings.sol";

abstract contract DELEGATION is DYSNOMIA {
    string public constant Type = "DELEGATION";

    VOID internal Void;
    uint64[3] internal Saat;
    Bao internal On;

    mapping(address => User) internal Delegates;

    constructor(string memory name, string memory symbol, address VoidAddress, address LibStringsAddress) DYSNOMIA(name, symbol, address(DYSNOMIA(VoidAddress).Xiao()), LibStringsAddress) {
        Void = VOID(VoidAddress);

        //Void.addOwner(address(this));
        //Void.Nu().addOwner(address(this));
        //Void.Nu().Psi().addOwner(address(this));
        //Void.Nu().Psi().Mu().addOwner(address(this));
        //Void.Nu().Psi().Mu().Tau().addOwner(address(this));
        //Void.Nu().Psi().Mu().Tau().Upsilon().addOwner(address(this));
        Void.Nu().Psi().Mu().Tau().Upsilon().Eta().addOwner(address(this));

        (Saat, On) = Void.Enter(name, symbol);
        On.Phi = address(this);
        On.Shio.addOwner(address(this));
        On.Shio.Rho().Rod.addOwner(address(this));
        Void.Nu().Psi().Mu().Tau().Upsilon().Eta().Bing(On); // User LAU are not tracked in Bing except at "latest birth" but delegations are

        (On.Omicron, On.Omega) = On.Shio.React(Void.Nu().Psi().Rho().Bang.Omicron);
        (On.Omega, On.Omicron) = Void.Nu().Psi().Rho().Bang.Shio.React(On.Omega);
        (On.Omicron, On.Omega) = On.Shio.React(Void.Nu().Psi().Rho().Le.Omicron);

        //Void.SetAttribute("Username", symbol);

        _mintToCap();
    }

    error InvalidOwnership(address UserToken, address User);
    error AlreadyEntered(address UserAddress, address User);
    function Enter(address UserToken) internal returns(User memory) { // inheriting class should implement Join() & Call Enter(), see nym.sol
        if(Delegates[tx.origin].Soul != 0) revert AlreadyEntered(UserToken, tx.origin);
        LAU UserLau = LAU(UserToken);
        if(!UserLau.owner(msg.sender)) revert InvalidOwnership(UserToken, msg.sender);

        User memory Alpha;
        Alpha.Soul = UserLau.Saat(1);
        Alpha.On = UserLau.On();
        Alpha.On.Phi = UserToken;
        Alpha.Username = LAU(Alpha.On.Phi).Username();
        
        (Alpha.On.Omicron, Alpha.On.Omega) = React(Alpha, Saat[2]);
        Delegates[tx.origin] = Alpha;
        return Alpha;
    }

    function ReactUserRod(User memory Alpha, uint64 Theta) internal returns (uint64, uint64) {
        return Alpha.On.Shio.Rod().React(Theta ^ Void.Nu().Psi().Rho().Bang.Omega, Alpha.On.Shio.Cone().View().Channel);
    }

    function ReactUserCone(User memory Alpha, uint64 Theta) internal returns (uint64, uint64) {
        return Alpha.On.Shio.Cone().React(Theta ^ On.Omega, Alpha.On.Shio.Cone().View().Channel);
    }

    function React(User memory Alpha, uint64 Theta) internal returns (uint64, uint64) {
        (Alpha.On.Omicron, Alpha.On.Omega) = ReactUserRod(Alpha, Theta);
        (On.Omicron, On.Omega) = On.Shio.React(On.Omicron ^ Alpha.On.Omega);
        return ReactUserCone(Alpha, Alpha.On.Omicron);
    }

    error UserNotEntered(address User);
    error InvalidUser();
    function GetUser() internal view returns(User memory Alpha) {
        if(Delegates[tx.origin].Soul == 0) revert UserNotEntered(tx.origin);
        Alpha = Delegates[tx.origin];
    }

    function OperatorSendMSG(string memory chatline) public onlyOwners {
        On.Shio.Log(Saat[1], Void.Nu().Aura(), string.concat(chatline));
        _mintToCap();
    }
}