// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "05_zheng.sol";

contract ZHOU is SH {
    string public constant Type = "ZHOU";
    address public constant MathLib = 0x48087234ed7bc38e04347176b08B860E643806e2;

    ZHENG public Tau;

    constructor(address YiContract, address ZhengContract) ERC20(unicode"VM Zhou", unicode"ZHOU") SH(MathLib, 313) MultiOwnable(msg.sender) {
        Tau = ZHENG(ZhengContract);
        Tau.addOwner(address(this));

        Bao memory Theta;

        Theta.Mu = Alpha("Zheng Rod", "ZROD");
        SHA Cone = Tau.Eta().Beta("Yi Shio Cone", "ZCONE");
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
        Theta = Tau.InstallRod(0, Theta);

        address[] memory ZhengAddresses = Tau.KnownAddresses();
        for(uint256 i = 0; i < ZhengAddresses.length; i++) {
            RegisterAddress(ZhengAddresses[i]);
        }
        Tau.RegisterAddress(address(this));
        RegisterAddress(address(Tau));
    }

    function Alpha(string memory Name, string memory Symbol) public onlyOwners returns (SHA) {
        return new SHA(Name, Symbol, address(Xiao));
    }

    function Beta(uint64 _theta, uint64 Iota) public returns (Bao memory) {
        Bao memory Theta = Tau.GetRodByIdx(_theta);
        uint64 Interference = SHIO(Theta.Shio).Monopole();
        uint64 DIRAC = Theta.Mu.Torque(Iota) ^ Interference; // torque
        DIRAC = Theta.Mu.Torque(DIRAC) ^ Interference;       // amplify
        DIRAC = Theta.Mu.Torque(DIRAC) ^ Interference;       // sustain
        Theta = Tau.Eta().React(Theta, DIRAC);
        mintToCap();
        return Theta;
    }

    function Pi(uint64 _theta, uint64 _a) public returns(Bao memory) {
        Bao memory Theta = Tau.GetRodByIdx(_theta);
        uint64 Watt = Theta.Mu.Torque(Theta.Omega);          // torque
        uint64 Ohm = Theta.Mu.Flux(Watt);                    // amplify
        uint64 _pi = SHA(SHIO(Theta.Shio).Cone()).Flux(Ohm); // sustain
        Theta = Tau.Eta().React(Theta, _pi);
        Theta.Pi = _a ^ Ohm;
        mintToCap();
        return Theta;
    }
}