// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "05_zheng.sol";

contract ZHOU is SH {
    string public constant Type = "ZHOU";
    address public constant MathLib = 0x48087234ed7bc38e04347176b08B860E643806e2;

    ZHENG public Tau;
    Bao private Theta;
    YI public Upsilon;

    constructor(address YiContract) ERC20(unicode"VM Zhou", unicode"ZHOU") SH(MathLib, 313) MultiOwnable(msg.sender) {
        Tau = new ZHENG(MathLib, YiContract);
        Tau.addOwner(tx.origin);
        Upsilon = YI(YiContract);
        Theta.Mu = new SHA("Zheng Rod", "ZROD", MathLib);
        SHA Cone = Upsilon.Beta("Yi Shio Cone", "ZCONE");
        SHIO ZhengShio = new SHIO(address(Theta.Mu), address(Cone), MathLib);
        Theta.Mu.addOwner(address(ZhengShio));
        Cone.addOwner(address(ZhengShio));
        ZhengShio.addOwner(address(Tau));
        ZhengShio.addOwner(YiContract);
        Theta.Xi = Xiao.Random();
        ZhengShio.Generate(Theta.Xi, Xiao.Random(), Xiao.Random());
        Tau.Ionize(ZhengShio);
        Theta.Shio = address(ZhengShio);
        Theta.Ring = ZhengShio.Magnetize();
        Theta = Tau.InstallRod(Theta);

        address[] memory ZhengAddresses = Tau.KnownAddresses();
        for(uint256 i = 0; i < ZhengAddresses.length; i++) {
            RegisterAddress(ZhengAddresses[i]);
        }
        Tau.RegisterAddress(address(this));
        RegisterAddress(address(Tau));
    }

    function Beta(uint64 Iota) public onlyOwners returns (Bao memory) {      
        uint64 DIRAC = Theta.Mu.Torque(Iota); // torque
        DIRAC = Theta.Mu.Torque(DIRAC);       // amplify
        DIRAC = Theta.Mu.Torque(DIRAC);       // sustain
        Theta = Upsilon.React(Theta, DIRAC);
        mintToCap();
        return Theta;
    }
}