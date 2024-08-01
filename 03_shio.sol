// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "addresses.sol";
import "02_sha.sol";
//import "faung.sol";

struct Shao {
    SHA Rod;
    SHA Cone;

    uint64 Xi;
    uint64 Barn;
}

contract SHIO is SH {
    Shao internal Rho;
    uint64 internal Ring;
    uint64 internal Monopole;

    address constant MathLib = 0x430d082e46091173B8A4f9f48752e16e3A3a4c62;

    constructor() ERC20(unicode"VM Shio", unicode"SHIO") SH(MathLib, 1551) Ownable(msg.sender) {
        Rho.Rod = new SHA("Shio Rod", "SROD", MathLib);
        Rho.Cone = new SHA("Shio Rod", "SCONE", MathLib);

        Rho.Xi = Xiao.Random();
        OpenManifolds(Rho.Xi, Xiao.Random(), Xiao.Random());
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

    function OpenManifolds(uint64 Xi, uint64 Alpha, uint64 Beta) internal {
        ConductorGenerate(Xi);

        Rho.Rod.Conjugate(Rho.Cone.View().Pole);
        Rho.Cone.Conjugate(Rho.Rod.View().Pole);

        Rho.Cone.Conify(Alpha);

        Rho.Rod.Saturate(Alpha, Rho.Cone.View().Foundation, Rho.Cone.View().Channel);
        Rho.Cone.Saturate(Beta, Rho.Rod.View().Foundation, Rho.Rod.View().Channel);

        Ratchet();

        Rho.Rod.Adduct(Rho.Cone.Calibrate());
        Rho.Cone.Adduct(Rho.Rod.Calibrate());

        uint64 RodRing;
        uint64 RodBarn;
        uint64 ConeRing;
        uint64 ConeBarn;
        (RodRing, RodBarn) = Rho.Rod.Open();
        (ConeRing, ConeBarn) = Rho.Cone.Open();

        assert(Rho.Rod.Yao() == Rho.Cone.Yao() && RodRing == ConeRing && RodBarn == ConeBarn);
        Ring = RodRing;
        Rho.Barn = ConeBarn;
        Monopole = Xiao.modExp64(Rho.Rod.View().Chin, Rho.Cone.View().Chin, MotzkinPrime);
        _mintToCap();
    }

    function Ratchet() internal {
        Rho.Rod.Bond();
        Rho.Cone.Bond();
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