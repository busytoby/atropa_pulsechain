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

    constructor(address SiuAddress) DYSNOMIA(unicode"VM Void", unicode"VOID", address(DYSNOMIA(SiuAddress).Xiao()), 1) MultiOwnable(msg.sender) {
        Nu = SIU(SiuAddress);
        Nu.addOwner(address(this));
        _kecNames["ZHOU"] = keccak256("ZHOU");
        _kecNames["YAU"] = keccak256("YAU");
        _kecNames["YANG"] = keccak256("YANG");
        mintToCap();
    }

    function AddLibrary(string memory name, address _a) public onlyOwners {
        _libraries[name] = _a;
    }

    function Log(string memory LogLine) public onlyOwners {
        SHIO(GetBySoul(_activeUsers[msg.sender]).Shio).Log(LogLine);
    }

    function Log(uint64 Sigma, string memory LogLine) public onlyOwners {
        SHIO(GetBySoul(Sigma).Shio).Log(LogLine);
    }

    function Log(address Sigma, string memory LogLine) public onlyOwners {
        SHIO(GetBySoul(_activeUsers[Sigma]).Shio).Log(LogLine);
    }

    function Log(string memory Xi, string memory LogLine) public onlyOwners {
        bytes32 XiKec = keccak256(bytes(Xi));
        if(XiKec == _kecNames["ZHOU"]) {
          SHIO(GetBySoul(ZHOU(Nu.Psi().Mu().Tau()).Xi()).Shio).Log(LogLine);  
        } else if(XiKec == _kecNames["YAU"]) {
            SHIO(GetBySoul(YAU(Nu.Psi().Mu()).Theta().Xi).Shio).Log(LogLine);  
        } else if(XiKec == _kecNames["YANG"]) {
            SHIO(GetBySoul(YANG(Nu.Psi()).Rho().Lai.Xi).Shio).Log(LogLine);  
        } else assert(false);        
    }

    function SetAttribute(string memory name, string memory value) public {
        assert(_activeUsers[msg.sender] != 0);
        LIBATTRIBUTE(_libraries["libattribute"]).Set(_activeUsers[msg.sender], name, value);
    }

    function GetAttribute(string memory name) public view returns (string memory) {
        return LIBATTRIBUTE(_libraries["libattribute"]).Get(_activeUsers[msg.sender], name);
    }

    function GetBySoul(uint64 Sigma) internal returns(Bao memory On) {
        return Nu.Psi().Mu().Tau().Upsilon().GetRodByIdx(Sigma);
    }

    function Enter() public returns(uint64[3] memory Saat, Bao memory On) {
        assert(_activeUsers[msg.sender] != 0);
        Saat[0] = Nu.Psi().Pole(2);
        Saat[1] = _activeUsers[msg.sender];
        Saat[2] = Nu.Psi().Mu().Tau().Qin(uint64(uint160(msg.sender) % Xiao.MotzkinPrime()));

        On = GetBySoul(Saat[1]);
        Nu.Psi().Mu().Tau().Upsilon().AssertAccess(On);
    }

    function Enter(string memory name, string memory symbol) public returns(uint64[3] memory Saat, Bao memory On) {
        assert(_activeUsers[msg.sender] == 0);
        (Saat, On) = Nu.Miu(name, symbol);
        Nu.Psi().Mu().Tau().Upsilon().AssertAccess(On);
        _activeUsers[msg.sender] = Saat[1];
    }
}
