// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./12_delegation.sol";

contract QING is DELEGATION {
    string public constant Type = "QING";

    DYSNOMIA Asset;
    uint64 public Entropy;
    uint16 public BouncerDivisor;
    mapping(address => bool) private _staff;

    constructor(DYSNOMIA Integrative, address VoidAddress) 
                DELEGATION(string.concat(Integrative.name(), " QING"), 
                           string.concat("q", Integrative.symbol()), 
                           VoidAddress) {

        Asset = Integrative;
        Entropy = Xiao.modExp64(On.Shio.Rho().Cone.View().Chin, On.Shio.Rho().Rod.View().Chin, MotzkinPrime);
        setBouncerDivisor(32); // Default Based On Holding 25 CROWS
        _mintToCap();
    }

    function setBouncerDivisor(uint16 _d) public onlyBouncers {
        BouncerDivisor = _d;
    }

    function setStaff(address _a, bool active) public onlyBouncers {
        _staff[_a] = active;
    }

    modifier onlyBouncers() {
        _checkOwner();
        _;
    }

    function Kick(uint64 Soul) public onlyBouncers {
        User memory Alpha = GetUser();
        Log(Alpha.Soul, Void.Nu().Aura(), string.concat(Alpha.Username, "Kicked User :: (", CYUN().String(Soul), ") ", Delegates[DelegateAddresses[Soul]].Username));
        delete Delegates[DelegateAddresses[Soul]];
    }

    function bouncer(address cBouncer) public view virtual returns (bool) {
        if(Asset.owner(cBouncer)) return true;
        if(_staff[cBouncer]) return true;

        uint256 _ts = Asset.totalSupply();
        if(Asset.balanceOf(cBouncer) >= (_ts / BouncerDivisor)) return true;

        return false;
    }

    error BouncerUnauthorized(address origin, address account, address what);
    function _checkBouncer() internal view virtual {
        if (!bouncer(msg.sender) && !bouncer(tx.origin)) {
            revert BouncerUnauthorized(tx.origin, msg.sender, address(this));
        }
    }

    error NoUserName(address User);
    function Chat(string memory chatline) public {
        User memory Alpha = GetUser();
        if(bytes(Alpha.Username).length < 1) revert NoUserName(tx.origin); 
        Log(Alpha.Soul, Void.Nu().Aura(), string.concat("<", Alpha.Username, "> ", chatline));
        _mintToCap();
    }
}
