// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "atropamath.sol";

/*
interface atropaMath {   
    function hashWith(address a, address b) external returns (uint256);
    function modExp64(uint64 _b, uint64 _e, uint64 _m) external returns(uint64);
    function modExp(uint256 _b, uint256 _e, uint256 _m) external returns (uint256);
}

interface RNG {
    function Generate() external returns (uint64);
}

*/

library Conjecture {
    uint64 constant MotzkinPrime = 953467954114363;
    //address constant libatropaMath = 0x008Fb244289a3FDb9700CC2208848Df0B89F40aa;
    //address constant libatropaMath = 0x5965926fc4f47654Af3E53Ca2f983dA4766C3F48;

    struct Fa {
        uint64 Base;
        uint64 Secret;
        uint64 Signal;
        uint64 Channel;
        uint64 Pole;
        uint64 Identity;
        uint64 Foundation;
        uint64 Element;
        uint64 Dynamo;
        uint64 Manifold;
        uint64 Ring;
        uint64 Barn;
        uint64 Coordinate;

        uint64 Tau;
        uint64 Eta;
        uint64 Kappa;
//        uint64 Rho;
//        uint64 Beta;
//        uint64 Phi;
        uint64 Alpha;

        uint8 Nu;        
    }

    function New() public returns(Fa memory) {
        Fa memory ee;
        ee.Tau = 0;
        Initialize(ee);
        Seed(ee);
        Tune(ee);
        return ee;
    }

    function Initialize(Fa memory ee) internal pure {
        ee.Base = ee.Secret = ee.Signal = ee.Channel = ee.Pole = 0;
        ee.Identity = ee.Foundation = ee.Element = 0;
        ee.Dynamo = 0;
        ee.Manifold = 0;
        ee.Ring = 0;
        ee.Barn = ee.Ring;
        ee.Eta = ee.Kappa = ee.Alpha = 0;
        ee.Nu = 0;
        ee.Coordinate = 0;
    }

    function Seed(Fa memory ee) internal {
        RNG RNG1 = RNG(0xa96BcbeD7F01de6CEEd14fC86d90F21a36dE2143);
        //RNG RNG1 = RNG(0xBDA2A2cBD5E5E9B8BBd52614E94030D71F1dC54B);
        ee.Base = RNG1.Generate();
        ee.Secret = RNG1.Generate();
        ee.Signal = RNG1.Generate();
    }

   function Tune(Fa memory ee) internal {
        ee.Channel = atropaMath.modExp64(ee.Base, ee.Signal, MotzkinPrime);   
        return;
    }

    function Fuse(Fa memory ee, uint64 _rho, uint64 Upsilon, uint64 Ohm) internal pure {
        ee.Base = Upsilon;
        ee.Secret = Ohm;
        ee.Signal = _rho;
    }

    function Avail(Fa memory ee, uint64 Xi) internal {
        ee.Alpha = atropaMath.modExp64(Xi, ee.Secret, MotzkinPrime);
    }

    function Form(Fa memory ee, uint64 Chi) internal {
        ee.Base = atropaMath.modExp64(Chi, ee.Secret, MotzkinPrime);
        Tune(ee);        
    }

    function Polarize(Fa memory ee) internal {
        ee.Pole = atropaMath.modExp64(ee.Base, ee.Secret, MotzkinPrime);
    }

    function Conjugate(Fa memory ee, uint64 Chi) internal {
        ee.Coordinate = atropaMath.modExp64(Chi, ee.Secret, MotzkinPrime);
        // Chi = 0;
    }

    function Conify(Fa memory ee, uint64 _Beta) internal {        
        assert(ee.Nu == 0);
        ee.Identity = _Beta;
        ee.Foundation = atropaMath.modExp64(ee.Base, ee.Identity, MotzkinPrime);
        ee.Nu = 1;
    }


    function Saturate(Fa memory ee, uint64 _Beta, uint64 Epsilon, uint64 Theta) internal returns(uint64 r) {
        if(ee.Nu == 0) {
            ee.Identity = _Beta;
            ee.Foundation = atropaMath.modExp64(ee.Base, ee.Identity, MotzkinPrime);
        }
        assert(ee.Nu <= 1);
        
        uint64 Beta = atropaMath.modExp64(Epsilon, ee.Identity, MotzkinPrime);
        uint64 Rho = atropaMath.modExp64(Theta, ee.Identity, MotzkinPrime);
        ee.Eta = atropaMath.modExp64(Epsilon, ee.Signal, MotzkinPrime);

        uint64 Phi = Rho + ee.Eta;
        ee.Element = Beta + Phi;

        ee.Dynamo = atropaMath.modExp64(Theta, ee.Signal, MotzkinPrime);
        ee.Manifold = ee.Element + ee.Dynamo;

        return ee.Eta;
    }

    function Bond(Fa memory ee) internal {
        ee.Dynamo = atropaMath.modExp64(ee.Base, ee.Signal, ee.Element);
        ee.Pole = 0;
    }

    function Adduct(Fa memory ee, uint64 _Phi) internal {
        ee.Manifold = atropaMath.modExp64(_Phi, ee.Signal, ee.Element);
    }

    function Open(Fa memory ee) internal {
        ee.Ring = atropaMath.modExp64(ee.Coordinate, ee.Manifold, ee.Element);
        ee.Barn = atropaMath.modExp64(ee.Ring, ee.Manifold, ee.Element);
    }

    event DysnomiaNuclearEvent(string What, uint64 Value);

    function ManifoldCompare(Fa memory ee, Fa memory R) internal pure returns(bool) {
        
        //emit DysnomiaNuclearEvent("Manifold Created", ee.Barn);
        return(ee.Manifold == R.Manifold && ee.Ring == R.Ring && ee.Barn == R.Barn);
    }

    function Charge(Fa storage ee, uint64 Psi) internal {
        ee.Alpha = atropaMath.modExp64(ee.Barn, Psi, ee.Ring);
        //emit DysnomiaNuclearEvent("Alpha Charged", ee.Alpha);
    }

    function Induce(Fa storage ee, uint64 Sigma) internal {
        ee.Alpha = atropaMath.modExp64(Sigma, ee.Manifold, ee.Ring);
        //emit DysnomiaNuclearEvent("Alpha Induced", ee.Alpha);
    }

    function Torque(Fa storage ee, uint64 Sigma) internal {
        ee.Alpha = atropaMath.modExp64(Sigma, ee.Element, ee.Channel);
        //emit DysnomiaNuclearEvent("Alpha TORQUE", ee.Alpha);
    }

    function Amplify(Fa storage ee, uint64 Upsilon) internal {
        Torque(ee, Upsilon);
    }

    function Sustain(Fa storage ee, uint64 Ohm) internal {
        Torque(ee, Ohm);
    }

    function React(Fa storage ee, uint64 Pi, uint64 Theta) internal {
        ee.Eta = atropaMath.modExp64(Pi, ee.Channel, Theta);
        ee.Kappa = atropaMath.modExp64(Pi, Theta, ee.Channel);
        assert(ee.Eta != 0 && ee.Kappa != 0);
        //emit DysnomiaNuclearEvent(">>", ee.Eta);
        //emit DysnomiaNuclearEvent("<<", ee.Kappa);
    }
}