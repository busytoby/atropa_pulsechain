// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia.sol";
import "./interfaces/08b_yanginterface.sol";

contract SIU is DYSNOMIA {
    string public constant Type = "SIU";

    YANG public Psi;

    constructor(address YangAddress) DYSNOMIA(unicode"VM Siu", unicode"SIU", address(DYSNOMIA(YangAddress).Xiao())) MultiOwnable(msg.sender) {
        Psi = YANG(YangAddress);
        AddMarketRate(address(Psi), 1 * 10 ** decimals());
        mintToCap();
    }

    function Soul() public view returns (uint64) {
        return uint64(uint160(tx.origin) % MotzkinPrime);
    }

    function Miu(string memory name, string memory symbol) public onlyOwners returns(uint64[3] memory Saat, Bao memory On) {
        Saat[0] = Psi.Pole(2);
        Saat[1] = Xiao.Random();
        Saat[2] = Soul();

        On.Mu = Psi.Mu().Tau().Upsilon().Eta().SHAFactoryInterface().New(string.concat(name, " Rod"), string.concat(symbol, "ROD"), address(address(Xiao)));
        SHA Cone = Psi.Mu().Tau().Upsilon().Eta().Beta("Siu Cone", "VCONE");
        SHIO ZhengShio = Psi.Mu().Tau().Upsilon().Eta().SHIOFactoryInterface().New(address(On.Mu), address(Cone), address(Xiao));
        On.Phi = address(ZhengShio);
        On.Mu.addOwner(address(ZhengShio));
        Cone.addOwner(address(ZhengShio));
        Cone.renounceOwnership(tx.origin);
        ZhengShio.addOwner(address(Psi.Mu().Tau().Upsilon()));
        ZhengShio.addOwner(address(Psi.Mu().Tau().Upsilon().Eta()));
        On.Xi = Saat[0];
        ZhengShio.Generate(On.Xi, Saat[2], Saat[1]);
        Psi.Mu().Tau().Upsilon().Iodize(ZhengShio);
        On.Shio = address(ZhengShio);
        On.Ring = ZhengShio.Magnetize();

        On = Psi.Mu().Tau().Upsilon().InstallCone(Saat[1], On, Saat[2]);
        AddMarketRate(address(Cone), 1 * 10 ** decimals());
        mintToCap();
        return (Saat, On);
    }
}
