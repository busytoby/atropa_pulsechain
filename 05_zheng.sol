// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "04_yi.sol";

contract ZHENG is SH {
    string public constant Type = "ZHENG";

    YI public Eta;
    mapping(uint64 => Bao) private Sigma;  // Rods

    constructor(address YiContract) ERC20(unicode"VM Zheng", unicode"ZHENG") SH(address(SH(YiContract).Xiao()), 100) MultiOwnable(msg.sender) {
        Eta = YI(YiContract);
        Eta.addOwner(msg.sender);

        address[] memory YiAddresses = Eta.KnownAddresses();
        for(uint256 i = 0; i < YiAddresses.length; i++) {
            RegisterAddress(YiAddresses[i]);
        }
        Eta.RegisterAddress(address(this));
        RegisterAddress(address(Eta));
    }

    function Alpha(Bao memory Beta, uint64 Iota) public returns(Bao memory) {
        uint64 DIRAC = Eta.Charge(Beta, Iota);
        DIRAC = Eta.Induce(Beta, DIRAC);
        DIRAC = Eta.Torque(Beta, DIRAC);
        DIRAC = Eta.Amplify(Beta, DIRAC);
        DIRAC = Eta.Sustain(Beta, DIRAC);
        Beta = Eta.React(Beta, DIRAC);
        return Beta;
    }

    function GetRodByIdx(uint64 _theta) public view returns(Bao memory) {
        return Sigma[_theta];
    }

    function InstallRod(uint64 Theta, Bao memory Beta) public onlyOwners returns(Bao memory) {
        Eta.Bing(Beta);
        Beta = Alpha(Beta, Theta);
        Sigma[Theta] = Beta;
        return Beta;
    }

    function Ionize(SHIO Shio) public {
        Shio.Isomerize();
        Shio.Isolate();
    }
}