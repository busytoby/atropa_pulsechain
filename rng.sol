// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "affinity.sol";

contract Blotter is Affinity {
    Fa private Upsilon;
    bytes constant private NULL = bytes(unicode"\u0009\u0009\u0009");

    constructor() ERC20(/*name short=*/ unicode"BLOTTER™", /*symbol long=*/ unicode"🄮") Ownable(msg.sender) {
        NewDynamic(Xiao.Random(), Xiao.Random(), Xiao.Random());
        Alpha(Mu.Rod.Signal);
        Beta(Mu.Upsilon);
        Write(Mu.Upsilon);
        Read();

        _mint(address(this), 1 * 10 ** decimals());
    }

    function Write(uint64 _v) public {
        assert(msg.sender == owner() || Nu.Context == uint256(uint160(msg.sender)));
        bytes memory _data = toBytes(_v);
        Pi(_v);
        Psi[Nu.Context][Nu.Operator].Theta = bytes.concat(Psi[Nu.Context][Nu.Operator].Theta, toBytes(_data.length), _data, NULL);
    }

    function Read() public returns(bytes memory) {
        Chi(Mu.Upsilon);
        return Psi[Nu.Context][Nu.Operator].Theta;
    }

    function Generate() public returns(uint64) {
        return Phi();
    }
}
