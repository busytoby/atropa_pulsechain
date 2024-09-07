// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./include/user.sol";
import "./01_dysnomia_v2.sol";
import "./interfaces/11b_lauinterface.sol";
import "./interfaces/libstrings.sol";
import "./interfaces/libyai.sol";

abstract contract DELEGATION is DYSNOMIA {
    VOID internal Void;
    uint64[3] internal Saat;
    Bao internal On;
    string public Description;

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

    function CYUN() internal view returns(LIBSTRINGS) {
        return LIBSTRINGS(Void.GetLibraryAddress("strings"));
    }

    function VAT() internal view returns (LIBYAI) {
        return LIBYAI(Void.GetLibraryAddress("yai"));
    }

    function SetDescription(string memory _d) public onlyOwners {
        Description = _d;
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
        DelegateAddresses[Alpha.Soul] = tx.origin;
        return Alpha;
    }

    function ReactShioRod(SHIO Beta, uint64 Theta) internal returns (uint64, uint64) {
        return Beta.Rod().React(Theta ^ Void.Nu().Psi().Rho().Bang.Omega, Beta.Cone().View().Channel);
    }

    function ReactShioCone(SHIO Beta, uint64 Theta) internal returns (uint64, uint64) {
        return Beta.Cone().React(Theta ^ On.Omega, Beta.Cone().View().Channel);
    }

    function React(User memory Alpha, uint64 Theta) internal returns (uint64, uint64) {
        (Alpha.On.Omicron, Alpha.On.Omega) = ReactShioRod(Alpha.On.Shio, Theta);
        (On.Omicron, On.Omega) = On.Shio.React(On.Omicron ^ Alpha.On.Omega);
        return ReactShioCone(Alpha.On.Shio, Alpha.On.Omicron);
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