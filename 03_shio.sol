// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "02_sha.sol";

struct Shao {
    SHA Rod;
    SHA Cone;

    uint64 Barn;
}

contract SHIO is SH {
    string public constant Type = "SHIO";
        
    Shao private Rho;
    uint64 private Manifold;
    uint64 private Monopole;

    constructor(address Rod, address Cone, address MathLib) ERC20(unicode"VM Shio", unicode"SHIO") SH(MathLib, 1551) MultiOwnable(msg.sender) {
        Rho.Rod = SHA(Rod);
        Rho.Cone = SHA(Cone);
        RegisterAddress(Rod);
        RegisterAddress(Cone);
    }

    function ConductorGenerate(uint64 Xi) private {
        Rho.Rod.Avail(Xi);
        Rho.Cone.Avail(Xi);

        Rho.Rod.Form(Rho.Cone.View().Contour);
        Rho.Cone.Form(Rho.Rod.View().Contour);

        Rho.Rod.Polarize();
        Rho.Cone.Polarize();
    }

    function Generate(uint64 Xi, uint64 Alpha, uint64 Beta) public onlyOwners {
        ConductorGenerate(Xi);

        Rho.Rod.Conjugate(Rho.Cone.View().Pole);
        Rho.Cone.Conjugate(Rho.Rod.View().Pole);

        Rho.Cone.Conify(Alpha);

        Rho.Rod.Saturate(Alpha, Rho.Cone.View().Foundation, Rho.Cone.View().Channel);
        Rho.Cone.Saturate(Beta, Rho.Rod.View().Foundation, Rho.Rod.View().Channel);
    }

    function Isomerize() public onlyOwners {
        Rho.Rod.Bond();
    }

    function Isolate() public onlyOwners {
        Rho.Cone.Bond();
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
}