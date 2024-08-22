// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia.sol";
import "./interfaces/04b_yiinterface.sol";

contract ZHENG is DYSNOMIA {
    string public constant Type = "ZHENG";

    YI public Eta;
    mapping(uint64 => Bao) private Sigma;  // Rods

    constructor(address YiContract) DYSNOMIA(unicode"VM Zheng", unicode"ZHENG", address(DYSNOMIA(YiContract).Xiao())) {
        Eta = YI(YiContract);
        Eta.addOwner(address(this));
        Augment();
    }

    function Augment() internal {
        AddMarketRate(address(Eta), 1 * 10 ** decimals());
        _mintToCap();
    }

    function GetRodByIdx(uint64 _theta) public returns(Bao memory) {
        _mintToCap();
        return Sigma[_theta];
    }

    error SigmaAlreadyInstalled(address Phi);
    function InstallRod(uint64 Theta, Bao memory Beta, uint64 Monopole) public onlyOwners returns(Bao memory) {
        if(Sigma[Theta].Phi != address(0x0)) revert SigmaAlreadyInstalled(Sigma[Theta].Phi);
        _mintToCap();
        Eta.Bing(Beta);
        Beta = Eta.React(Beta, Monopole);
        Sigma[Theta] = Beta;
        SHIO(Beta.Shio).Log(Theta, 0, "Rod Installed In Zheng Sigma");
        AddMarketRate(Beta.Phi, 1 * 10 ** decimals());
        AddMarketRate(Beta.Shio, 1 * 10 ** decimals());
        AddMarketRate(address(SHIO(Beta.Shio).Rod()), 1 * 10 ** decimals());
        AddMarketRate(address(SHIO(Beta.Shio).Cone()), 1 * 10 ** decimals());
        return Beta;
    }

    function InstallCone(uint64 Theta, Bao memory Beta, uint64 Monopole) public onlyOwners returns(Bao memory) {
        if(Sigma[Theta].Phi != address(0x0)) revert SigmaAlreadyInstalled(Sigma[Theta].Phi);
        _mintToCap();
        Eta.Bing(Beta);
        Beta = Eta.React(Beta, Monopole);
        Sigma[Theta] = Beta;
        SHIO(Beta.Shio).Log(Theta, 0, "Cone Installed In Zheng Sigma");
        AddMarketRate(address(SHIO(Beta.Shio).Cone()), 1 * 10 ** decimals());
        return Beta;
    }

    function Mau(address Rod, address Cone, uint64 Xi, uint64 Rho, uint64 Upsilon) public onlyOwners returns (Bao memory On) {
        On.Phi = msg.sender;
        On.Mu = SHA(Rod);
        SHIO ZhengShio = Eta.SHIOFactoryInterface().New(Rod, Cone, address(Xiao));
        On.Phi = address(ZhengShio);
        ZhengShio.addOwner(msg.sender);
        ZhengShio.addOwner(address(Eta));
        On.Xi = Xi;
        ZhengShio.Generate(On.Xi, Rho, Upsilon);
        Iodize(ZhengShio);
        On.Shio = address(ZhengShio);
        On.Ring = ZhengShio.Magnetize();

        On = InstallCone(Upsilon, On, Rho);        
        _mintToCap();
    }

    function Iodize(SHIO Shio) public {
        Shio.Isomerize();
        Shio.Isolate();
        _mintToCap();
    }
}