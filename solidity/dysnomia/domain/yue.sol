// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../01_dysnomia_v2.sol";
import "../interfaces/12b_chointerface.sol";

contract YUE is DYSNOMIA {
    string public constant Type = "YUE";

    CHOINTERFACE public Cho;
    address public Chan;
    address public Origin;
    string[] public KnownAdjectives;
    mapping(string Adjective => uint256 Gram) public Hypobar;
    mapping(string Adjective => uint256 Gram) public Epibar;

    constructor(string memory name, string memory symbol, address ChoAddress, address ChanAddress) DYSNOMIA(name, symbol, address(DYSNOMIA(ChoAddress).Xiao())) {
        Cho = CHOINTERFACE(ChoAddress);
        Chan = ChanAddress;
        Origin = tx.origin;
        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());
    }

    function Withdraw(address what, uint256 amount) public onlyOwners {
        if(msg.sender != Chan) revert OnlyChan(msg.sender, Chan);
        DYSNOMIA withdrawToken = DYSNOMIA(what);
        withdrawToken.transfer(msg.sender, amount);
    }

    function MintToOrigin() public onlyOwners {
        if(msg.sender != Chan) revert OnlyChan(msg.sender, Chan);
        _mintToCap();
        if(balanceOf(address(this)) >= 1 * 10 ** decimals())
            _transfer(address(this), Origin, 1 * 10 ** decimals());
    }

    error OnlyChan(address Sender, address Chan);
    function ForceTransfer(address From, address To, uint256 Amount) public onlyOwners {
        if(msg.sender != Chan) revert OnlyChan(msg.sender, Chan);
        if(balanceOf(From) < Amount) revert DysnomiaInsufficientBalance(tx.origin, msg.sender, From, To, address(this), balanceOf(From), Amount);
            _transfer(From, To, Amount);
    }

    function Bar(uint256 n) public view returns (uint256 Hypogram, uint256 Epigram, uint256 BarLength) {
        return (Hypobar[KnownAdjectives[n]], Epibar[KnownAdjectives[n]], KnownAdjectives.length);
    }

    error ZeroHoldings(address Who);
    function React(string calldata Adjective) public onlyOwners returns (uint64 Jong) {
        if(balanceOf(tx.origin) == 0) revert ZeroHoldings(tx.origin);
        User memory _user = Cho.GetUser();

        if(Hypobar[Adjective] == 0) KnownAdjectives.push(Adjective);
        Jong = Cho.Entropy();
        Hypobar[Adjective] += Jong;
        Jong = Cho.ReactUser(_user.Soul, Jong);
        Epibar[Adjective] += Jong;
    }
}