// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./14_qi.sol";
import "./interfaces/libencrypt.sol";

contract QIN is DYSNOMIA {
    Bao public On;
    LAU public Alt;
    VOID public Void;
    uint256 public qp;
    uint64 public Entropy;
    address[5][9] private _inventory;

    constructor(address VoidQingAddress. address UserToken) DYSNOMIA("Player", "QIN", address(DYSNOMIA(VoidQingAddress).Xiao())) {
        Void = QING(VoidQingAddress).Void();
        SelectAlt(UserToken);
        addOwner(tx.origin);
    }

    function AddMarketRate(address _a, uint256 _r) public onlyOwners {
        _addMarketRate(_a, _r);
    }

    function SelectAlt(address UserToken) public onlyOwners {
        Alt = LAU(UserToken);
        On = Alt.On();
        Entropy = Entropy ^ Xiao.modExp64(On.Shio.Rho().Cone.View().Chin, On.Shio.Rho().Rod.View().Chin, MotzkinPrime);
    }

    function SUN() public view returns (LIBEncrypt) {
        return LIBEncrypt(Void.GetLibraryAddress("encrypt"));
    }

    function Conjure(uint64 Gamma) public onlyOwners returns (bytes memory Geng) {        
        (Entropy, Geng) = SUN().Encapsulate(On, Entropy ^ Gamma, Alt.Saat(0), Alt.Saat(1), Alt.Saat(2));
    }

    function GetInventoryCount(uint256 class) public view returns (uint256) {
        return _inventory[class].length;
    }

    function GetDefense(uint256 n) public view returns (address) {
        return _inventory[0][n];
    }

    function GetDecoration(uint256 n) public view returns (address) {
        return _inventory[1][n];
    }

    function GetOffense(uint256 n) public view returns (address) {
        return _inventory[2][n];
    }

    function Throw() public view returns (uint64) {
        // TODO
    }

    function Save(uint64 pQi) public view returns (uint64) {
        // TODO
    }

    function Equip(address qi) public onlyOwners {
        // TODO
    }

    function Remove(address qi) public onlyOwners {
        // TODO
    }
}