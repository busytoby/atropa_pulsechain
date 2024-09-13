// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../01_dysnomia_v2.sol";
import "../interfaces/12b_chointerface.sol";

contract COREREACTIONSLIB is DYSNOMIA {
    string public constant Type = "ReactionsLib";

    CHOINTERFACE public Cho;

    constructor(address ChoAddress) DYSNOMIA(unicode"DYSNOMIA ReactionsLib", unicode"ReactionsLib", address(DYSNOMIA(ChoAddress).Xiao())) {
        Cho = CHOINTERFACE(ChoAddress);
        Cho.Void().AddLibrary("corereactions", address(this));
        _mintToCap();
    }

    function Entropy(Bao memory On) public returns (uint64) {
        _mintToCap();
        return Xiao.modExp64(On.Shio.Rho().Cone.View().Chin, On.Shio.Rho().Rod.View().Chin, MotzkinPrime);
    }

    function Initialize(Bao memory On) public returns (uint64, uint64) {
        _mintToCap();
        (On.Omicron, On.Omega) = ReactShioRod(On.Shio, Cho.Void().Nu().Psi().Rho().Bang.Omicron);
        (On.Omicron, On.Omega) = ReactLai(On.Omega);
        (On.Omicron, On.Omega) = ReactShioRod(On.Shio, Cho.Void().Nu().Psi().Rho().Le.Omicron);
        return (On.Omicron, On.Omega);
    }

    function ReactToTalk(User memory Alpha) public returns (uint64, uint64) {
        _mintToCap();
        (Alpha.On.Omicron, Alpha.On.Omega) = Cho.ReactUser(Alpha.Soul, Cho.Void().Nu().Psi().Rho().Lai.Omicron);
        (Alpha.On.Omicron, Alpha.On.Omega) = Cho.ReactLai(Alpha.Soul);
        (Alpha.On.Omicron, Alpha.On.Omega) = Cho.ReactUser(Alpha.Soul, Alpha.On.Omega);
        return (Alpha.On.Omicron, Alpha.On.Omega);
    }

    function React(Bao memory On, uint64 Omicron, uint64 Omega) public returns (uint64, uint64) {
        (On.Omega, On.Omicron) = ReactShioRod(On.Shio, Omicron);
        (On.Omega, On.Omicron) = ReactShioRod(On.Shio, Omega);
        return (On.Omicron, On.Omega);
    }

    function ReactToBang(Bao memory On, uint64 Omicron, uint64 Omega) public returns (uint64, uint64) {
        React(On, Omicron ^ Cho.Void().Nu().Psi().Rho().Bang.Omicron, Omega ^ Cho.Void().Nu().Psi().Rho().Bang.Omega);
        return (On.Omicron, On.Omega);
    }

    function ReactToLai(Bao memory On, uint64 Omicron, uint64 Omega) public returns (uint64, uint64) {
        React(On, Omicron ^ Cho.Void().Nu().Psi().Rho().Lai.Omicron, Omega ^ Cho.Void().Nu().Psi().Rho().Lai.Omega);
        return (On.Omicron, On.Omega);
    }

    function ReactToLe(Bao memory On, uint64 Omicron, uint64 Omega) public returns (uint64, uint64) {
        React(On, Omicron ^ Cho.Void().Nu().Psi().Rho().Le.Omicron, Omega ^ Cho.Void().Nu().Psi().Rho().Le.Omega);
        return (On.Omicron, On.Omega);
    }

    function ReactToNew(Bao memory On, uint64[3] memory Saat) public returns (uint64, uint64) {
        (On.Omicron, On.Omega) = React(On, On.Omicron ^ Saat[2], On.Omega ^ Saat[1]);
        (On.Omicron, On.Omega) = ReactShioRod(On.Shio, On.Omega ^ Cho.Void().Nu().Psi().Rho().Le.Omega);
        return (On.Omicron, On.Omega);
    }

    function OperatorReact(Bao memory On, uint64 Omega) public returns (uint64, uint64) {
        (On.Omicron, On.Omega) = React(On, On.Omicron ^ Cho.Void().Nu().Aura(), On.Omega ^ Omega);
        return (On.Omicron, On.Omega);
    }

    function ReactShioRod(SHIO Beta, uint64 Theta) public returns (uint64, uint64) {
        _mintToCap();
        return Beta.Rod().React(Theta, Beta.Cone().View().Channel);
    }

    function ReactShioCone(SHIO Beta, uint64 Theta) public returns (uint64, uint64) {
        _mintToCap();
        return Beta.Cone().React(Theta, Beta.Rod().View().Channel);
    }

    function ReactBang(uint64 Eta) public returns (uint64, uint64) {
        _mintToCap();
        return ReactShioRod(Cho.Void().Nu().Psi().Rho().Bang.Shio, Eta);
    }

    function ReactLai(uint64 Gamma) public returns (uint64, uint64) {
        _mintToCap();
        return ReactShioRod(Cho.Void().Nu().Psi().Rho().Lai.Shio, Gamma);
    }

    function ReactLe(uint64 Delta) public returns (uint64, uint64) {
        _mintToCap();
        return ReactShioCone(Cho.Void().Nu().Psi().Rho().Le.Shio, Delta);
    }
}