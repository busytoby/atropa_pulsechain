// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "./interfaces/01b_chointerface.sol";
import "./interfaces/04d_qingfactoryinterface.sol";

contract QING is DYSNOMIA {
    string public constant Type = "QING";

    CHOINTERFACE public Cho;
    DYSNOMIA public Asset;
    QINGFactoryInterface public Factory;
    uint256 public Waat;
    uint64 public Entropy;
    uint16 public BouncerDivisor;
    uint256 public CoverCharge;
    bool public NoCROWS;
    mapping(address => bool) private _staff;
    mapping(address => uint256) private _list;

    constructor(uint256 Luo, address Integrative, address ChoAddress) DYSNOMIA("Mysterious Qing", "q", address(DYSNOMIA(ChoAddress).Xiao())) {
        Cho = CHOINTERFACE(ChoAddress);
        Waat = Luo;

        Asset = DYSNOMIA(Integrative);
        Factory = QINGFactoryInterface(msg.sender);

        _staff[tx.origin] = true;
        setBouncerDivisor(32); // Default Based On Holding 25 CROWS
        setCoverCharge(0);
        NoCROWS = false;

        (Entropy, ) = Cho.React(Xiao.Random());

        _mintToCap();
    }

    function AddMarketRate(address Contract, uint256 Rate) public onlyBouncers {
        _addMarketRate(Contract, Rate);
    }

    function GetQing(uint256 QingWaat) public view returns (QINGINTERFACE) {
        return Factory.GetQing(QingWaat);
    }

    event Withdrawal(uint64 Soul, uint64 Aura, address Token, uint256 amount);
    function Withdraw(address what, uint256 amount) public onlyOwners {
        User memory Alpha = Cho.GetUser();
        DYSNOMIA withdrawToken = DYSNOMIA(what);
        withdrawToken.transfer(msg.sender, amount);
        emit Withdrawal(Alpha.Soul, Cho.Void().Nu().Aura(), what, amount);
    }

    function AllowCROWS(bool _b) public onlyOwners {
        NoCROWS = _b;
    }

    function setBouncerDivisor(uint16 _d) public onlyBouncers {
        BouncerDivisor = _d;
    }

    function setCoverCharge(uint256 _c) public onlyBouncers {        
        CoverCharge = _c;
    }

    function setStaff(address _a, bool active) public onlyBouncers {        
        _staff[_a] = active;
    }

    function setGuestlist(address _a) public onlyBouncers {        
        _list[_a] = block.timestamp + 1 days;
    }

    function removeGuest(address _a) public onlyBouncers {        
        _list[_a] = 0;
    }

    modifier onlyBouncers() {
        _checkBouncer();
        _;
    }

    function bouncer(address cBouncer) public view returns (bool) {
        if(_staff[cBouncer]) return true;
        if(NoCROWS == false && DYSNOMIA(CROWSContract).balanceOf(cBouncer) >= 25 * 10 ** decimals()) return true;

        uint256 _ts = Asset.totalSupply();
        if(Asset.balanceOf(cBouncer) >= (_ts / BouncerDivisor)) return true;

        if(Factory.hasOwner(address(Asset)))
            return Asset.owner(cBouncer);
        return false;
    }

    error BouncerUnauthorized(address origin, address account, address what);
    function _checkBouncer() internal view {
        if(!bouncer(tx.origin) && !bouncer(msg.sender)) {
            address UserToken = Cho.GetUserTokenAddress(tx.origin);
            if(!bouncer(UserToken))
                revert BouncerUnauthorized(tx.origin, msg.sender, address(this));
        }
    }

    error AlreadyJoined(address UserToken);
    error CoverChargeUnauthorized(address AssetAddress, uint256 Amount);
    function Join(address UserToken) public {
        Cho.VerifyUserTokenPermissions(UserToken);

        if(CoverCharge > 0 && _list[UserToken] < block.timestamp) {
            if(Asset.allowance(msg.sender, address(this)) <= CoverCharge) revert CoverChargeUnauthorized(address(Asset), CoverCharge);
            bool paid = Asset.transferFrom(msg.sender, address(this), CoverCharge);
            if(!paid) revert CoverChargeUnauthorized(address(Asset), CoverCharge);
        }

        if(_list[UserToken] < block.timestamp)
            _list[UserToken] = block.timestamp + 1 days;

        if(Cho.GetUserTokenAddress(tx.origin) == address(0x0))
            Cho.Enter(UserToken);

        _mintToCap();
    }

    function Admitted(address UserToken) public view returns (bool) {
        if(CoverCharge != 0 && _list[UserToken] < block.timestamp) return false;
        return true;
    }

    error PayCover(address Asset, uint256 CoverCharge);
    error Forbidden(address Asset);
    error NotAdmitted(uint64 Soul);
    event LogEvent(string Username, uint64 Soul, uint64 Aura, string LogLine);
    function Chat(LAU UserToken, string memory MSG) public {
        uint64 _soul = UserToken.Saat(1);
        Cho.VerifyUserTokenPermissions(address(UserToken));
        if(!Admitted(address(UserToken))) revert NotAdmitted(_soul);
        if(CoverCharge != 0 && !(_list[address(UserToken)] > block.timestamp)) revert PayCover(address(Asset), CoverCharge);
        emit LogEvent(UserToken.Username(), _soul, UserToken.Saat(2), MSG);
    }
}
