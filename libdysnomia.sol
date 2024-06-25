// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "affinity.sol";

contract libDysnomia is Affinity {
    constructor() ERC20(/*name short=*/ unicode"libDysnomia v1.0", /*symbol long=*/ unicode"DYS") Ownable(msg.sender) Feh(libAtropaMathContract, 1111111111) {        
        NewDynamic(Xiao.Random(), Xiao.Random(), Xiao.Random());
        //NewDynamic(12437235, 1578915, 1547891345);
        Alpha(Mu.Rod.Signal);
        Beta(Mu.Upsilon);
        Write(Mu.Upsilon);
        Read();

        _mint(address(this), 1 * 10 ** decimals());
    }

    function t2() public returns(uint64) {
        return Xiao.Random();
    }

    function Write(uint64 _v) public {
        assert(msg.sender == owner() || Nu.Context == uint256(uint160(msg.sender)));
        //bytes memory _data = toBytes(_v);
        Pi(_v);
        //Psi[Nu.Context][Nu.Operator].Theta = bytes.concat(Psi[Nu.Context][Nu.Operator].Theta, toBytes(_data.length), _data, NULL);
        Psi[Nu.Context][Nu.Operator].Theta = bytes.concat(Psi[Nu.Context][Nu.Operator].Theta, abi.encodePacked(_v));
    }

    function Read() public returns(bytes memory) {
        Chi(Mu.Upsilon);
        return Psi[Nu.Context][Nu.Operator].Theta;
    }

    function Random() public returns(uint64) {
        return Phi();
    }
}
