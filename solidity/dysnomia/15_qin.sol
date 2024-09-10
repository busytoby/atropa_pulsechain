// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01_dysnomia_v2.sol";
import "./interfaces/16b_zaointerface.sol";

contract QIN is DYSNOMIA {
    string public constant Type = "QIN";

    ZAOINTERFACE public Zao;
    QING public Location;
    LAU public Alt;
    Bao public On;
    uint256 public qp;
    uint64 public Entropy;
    address[5][9] private _inventory;
    uint256 private _lastMove;

    constructor(address ZaoAddress, address UserToken) DYSNOMIA("Player", "QIN", address(DYSNOMIA(ZaoAddress).Xiao())) {
        Zao = ZAOINTERFACE(ZaoAddress);
        Location = Zao.VoidQing();
        _selectAlt(UserToken);
        addOwner(tx.origin);
    }

    function AddLibraryOwner(string memory what) public onlyOwners {
        _addLibraryOwner(Zao.VoidQing().Void(), what);
    }

    function Withdraw(address what, uint256 amount) public onlyPlayer {
        DYSNOMIA withdrawToken = DYSNOMIA(what);
        withdrawToken.transfer(msg.sender, amount);
    }

    function AddMarketRate(address _a, uint256 _r) public onlyPlayer {
        _addMarketRate(_a, _r);
    }

    modifier onlyPlayer() {
        _checkPlayer();
        _;
    }

    error InvalidOwnership(address UserToken, address User);
    function _checkPlayer() internal view virtual {
        if(!Alt.owner(tx.origin)) revert InvalidOwnership(address(Alt), tx.origin);
    }

    function SelectAlt(address UserToken) public onlyPlayer {
        _selectAlt(UserToken);
        Zao.SetQinEntropy(address(this), Entropy);
    }

    function _selectAlt(address UserToken) internal {
        Alt = LAU(UserToken);
        if(!Alt.owner(tx.origin)) revert InvalidOwnership(UserToken, tx.origin);
        On = Alt.On();
        Entropy = Entropy ^ Xiao.modExp64(On.Shio.Rho().Cone.View().Chin, On.Shio.Rho().Rod.View().Chin, MotzkinPrime);
    }

    function SUN() public view returns (LIBEncrypt) {
        return LIBEncrypt(Location.Void().GetLibraryAddress("encrypt"));
    }

    error OneMovePerBlock();
    function Move(address toQing) public onlyOwners {
        if(_lastMove >= block.timestamp) revert OneMovePerBlock();
        Location = QING(toQing);
        _lastMove = block.timestamp;
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

    function Equip(address qi) public onlyPlayer {
        // TODO
    }

    function Remove(address qi) public onlyPlayer {
        // TODO
    }
}