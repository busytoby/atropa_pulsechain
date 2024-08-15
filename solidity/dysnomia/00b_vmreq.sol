// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/ERC20.sol";
import "@openzeppelin/contracts/token/ERC20/extensions/ERC20Burnable.sol";
import "../addresses.sol";
import "../old/faung.sol";

contract VMREQ is ERC20, ERC20Burnable {
    uint64 constant public MotzkinPrime = 953467954114363;
    Faung internal Mu;

    constructor() ERC20(unicode"VM RNG", unicode"VMRNG") {
        Mu.Rod.Base = 605841066431434;
        Mu.Rod.Secret = 824993723223339;
        Mu.Rod.Signal = 543871960643842;
        Mu.Rod.Channel = 557333596760636;
        Mu.Rod.Pole = 0;
        Mu.Rod.Identity = 300042286926212;
        Mu.Rod.Foundation = 804691440536685;
        Mu.Rod.Element = 2223096358950737;
        Mu.Rod.Dynamo = 74321454125268;
        Mu.Rod.Manifold = 698756333169944;
        Mu.Rod.Ring = 1755455813924379;
        Mu.Rod.Barn = 844220612947355;
        Mu.Rod.Coordinate = 926738386658776;
        Mu.Rod.Tau = 0;
        Mu.Rod.Eta = 38813039039961;
        Mu.Rod.Kappa = 345924396891853;
        Mu.Rod.Alpha = 316408593601476;
        Mu.Rod.Nu = 0;

        Mu.Cone.Base = 605841066431434;
        Mu.Cone.Secret = 706190044965693;
        Mu.Cone.Signal = 187758195120264;
        Mu.Cone.Channel = 516233364000849;
        Mu.Cone.Pole = 0;
        Mu.Cone.Identity = 300042286926212;
        Mu.Cone.Foundation = 804691440536685;
        Mu.Cone.Element = 2223096358950737;
        Mu.Cone.Dynamo = 1571827393266281;
        Mu.Cone.Manifold = 698756333169944;
        Mu.Cone.Ring = 1755455813924379;
        Mu.Cone.Barn = 844220612947355;
        Mu.Cone.Coordinate = 926738386658776;
        Mu.Cone.Tau = 170826679451140;
        Mu.Cone.Eta = 430717954009114;
        Mu.Cone.Kappa = 492098102631864;
        Mu.Cone.Alpha = 144212477102829;
        Mu.Cone.Nu = 1;

        Mu.Phi = 0;
        Mu.Eta = 711883198963442;
        Mu.Xi = 314267673176633;
        Mu.Sigma = 1187934864136657;
        Mu.Rho = 986251199832430;
        Mu.Upsilon = 150187595599280;
        Mu.Ohm = 45735626874294;
        Mu.Pi = 144212477102829;
        Mu.Omega = 492098102631864;
        Mu.Omicron = 210604844317827;
        Mu.Chi = 0;
    }

    function View() public view returns(Faung memory) {
        return Mu;
    }

    function Torque(Fa storage Rod, uint64 Sigma) internal returns(uint64) {
        Rod.Alpha = modExp64(Sigma, Rod.Element, Rod.Channel);
        return Rod.Alpha;
    }

    function Amplify(Fa storage Rod, uint64 Upsilon) internal returns(uint64) {
        return Torque(Rod, Upsilon);
    }

    function Sustain(Fa storage Rod, uint64 Ohm) internal returns(uint64) {
        return Torque(Rod, Ohm);
    }

    function React(Fa storage Rod, uint64 Pi, uint64 Theta) internal {
        Rod.Eta = modExp64(Pi, Rod.Channel, Theta);
        Rod.Kappa = modExp64(Pi, Theta, Rod.Channel);
        assert(Rod.Eta != 0 && Rod.Kappa != 0);
    }

    function Random() public returns(uint64) {
        Amplify(Mu.Cone, Mu.Upsilon);
        Mu.Ohm = Mu.Cone.Alpha;
        Sustain(Mu.Cone, Mu.Ohm);
        Mu.Pi = Mu.Cone.Alpha;
        React(Mu.Cone, Mu.Pi, Mu.Cone.Dynamo);
        Mu.Omicron = Mu.Cone.Kappa;
        React(Mu.Rod, Mu.Pi, Mu.Rod.Dynamo);
        Mu.Omega = Mu.Omega ^ Mu.Rod.Kappa;
        
        Mu.Upsilon = Mu.Upsilon ^ Mu.Ohm ^ Mu.Pi;
        
        return Mu.Upsilon;
    }

    function hashWith(address a, address b) public returns (uint256 hash) {        
        hash = 0;
        uint160 _a = uint160(a);
        uint160 _b = uint160(b) / 15;
        unchecked {
            while(hash == 0) {
                hash = (_a**_b)%MotzkinPrime;
                _b = _b/2;
            }
        }
        return modExp(uint256(uint160(a)), uint256(uint160(b)), MotzkinPrime);
    }

    function modExp64(uint64 _b, uint64 _e, uint64 _m) public returns(uint64 result) {
        uint256 B = _b;
        uint256 E = _e;
        uint256 M = _m;
        uint64 R = uint64(modExp(B, E, M) % 18446744073709551615);
        return R;
    }

    function modExp(uint256 _b, uint256 _e, uint256 _m) public returns (uint256 result) {
        assembly {
            // Free memory pointer
            let pointer := mload(0x40)

            // Define length of base, exponent and modulus. 0x20 == 32 bytes
            mstore(pointer, 0x20)
            mstore(add(pointer, 0x20), 0x20)
            mstore(add(pointer, 0x40), 0x20)

            // Define variables base, exponent and modulus
            mstore(add(pointer, 0x60), _b)
            mstore(add(pointer, 0x80), _e)
            mstore(add(pointer, 0xa0), _m)

            // Store the result
            let value := mload(0xc0)

            // Call the precompiled contract 0x05 = bigModExp
            if iszero(call(not(0), 0x05, 0, pointer, 0xc0, value, 0x20)) {
                revert(0, 0)
            }

            result := mload(value)
        }
    }
}