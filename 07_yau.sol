// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "01_sh.sol";
import "06_zhou.sol";

contract YAU is SH {
    string public constant Type = "YAU";

    ZHOU private Theta;
    Bao private Alpha;
    SHIO private Shio;
    uint64 public Coulomb;

    mapping(uint64 => uint64) private Connections;
    mapping(uint64 => address) private ConnectionAddresses;

    constructor(address ZhouAddress) ERC20(unicode"VM Yau", unicode"YAU") SH(SH(ZhouAddress).Xiao.address, 111) MultiOwnable(msg.sender) {
        Theta = ZHOU(ZhouAddress);
        Theta.addOwner(address(this));

        Alpha.Mu = Theta.Alpha("Yau Rod", "YROD");
        SHA Cone = Theta.Tau().Eta().Beta("Yi Cone", "XCONE");
        SHIO ZhengShio = new SHIO(address(Alpha.Mu), address(Cone), SH(ZhouAddress).Xiao.address);
        Alpha.Mu.addOwner(address(ZhengShio));
        Cone.addOwner(address(ZhengShio));
        ZhengShio.addOwner(address(Theta));
        Alpha.Xi = Xiao.Random();
        ZhengShio.Generate(Alpha.Xi, Xiao.Random(), Xiao.Random());
        Theta.Tau().Ionize(ZhengShio);
        Alpha.Shio = address(ZhengShio);
        Alpha.Ring = ZhengShio.Magnetize();
        Alpha = Theta.Tau().InstallRod(Alpha.Ring, Alpha);
        Shio = SHIO(Alpha.Shio);
        Chi();

        address[] memory ZhouAddresses = Theta.KnownAddresses();
        for(uint256 i = 0; i < ZhouAddresses.length; i++) {
            RegisterAddress(ZhouAddresses[i]);
        }
        Theta.RegisterAddress(address(this));
        RegisterAddress(address(Theta));
    }

    function Chi() internal {
        Coulomb = SHA(SHIO(Alpha.Shio).Cone()).Torque(Alpha.Omicron);
        uint64 Ohm = SHA(SHIO(Alpha.Shio).Cone()).Flux(Coulomb);
        uint64 Pi = Alpha.Mu.Flux(Ohm);
        Alpha = Theta.Tau().Eta().React(Alpha, Pi);
        Alpha.Omega = Alpha.Omega ^ Shio.Monopole();
        mintToCap();
    }

    function Phi() public onlyOwners returns(Bao memory, uint64) {
        uint64 _b = Shio.Flow(Coulomb);
        uint64 _e = Alpha.Mu.Flux(Shio.Monopole());
        uint64 _m = Alpha.Mu.Flux(SHA(SHIO(Alpha.Shio).Cone()).View().Monopole);
        Bao memory Delta = Theta.Tau().Eta().React(Alpha, _m);
        Coulomb = Xiao.modExp64(_b, _e, _m);
        
        mintToCap();
        return (Delta, Coulomb);
    }
}