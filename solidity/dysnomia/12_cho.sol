// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./include/user.sol";
import "./01_dysnomia_v2.sol";
import "./interfaces/11b_lauinterface.sol";
import "./interfaces/libstrings.sol";
import "./interfaces/libyai.sol";

contract CHO is DYSNOMIA {
    string public constant Type = "CHO";

    VOID public Void;
    uint64[3] public Saat;
    Bao public On;

    mapping(address => User) internal Delegates;
    mapping(uint64 => address) internal DelegateAddresses;

    constructor(address VoidAddress) DYSNOMIA("Dysnomia Cho", "CHO", address(DYSNOMIA(VoidAddress).Xiao())) {
        Void = VOID(VoidAddress);
        Void.addOwner(address(this));

        (Saat, On) = Void.Enter(__name, __symbol);
        On.Phi = address(this);
        On.Shio.addOwner(address(this));
        On.Shio.Rho().Rod.addOwner(address(this));


        React(Void.Nu().Psi().Rho().Bang.Omicron);
        (On.Omega, On.Omicron) = ReactLai(On.Omicron);
        React(Void.Nu().Psi().Rho().Le.Omicron);

        _mintToCap();
    }

    function AddLibraryOwner(string memory what) public onlyOwners {
        _addLibraryOwner(Void, what);
    }

    function CYUN() public view returns(LIBSTRINGS) {
        return LIBSTRINGS(Void.GetLibraryAddress("strings"));
    }

    error InvalidOwnership(address UserToken, address User);
    function Enter(address UserToken) public onlyOwners returns(User memory) {
        LAU UserLau = LAU(UserToken);
        if(!UserLau.owner(tx.origin)) revert InvalidOwnership(UserToken, tx.origin);

        User memory Alpha;
        Alpha.Soul = UserLau.Saat(1);
        Alpha.On = UserLau.On();
        Alpha.On.Phi = UserToken;
        Alpha.Username = LAU(Alpha.On.Phi).Username();
        
        Delegates[tx.origin] = Alpha;
        DelegateAddresses[Alpha.Soul] = tx.origin;
        ReactUser(Alpha.Soul, Saat[1]);
        return Alpha;
    }

    error UserNotEntered(address User);
    error SoulNotEntered(uint64 Soul);
    error InvalidUser();
    function GetUser() public view returns(User memory Alpha) {
        if(Delegates[tx.origin].Soul == 0) revert UserNotEntered(tx.origin);
        Alpha = Delegates[tx.origin];
    }

    function React(uint64 Eta) internal returns (uint64, uint64) {
        (On.Omicron, On.Omega) = ReactShioRod(On.Shio, Eta);
        return (On.Omicron, On.Omega);
    }

    function ReactShioRod(SHIO Beta, uint64 Theta) public returns (uint64, uint64) {
        return Beta.Rod().React(Theta, Beta.Cone().View().Channel);
    }

    function ReactShioCone(SHIO Beta, uint64 Theta) public returns (uint64, uint64) {
        return Beta.Cone().React(Theta, Beta.Rod().View().Channel);
    }

    function ReactBang(uint64 Eta) public returns (uint64, uint64) {
        return ReactShioRod(Void.Nu().Psi().Rho().Bang.Shio, Eta);
    }

    function ReactLai(uint64 Gamma) public returns (uint64, uint64) {
        return ReactShioRod(Void.Nu().Psi().Rho().Lai.Shio, Gamma);
    }

    function ReactLe(uint64 Delta) public returns (uint64, uint64) {
        return ReactShioCone(Void.Nu().Psi().Rho().Le.Shio, Delta);
    }

    function ReactUser(uint64 Soul, uint64 Epsilon) public returns (User memory) {
        User memory Alpha = Delegates[DelegateAddresses[Soul]];
        (Alpha.On.Omicron, Alpha.On.Omega) = ReactShioCone(Alpha.On.Shio, Epsilon);
        Delegates[DelegateAddresses[Soul]] = Alpha;
        return(Alpha);
    }

    function GetUserByAddress(address who) public onlyOwners view returns(User memory Alpha) {
        if(Delegates[who].Soul == 0) revert UserNotEntered(who);
        Alpha = Delegates[who];
    }

    function GetAddressBySoul(uint64 soul) public onlyOwners view returns(address UserAddress) {
        if(DelegateAddresses[soul] == address(0x0)) revert SoulNotEntered(soul);
        return DelegateAddresses[soul];
    }

    function GetUserTokenAddress(address wallet) public view returns(address UserToken) {
        return Delegates[wallet].On.Phi;
    }
}