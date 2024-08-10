// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "03_shio.sol";

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
        Xi = Xiao.Random();
        Psi.Generate(Xi, Xiao.Random(), Xiao.Random());
        Ionize();
        Ring = Psi.Magnetize();
        mintToCap();
    }

    function Beta(string calldata Name, string calldata Symbol) public onlyOwners returns(SHA) {
        SHA Xun = new SHA(Name, Symbol, address(Xiao));
        Xun.addOwner(msg.sender);
        mintToCap();
        return Xun;
    }

    function Bing(Bao memory _b) public onlyOwners {
        Nu[msg.sender] = _b;
        mintToCap();
    }
    
    function Ionize() private {
        Psi.Isomerize();
        Psi.Isolate();
        mintToCap();
    }

    function Charge(Bao memory Gamma, uint64 Rho) public returns(uint64) {
        mintToCap();
        return SHIO(Gamma.Shio).Charge(Gamma.Ring, Rho);
    }

    function Induce(Bao memory Gamma, uint64 Epsilon) public returns(uint64) {
        mintToCap();
        return SHIO(Gamma.Shio).Induce(Gamma.Ring, Epsilon);
    }

    function Torque(Bao memory Gamma, uint64 Delta) public returns(uint64) {
        mintToCap();
        return SHIO(Gamma.Shio).Torque(Delta);
    }

    function Amplify(Bao memory Gamma, uint64 Upsilon) public returns(uint64) {
        mintToCap();
        return Torque(Gamma, Upsilon);
    }

    function Sustain(Bao memory Gamma, uint64 Ohm) public returns(uint64) {
        mintToCap();
        return SHIO(Gamma.Shio).Sustain(Ohm);
    }

    function React(Bao memory Gamma, uint64 Pi) public returns(Bao memory) {
        (Gamma.Omicron, Gamma.Omega) = SHIO(Gamma.Shio).React(Pi);
        mintToCap();
        return Gamma;
    }
}