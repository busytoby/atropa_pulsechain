// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./07_yau.sol";

contract YANG is SH {
    string public constant Type = "YANG";

    YAU private Upsilon;

    uint64 private Context;
    uint64 private Operator;
    address private Value;

    constructor(address YauAddress) ERC20(unicode"VM Yang", unicode"YANG") SH(SH(YauAddress).Xiao.address, 8) MultiOwnable(msg.sender) {
        Upsilon = YAU(YauAddress);
        address[] memory YauAddresses = Upsilon.KnownAddresses();
        for(uint256 i = 0; i < YauAddresses.length; i++) {
            RegisterAddress(YauAddresses[i]);
        }
        Upsilon.RegisterAddress(address(this));
        RegisterAddress(address(Upsilon));
    }
}