// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "03_shio.sol";

struct Bao {
    SHA Mu;
    uint64 Xi;

    address Shio;
    uint64 Ring;

    uint64 Omicron;
    uint64 Omega;
}

contract YI is SH {
    string public constant Type = "YI";
        
    SHIO private Psi;
    mapping(address => Bao) private Nu;
    uint64 private Xi;
    uint64 private Ring;
    uint64 public Alpha;

    constructor(address MathLib) ERC20(unicode"VM Yi", unicode"YI") SH(MathLib, 999) MultiOwnable(msg.sender) {
        SHA Rod = new SHA("Shio Rod", "SROD", MathLib);
        SHA Cone = new SHA("Shio Cone", "SCONE", MathLib);
        Psi = new SHIO(address(Rod), address(Cone), MathLib);
        Rod.addOwner(address(Psi));
        Cone.addOwner(address(Psi));
        Xi = Xiao.Random();
        Psi.Generate(Xi, Xiao.Random(), Xiao.Random());
        Ionize();
        Ring = Psi.Magnetize();
        Rod.RegisterAddress(address(Cone));
        Rod.RegisterAddress(address(Psi));
        Rod.RegisterAddress(address(this));
        Cone.RegisterAddress(address(Rod));
        Cone.RegisterAddress(address(Psi));
        Cone.RegisterAddress(address(this));
        Psi.RegisterAddress(address(this));
        RegisterAddress(address(Rod));
        RegisterAddress(address(Cone));
        RegisterAddress(address(Psi));
    }

    function Beta(string calldata Name, string calldata Symbol) public onlyOwners returns(SHA) {
        SHA Xun = new SHA(Name, Symbol, address(Xiao));
        Xun.addOwner(msg.sender);
        return Xun;
    }

    function Bing(Bao memory _b) public onlyOwners {
        Nu[msg.sender] = _b;
    }
    
    function Ionize() private {
        Psi.Isomerize();
        Psi.Isolate();
    }

    function Charge(Bao memory Gamma, uint64 Rho) public onlyOwners returns(uint64) {
        return SHIO(Gamma.Shio).Charge(Gamma.Ring, Rho);
    }

    function Induce(Bao memory Gamma, uint64 Epsilon) public onlyOwners returns(uint64) {
        return SHIO(Gamma.Shio).Induce(Gamma.Ring, Epsilon);
    }

    function Torque(Bao memory Gamma, uint64 Delta) public onlyOwners returns(uint64) {
        return SHIO(Gamma.Shio).Torque(Delta);
    }

    function Amplify(Bao memory Gamma, uint64 Upsilon) public onlyOwners returns(uint64) {
        return Torque(Gamma, Upsilon);
    }

    function Sustain(Bao memory Gamma, uint64 Ohm) public onlyOwners returns(uint64) {
        return Torque(Gamma, Ohm);
    }

    function React(Bao memory Gamma, uint64 Pi) public onlyOwners returns(Bao memory) {
        (Gamma.Omicron, Gamma.Omega) = SHIO(Gamma.Shio).React(Pi);
        mintToCap();
        return Gamma;
    }
}