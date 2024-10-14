// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./dan/03_qing.sol";
import "../interfaces/heckeinterface.sol";

contract MAP is DYSNOMIA {
    CHOINTERFACE public Cho;
    HECKE public Map;
    mapping(address Asset => address Qing) private _qings;
    mapping(uint256 Waat => QING) private _waats;
    mapping(address Asset => bool Forbidden) private _forbidden;
    mapping(int256 Latitude => mapping(int256 Longitude => mapping(int256 Altitude => address Qing))) private _map;

    constructor(address ChoAddress, address HeckeAddress) DYSNOMIA("Dysnomia Map", "MAP", address(DYSNOMIA(ChoAddress).Xiao())) {
        Cho = CHOINTERFACE(ChoAddress);
        Map = HECKE(HeckeAddress);
        Cho.addOwner(address(this));
        Cho.Void().addOwner(address(this));

        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());

        _addMarketRate(AFFECTIONContract, 1 * 10 ** decimals());
        _addMarketRate(ChoAddress, 1 * 10 ** decimals());
    }

    function addToMap(uint256 Waat, address Qing) internal {
        (int256 Longitude, int256 Latitude) = Map.Compliment(Waat);
        assert(_map[Latitude][Longitude][0] == address(0x0));
        _map[Latitude][Longitude][0] = Qing;
    }

    function GetMapQing(int256 Latitude, int256 Longitude, int256 Altitude) public view returns (QINGINTERFACE) {
        return QINGINTERFACE(_map[Latitude][Longitude][Altitude]);
    }

    function hasOwner(address _contract) public view returns (bool does) {
        bytes4 selector = bytes4(keccak256(bytes("owner(address)")));
        bytes memory data = abi.encodeWithSelector(selector, address(0x0));
        assembly { does := staticcall(gas(), _contract, add(data, 32), mload(data), 0, 0) }
    }

    function has(address _contract, string memory what) public view returns (bool does) {
        bytes4 selector = bytes4(keccak256(bytes(what)));
        bytes memory data = abi.encodeWithSelector(selector);
        assembly { does := staticcall(gas(), _contract, add(data, 32), mload(data), 0, 0) }
    }

    function Forbidden(address Asset) public view returns (bool) {
        return _forbidden[Asset];
    }

    error NotOwner(address what, address who);
    function Forbid(address Token, bool Disallow) public {
        if(has(Token, "owner()") && DYSNOMIA(Token).owner() == msg.sender) _forbidden[Token] = Disallow;
        else if(DYSNOMIA(Token).owner(msg.sender)) _forbidden[Token] = Disallow;
        else revert NotOwner(Token, msg.sender);
    }

    function GetQing(uint256 Waat) public view returns (QING) {
        return _waats[Waat];
    }

    error QingAlreadyExists(address Token, address Qing);
    error ForbiddenToken(address which);
    event NewQing(address Qing, address Integrative, uint256 Waat);
    function New(address Integrative) public returns(QING Mu) {
        uint256 Luo;
        if(_qings[Integrative] != address(0x0)) revert QingAlreadyExists(Integrative, _qings[Integrative]);
        if(_forbidden[Integrative]) revert ForbiddenToken(Integrative);
        DYSNOMIA Asset = DYSNOMIA(Integrative);
        Luo = Cho.Luo();
        Mu = new QING(Luo, Integrative, address(Cho));
        if(has(Integrative, "owner()")) Mu.addOwner(Asset.owner());
        if(has(Integrative, "name()") && has(Integrative, "symbol()")) Mu.Rename(string.concat(Asset.name(), " QING"), string.concat("q", Asset.symbol()));
        Cho.addOwner(address(Mu));
        Mu.addOwner(address(Cho));
        Mu.renounceOwnership(address(this));
        _qings[Integrative] = address(Mu);
        _waats[Luo] = Mu;
        addToMap(Luo, address(Mu));
        _addMarketRate(address(Mu), 1 * 10 ** decimals());
        emit NewQing(address(Mu), Integrative, Luo);
    }
}