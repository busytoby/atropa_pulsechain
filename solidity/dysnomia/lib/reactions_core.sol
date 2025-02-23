// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../01_dysnomia_v2.sol";
import "../domain/dan/interfaces/01b_chointerface.sol";
import "../interfaces/10b_voidinterface.sol";

contract COREREACTIONSLIB is DYSNOMIA {
    string public constant Type = "ReactionsLib";

    VOID public Void;
    CHOINTERFACE public Cho;
    uint64 public BLACK;
    uint64 public HYUN;
    uint64 public SHUTTER;
    uint64 public BRIGHTNESS;
    uint64 public HUE;
    uint64 public WHITE;

    constructor(address VoidAddress) DYSNOMIA(unicode"DYSNOMIA ReactionsLib", unicode"ReactionsLib", address(DYSNOMIA(VoidAddress).Xiao())) {
        Void = VOID(VoidAddress);
        Void.AddLibrary("corereactions", address(this));
        _addMarketRate(AFFECTIONContract, 1 * 10 ** decimals());

        (uint64[3] memory Saat, Bao memory On) = Void.Enter(unicode"DYSNOMIA ReactionsLib", unicode"ReactionsLib");
        On.Shio.addOwner(address(this));
        On.Shio.Rho().Rod.addOwner(address(this));

        BLACK = Entropy(On);
        (BRIGHTNESS, HUE) = Initialize(On);
        (BRIGHTNESS, HUE) = React(On, BRIGHTNESS, HUE);
        (HYUN, SHUTTER) = ReactToBang(On, BRIGHTNESS, HUE);
        (BRIGHTNESS, HUE) = ReactToNew(On, Saat);
        (BRIGHTNESS, HUE) = ReactToLai(On, BRIGHTNESS, HUE);
        (BRIGHTNESS, HUE) = OperatorReact(On, BLACK);
        (BRIGHTNESS/**/, HUE/**/) = ReactToLe(On, BLACK, HYUN);
        (BRIGHTNESS, HUE) = ReactBang(BRIGHTNESS);
        (BRIGHTNESS, HUE) = ReactEris(HUE);
        (BRIGHTNESS, HUE) = ReactLai(BRIGHTNESS);
        (BRIGHTNESS, HUE) = ReactFomalhaute(HUE);
        (BRIGHTNESS, HUE) = ReactLe(BRIGHTNESS);
        (BRIGHTNESS, HUE) = ReactFornax(HUE);
        (BRIGHTNESS, WHITE) = ReactShioCone(On.Shio, BLACK);
        assert(BLACK > 1551);
        assert(BRIGHTNESS > 1551);
        assert(HUE > 1551);
        assert(WHITE > 1551);
        _mintToCap();
    }

    function RegisterChoForTalk(address ChoAddress) public onlyOwners {
        Cho = CHOINTERFACE(ChoAddress);
    }

    function Entropy(Bao memory On) public returns (uint64) {
        _mintToCap();
        return Xiao.modExp64(On.Shio.Rho().Cone.View().Chin, On.Shio.Rho().Rod.View().Chin, MotzkinPrime);
    }

    function Initialize(Bao memory On) public returns (uint64, uint64) {
        (On.Omicron, On.Omega) = ReactShioRod(On.Shio, Void.Nu().Psi().Rho().Bang.Omicron);
        (On.Omicron, On.Omega) = ReactLai(On.Omega);
        (On.Omicron, On.Omega) = ReactShioRod(On.Shio, Void.Nu().Psi().Rho().Le.Omicron);
        return (On.Omicron, On.Omega);
    }

    function ReactToTalk(User memory Alpha) public returns (uint64, uint64) {
        _mintToCap();
        Alpha.On.Omega = Cho.ReactUser(Alpha.Soul, Void.Nu().Psi().Rho().Lai.Omicron);
        (Alpha.On.Omicron, Alpha.On.Omega) = ReactLai(Alpha.Soul);
        Alpha.On.Omicron = Cho.ReactUser(Alpha.Soul, Alpha.On.Omega);
        return (Alpha.On.Omicron, Alpha.On.Omega);
    }

    function React(Bao memory On, uint64 Omicron, uint64 Omega) public returns (uint64, uint64) {
        (On.Omega, On.Omicron) = ReactShioRod(On.Shio, Omicron);
        (On.Omega, On.Omicron) = ReactShioRod(On.Shio, Omega);
        return (On.Omicron, On.Omega);
    }

    function ReactToBang(Bao memory On, uint64 Omicron, uint64 Omega) public returns (uint64, uint64) {
        return React(On, Omicron ^ Void.Nu().Psi().Rho().Bang.Omicron, Omega ^ Void.Nu().Psi().Rho().Bang.Omega);
    }

    function ReactToLai(Bao memory On, uint64 Omicron, uint64 Omega) public returns (uint64, uint64) { 
        return React(On, Omicron ^ Void.Nu().Psi().Rho().Lai.Omicron, Omega ^ Void.Nu().Psi().Rho().Lai.Omega); 
    }

    function ReactToLe(Bao memory On, uint64 Omicron, uint64 Omega) public returns (uint64, uint64) { 
        return React(On, Omicron ^ Void.Nu().Psi().Rho().Le.Omicron, Omega ^ Void.Nu().Psi().Rho().Le.Omega); 
    }

    function ReactToNew(Bao memory On, uint64[3] memory Saat) public returns (uint64, uint64) {
        (On.Omicron, On.Omega) = React(On, On.Omicron ^ Saat[2], On.Omega ^ Saat[1]);
        (On.Omicron, On.Omega) = ReactShioRod(On.Shio, On.Omega ^ Void.Nu().Psi().Rho().Le.Omega);
        return (On.Omicron, On.Omega);
    }

    function OperatorReact(Bao memory On, uint64 Omega) public returns (uint64, uint64) { return React(On, On.Omicron ^ Void.Nu().Aura(), On.Omega ^ Omega); }

    function ReactBang(uint64 Eta) public returns (uint64, uint64) {  return ReactShioCone(Void.Nu().Psi().Rho().Bang.Shio, Eta); }
    function ReactLai(uint64 Gamma) public returns (uint64, uint64) { return ReactShioCone(Void.Nu().Psi().Rho().Lai.Shio, Gamma); }
    function ReactLe(uint64 Delta) public returns (uint64, uint64) {  return ReactShioCone(Void.Nu().Psi().Rho().Le.Shio, Delta); }
    function ReactEris(uint64 Iota) public returns (uint64, uint64) { return ReactShioCone(Void.Nu().Psi().Mu().Tau().Upsilon().GetRodByIdx(Void.Nu().Psi().Rho().Lai.Xi).Shio, Iota);}
    function ReactFomalhaute(uint64 Mu) public returns (uint64, uint64) { return ReactShioCone(Void.Nu().Psi().Mu().Tau().Upsilon().GetRodByIdx(Void.Nu().Psi().Mu().Tau().Xi()).Shio, Mu); }
    function ReactFornax(uint64 Rho) public returns (uint64, uint64) { return ReactShioCone(Void.Nu().Psi().Mu().Tau().Upsilon().Eta().Psi(), Rho); }

    function ReactShioRod(SHIO Beta, uint64 Theta) public returns (uint64, uint64) {
        _mintToCap();
        return Beta.Rod().React(Theta, Beta.Cone().View().Channel);
    }

    function ReactShioCone(SHIO Beta, uint64 Theta) public returns (uint64, uint64) {
        _mintToCap();
        return Beta.Cone().React(Theta, Beta.Rod().View().Channel);
    }
}
