// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia.sol";
import "./interfaces/04b_yiinterface.sol";

contract ZHENG is DYSNOMIA {
    string public constant Type = "ZHENG";

    YI public Eta;
    mapping(uint64 => Bao) private Sigma;  // Rods

    constructor(address YiContract) DYSNOMIA(unicode"VM Zheng", unicode"ZHENG", address(DYSNOMIA(YiContract).Xiao()), 100) MultiOwnable(msg.sender) {
        Eta = YI(YiContract);
        mintToCap();
    }

    function Clear(Bao memory Beta, uint64 Iota) public returns(uint64) {
        uint64 DIRAC = Eta.Charge(Beta, Iota);
        DIRAC = Eta.Induce(Beta, DIRAC);
        mintToCap();
        return DIRAC;
    }

    function Sign(Bao memory Beta, uint64 DIRAC1) public returns(Bao memory) {
        uint64 DIRAC2 = Eta.Torque(Beta, DIRAC1);
        DIRAC2 = Eta.Amplify(Beta, DIRAC2);
        assert(DIRAC1 == DIRAC2);
        DIRAC2 = Eta.Sustain(Beta, DIRAC2);
        Beta = Eta.React(Beta, DIRAC2);
        mintToCap();
        return Beta;
    }

    function GetRodByIdx(uint64 _theta) public returns(Bao memory) {
        mintToCap();
        return Sigma[_theta];
    }

    function InstallRod(uint64 Theta, Bao memory Beta, uint64 DIRAC) public onlyOwners returns(Bao memory) {
        assert(Sigma[Theta].Phi == address(0x0));
        mintToCap();
        Eta.Bing(Beta);
        Beta = Sign(Beta, DIRAC);
        Sigma[Theta] = Beta;
        return Beta;
    }

    function InstallCone(uint64 Theta, Bao memory Beta, uint64 DIRAC) public onlyOwners returns(Bao memory) {
        assert(Sigma[Theta].Phi == address(0x0));
        mintToCap();
        Eta.Bing(Beta);
        Beta = Sign(Beta, DIRAC);
        Sigma[Theta] = Beta;
        return Beta;
    }

    function Iodize(SHIO Shio) public {
        Shio.Isomerize();
        Shio.Isolate();
        mintToCap();
    }
}