// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "01_dysnomia.sol";
import "05b_zhenginterface.sol";

contract ZHOU is DYSNOMIA {
    string public constant Type = "ZHOU";

    ZHENG public Upsilon;
    uint64 public Xi;
    uint64 public Monopole;

    constructor(address ZhengContract) DYSNOMIA(unicode"VM Zhou", unicode"ZHOU", address(DYSNOMIA(ZhengContract).Xiao()), 313) MultiOwnable(msg.sender) {
        Upsilon = ZHENG(ZhengContract);
        Upsilon.addOwner(address(this));

        Bao memory Theta;
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
        mintToCap();
    }

    function Alpha(string memory Name, string memory Symbol) public onlyOwners returns (SHA) {
        mintToCap();
        return Upsilon.Eta().SHAFactoryInterface().New(Name, Symbol, address(address(Xiao)));
    }

    function Qin(uint64 Iota) public returns (uint64) {
        Bao memory Theta = Upsilon.GetRodByIdx(Xi);
        uint64 Interference = SHIO(Theta.Shio).Monopole();
        uint64 DREYFUS = Theta.Mu.Torque(Iota); // torque
        DREYFUS = Theta.Mu.Torque(DREYFUS);       // amplify
        uint64 AMP = DREYFUS ^ Interference;
        DREYFUS = Theta.Mu.Sustain(DREYFUS);       // sustain
        Theta = Upsilon.Eta().React(Theta, DREYFUS);
        mintToCap();
        return AMP;
    }

    /*
    function Pi(uint64 _a) public returns(Bao memory) {
        Bao memory Theta = Upsilon.GetRodByIdx(Xi);
        uint64 Watt = Theta.Mu.Torque(Theta.Omega);          // torque
        uint64 Ohm = Theta.Mu.Flux(Watt);                    // amplify
        uint64 _pi = SHA(SHIO(Theta.Shio).Cone()).Flux(Ohm); // sustain
        Theta = Upsilon.Eta().React(Theta, _pi);
        Theta.Pi = _a ^ Ohm;
        mintToCap();
        return Theta;
    }
    */
}