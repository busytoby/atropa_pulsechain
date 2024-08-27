// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./include/user.sol";
import "./01_dysnomia.sol";
import "./interfaces/10b_voidinterface.sol";
import "./interfaces/11b_lauinterface.sol";

abstract contract DELEGATION is DYSNOMIA {
    string public constant Type = "DELEGATION";

    VOID public Void;
    uint64[3] public Saat;
    Bao public On;

    mapping(address => User) public Delegates;

    constructor(string memory name, string memory symbol, address VoidAddress) DYSNOMIA(name, symbol, address(DYSNOMIA(VoidAddress).Xiao())) {
        Void = VOID(VoidAddress);
        Void.Nu().addOwner(address(this));
        Void.Nu().Psi().addOwner(address(this));
        Void.Nu().Psi().Mu().addOwner(address(this));
        Void.Nu().Psi().Mu().Tau().addOwner(address(this));
        Void.Nu().Psi().Mu().Tau().Upsilon().addOwner(address(this));
        Void.Nu().Psi().Mu().Tau().Upsilon().Eta().addOwner(address(this));

        (Saat, On) = Void.Enter(name, symbol);
        On.Phi = address(this);
        On.Shio.addOwner(address(this));
        On.Shio.Rho().Rod.addOwner(address(this));
        Void.Nu().Psi().Mu().Tau().Upsilon().Eta().Bing(On); // User LAU are not tracked in Bing except at "latest birth" but delegations are

        Void.SetAttribute("Username", symbol);

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
        Alpha.Username = GetUsername(Alpha);
        Delegates[tx.origin] = Alpha;
        
        (Alpha.On.Omicron, Alpha.On.Omega) = Alpha.On.Shio.React(Saat[2]);
        return Alpha;
    }

    function React(User memory Alpha, uint64 Theta) public onlyOwners returns (uint64, uint64) {
        (On.Omicron, On.Omega) = On.Shio.React(Theta);
        (Alpha.On.Omicron, Alpha.On.Omega) = Alpha.On.Shio.React(On.Omega);
        return (On.Omicron, On.Omega);
    }

    error UserNotEntered(address User);
    error InvalidUser();
    function GetUser() internal view returns(User memory Alpha) {
        if(Delegates[tx.origin].Soul == 0) revert UserNotEntered(tx.origin);
        Alpha = Delegates[tx.origin];
    }

    function GetUsername(User memory Alpha) public view onlyOwners returns (string memory) {
        if(Alpha.Soul == 0) revert InvalidUser();
        return LAU(Alpha.On.Phi).Username();
    }

    error ChatNotImplemented();
    function Chat(string memory chatline) public virtual {
        chatline = "";
        revert ChatNotImplemented();
    }

    error OperatorSendMSGNotImplemented();
    function OperatorSendMSG(string memory chatline) public virtual onlyOwners {
        chatline = "";
        revert OperatorSendMSGNotImplemented();
    }
}