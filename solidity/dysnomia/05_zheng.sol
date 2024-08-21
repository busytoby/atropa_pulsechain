// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia.sol";
import "./interfaces/04b_yiinterface.sol";

contract ZHENG is DYSNOMIA {
    string public constant Type = "ZHENG";

    YI public Eta;
    mapping(uint64 => Bao) private Sigma;  // Rods

    constructor(address YiContract) DYSNOMIA(unicode"VM Zheng", unicode"ZHENG", address(DYSNOMIA(YiContract).Xiao())) MultiOwnable(msg.sender) {
        Eta = YI(YiContract);
        AddMarketRate(address(Eta), 1 * 10 ** decimals());
        mintToCap();
    }

    function GetRodByIdx(uint64 _theta) public returns(Bao memory) {
        mintToCap();
        return Sigma[_theta];
    }

    function AssertAccess(Bao memory Beta) public onlyOwners {
        if(!(SHIO(Beta.Shio).owner(msg.sender)))
            SHIO(Beta.Shio).addOwner(msg.sender);
    }

    function InstallRod(uint64 Theta, Bao memory Beta, uint64 Monopole) public onlyOwners returns(Bao memory) {
        assert(Sigma[Theta].Phi == address(0x0));
        mintToCap();
        Eta.Bing(Beta);
        Beta = Eta.React(Beta, Monopole);
        Sigma[Theta] = Beta;
        SHIO(Beta.Shio).Log(Theta, "Rod Installed In Zheng Sigma");
        AddMarketRate(Beta.Phi, 1 * 10 ** decimals());
        AddMarketRate(Beta.Shio, 1 * 10 ** decimals());
        AddMarketRate(address(SHIO(Beta.Shio).Rod()), 1 * 10 ** decimals());
        AddMarketRate(address(SHIO(Beta.Shio).Cone()), 1 * 10 ** decimals());
        return Beta;
    }

    function InstallCone(uint64 Theta, Bao memory Beta, uint64 Monopole) public onlyOwners returns(Bao memory) {
        assert(Sigma[Theta].Phi == address(0x0));
        mintToCap();
        Eta.Bing(Beta);
        Beta = Eta.React(Beta, Monopole);
        Sigma[Theta] = Beta;
        SHIO(Beta.Shio).Log(Theta, "Cone Installed In Zheng Sigma");
        AddMarketRate(address(SHIO(Beta.Shio).Cone()), 1 * 10 ** decimals());
        return Beta;
    }

    function Iodize(SHIO Shio) public {
        Shio.Isomerize();
        Shio.Isolate();
        mintToCap();
    }
}