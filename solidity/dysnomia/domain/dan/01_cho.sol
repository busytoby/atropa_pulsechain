// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../include/user.sol";
import "../../01_dysnomia_v2.sol";
import "../../interfaces/11b_lauinterface.sol";
import "../../interfaces/libstrings.sol";
import "../../interfaces/libcorereactions.sol";

contract CHO is DYSNOMIA {
    string public constant Type = "CHO";

    uint256 constant public Gua = 1652929763764148448182513644633101239607891671119935657884642;

    VOID public Void;
    uint64[3] public Saat;
    Bao public On;
    uint64 public Entropy;

    mapping(address => User) private Delegates;
    mapping(uint64 => address) private DelegateAddresses;
    mapping(string => address) public Addresses;
    mapping(uint256 => address) public Qu;
    string[] public Aliases;

    constructor(address VoidAddress) DYSNOMIA("Dysnomia Cho", "CHO", address(DYSNOMIA(VoidAddress).Xiao())) {
        Void = VOID(VoidAddress);
        Void.addOwner(address(this));
        addOwner(tx.origin);

        (Saat, On) = Void.Enter(__name, __symbol);
        On.Phi = address(this);
        On.Shio.addOwner(address(this));
        On.Shio.Rho().Rod.addOwner(address(this));

        Entropy = Reactor().Entropy(On);

        React(Void.Nu().Psi().Rho().Bang.Omicron);
        (On.Omega, On.Omicron) = Reactor().ReactLai(On.Omega);
        React(Void.Nu().Psi().Rho().Le.Omicron);

        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());

        _addMarketRate(AFFECTIONContract, 1 * 10 ** decimals());
        _addMarketRate(VoidAddress, 1 * 10 ** decimals());
        _addMarketRate(address(On.Shio), 1 * 10 ** decimals());
        _addMarketRate(address(On.Shio.Rod()), 1 * 10 ** decimals());
        _addMarketRate(address(On.Shio.Cone()), 1 * 10 ** decimals());

        _mintToCap();
    }

    function AddContractOwner(address Contract, address Owner) public onlyOwners {
        DYSNOMIA(Contract).addOwner(Owner);
    }

    function has(address _contract, string memory what) public view returns (bool does) {
        bytes4 selector = bytes4(keccak256(bytes(what)));
        bytes memory data = abi.encodeWithSelector(selector);
        assembly { does := staticcall(gas(), _contract, add(data, 32), mload(data), 0, 0) }
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

    function Reactor() public view returns(LIBCOREREACTIONS) {
        return LIBCOREREACTIONS(Void.GetLibraryAddress("corereactions"));
    }

    function Luo() public onlyOwners returns (uint256 De) {
        De = 0;
        while(De == 0 || Qu[De] != address(0x0)) {
            React(On.Omicron);
            De = Xiao.modExp(On.Omega, On.Omicron, Gua);
        }
        Qu[De] = msg.sender;
        _mintToCap();
    }

    error InvalidUserToken(address Asset);
    function VerifyUserTokenPermissions(address UserToken) public {
        assert((DYSNOMIA(WITHOUTContract).balanceOf(tx.origin) == 0));
        Bao memory _on = Void.Nu().Psi().Mu().Tau().Upsilon().GetRodByIdx(LAU(UserToken).Saat(1));
        if(!_on.Shio.owner(UserToken) || !_on.Shio.Rod().owner(UserToken)) revert InvalidUserToken(UserToken);
    }

    error InvalidOwnership(address UserToken, address User);
    function Enter(address UserToken) public returns(User memory) {
        _mintToCap();
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
        Alpha.Entropy = Recall(Alpha);
        Delegates[tx.origin].Entropy = Alpha.Entropy;
        return Alpha;
    }

    function Recall(User memory Alpha) public returns (uint64 UserEntropy) {
        (On.Omicron, On.Omega) = Reactor().ReactToLai(On, Entropy ^ Alpha.On.Omicron, Entropy ^ Alpha.On.Omega);
        Entropy = On.Omega;
        return On.Omicron;
    }

    error UserNotEntered(address User);
    error SoulNotEntered(uint64 Soul);
    error InvalidUser();
    function GetUser() public returns(User memory Alpha) {
        if(Delegates[tx.origin].Soul == 0) revert UserNotEntered(tx.origin);
        Delegates[tx.origin].Entropy = Recall(Alpha);
        return Delegates[tx.origin];
    }

    function GetUserSoul() public view returns(uint64) {
        if(Delegates[tx.origin].Soul == 0) revert UserNotEntered(tx.origin);
        return Delegates[tx.origin].Soul;
    }

    function React(uint64 Eta) public returns (uint64, uint64) {
        _mintToCap();
        (On.Omicron, On.Omega) = Reactor().ReactShioRod(On.Shio, Entropy ^ Eta);
        Entropy = On.Omega;
        return (On.Omicron, On.Omega);
    }

    function ReactUser(uint64 Soul, uint64 Epsilon) public returns (uint64 Omicron) {
        if(DelegateAddresses[Soul] == address(0x0)) revert SoulNotEntered(Soul);
        _mintToCap();
        User memory Alpha = Delegates[DelegateAddresses[Soul]];
        Epsilon = Recall(Alpha) ^ Epsilon ^ Alpha.Entropy;
        (Alpha.On.Omicron, Alpha.On.Omega) = Reactor().ReactShioCone(Alpha.On.Shio, Epsilon);
        Entropy = Alpha.On.Omega;
        Alpha.Entropy = Alpha.On.Omicron;
        Delegates[DelegateAddresses[Soul]] = Alpha;
        return (Alpha.On.Omicron);
    }

    function GetUserBySoul(uint64 Soul) public returns (User memory Alpha) {
        address _a = GetAddressBySoul(Soul);
        if(Delegates[_a].Soul == 0) revert UserNotEntered(_a);
        Delegates[_a].Entropy = Recall(Delegates[_a]);
        return Delegates[_a];
    }

    function GetAddressBySoul(uint64 soul) public view returns(address UserAddress) {
        if(DelegateAddresses[soul] == address(0x0)) revert SoulNotEntered(soul);
        return DelegateAddresses[soul];
    }

    function GetUserTokenAddress(address wallet) public view returns(address UserToken) {
        return Delegates[wallet].On.Phi;
    }
}