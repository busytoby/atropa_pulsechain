// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "04_yi.sol";

/*
struct Bao {
    SHA Mu;
    uint64[] Theta;

    uint64 Xi;    // Context
    uint64 Tau;   // Operator
    uint64 Alpha; // Value
    uint64 PeerChannel;

    address Contract;
    uint64 Ring;
}*/

struct Fan {
    Bao[] Rods;
    Bao[] Cones;
}

contract ZHENG is SH {
    string public constant Type = "ZHENG";

    mapping(address => mapping(address => uint64[])) private Eta;
    Fan private Sigma;
    YI private Upsilon;

    Bao public temp;

    constructor(address MathLib, address YiContract) ERC20(unicode"VM Zheng", unicode"ZHENG") SH(MathLib, 100) MultiOwnable(msg.sender) {
        Upsilon = YI(YiContract);
        Bao memory ZhengBao;

        ZhengBao.Mu = new SHA("Zheng Rod", "ZROD", MathLib);
        SHA Cone = Upsilon.Beta("Shio Cone", "ZCONE");
        SHIO ZhengShio = new SHIO(address(ZhengBao.Mu), address(Cone), MathLib);
        ZhengBao.Mu.addOwner(address(ZhengShio));
        Cone.addOwner(address(ZhengShio));
        ZhengShio.addOwner(address(Upsilon));
        ZhengBao.Xi = Xiao.Random();
        ZhengShio.Generate(ZhengBao.Xi, Xiao.Random(), Xiao.Random());
        Ionize(ZhengShio);
        ZhengBao.Shio = address(ZhengShio);
        ZhengBao.Ring = ZhengShio.Magnetize();
        Upsilon.Bing(ZhengBao);
        Sigma.Rods.push(Alpha(ZhengBao, 0));

        address[] memory YiAddresses = Upsilon.KnownAddresses();
        for(uint256 i = 0; i < YiAddresses.length; i++) {
            RegisterAddress(YiAddresses[i]);
        }
        Upsilon.RegisterAddress(address(this));
        RegisterAddress(address(Upsilon));
    }

    function Alpha(Bao memory Beta, uint64 Iota) public returns(Bao memory) {
        uint64 Gamma = Upsilon.Charge(Beta, Iota);
        Gamma = Upsilon.Induce(Beta, Gamma);
        Gamma = Upsilon.Torque(Beta, Gamma);
        Gamma = Upsilon.Amplify(Beta, Gamma);
        Gamma = Upsilon.Sustain(Beta, Gamma);
        Beta = Upsilon.React(Beta, Gamma);
        return Beta;
    }

    function Ionize(SHIO Shio) private {
        Shio.Isomerize();
        Shio.Isolate();
    }
}