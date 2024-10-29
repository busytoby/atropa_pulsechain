// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../01_dysnomia_v2.sol";
import "./dan/interfaces/01b_chointerface.sol";
import "./sky/interfaces/01b_chaninterface.sol";

contract YUE is DYSNOMIA {
    string public constant Type = "YUE";

    CHAN public Chan;
    address public Origin;
    mapping(address Qing => uint256 Gram) private Hypobar;
    mapping(address Qing => uint256 Gram) private Epibar;

    constructor(string memory name, string memory symbol, address ChanAddress) DYSNOMIA(name, symbol, address(DYSNOMIA(ChanAddress).Xiao())) {
        Chan = CHAN(ChanAddress);
        Origin = tx.origin;
        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());
    }

    function hasMint(address _contract) public view returns (bool does) {
        bytes4 selector = bytes4(keccak256(bytes("Mint(address, uint256)")));
        bytes memory data = abi.encodeWithSelector(selector, address(0x0), 0);
        assembly { does := staticcall(gas(), _contract, add(data, 32), mload(data), 0, 0) }
    }

    function ChangeOrigin(address NewOrigin) public onlyOwners {
        Origin = NewOrigin;
    }

    function IsValidAsset(address GwatAsset, address Integrative) public view returns (bool) {
        bytes32 QINGCHECK = keccak256(abi.encodePacked("QING"));
        QINGINTERFACE Gwat = QINGINTERFACE(GwatAsset);
        while (keccak256(abi.encodePacked(Gwat.Type())) != QINGCHECK) {
            if(address(Gwat.Asset()) == Integrative) return true;
            Gwat = QINGINTERFACE(address(Gwat.Asset()));
        } 
        if(address(Gwat.Asset()) == Integrative) return true;
        return false;
    }

    error InvalidPair(address SpendAsset, address ReceiveAsset);
    error ExchangeRateNotFound(address SpendAsset, address ReceiveAsset);
    function Hong(address SpendAsset, address QingAsset, uint256 PurchaseAmount) public {
        if(!IsValidAsset(QingAsset, SpendAsset)) revert InvalidPair(SpendAsset, QingAsset);
        QINGINTERFACE ReceiveToken = QINGINTERFACE(QingAsset);
        uint256 Rate = GetAssetRate(QingAsset, SpendAsset);
        DYSNOMIA SpendToken = DYSNOMIA(SpendAsset);
        if(Rate == 0) revert ExchangeRateNotFound(SpendAsset, QingAsset);
        uint256 cost = (PurchaseAmount * Rate) / (10 ** decimals());
        bool success1 = SpendToken.transferFrom(msg.sender, address(this), cost);
        require(success1, string.concat(unicode"Need Approved ", SpendToken.name()));
        ReceiveToken.transfer(msg.sender, PurchaseAmount);
    }

    function Hung(address QingAsset, address ReceiveAsset, uint256 RedeemAmount) public {
        if(!IsValidAsset(QingAsset, ReceiveAsset)) revert InvalidPair(QingAsset, ReceiveAsset);
        QINGINTERFACE SpendToken = QINGINTERFACE(QingAsset);
        uint256 Rate = GetAssetRate(QingAsset, ReceiveAsset);
        DYSNOMIA ReceiveToken = DYSNOMIA(ReceiveAsset);
        if(Rate == 0) revert ExchangeRateNotFound(QingAsset, ReceiveAsset);
        uint256 cost = (RedeemAmount * Rate) / (10 ** decimals());
        bool success1 = SpendToken.transferFrom(msg.sender, address(this), RedeemAmount);
        require(success1, string.concat(unicode"Need Approved ", SpendToken.name()));
        ReceiveToken.transfer(msg.sender, cost);
    }

    function GetAssetRate(address QingAsset, address Asset) public view returns (uint256 Rate) {
        QINGINTERFACE Qing = QINGINTERFACE(QingAsset);
        return Qing.GetMarketRate(Asset);
    }   

    error OnlyGameTokens(address what);
    function Withdraw(address what, address To, uint256 amount) public onlyOwners {
        if(msg.sender != address(Chan)) revert OnlyChan(msg.sender, address(Chan));
        if(!hasMint(what)) revert OnlyGameTokens(what);
        DYSNOMIA withdrawToken = DYSNOMIA(what);
        withdrawToken.transfer(To, amount);
    }

    function MintToOrigin() public onlyOwners {
        if(msg.sender != address(Chan)) revert OnlyChan(msg.sender, address(Chan));
        _mintToCap();
        if(balanceOf(address(this)) >= 1 * 10 ** decimals())
            _transfer(address(this), Origin, 1 * 10 ** decimals());
    }

    error OnlyChan(address Sender, address Chan);
    /*
    function ForceTransfer(address From, address To, uint256 Amount) public onlyOwners {
        if(msg.sender != address(Chan)) revert OnlyChan(msg.sender, address(Chan));
        if(balanceOf(From) < Amount) revert DysnomiaInsufficientBalance(tx.origin, msg.sender, From, To, address(this), balanceOf(From), Amount);
            _transfer(From, To, Amount);
    }
    */

    function Bar(address Qing) public view returns (uint256 Hypogram, uint256 Epigram) {
        return (Hypobar[Qing], Epibar[Qing]);
    }

    error ZeroHoldings(address Who);
    function React(address Qing) public onlyOwners returns (uint256 Charge) {
        if(balanceOf(tx.origin) == 0) revert ZeroHoldings(tx.origin);
        QINGINTERFACE _qing = QINGINTERFACE(Qing);
        uint256 Omicron;
        uint256 Omega;
        (Charge, Omicron, Omega) = Chan.Xie().Power(_qing.Waat());        
        Hypobar[Qing] += Omega;
        Epibar[Qing] += Omicron;
    }
}