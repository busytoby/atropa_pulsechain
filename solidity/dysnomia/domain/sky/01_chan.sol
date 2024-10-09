// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "../interfaces/yueinterface.sol";

contract CHAN is DYSNOMIA {
    string public constant Type = "CHAN";

    XIE public Xie;

    mapping(address => address Yue) public Yan;
    mapping(address Holder => mapping(address Contract => bool MayTransfer)) private _optInList;

    constructor(address XieAddress) DYSNOMIA("Dysnomia Chan", "CHAN", address(DYSNOMIA(XieAddress).Xiao())) {
        Xie = XIE(XieAddress);
        addOwner(tx.origin);
        _mintToCap();

        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());

        _addMarketRate(AFFECTIONContract, 1 * 10 ** decimals());
        _addMarketRate(XieAddress, 1 * 10 ** decimals());
    }

    error AlreadyAdded(address Origin, address Yue, address New);
    function AddYue(address Origin, address Yue) public onlyOwners {
        if(Yan[Origin] != address(0x0)) revert AlreadyAdded(Origin, Yan[Origin], Yue);
        Yan[Origin] = Yue;
        _mintToCap();
    }

    error NotOrigin(address YueOrigin, address Requestor);
    function TransferYue(address Yue, address NewOrigin) public {
        if(Yan[NewOrigin] != address(0x0)) revert AlreadyAdded(YUEINTERFACE(Yan[NewOrigin]).Origin(), Yue, NewOrigin);
        YUEINTERFACE _chi = YUEINTERFACE(Yue);
        if(_chi.Origin() != tx.origin) revert NotOrigin(_chi.Origin(), tx.origin);
        Yan[_chi.Origin()] = address(0x0);
        _chi.ChangeOrigin(NewOrigin);
        Yan[NewOrigin] = Yue;
        _mintToCap();
    }

    function ReactYue(YUEINTERFACE Yue, address Qing) public onlyOwners returns(uint256 Charge) {
        return Yue.React(Qing);
    }

    event NewSpenderContractOptIn(address Origin, address Yue, address Contract, bool Allow);
    function OptIn(address Contract, bool Allow) public {
        address Yue = Yan[tx.origin];
        _optInList[Yue][Contract] = Allow;
        emit NewSpenderContractOptIn(tx.origin, Yue, Contract, Allow);
        _mintToCap();
    }

    function OptedIn(YUEINTERFACE Yue, address Contract) public view returns (bool) {
        return _optInList[address(Yue)][Contract];
    }

    error PlayerMustOptIn(address Player, address Yue, address Contract);
    function YueWithdraw(YUEINTERFACE Yue, address Asset, uint256 Amount) public onlyOwners {
        if(!_optInList[address(Yue)][msg.sender]) revert PlayerMustOptIn(Yue.Origin(), address(Yue), msg.sender);
        Yue.Withdraw(Asset, Amount);
        _mintToCap();
    }

    function YueMintToOrigin(YUEINTERFACE Yue) public onlyOwners {
        Yue.MintToOrigin();
        _mintToCap();
    }

    function YueForceTransfer(YUEINTERFACE Yue, address From, address To, uint256 Amount) public onlyOwners {
        if(!_optInList[address(Yue)][msg.sender]) revert PlayerMustOptIn(Yue.Origin(), address(Yue), msg.sender);
        Yue.ForceTransfer(From, To, Amount);
        _mintToCap();
    }
}