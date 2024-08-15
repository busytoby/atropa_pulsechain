// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia.sol";
import "./interfaces/09b_siuinterface.sol";
import "./lib/registry.sol";
using LibRegistry for LibRegistry.Registry;

contract VOID is DYSNOMIA {
    string public constant Type = "VOID";

    SIU public Nu;
    mapping(address => uint64) private _activeUsers;

    constructor(address SiuAddress) DYSNOMIA(unicode"VM Void", unicode"VOID", address(DYSNOMIA(SiuAddress).Xiao()), 1) MultiOwnable(msg.sender) {
        Nu = SIU(SiuAddress);
        Nu.addOwner(address(this));
        mintToCap();
    }

    function Enter() public returns(uint64[3] memory Saat, Bao memory On) {
        assert(_activeUsers[msg.sender] != 0);
        Saat[0] = Nu.Psi().Pole(2);
        Saat[1] = _activeUsers[msg.sender];
        Saat[2] = Nu.Psi().Mu().Tau().Qin(uint64(uint160(msg.sender) % Xiao.MotzkinPrime()));

        On = Nu.Psi().Mu().Tau().Upsilon().GetRodByIdx(Saat[1]);
    }

    function Enter(string memory name, string memory symbol) public returns(uint64[3] memory Saat, Bao memory On) {
        assert(_activeUsers[msg.sender] == 0);
        (Saat, On) = Nu.Miu(name, symbol);
        _activeUsers[msg.sender] = Saat[1];
    }
}
