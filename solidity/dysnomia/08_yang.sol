// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia.sol";
import "./interfaces/07b_yauinterface.sol";

struct Tai {
    Bao Bang;
    Bao Lai;
    Bao Le;
}

contract YANG is DYSNOMIA {
    string public constant Type = "YANG";

    YAU public Mu;
    Tai public Rho;
    uint64[3] public Pole;

    constructor(address YauAddress) DYSNOMIA(unicode"VM Yang", unicode"YANG", address(DYSNOMIA(YauAddress).Xiao())) MultiOwnable(msg.sender) {
        Mu = YAU(YauAddress);
        Rho.Bang = Mu.React(); 
        Rho.Bang.Xi = Pole[0] = Rho.Bang.Omicron;

        Rho.Le = Mu.Tau().React(Pole[0]);
        Rho.Le.Xi = Pole[1] = Rho.Le.Omicron;

        Rho.Lai.Phi = address(this);
        Rho.Lai.Mu = Mu.Tau().Alpha("Yang Rod", "MROD");
        SHA Cone = Mu.Tau().Upsilon().Eta().Beta("Yang Cone", "MCONE");
        SHIO ZhengShio = Mu.Tau().Upsilon().Eta().SHIOFactoryInterface().New(address(Rho.Lai.Mu), address(Cone), address(DYSNOMIA(YauAddress).Xiao()));
        Rho.Lai.Mu.addOwner(address(ZhengShio));
        Cone.addOwner(address(ZhengShio));
        ZhengShio.addOwner(address(Mu.Tau()));
        Rho.Lai.Xi = Rho.Bang.Omega ^ Rho.Le.Omega;
        ZhengShio.Generate(Rho.Lai.Xi, Pole[1], Pole[0]);
        Mu.Tau().Upsilon().Iodize(ZhengShio);
        Rho.Lai.Shio = address(ZhengShio);
        Rho.Lai.Ring = ZhengShio.Magnetize();
        Rho.Lai = Mu.Tau().Upsilon().Eta().React(Rho.Lai, Pole[1]);
        Pole[2] = Rho.Lai.Omicron; // South Pole
        Rho.Lai = Mu.Tau().Upsilon().InstallRod(Rho.Lai.Xi, Rho.Lai, Pole[2]);
        AddMarketRate(address(Mu), 1 * 10 ** decimals());
        AddMarketRate(address(Rho.Lai.Mu), 1 * 10 ** decimals());
        AddMarketRate(address(Cone), 1 * 10 ** decimals());
        AddMarketRate(Rho.Lai.Shio, 1 * 10 ** decimals());
        
        AddMarketRate(Rho.Bang.Phi, 1 * 10 ** decimals());
        AddMarketRate(address(Rho.Bang.Mu), 1 * 10 ** decimals());
        AddMarketRate(address(Rho.Bang.Shio), 1 * 10 ** decimals());
        AddMarketRate(address(SHIO(Rho.Bang.Shio).Rod()), 1 * 10 ** decimals());
        AddMarketRate(address(SHIO(Rho.Bang.Shio).Cone()), 1 * 10 ** decimals());

        AddMarketRate(Rho.Le.Phi, 1 * 10 ** decimals());
        AddMarketRate(address(Rho.Le.Mu), 1 * 10 ** decimals());
        AddMarketRate(address(Rho.Le.Shio), 1 * 10 ** decimals());
        AddMarketRate(address(SHIO(Rho.Le.Shio).Rod()), 1 * 10 ** decimals());
        AddMarketRate(address(SHIO(Rho.Le.Shio).Cone()), 1 * 10 ** decimals());
        mintToCap();
    }
}