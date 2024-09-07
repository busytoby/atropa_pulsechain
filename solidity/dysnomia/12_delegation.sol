// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./include/user.sol";
import "./01_dysnomia_v2.sol";
import "./interfaces/11b_lauinterface.sol";
import "./interfaces/libstrings.sol";
import "./interfaces/libyai.sol";

abstract contract DELEGATION is DYSNOMIA {
    VOID public Void;
    uint64[3] public Saat;
    Bao public On;

    mapping(address => User) internal Delegates;
    mapping(uint64 => address) internal DelegateAddresses;

    constructor(string memory name, string memory symbol, address VoidAddress) DYSNOMIA(name, symbol, address(DYSNOMIA(VoidAddress).Xiao())) {
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

    function CYUN() public view returns(LIBSTRINGS) {
        return LIBSTRINGS(Void.GetLibraryAddress("strings"));
    }

    function VAT() public view returns (LIBYAI) {
        return LIBYAI(Void.GetLibraryAddress("yai"));
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
        
        Delegates[tx.origin] = Alpha;
        DelegateAddresses[Alpha.Soul] = tx.origin;
        return Alpha;
    }

    error UserNotEntered(address User);
    error InvalidUser();
    function GetUser() internal view returns(User memory Alpha) {
        if(Delegates[tx.origin].Soul == 0) revert UserNotEntered(tx.origin);
        Alpha = Delegates[tx.origin];
    }

    function GetUserTokenAddress(address wallet) public view returns(address UserToken) {
        return Delegates[wallet].On.Phi;
    }

    function Log(uint64 Soul, uint64 Aura, string memory LogLine) internal {
        On.Shio.Log(Soul, Aura, LogLine);
    }

    function OperatorSendMSG(string memory chatline) public onlyOwners {
        (On.Omicron, On.Omega) = On.Shio.React(On.Omicron ^ Void.Nu().Aura());
        Log(Saat[1], Void.Nu().Aura(), string.concat(chatline));
        _mintToCap();
        (On.Omicron, On.Omega) = On.Shio.React(On.Omega ^ Saat[1]);
    }
}