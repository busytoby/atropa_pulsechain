// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "dai.sol";

contract Blotter is Dai {
    Fa private Upsilon;

    constructor() ERC20(/*name short=*/ unicode"BLOTTER™", /*symbol long=*/ unicode"🄮") Ownable(msg.sender) {
        NewDynamic(Xiao.Random(), Xiao.Random(), Xiao.Random());
        Alpha(Mu.Rod.Signal);
        Beta(Mu.Upsilon);
        Write(Mu.Upsilon);
        Read();

        _mint(address(this), 1 * 10 ** decimals());
    }
}
