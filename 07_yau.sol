// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "06_zhou.sol";

contract YAU is SH {
    string public constant Type = "YAU";

    ZHOU private Tau;
    Bao private Theta;
    uint64[2] public Gamma;
    SHIO private Shio;
    uint64 public Coulomb;

    constructor(address ZhouAddress) ERC20(unicode"VM Yau", unicode"YAU") SH(address(SH(ZhouAddress).Xiao()), 111) MultiOwnable(msg.sender) {
        Tau = ZHOU(ZhouAddress);
        Tau.addOwner(address(this));

        Theta.Mu = Tau.Alpha("Yau Rod", "YROD");
        SHA Cone = Tau.Upsilon().Eta().Beta("Yi Cone", "XCONE");
        SHIO ZhengShio = new SHIO(address(Theta.Mu), address(Cone), address(SH(ZhouAddress).Xiao()));
        Theta.Mu.addOwner(address(ZhengShio));
        Cone.addOwner(address(ZhengShio));
        ZhengShio.addOwner(address(Tau));
        Theta.Xi = Xiao.Random();
        ZhengShio.Generate(Theta.Xi, Tau.Gamma(1), Tau.Gamma(0));
        Tau.Upsilon().Iodize(ZhengShio);
        Theta.Shio = address(ZhengShio);
        Theta.Ring = ZhengShio.Magnetize();
        Gamma[0] = Tau.Upsilon().Clear(Theta, Theta.Ring);
        Theta = Tau.Upsilon().InstallRod(Theta.Xi, Theta, Gamma[0]);
        Shio = SHIO(Theta.Shio);
        Chi();

        address[] memory ZhouAddresses = Tau.KnownAddresses();
        for(uint256 i = 0; i < ZhouAddresses.length; i++) {
            RegisterAddress(ZhouAddresses[i]);
        }
        Tau.RegisterAddress(address(this));
        RegisterAddress(address(Tau));
    }

    function Chi() internal {
        Coulomb = SHA(SHIO(Theta.Shio).Cone()).Sustain(Theta.Omicron);
        uint64 Ohm = SHA(SHIO(Theta.Shio).Cone()).Flux(Coulomb);
        Gamma[1] = Theta.Mu.Flux(Ohm);
        Theta = Tau.Upsilon().Eta().React(Theta, Gamma[1]);
        Theta.Omega = Theta.Omega ^ Shio.Monopole();
        mintToCap();
    }

    function Phi() public onlyOwners returns(Bao memory, uint64) {
        uint64 _b = Shio.Flow(Coulomb);
        uint64 _e = Theta.Mu.Flux(Shio.Monopole());
        uint64 _m = Theta.Mu.Flux(SHA(SHIO(Theta.Shio).Cone()).View().Monopole);
        Bao memory Delta = Tau.Upsilon().Eta().React(Theta, _m);
        Coulomb = Xiao.modExp64(_b, _e, _m);
        
        mintToCap();
        return (Delta, Coulomb);
    }
}