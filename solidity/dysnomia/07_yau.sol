// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia.sol";
import "./interfaces/06b_zhouinterface.sol";

contract YAU is DYSNOMIA {
    string public constant Type = "YAU";

    ZHOU public Tau;
    Bao public Theta;
    uint64[2] public Monopole;
    SHIO public Shio;
    uint64 public Coulomb;

    constructor(address ZhouAddress) DYSNOMIA(unicode"VM Yau", unicode"YAU", address(DYSNOMIA(ZhouAddress).Xiao())) MultiOwnable(msg.sender) {
        Tau = ZHOU(ZhouAddress);
        Tau.addOwner(address(this));

        Theta.Phi = address(this);
        Theta.Mu = Tau.Alpha("Yau Rod", "YROD");
        SHA Cone = Tau.Upsilon().Eta().Beta("Yi Cone", "XCONE");
        SHIO ZhengShio = Tau.Upsilon().Eta().SHIOFactoryInterface().New(address(Theta.Mu), address(Cone), address(DYSNOMIA(ZhouAddress).Xiao()));
        Theta.Mu.addOwner(address(ZhengShio));
        Cone.addOwner(address(ZhengShio));
        ZhengShio.addOwner(address(Tau));
        Theta.Xi = Xiao.Random();
        Monopole[1] = Tau.Qin(Theta.Xi);
        ZhengShio.Generate(Theta.Xi, Monopole[1], Tau.Monopole());
        Tau.Upsilon().Iodize(ZhengShio);
        Theta.Shio = address(ZhengShio);
        Theta.Ring = ZhengShio.Magnetize();
        Monopole[0] = Tau.Upsilon().Clear(Theta, Theta.Ring);
        Theta = Tau.Upsilon().InstallRod(Theta.Xi, Theta, Monopole[0]);
        Shio = SHIO(Theta.Shio);
        Chi();
        mintToCap();
    }

    function Chi() internal {
        Coulomb = SHA(SHIO(Theta.Shio).Rod()).Sustain(Theta.Omicron);
        uint64 Ohm = SHA(SHIO(Theta.Shio).Cone()).Flux(Coulomb);
        uint64 Joule = Theta.Mu.Flux(Ohm);
        Theta = Tau.Upsilon().Eta().React(Theta, Joule);
        Theta.Omega = Theta.Omega ^ Shio.Monopole();
        Theta.Omicron = Theta.Omicron ^ SHA(Shio.Rod()).View().Monopole;
        mintToCap();
    }

    function Phi() public onlyOwners returns(Bao memory, uint64) {
        uint64 _b = Shio.Flow(Coulomb);
        uint64 _e = Theta.Mu.Flux(SHA(SHIO(Theta.Shio).Cone()).View().Monopole);
        uint64 _m = Theta.Mu.Flux(Shio.Monopole());
        Bao memory Delta = Tau.Upsilon().Eta().React(Theta, _m);
        Coulomb = _b ^ _e % _m;
        
        mintToCap();
        return (Delta, Coulomb);
    }
}