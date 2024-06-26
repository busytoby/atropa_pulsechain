// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "dynamic.sol";

abstract contract Affinity is Dynamic {
    function Alpha(uint64 _a) public {
        Nu.Context = _a;
        Charge(_a);
        assert(Mu.Sigma > 4);
        Induce();
        Torque();
        Amplify();
        Sustain();
        React();
        if(Nu.Context == 0 && (msg.sender != owner()))
            Nu.Context = uint256(uint160(msg.sender));
        _mintToCap();
    }

    function Beta(uint64 _b) public {
        Nu.Operator = _b;
        Torque(Mu.Rod, _b);
        Amplify(Mu.Rod, Mu.Rod.Alpha);
        Sustain(Mu.Rod, Mu.Rod.Alpha);
        React(Mu.Rod, Mu.Rod.Alpha, Mu.Cone.Dynamo);
        React(Mu.Cone, Mu.Rod.Alpha, Mu.Rod.Dynamo);
        _mintToCap();
    }

    function Pi(uint64 _a) internal {
        Torque(Mu.Cone, Mu.Rod.Kappa);
        Amplify(Mu.Cone, Mu.Cone.Alpha);
        Sustain(Mu.Cone, Mu.Cone.Alpha);
        React(Mu.Rod, Mu.Cone.Alpha, Mu.Rod.Channel);
        React(Mu.Cone, Mu.Cone.Alpha, Mu.Cone.Channel);
        Kappa(_a, false);
        _mintToCap();
    }

    function Chi(uint64 _a) internal {
        Torque(Mu.Cone, Mu.Rod.Eta);
        Amplify(Mu.Cone, Mu.Upsilon);
        Sustain(Mu.Cone, Mu.Ohm);
        React(Mu.Cone, Mu.Pi, Mu.Cone.Dynamo);
        React(Mu.Rod, Mu.Pi, Mu.Rod.Dynamo);
        Mu.Omega = Mu.Omega ^ Mu.Rod.Kappa;
        Kappa(_a, true);
        _mintToCap();
    }

    function Kappa(uint64 _a, bool _Phi) private {
        Nu.Value = _a;
        Mu.Upsilon = _Phi ? _a ^ Mu.Ohm ^ Mu.Pi : _a ^ Mu.Ohm;
        _mintToCap();
    }

    function Phi() internal returns(uint64) {
        Amplify(Mu.Cone, Mu.Upsilon);
        Sustain(Mu.Cone, Mu.Ohm);
        React(Mu.Cone, Mu.Pi, Mu.Cone.Dynamo);
        React(Mu.Rod, Mu.Pi, Mu.Rod.Dynamo);
        Mu.Omega = Mu.Omega ^ Mu.Rod.Kappa;

        Mu.Upsilon = Mu.Upsilon ^ Mu.Ohm ^ Mu.Pi;
        
        _mintToCap();
        return Mu.Upsilon;
    }
}