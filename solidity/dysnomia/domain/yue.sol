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

    function ChangeOrigin(address NewOrigin) public onlyOwners {
        Origin = NewOrigin;
    }

    function Withdraw(address what, address To, uint256 amount) public onlyOwners {
        if(msg.sender != address(Chan)) revert OnlyChan(msg.sender, address(Chan));
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
    function ForceTransfer(address From, address To, uint256 Amount) public onlyOwners {
        if(msg.sender != address(Chan)) revert OnlyChan(msg.sender, address(Chan));
        if(balanceOf(From) < Amount) revert DysnomiaInsufficientBalance(tx.origin, msg.sender, From, To, address(this), balanceOf(From), Amount);
            _transfer(From, To, Amount);
    }


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