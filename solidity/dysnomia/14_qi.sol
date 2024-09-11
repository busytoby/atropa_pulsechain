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
    address public ConjureLib;
    VOID public Void;
    uint64[3] public Saat;

    constructor(bytes memory Geng, address Location) DYSNOMIA("Geng Qi", "QI", address(DYSNOMIA(Location).Xiao())) {
        Creator = tx.origin;
        Void = QING(Location).Void();
        Saat = SUN().Saat(Geng);
        addOwner(tx.origin);
        ConjureLib = address(VAI());
        addOwner(ConjureLib);
    }

    function Modify(uint64 Rho, uint64 Upsilon, uint64 Ohm) public onlyOwners {
        if(Rho < MotzkinPrime)
            Saat[0] = Rho;
        if(Upsilon < MotzkinPrime)
            Saat[1] = Upsilon;
        if(Ohm < MotzkinPrime)
            Saat[2] = Ohm;
    }

    function Rename(string memory Adverb, string memory Adjective, string memory Noun) public onlyOwners {
        VAI().RenameQi(Adverb, Noun, Adjective);
    }

    function AddLibraryOwner(string memory what) public onlyOwners {
        _addLibraryOwner(Void, what);
    }

    error OnlyAvailableToConjureLib(address libconjure);
    function ForceTransfer(address from, address to, uint256 amount) public onlyOwners {
        if(msg.sender != ConjureLib) revert OnlyAvailableToConjureLib(ConjureLib);
        _transfer(from, to, amount);
    }

    modifier onlyPlayer() {
        _checkPlayer();
        _;
    }

    error InvalidOwnership(address UserToken, address User);
    function _checkPlayer() internal view virtual {
        if(Creator != tx.origin) revert InvalidOwnership(Creator, tx.origin);
    }

    function acceptConjureLib() public onlyPlayer {
        renounceOwnership(ConjureLib);
        ConjureLib = Void.GetLibraryAddress("conjure");
        addOwner(ConjureLib);
    }

    function transferOwnership(address to) public onlyPlayer {
        Creator = to;
    }

    function VAI() public view returns (LIBCONJURE) {
        return LIBCONJURE(ConjureLib);
    }

    function name() public view override returns (string memory) {
        if(Saat[1] > VAI().Levels(7)) return __name;
        return VAI().qName();
    }

    function symbol() public view override returns (string memory) {
        if(Saat[1] > VAI().Levels(7)) return __symbol;
        return VAI().qSymbol();
    }

    function AddMarketRate(address _a, uint256 _r) public onlyPlayer {
        _addMarketRate(_a, _r);
    }

    function Withdraw(address what, uint256 amount) public onlyPlayer {
        DYSNOMIA withdrawToken = DYSNOMIA(what);
        withdrawToken.transfer(msg.sender, amount);
    }

    function SUN() public view returns (LIBEncrypt) {
        return LIBEncrypt(Void.GetLibraryAddress("encrypt"));
    }
}