// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./include/user.sol";
import "./01_dysnomia_v2.sol";
import "./interfaces/11b_lauinterface.sol";
import "./interfaces/libstrings.sol";
import "./interfaces/libcorereactions.sol";

contract CHO is DYSNOMIA {
    string public constant Type = "CHO";

    uint256 public Prime = 788007780075465850682698395781331007488376212737722608030423588863419947711;

    VOID public Void;
    uint64[3] public Saat;
    Bao public On;
    function Shio() public view returns(SHIO) { return On.Shio; }
    function Omicron() public view returns(uint64) { return On.Omicron; }
    function Omega() public view returns(uint64) { return On.Omega; }
    uint64 public Entropy;

    mapping(address => User) private Delegates;
    mapping(uint64 => address) private DelegateAddresses;
    mapping(string => address) public Addresses;
    mapping(uint256 => address) public Qu;
    string[] public Aliases;

    constructor(address VoidAddress) DYSNOMIA("Dysnomia Cho", "CHO", address(DYSNOMIA(VoidAddress).Xiao())) {
        Void = VOID(VoidAddress);
        Void.addOwner(address(this));

        (Saat, On) = Void.Enter(__name, __symbol);
        On.Phi = address(this);
        On.Shio.addOwner(address(this));
        On.Shio.Rho().Rod.addOwner(address(this));

        Entropy = Reactor().Entropy(On);

        React(Void.Nu().Psi().Rho().Bang.Omicron);
        (On.Omega, On.Omicron) = Reactor().ReactLai(On.Omega);
        React(Void.Nu().Psi().Rho().Le.Omicron);

        _mintToCap();
    }

    error SystemAddress(address Address, string Alias);
    function AddSystemAddress(string memory Alias, address Address) public onlyOwners {
        if(Addresses[Alias] != address(0x0)) revert SystemAddress(Addresses[Alias], Alias);
        Addresses[Alias] = Address;
        Aliases.push(Alias);
    }

    function AddLibraryOwner(string memory what) public onlyOwners {
        _addLibraryOwner(Void, what);
    }

    function CYUN() public view returns(LIBSTRINGS) {
        return LIBSTRINGS(Void.GetLibraryAddress("strings"));
    }

    function Reactor() public view returns(LIBCOREREACTIONS) {
        return LIBCOREREACTIONS(Void.GetLibraryAddress("corereactions"));
    }

    function Luo() public returns (uint256 De) {
        React(On.Omicron);
        De = Xiao.modExp(On.Omega, On.Omicron, Prime);
        if(Qu[De] == address(0x0))
            Qu[De] = msg.sender;
        else
            De = Luo();
    }

    error InvalidUserToken(address Asset);
    function VerifyUserTokenPermissions(address UserToken) public {
        Bao memory _on = Void.Nu().Psi().Mu().Tau().Upsilon().GetRodByIdx(LAU(UserToken).Saat(1));
        if(!_on.Shio.owner(UserToken) || !_on.Shio.Rod().owner(UserToken)) revert InvalidUserToken(UserToken);
    }

    error InvalidOwnership(address UserToken, address User);
    function Enter(address UserToken) public onlyOwners returns(User memory) {
        VerifyUserTokenPermissions(UserToken);
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

        Alpha.Entropy = Recall(Alpha);
        return Alpha;
    }

    function Recall(User memory Alpha) internal returns (uint64 UserEntropy) {
        (On.Omicron, On.Omega) = Reactor().ReactToLai(On, Entropy ^ Alpha.On.Omicron, Entropy ^ Alpha.On.Omega);
        Entropy = On.Omega;
        return On.Omicron;
    }

    error UserNotEntered(address User);
    error SoulNotEntered(uint64 Soul);
    error InvalidUser();
    function GetUser() public returns(User memory Alpha) {
        if(Delegates[tx.origin].Soul == 0) revert UserNotEntered(tx.origin);
        Alpha = Delegates[tx.origin];
        Alpha.Entropy = Recall(Alpha);
    }

    function GetUserSoul() public view returns(uint64) {
        if(Delegates[tx.origin].Soul == 0) revert UserNotEntered(tx.origin);
        return Delegates[tx.origin].Soul;
    }

    function React(uint64 Eta) public returns (uint64, uint64) {
        (On.Omicron, On.Omega) = Reactor().ReactShioRod(On.Shio, Entropy ^ Eta);
        Entropy = On.Omega;
        return (On.Omicron, On.Omega);
    }

    function ReactUser(uint64 Soul, uint64 Epsilon) public returns (uint64 _omicron, uint64 _omega) {
        if(DelegateAddresses[Soul] == address(0x0)) revert SoulNotEntered(Soul);
        User memory Alpha = Delegates[DelegateAddresses[Soul]];
        Epsilon = Recall(Alpha);
        (Alpha.On.Omicron, Alpha.On.Omega) = Reactor().ReactShioCone(Alpha.On.Shio, Epsilon);
        Delegates[DelegateAddresses[Soul]] = Alpha;
        Entropy = Alpha.On.Omicron;
        return (Alpha.On.Omicron, Alpha.On.Omega);
    }

    function GetUserBySoul(uint64 Soul) public onlyOwners returns (User memory Alpha) {
        address _a = GetAddressBySoul(Soul);
        return GetUserByAddress(_a);
    }

    function GetUserByAddress(address who) public onlyOwners returns(User memory Alpha) {
        if(Delegates[who].Soul == 0) revert UserNotEntered(who);
        Alpha = Delegates[who];
        Alpha.Entropy = Recall(Alpha);
    }

    function GetAddressBySoul(uint64 soul) public onlyOwners view returns(address UserAddress) {
        if(DelegateAddresses[soul] == address(0x0)) revert SoulNotEntered(soul);
        return DelegateAddresses[soul];
    }

    function GetUserTokenAddress(address wallet) public view returns(address UserToken) {
        return Delegates[wallet].On.Phi;
    }
}