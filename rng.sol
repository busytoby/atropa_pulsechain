// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "affinity.sol";

contract Affection is Affinity {
    Fa private Upsilon;

    constructor() ERC20(/*name short=*/ unicode"AFFECTION™", /*symbol long=*/ unicode"Ⓐ") Ownable(msg.sender) {
        NewDynamic(Xiao.Random(), Xiao.Random(), Xiao.Random());
        Alpha(Mu.Rod.Signal);
        Beta(Mu.Upsilon);
        Write(Mu.Upsilon);
        Read();

        _mint(address(this), 1 * 10 ** decimals());
    }

    function Write(uint64 _v) public {
        Pi(_v);
        Psi[Nu.Context][Nu.Operator].Theta = bytes.concat(Psi[Nu.Context][Nu.Operator].Theta, toBytes(_v));
    }

    function Read() public returns(bytes memory) {
        Chi(Mu.Upsilon);
        return Psi[Nu.Context][Nu.Operator].Theta;
    }

    function View() public view returns(Faung memory) {
        return Mu;
    }

    function Generate() public returns(uint64) {
        return Phi();
    }
}
