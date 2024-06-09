// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "affinity.sol";

contract Affection is Affinity {
    constructor() ERC20(/*name short=*/ unicode"AFFECTION™", /*symbol long=*/ unicode"Ⓐ") Ownable(msg.sender) {
        NewDynamic(Xiao.Random(), Xiao.Random(), Xiao.Random());
        Alpha(Mu.Rod.Signal);
        Beta(Mu.Upsilon);
        Upsilon(Mu.Upsilon, false);
        Write(Mu.Rod.Signal, Mu.Upsilon, Mu.Upsilon);
        Read();
        Upsilon(Mu.Upsilon, true);

        _mint(address(this), 1 * 10 ** decimals());
    }

    function Upsilon(uint64 _a, bool Phi) public {
        Nu.Value = _a;
        Mu.Upsilon = Phi ? _a ^ Mu.Ohm ^ Mu.Pi : _a ^ Mu.Ohm;
        _mintToCap();
    }

    function Write(uint64 _c, uint64 _o, uint64 _v) public {
        Alpha(_c);
        Beta(_o);
        Upsilon(_v, false);
        Pi();
        Psi[_c][_o].Theta = bytes.concat(Psi[_c][_o].Theta, toBytes(_v));
    }

    function Read() public returns(bytes memory) {
        Chi();
        return Psi[Nu.Context][Nu.Operator].Theta;
    }

    function View() public view returns(Faung memory) {
        return Mu;
    }

    function Generate() public returns(uint64) {
        return Phi();
    }
}
