// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./03_shio.sol";

struct Bao {
    SHA Mu;
    uint64 Xi;
    uint64 Pi;

    address Shio;
    uint64 Ring;

    uint64 Omicron;
    uint64 Omega;
}

contract YI is SH {
    string public constant Type = "YI";
        
    SHIO public Psi;
    mapping(address => Bao) private Nu;
    uint64 public Xi;
    uint64 public Ring;
    uint64 public Alpha;

    constructor(address MathLib) ERC20(unicode"VM Yi", unicode"YI") SH(MathLib, 999) MultiOwnable(msg.sender) {
        SHA Rod = new SHA("Shio Rod", "SROD", MathLib);
        SHA Cone = new SHA("Shio Cone", "SCONE", MathLib);
        Psi = new SHIO(address(Rod), address(Cone), MathLib);
        Rod.addOwner(tx.origin);
        Rod.addOwner(address(Psi));
        Cone.addOwner(address(Psi));
        Cone.addOwner(tx.origin);
        Psi.addOwner(tx.origin);
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

    function Charge(Bao memory Gamma, uint64 Rho) public returns(uint64) {
        return SHIO(Gamma.Shio).Charge(Gamma.Ring, Rho);
    }

    function Induce(Bao memory Gamma, uint64 Epsilon) public returns(uint64) {
        return SHIO(Gamma.Shio).Induce(Gamma.Ring, Epsilon);
    }

    function Torque(Bao memory Gamma, uint64 Delta) public returns(uint64) {
        return SHIO(Gamma.Shio).Torque(Delta);
    }

    function Amplify(Bao memory Gamma, uint64 Upsilon) public returns(uint64) {
        return Torque(Gamma, Upsilon);
    }

    function Sustain(Bao memory Gamma, uint64 Ohm) public returns(uint64) {
        return Torque(Gamma, Ohm);
    }

    function React(Bao memory Gamma, uint64 Pi) public returns(Bao memory) {
        (Gamma.Omicron, Gamma.Omega) = SHIO(Gamma.Shio).React(Pi);
        mintToCap();
        return Gamma;
    }
}