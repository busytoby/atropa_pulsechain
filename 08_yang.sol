// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "01_dysnomia.sol";
import "07b_yauinterface.sol";

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

    constructor(address YauAddress) DYSNOMIA(unicode"VM Yang", unicode"YANG", address(DYSNOMIA(YauAddress).Xiao()), 8) MultiOwnable(msg.sender) {
        Mu = YAU(YauAddress);
        (Rho.Bang, Pole[0]) = Mu.Phi(); // North Pole
        Rho.Le = Mu.Tau().Pi();

        Rho.Lai.Mu = Mu.Tau().Alpha("Yang Rod", "MROD");
        SHA Cone = Mu.Tau().Upsilon().Eta().Beta("Yang Cone", "MCONE");
        SHIO ZhengShio = Mu.Tau().Upsilon().Eta().SHIOFactoryInterface().New(address(Rho.Lai.Mu), address(Cone), address(DYSNOMIA(YauAddress).Xiao()));
        Rho.Lai.Mu.addOwner(address(ZhengShio));
        Cone.addOwner(address(ZhengShio));
        ZhengShio.addOwner(address(Mu.Tau()));
        Rho.Lai.Xi = Xiao.Random();
        Pole[1] = SHA(SHIO(Mu.Theta().Shio).Rod()).View().Monopole;
        ZhengShio.Generate(Rho.Lai.Xi, Pole[1], Pole[0]);
        Mu.Tau().Upsilon().Iodize(ZhengShio);
        Rho.Lai.Shio = address(ZhengShio);
        Rho.Lai.Ring = ZhengShio.Magnetize();
        Pole[2] = Mu.Tau().Qin(Rho.Lai.Xi); // South Pole
        Rho.Lai = Mu.Tau().Upsilon().InstallRod(Rho.Lai.Xi, Rho.Lai, Pole[1]);

        mintToCap();
    }


}