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

    function Alpha2(Bao memory Beta, uint64 Iota) public returns(uint64[5] memory, uint64, uint64) {
        uint64 B;
        uint64[5] memory A;
        B = Eta.Charge(Beta, Iota);
        A[0] = B;
        B = Eta.Induce(Beta, B);
        A[1] = B;
        B = Eta.Torque(Beta, B);
        A[2] = B;
        B = Eta.Amplify(Beta, B);
        A[3] = B;
        B = Eta.Sustain(Beta, B);
        A[4] = B;
        Beta = Eta.React(Beta, B);
        return (A, Beta.Omicron, Beta.Omega);
    }

    function Alpha(Bao memory Beta, uint64 Iota) public returns(Bao memory) {
        uint64 DIRAC1 = Eta.Charge(Beta, Iota);
        DIRAC1 = Eta.Induce(Beta, DIRAC1);
        uint64 DIRAC2 = Eta.Torque(Beta, DIRAC1);
        DIRAC2 = Eta.Amplify(Beta, DIRAC2);
        assert(DIRAC1 == DIRAC2);
        DIRAC2 = Eta.Sustain(Beta, DIRAC2);
        Beta = Eta.React(Beta, DIRAC2);
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