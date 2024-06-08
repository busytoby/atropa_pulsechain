// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "affinity.sol";

contract Affection is Affinity {
    constructor() ERC20(/*name short=*/ unicode"AFFECTION™", /*symbol long=*/ unicode"Ⓐ") Ownable(msg.sender) {
        NewDynamic(Xiao.Random(), Xiao.Random(), Xiao.Random());
        Alpha(Mu.Rod.Signal);
        Beta(Mu.Upsilon);
        Upsilon(Mu.Upsilon, false);
        Write();
        Read();
        Upsilon(Mu.Upsilon, true);

        _mint(address(this), 1 * 10 ** decimals());
    }

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

    function Upsilon(uint64 _a, bool Phi) public {
        Nu.Value = _a;
        Mu.Upsilon = Phi ? _a ^ Mu.Ohm ^ Mu.Pi : _a ^ Mu.Ohm;
        _mintToCap();
    }

    function Write() public {
        Torque(Mu.Cone, Mu.Rod.Kappa);
        Amplify(Mu.Cone, Mu.Cone.Alpha);
        Sustain(Mu.Cone, Mu.Cone.Alpha);
        React(Mu.Rod, Mu.Cone.Alpha, Mu.Rod.Channel);
        React(Mu.Cone, Mu.Cone.Alpha, Mu.Cone.Channel);
        _mintToCap();
        Psi[Nu.Context][Nu.Operator].Theta = bytes.concat(Psi[Nu.Context][Nu.Operator].Theta, toBytes(Nu.Value));
    }

    function toBytes(uint256 x) internal pure returns (bytes memory b) { 
        b = new bytes(32); 
        assembly { 
            mstore(add(b, 32), x) 
            } 
    } 

    function Read() public returns(bytes memory) {
        Torque(Mu.Cone, Mu.Rod.Eta);
        Amplify(Mu.Cone, Mu.Upsilon);
        Sustain(Mu.Cone, Mu.Ohm);
        React(Mu.Cone, Mu.Pi, Mu.Cone.Dynamo);
        React(Mu.Rod, Mu.Pi, Mu.Rod.Dynamo);
        Mu.Omega = Mu.Omega ^ Mu.Rod.Kappa;
        _mintToCap();
        return Psi[Nu.Context][Nu.Operator].Theta;
    }

    function View() public view returns(Faung memory) {
        return Mu;
    }

    function Generate() public returns(uint64) {
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
