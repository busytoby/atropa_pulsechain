// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./03_shio.sol";

contract SHIOFactory {
    constructor() {} 

    function New(address Rod, address Cone, address MathLib) public returns(SHIO Zeta) {
        Zeta = new SHIO(Rod, Cone, MathLib);
        Zeta.addOwner(msg.sender);
        Zeta.addOwner(tx.origin);
        Zeta.renounceOwnership(address(this));

        SHA(Rod).addOwner(address(Zeta));
        SHA(Cone).addOwner(address(Zeta));
    }
}