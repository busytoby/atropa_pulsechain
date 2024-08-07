// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "05_zheng.sol";

contract ZHOU is SH {
    string public constant Type = "ZHOU";

    ZHENG public Upsilon;

    constructor(address ZhengContract) ERC20(unicode"VM Zhou", unicode"ZHOU") SH(address(SH(ZhengContract).Xiao()), 313) MultiOwnable(msg.sender) {
        Upsilon = ZHENG(ZhengContract);
        Upsilon.addOwner(address(this));

        Bao memory Theta;

        Theta.Mu = Alpha("Zheng Rod", "ZROD");
        SHA Cone = Upsilon.Eta().Beta("Yi Shio Cone", "ZCONE");
        SHIO ZhengShio = new SHIO(address(Theta.Mu), address(Cone), address(SH(ZhengContract).Xiao()));
        Theta.Mu.addOwner(address(ZhengShio));
        Cone.addOwner(address(ZhengShio));
        ZhengShio.addOwner(address(Upsilon));
        ZhengShio.addOwner(address(Upsilon.Eta()));
        Theta.Xi = Xiao.Random();
        ZhengShio.Generate(Theta.Xi, Xiao.Random(), Xiao.Random());
        Upsilon.Ionize(ZhengShio);
        Theta.Shio = address(ZhengShio);
        Theta.Ring = ZhengShio.Magnetize();
        Theta = Upsilon.InstallRod(Theta.Xi, Theta);

        address[] memory ZhengAddresses = Upsilon.KnownAddresses();
        for(uint256 i = 0; i < ZhengAddresses.length; i++) {
            RegisterAddress(ZhengAddresses[i]);
        }
        Upsilon.RegisterAddress(address(this));
        RegisterAddress(address(Upsilon));
    }

    function Alpha(string memory Name, string memory Symbol) public onlyOwners returns (SHA) {
        return new SHA(Name, Symbol, address(Xiao));
    }

    function Beta(uint64 _theta, uint64 Iota) public returns (Bao memory) {
        Bao memory Theta = Upsilon.GetRodByIdx(_theta);
        uint64 Interference = SHIO(Theta.Shio).Monopole();
        uint64 DIRAC = Theta.Mu.Torque(Iota) ^ Interference; // torque
        DIRAC = Theta.Mu.Torque(DIRAC) ^ Interference;       // amplify
        DIRAC = Theta.Mu.Torque(DIRAC) ^ Interference;       // sustain
        Theta = Upsilon.Eta().React(Theta, DIRAC);
        mintToCap();
        return Theta;
    }

    function Pi(uint64 _theta, uint64 _a) public returns(Bao memory) {
        Bao memory Theta = Upsilon.GetRodByIdx(_theta);
        uint64 Watt = Theta.Mu.Torque(Theta.Omega);          // torque
        uint64 Ohm = Theta.Mu.Flux(Watt);                    // amplify
        uint64 _pi = SHA(SHIO(Theta.Shio).Cone()).Flux(Ohm); // sustain
        Theta = Upsilon.Eta().React(Theta, _pi);
        Theta.Pi = _a ^ Ohm;
        mintToCap();
        return Theta;
    }
}