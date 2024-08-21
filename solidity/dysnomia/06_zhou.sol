// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia.sol";
import "./interfaces/05b_zhenginterface.sol";

contract ZHOU is DYSNOMIA {
    string public constant Type = "ZHOU";

    ZHENG public Upsilon;
    uint64 public Xi;
    uint64 public Monopole;

    constructor(address ZhengContract) DYSNOMIA(unicode"VM Zhou", unicode"ZHOU", address(DYSNOMIA(ZhengContract).Xiao())) MultiOwnable(msg.sender) {
        Upsilon = ZHENG(ZhengContract);
        Upsilon.addOwner(address(this));

        Bao memory Theta;
        Theta.Phi = address(this);
        Theta.Mu = Alpha("Zheng Rod", "ZROD");
        SHA Cone = Upsilon.Eta().Beta("Yi Shio Cone", "ZCONE");
        SHIO ZhengShio = Upsilon.Eta().SHIOFactoryInterface().New(address(Theta.Mu), address(Cone), address(DYSNOMIA(ZhengContract).Xiao()));
        Theta.Mu.addOwner(address(ZhengShio));
        Cone.addOwner(address(ZhengShio));
        ZhengShio.addOwner(address(Upsilon));
        ZhengShio.addOwner(address(Upsilon.Eta()));
        Theta.Xi = Xiao.Random();
        ZhengShio.Generate(Theta.Xi, Xiao.Random(), Xiao.Random());
        Upsilon.Iodize(ZhengShio);
        Theta.Shio = address(ZhengShio);
        Theta.Ring = ZhengShio.Magnetize();
        Monopole = Upsilon.Clear(Theta, Theta.Xi);
        Theta = Upsilon.InstallRod(Theta.Xi, Theta, Monopole);
        Xi = Theta.Xi;
        AddMarketRate(address(Upsilon), 1 * 10 ** decimals());
        AddMarketRate(address(Theta.Mu), 1 * 10 ** decimals());
        AddMarketRate(address(Cone), 1 * 10 ** decimals());
        AddMarketRate(Theta.Shio, 1 * 10 ** decimals());
        mintToCap();
    }

    function Alpha(string memory Name, string memory Symbol) public onlyOwners returns (SHA) {
        mintToCap();
        return Upsilon.Eta().SHAFactoryInterface().New(Name, Symbol, address(address(Xiao)));
    }

    function Qin(uint64 Iota) public returns (uint64) {
        Bao memory Theta = Upsilon.GetRodByIdx(Xi);
        uint64 DREYFUS = Theta.Mu.Torque(Iota);
        DREYFUS = Theta.Mu.Flux(DREYFUS);
        DREYFUS = Theta.Mu.Sustain(DREYFUS);
        Theta = Upsilon.Eta().React(Theta, DREYFUS);
        mintToCap();
        return DREYFUS;
    }

    function Pi() public returns(Bao memory) {
        Bao memory Thetan = Upsilon.GetRodByIdx(Xi);
        uint64 Watt = SHA(SHIO(Thetan.Shio).Cone()).Torque(Thetan.Omega);
        uint64 Ohm = SHA(SHIO(Thetan.Shio).Cone()).Flux(Watt);
        uint64 _pi = SHA(SHIO(Thetan.Shio).Cone()).Sustain(Ohm);
        Thetan = Upsilon.Eta().React(Thetan, _pi);
        Thetan.Pi = Monopole ^ Ohm;
        mintToCap();
        return Thetan;
    }
}