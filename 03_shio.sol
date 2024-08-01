// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "02_sha.sol";

struct Shao {
    SHA Rod;
    SHA Cone;

    uint64 Barn;
}

contract SHIO is SH {
    Shao internal Rho;
    uint64 internal Manifold;
    uint64 internal Monopole;

    constructor(address MathLib) ERC20(unicode"VM Shio", unicode"SHIO") SH(MathLib, 1551) Ownable(msg.sender) {
        Rho.Rod = new SHA("Shio Rod", "SROD", MathLib);
        Rho.Cone = new SHA("Shio Rod", "SCONE", MathLib);
    }

    function View() public view returns(Shao memory) {
        return Rho;
    }

    function ConductorGenerate(uint64 Xi) internal {
        Rho.Rod.Avail(Xi);
        Rho.Cone.Avail(Xi);

        Rho.Rod.Form(Rho.Cone.View().Contour);
        Rho.Cone.Form(Rho.Rod.View().Contour);

        Rho.Rod.Polarize();
        Rho.Cone.Polarize();
    }

    function Generate(uint64 Xi, uint64 Alpha, uint64 Beta) public onlyOwner {
        ConductorGenerate(Xi);

        Rho.Rod.Conjugate(Rho.Cone.View().Pole);
        Rho.Cone.Conjugate(Rho.Rod.View().Pole);

        Rho.Cone.Conify(Alpha);

        Rho.Rod.Saturate(Alpha, Rho.Cone.View().Foundation, Rho.Cone.View().Channel);
        Rho.Cone.Saturate(Beta, Rho.Rod.View().Foundation, Rho.Rod.View().Channel);
    }

    function BondRod() public onlyOwner {
        Rho.Rod.Bond();
    }

    function BondCone() public onlyOwner {
        Rho.Cone.Bond();
    }

    function Magnetize() public onlyOwner returns(uint64) {
        Manifold = Rho.Rod.Adduct(Rho.Cone.Calibrate());
        assert(Manifold == Rho.Cone.Adduct(Rho.Rod.Calibrate()));

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