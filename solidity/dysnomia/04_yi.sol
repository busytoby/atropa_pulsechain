// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia.sol";
import "./interfaces/02d_shafactoryinterface.sol";
import "./interfaces/03b_shiointerface.sol";
import "./interfaces/03d_shiofactoryinterface.sol";

struct Bao {
    address Phi;
    SHA Mu;
    uint64 Xi;
    uint64 Pi;

    address Shio;
    uint64 Ring;

    uint64 Omicron;
    uint64 Omega;
}

contract YI is DYSNOMIA {
    string public constant Type = "YI";
        
    SHIO public Psi;
    mapping(address => Bao) private Nu;
    uint64 public Xi;
    uint64 public Ring;
    uint64 public Alpha;
    SHAFactory public SHAFactoryInterface;
    SHIOFactory public SHIOFactoryInterface;

    constructor(address _shaFactory, address _shioFactory, address MathLib) DYSNOMIA(unicode"VM Yi", unicode"YI", MathLib, 999) MultiOwnable(msg.sender) {
        SHAFactoryInterface = SHAFactory(_shaFactory);
        SHIOFactoryInterface = SHIOFactory(_shioFactory);
        SHA Rod = SHAFactoryInterface.New("Shio Rod", "SROD", MathLib);
        SHA Cone = SHAFactoryInterface.New("Shio Cone", "SCONE", MathLib);
        Psi = SHIOFactoryInterface.New(address(Rod), address(Cone), MathLib);
        Xi = Xiao.Random();
        Psi.Generate(Xi, Xiao.Random(), Xiao.Random());
        Ionize();
        Ring = Psi.Magnetize();
        mintToCap();
    }

    function Beta(string calldata Name, string calldata Symbol) public onlyOwners returns(SHA) {
        SHA Xun = SHAFactoryInterface.New(Name, Symbol, address(Xiao));
        Xun.addOwner(msg.sender);
        mintToCap();
        return Xun;
    }

    function Bing(Bao memory _b) public onlyOwners {
        Nu[_b.Phi] = _b;
        SHIO(_b.Shio).Log("Added to Yi.Nu");
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