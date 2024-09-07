// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./12_delegation.sol";

contract QING is DELEGATION {
    string public constant Type = "QING";

    DYSNOMIA Asset;
    uint64 public Entropy;
    uint16 public BouncerDivisor;
    uint256 public CoverCharge;
    mapping(address => bool) private _staff;
    mapping(address => uint256) private _list;
    uint64[] private _users;

    constructor(address Integrative, address VoidAddress) DELEGATION("Mysterious Qing", "q", VoidAddress) {
        Asset = DYSNOMIA(Integrative);
        Entropy = Xiao.modExp64(On.Shio.Rho().Cone.View().Chin, On.Shio.Rho().Rod.View().Chin, MotzkinPrime);
        setBouncerDivisor(32); // Default Based On Holding 25 CROWS
        setCoverCharge(0);

        if(has(Integrative, "owner()")) addOwner(Asset.owner());
        if(has(Integrative, "name()") && has(Integrative, "symbol()")) Rename(string.concat(Asset.name(), " QING"), string.concat("q", Asset.symbol()));
        _mintToCap();
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

    modifier onlyBouncers() {
        _checkBouncer();
        _;
    }

    function Kick(uint64 Soul, string memory Why) public onlyBouncers {
        User memory Alpha = GetUser();
        Log(Alpha.Soul, Void.Nu().Aura(), string.concat(Alpha.Username, "Kicked User :: (", CYUN().String(Soul), ") ", Delegates[DelegateAddresses[Soul]].Username, " :: ", Why));
        delete Delegates[DelegateAddresses[Soul]];
    }

    function bouncer(address cBouncer) public view returns (bool) {
        if(_staff[cBouncer]) return true;

        uint256 _ts = Asset.totalSupply();
        if(Asset.balanceOf(cBouncer) >= (_ts / BouncerDivisor)) return true;

        return Asset.owner(cBouncer);
    }

    error BouncerUnauthorized(address origin, address account, address what);
    function _checkBouncer() internal view {
        if (!bouncer(msg.sender) && !bouncer(tx.origin)) {
            revert BouncerUnauthorized(tx.origin, msg.sender, address(this));
        }
    }

    error AlreadyJoined(address UserToken);
    error CoverChargeUnauthorized(address AssetAddress, uint256 Amount);
    function Join(address UserToken) public {
        if(Delegates[tx.origin].On.Phi == UserToken) revert AlreadyJoined(UserToken);
        if(_list[UserToken] < block.timestamp && CoverCharge >= 0) {
            if(Asset.allowance(msg.sender, address(this)) <= CoverCharge) revert CoverChargeUnauthorized(address(Asset), CoverCharge + 1);
            bool paid = Asset.transferFrom(msg.sender, address(this), CoverCharge);
            if(!paid) revert CoverChargeUnauthorized(address(Asset), CoverCharge + 1);
        }
        User memory Alpha = Enter(UserToken);
        Bounce();
        Log(Alpha.Soul, Saat[2], string.concat("Joined :: ", Alpha.Username));
        if(_list[UserToken] < block.timestamp) _list[UserToken] = block.timestamp + 1 days;
        _users.push(Alpha.Soul);
    }

    function Alias(string memory name, address _qing) public {
        User memory Alpha = GetUser();
        VAT().Alias(Alpha.Soul, name, _qing);
    }

    function Alias(string memory name) public view returns (address) {
        User memory Alpha = GetUser();
        return VAT().Alias(Alpha.Soul, name);
    }

    function Connect(string memory name, address _qing) public onlyBouncers {
        VAT().Alias(Saat[1], name, _qing);
    }

    function Disconnect(string memory name) public onlyBouncers {
        VAT().Remove(Saat[1], name);
    }

    function Bounce() public {
        for(uint i = 0; i < _users.length; i++) {
            User memory Alpha = Delegates[DelegateAddresses[_users[i]]];
            if(_list[Alpha.On.Phi] < block.timestamp) {
                Log(Alpha.Soul, Void.Nu().Aura(), string.concat("Cover Charge Expired :: ", Alpha.Username));
                delete Delegates[DelegateAddresses[_users[i]]];   
            }
        }
    }

    error NoUserName(address User);
    function Chat(string memory chatline) public {
        User memory Alpha = GetUser();
        if(bytes(Alpha.Username).length < 1) revert NoUserName(tx.origin); 
        Log(Alpha.Soul, Void.Nu().Aura(), string.concat("<", Alpha.Username, "> ", chatline));
        Bounce();
        _mintToCap();
    }
}
