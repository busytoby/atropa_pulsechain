// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia.sol";
import "./interfaces/02b_shainterface.sol";

struct Shao {
    SHA Rod;
    SHA Cone;

    uint64 Barn;
}

contract SHIO is DYSNOMIA {
    string public constant Type = "SHIO";
        
    Shao public Rho;
    uint64 public Manifold;
    uint64 public Monopole;

    event LogEvent(string LogLine);

    constructor(address RodAddress, address ConeAddress, address MathLib) DYSNOMIA(unicode"VM Shio", unicode"SHIO", MathLib, 1551) MultiOwnable(msg.sender) {
        Rho.Rod = SHA(RodAddress);
        Rho.Cone = SHA(ConeAddress);
        mintToCap();
    }

    function Log(string memory LogLine) public onlyOwners {
        emit LogEvent(LogLine);
    }

    function Rod() public view returns(SHA) {
        return Rho.Rod;
    }

    function Cone() public view returns(SHA) {
        return Rho.Cone;
    }

    function ConductorGenerate(uint64 Xi) private {
        Rho.Rod.Avail(Xi);
        Rho.Cone.Avail(Xi);

        Rho.Rod.Form(Rho.Cone.View().Contour);
        Rho.Cone.Form(Rho.Rod.View().Contour);

        Rho.Rod.Polarize();
        Rho.Cone.Polarize();
        mintToCap();
    }

    function Generate(uint64 Xi, uint64 Alpha, uint64 Beta) public onlyOwners {
        ConductorGenerate(Xi);

        Rho.Rod.Conjugate(Rho.Cone.View().Pole);
        Rho.Cone.Conjugate(Rho.Rod.View().Pole);

        Rho.Cone.Conify(Alpha);

        Rho.Rod.Saturate(Alpha, Rho.Cone.View().Foundation, Rho.Cone.View().Channel);
        Rho.Cone.Saturate(Beta, Rho.Rod.View().Foundation, Rho.Rod.View().Channel);
        mintToCap();
    }

    function Isomerize() public onlyOwners {
        Rho.Rod.Bond();
        mintToCap();
    }

    function Isolate() public onlyOwners {
        Rho.Cone.Bond();
        mintToCap();
    }

    function Magnetize() public onlyOwners returns(uint64) {
        Manifold = Rho.Rod.Adduct(Rho.Cone.Dynamo());
        assert(Manifold == Rho.Cone.Adduct(Rho.Rod.Dynamo()));

        uint64 Ring = Xiao.modExp64(Rho.Rod.View().Coordinate, Manifold, Rho.Rod.View().Element);
        Rho.Barn = Xiao.modExp64(Ring, Manifold, Rho.Rod.View().Element);
        assert(Ring == Xiao.modExp64(Rho.Cone.View().Coordinate, Manifold, Rho.Cone.View().Element));
        assert(Rho.Barn == Xiao.modExp64(Ring, Manifold, Rho.Cone.View().Element));

        Monopole = Xiao.modExp64(Rho.Rod.View().Chin, Rho.Cone.View().Chin, MotzkinPrime);
        mintToCap();
        Rho.Rod.mintToCap();
        Rho.Cone.mintToCap();
        return Ring;
    }

    function Charge(uint64 Ring, uint64 Gamma) public onlyOwners returns(uint64) {
        mintToCap();
        return Xiao.modExp64(Rho.Barn, Gamma, Ring);
    }

    function Induce(uint64 Ring, uint64 Beta) public onlyOwners returns(uint64) {
        mintToCap();
        return Beta ^ Manifold % Ring;
    }

    function Torque(uint64 Pi) public onlyOwners returns(uint64) {
        mintToCap();
        return SHA(Rho.Cone).Torque(Pi);
    }

    function Sustain(uint64 Watt) public onlyOwners returns(uint64) {
        mintToCap();
        return SHA(Rho.Rod).Sustain(Watt);
    }

    function Flow(uint64 Pi) public onlyOwners returns(uint64) {
        mintToCap();
        return Xiao.modExp64(Pi, Monopole, Rho.Cone.View().Channel);
    }

    function React(uint64 Pi) public onlyOwners returns(uint64, uint64) {
        Pi = Pi ^ Monopole;
        (uint64 Eta, uint64 Kappa) = Rho.Rod.React(Pi, Rho.Cone.View().Channel);
        (uint64 Omicron, uint64 Omega) = Rho.Cone.React(Pi, Rho.Rod.View().Channel);
        assert((Omicron == Kappa && Omega == Eta));
        assert(Omega != 0 && Omicron != 0);
        mintToCap();
        return(Omicron, Omega);
    }
}