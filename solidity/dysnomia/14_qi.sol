// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./include/user.sol";
import "./01_dysnomia_v2.sol";
import "./interfaces/13b_qinginterface.sol";
import "./interfaces/13d_qingfactoryinterface.sol";
import "./interfaces/libencrypt.sol";
import "./interfaces/libconjure.sol";

contract QI is DYSNOMIA {
    string public constant Type = "QI";

    address public Creator;
    VOID public Void;
    uint64[3] public Saat;
    uint64 public Quality;
    mapping(uint64 => int64) private _flux;

    constructor(address  _creator, bytes memory Geng, address Location) DYSNOMIA("Geng Qi", "QI", address(DYSNOMIA(Location).Xiao())) {
        Creator = _creator;
        Void = QING(Location).Void();
        Saat = SUN().Saat(Geng);
        addOwner(tx.origin);
    }

    function VAI() public view returns (LIBCONJURE) {
        return LIBCONJURE(Void.GetLibraryAddress("conjure"));
    }

    function Rename(string memory newName, string memory newSymbol) public override onlyOwners {
        VAI().Rename(Saat[1], newName, newSymbol);
    }

    function name() public view override returns (string memory) {
        return VAI().name(Saat[1]);
    }

    function symbol() public view override returns (string memory) {
        return VAI().symbol(Saat[1]);
    }

    function AddMarketRate(address _a, uint256 _r) public onlyOwners {
        _addMarketRate(_a, _r);
    }

    function SUN() public view returns (LIBEncrypt) {
        return LIBEncrypt(Void.GetLibraryAddress("encrypt"));
    }

    function Consider(uint64 User) public onlyOwners returns (int64) {
        // TODO
        //return _flux[User];
    }

    function BlessForUser(uint64 User) public onlyOwners {
        // TODO
    }

    function CurseForUser(uint64 User) public onlyOwners {
        // TODO
    }

    function Bless() public onlyOwners {
        // TODO
    }

    function Curse() public onlyOwners {
        // TODO
    }
}