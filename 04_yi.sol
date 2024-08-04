// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "03_shio.sol";

struct Bao {
    SHA Mu;
    uint64[] Theta;

    uint64 Xi;    // Context
    uint64 Tau;   // Operator
    uint64 Alpha; // Value
    uint64 PeerChannel;

    address Contract;
}

contract YI is SH {
    string public constant Type = "YI";
        
    SHIO private Psi;
    mapping(uint64 => Bao) private Nu;
    uint64 private Xi;
    uint64 private Ring;

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
    
    function Ionize() private {
        Psi.Isomerize();
        Psi.Isolate();
    }

    /*
    function Charge(uint64 Signal) internal returns(uint64) {
        assert(Signal != 0);
        Charge(Rho.Cone, Signal);
        Rho.Sigma = Rho.Cone.Alpha;
        return Rho.Cone.Alpha;
    }

    function Induce() internal returns(uint64) {
        Rho.Cone.Alpha = Induce(Rho.Rod, Rho.Sigma);
        Rho.Rho = Rho.Rod.Alpha;
        return Rho.Cone.Alpha;
    }

    function Torque() internal returns(uint64) {
        Rho.Cone.Alpha = Torque(Rho.Cone, Rho.Rho);
        Rho.Upsilon = Rho.Cone.Alpha;
        return Rho.Cone.Alpha;
    }

    function Amplify() internal returns(uint64) {
        Rho.Cone.Alpha = Amplify(Rho.Cone, Rho.Upsilon);
        Rho.Ohm = Rho.Cone.Alpha;
        return Rho.Cone.Alpha;
    }

    function Sustain() internal returns(uint64) {
        Rho.Cone.Alpha = Sustain(Rho.Cone, Rho.Ohm);
        Rho.Pi = Rho.Cone.Alpha;
        return Rho.Cone.Alpha;
    }

    function React() internal {
        React(Rho.Rod, Rho.Pi, Rho.Cone.Channel);
        React(Rho.Cone, Rho.Pi, Rho.Rod.Channel);
        Rho.Omicron = Rho.Cone.Kappa;
        Rho.Omega = Rho.Rod.Kappa;
        _mintToCap();
    }
    */
}