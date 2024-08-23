// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia.sol";
import "./interfaces/09b_siuinterface.sol";
import "./interfaces/libattribute.sol";

contract VOID is DYSNOMIA {
    string public constant Type = "VOID";

    SIU public Nu;
    mapping(address => uint64) private _activeUsers;
    mapping(string => bytes32) private _kecNames;
    mapping(string => address) private _libraries;

    constructor(address SiuAddress) DYSNOMIA(unicode"CHATLOG Void", unicode"VOID", address(DYSNOMIA(SiuAddress).Xiao())) {
        Nu = SIU(SiuAddress);
        Nu.addOwner(address(this));
        Nu.Psi().addOwner(address(this));
        Nu.Psi().Mu().addOwner(address(this));
        Nu.Psi().Mu().Tau().addOwner(address(this));
        Nu.Psi().Mu().Tau().Upsilon().addOwner(address(this));
        Nu.Psi().Mu().Tau().Upsilon().Eta().addOwner(address(this));

        GetBySoul(ZHOU(Nu.Psi().Mu().Tau()).Xi()).Shio.addOwner(address(this));
        GetBySoul(YANG(Nu.Psi()).Rho().Lai.Xi).Shio.addOwner(address(this));

        _kecNames["ZHOU"] = keccak256("ZHOU");
        _kecNames["YANG"] = keccak256("YANG");
        Augment();
    }

    function Augment() internal {
        AddMarketRate(address(Nu), 1 * 10 ** decimals());
        _mintToCap();
    }

    function GetLibraryAddress(string memory name) public view returns (address) {
        return _libraries[name];
    }

    function AddLibrary(string memory name, address _a) public onlyOwners {
        _libraries[name] = _a;
        _mintToCap();
    }

    function Log(string memory LogLine) public {
        uint64 Soul = _activeUsers[msg.sender];
        GetBySoul(Soul).Shio.Log(Soul, Nu.Aura(), LogLine);
        _mintToCap();
    }

    function Log(uint64 Sigma, string memory LogLine) public onlyOwners {
        GetBySoul(Sigma).Shio.Log(Sigma, Nu.Aura(), LogLine);
        _mintToCap();
    }

    function Log(address Sigma, string memory LogLine) public onlyOwners {
        uint64 Soul = _activeUsers[Sigma];
        GetBySoul(_activeUsers[Sigma]).Shio.Log(Soul, Nu.Aura(), LogLine);
        _mintToCap();
    }

    error InvalidLogXi(string Xi);
    function Log(string memory Xi, string memory LogLine) internal {
        uint64 Soul = _activeUsers[msg.sender];
        bytes32 XiKec = keccak256(bytes(Xi));
        if(XiKec == _kecNames["ZHOU"]) {
          GetBySoul(ZHOU(Nu.Psi().Mu().Tau()).Xi()).Shio.Log(Soul, Nu.Aura(), LogLine);  
        } else if(XiKec == _kecNames["YANG"]) {
            GetBySoul(YANG(Nu.Psi()).Rho().Lai.Xi).Shio.Log(Soul, Nu.Aura(), LogLine);  
        } else revert InvalidLogXi(Xi);
        _mintToCap();
    }

    error NoUserEntry(address User);
    error NoUserName(address User);
    function Chat(string memory chatline) public {
        if(_activeUsers[msg.sender] == 0) revert NoUserEntry(msg.sender);
        string memory Username = GetAttribute("Username");
        if(bytes(Username).length < 1) revert NoUserName(msg.sender); 
        Log("ZHOU", string.concat("<", Username, "> ", chatline));
        _mintToCap();
    }

    function SetAttribute(string memory name, string memory value) public {
        if(_activeUsers[msg.sender] == 0) revert NoUserEntry(msg.sender);
        LIBATTRIBUTE(_libraries["libattribute"]).Set(_activeUsers[msg.sender], name, value);
        _mintToCap();
    }

    function GetAttribute(string memory name) public view returns (string memory) {
        return LIBATTRIBUTE(_libraries["libattribute"]).Get(_activeUsers[msg.sender], name);
    }

    function Alias(address name, string memory value) public {
        if(_activeUsers[msg.sender] == 0) revert NoUserEntry(msg.sender);
        LIBATTRIBUTE(_libraries["libattribute"]).Alias(_activeUsers[msg.sender], name, value);
        _mintToCap();
    }

    function Alias(address name) public view returns (string memory) {
        return LIBATTRIBUTE(_libraries["libattribute"]).Alias(_activeUsers[msg.sender], name);
    }

    function Alias(Bao memory Theta, string memory value) public {
        if(_activeUsers[msg.sender] == 0) revert NoUserEntry(msg.sender);
        LIBATTRIBUTE(_libraries["libattribute"]).Alias(_activeUsers[msg.sender], Theta, value);
        _mintToCap();
    }

    function Alias(Bao memory Theta) public view returns (string memory) {
        return LIBATTRIBUTE(_libraries["libattribute"]).Alias(_activeUsers[msg.sender], Theta);
    }

    function GetBySoul(uint64 Sigma) internal returns(Bao memory On) {
        return Nu.Psi().Mu().Tau().Upsilon().GetRodByIdx(Sigma);
    }

    error NotShioOwner(address Shio, address Requestor);
    function Enter() public returns(uint64[3] memory Saat, Bao memory On) {
        if(_activeUsers[msg.sender] == 0) revert NoUserEntry(msg.sender);
        Saat[0] = Nu.Psi().Pole(2);
        Saat[1] = _activeUsers[msg.sender];
        Saat[2] = Nu.Aura();

        On = GetBySoul(Saat[1]);
        if(!On.Shio.owner(msg.sender)) revert NotShioOwner(address(On.Shio), msg.sender);
        _mintToCap();
    }

    error UserAlreadyCreated(address User);
    function Enter(string memory name, string memory symbol) public returns(uint64[3] memory Saat, Bao memory On) {
        if(_activeUsers[msg.sender] != 0) revert UserAlreadyCreated(msg.sender);
        (Saat, On) = Nu.Miu(name, symbol);
        On.Shio.addOwner(msg.sender);
        if(!On.Shio.owner(msg.sender)) revert NotShioOwner(address(On.Shio), msg.sender);
        _activeUsers[msg.sender] = Saat[1];
        _mintToCap();
    }
}
