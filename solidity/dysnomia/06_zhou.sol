// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia.sol";
import "./interfaces/05b_zhenginterface.sol";

contract ZHOU is DYSNOMIA {
    string public constant Type = "ZHOU";

    ZHENG public Upsilon;
    uint64 public Xi;
    uint64 public Monopole;

    constructor(address ZhengContract) DYSNOMIA(unicode"CHATLOG Zhou", unicode"ZHOU", address(DYSNOMIA(ZhengContract).Xiao())) {
        Upsilon = ZHENG(ZhengContract);
        Upsilon.addOwner(address(this));
        Upsilon.Eta().addOwner(address(this));

        Bao memory Theta;
        Theta.Phi = address(this);
        Theta.Mu = Alpha("Zheng Rod", "ZROD");
        SHA Cone = Upsilon.Eta().Beta("Yi Shio Cone", "ZCONE");
        Cone.addOwner(address(Upsilon));
        Theta.Shio = Upsilon.Eta().Kappa(Theta.Mu, Cone);
        Theta.Shio.addOwner(address(Upsilon));
        Theta.Shio.addOwner(address(Upsilon.Eta()));
        Theta.Xi = Xiao.Random();
        Theta.Shio.Generate(Theta.Xi, Xiao.Random(), Xiao.Random());
        Upsilon.Iodize(Theta.Shio);
        Theta.Ring = Theta.Shio.Magnetize();
        Theta = Upsilon.Eta().React(Theta, Theta.Xi);
        Monopole = Theta.Omicron;
        Theta = Upsilon.InstallRod(Theta.Xi, Theta, Monopole);
        Xi = Theta.Xi;
        Augment(Theta);
    }

    function Augment(Bao memory Theta) internal {
        AddMarketRate(address(Upsilon), 1 * 10 ** decimals());
        AddMarketRate(address(Theta.Mu), 1 * 10 ** decimals());
        AddMarketRate(address(Theta.Shio.Cone()), 1 * 10 ** decimals());
        AddMarketRate(address(Theta.Shio), 1 * 10 ** decimals());
        _mintToCap();
    }

    function Alpha(string memory Name, string memory Symbol) public onlyOwners returns (SHA Epsilon) {
        _mintToCap();
        Epsilon = Upsilon.Eta().Beta(Name, Symbol);
        Epsilon.addOwner(msg.sender);
        Epsilon.addOwner(address(Upsilon));
        Epsilon.addOwner(address(Upsilon.Eta()));
    }

    function React(uint64 Iota) public returns (Bao memory) {
        Bao memory Theta = Upsilon.GetRodByIdx(Xi);        
        Theta = Upsilon.Eta().React(Theta, Iota);
        _mintToCap();
        return Theta;
    }
}