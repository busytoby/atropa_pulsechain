// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia.sol";
import "./interfaces/08b_yanginterface.sol";

contract SIU is DYSNOMIA {
    string public constant Type = "SIU";

    YANG public Psi;

    constructor(address YangAddress) DYSNOMIA(unicode"CHATLOG Siu", unicode"SIU", address(DYSNOMIA(YangAddress).Xiao())) {
        Psi = YANG(YangAddress);
        Psi.addOwner(address(this));
        Psi.Mu().addOwner(address(this));
        Psi.Mu().Tau().addOwner(address(this));
        Psi.Mu().Tau().Upsilon().addOwner(address(this));
        Psi.Mu().Tau().Upsilon().Eta().addOwner(address(this));

        Augment();
    }

    function Augment() internal {
        AddMarketRate(address(Psi), 1 * 10 ** decimals());
        _mintToCap();
    }

    function Aura() public view returns (uint64) {
        return uint64(uint160(tx.origin) % MotzkinPrime);
    }

    function Miu(string memory name, string memory symbol) public onlyOwners returns(uint64[3] memory Saat, Bao memory On) {
        Saat[0] = Psi.Pole(2);
        Saat[1] = Xiao.Random();
        Saat[2] = Aura();

        On = Psi.Mu().Tau().Upsilon().Mau(name, symbol, Saat[0], Saat[2], Saat[1]);        

        On.Shio.Cone().renounceOwnership(tx.origin);
        AddMarketRate(address(On.Shio.Cone()), 1 * 10 ** decimals());
        _mintToCap();
        return (Saat, On);
    }
}
