// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia.sol";
import "./interfaces/06b_zhouinterface.sol";

contract YAU is DYSNOMIA {
    string public constant Type = "YAU";

    ZHOU public Tau;
    Bao public Theta;
    uint64[2] public Monopole;

    constructor(address ZhouAddress) DYSNOMIA(unicode"VM Yau", unicode"YAU", address(DYSNOMIA(ZhouAddress).Xiao())) MultiOwnable(msg.sender) {
        Tau = ZHOU(ZhouAddress);
        Tau.addOwner(address(this));

        Theta = Tau.React(Xiao.Random());
        Theta.Phi = address(this);
        Theta.Xi = Monopole[0] = Theta.Omega;
        Monopole[1] = Theta.Omicron;
        Theta = Tau.Upsilon().InstallRod(Theta.Xi, Theta, Monopole[1]);
        AddMarketRate(address(Tau), 1 * 10 ** decimals());
        AddMarketRate(address(Theta.Mu), 1 * 10 ** decimals());
        AddMarketRate(address(SHIO(Theta.Shio).Cone()), 1 * 10 ** decimals());
        AddMarketRate(Theta.Shio, 1 * 10 ** decimals());
        mintToCap();
    }

    function React() public onlyOwners returns(Bao memory) {
        Bao memory Delta = Tau.Upsilon().Eta().React(Theta, Theta.Omicron);
        Theta.Omicron = Delta.Omicron;
        Theta.Omega = Theta.Omega ^ Delta.Omega;
        
        mintToCap();
        return Delta;
    }
}