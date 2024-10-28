// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "../interfaces/worldinterface.sol";
import "../dan/03_qing.sol";
import "../dan/interfaces/04b_warinterface.sol";

contract GWAT is DYSNOMIA {
    string public constant Type = "GWAT";

    WARINTERFACE public War;

    mapping(address Asset => address Qing) private _qings;
    mapping(uint256 Waat => QING) private _waats;
    mapping(int256 Latitude => mapping(int256 Longitude => address Qing)) private _map;

    constructor(address WarAddress) DYSNOMIA(unicode"骨", "GWAT", address(DYSNOMIA(WarAddress).Xiao())) {
        War = WARINTERFACE(WarAddress);

        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());

        _addMarketRate(AFFECTIONContract, 1 * 10 ** decimals());
        _addMarketRate(WarAddress, 1 * 10 ** decimals());
    }

    function addToMap(uint256 Waat, address Qing) internal {
        (int256 Longitude, int256 Latitude) = War.World().Map().Map().Compliment(Waat);
        assert(_map[Latitude][Longitude] == address(0x0));
        _map[Latitude][Longitude] = Qing;
    }

    function GetMapGwat(int256 Latitude, int256 Longitude) public view returns (QINGINTERFACE) {
        return QINGINTERFACE(_map[Latitude][Longitude]);
    }

    error QingAlreadyExists(address Token, address Qing);
    error ForbiddenToken(address which);
    error SpawningFailure(uint256 Lin);
    event NewQing(address Qing, address Integrative, uint256 Waat);
    function Gwat(address Qing, uint256 Lin) public returns (QING Mu) {
        if(_qings[Qing] != address(0x0)) revert QingAlreadyExists(Qing, _qings[Qing]);
        QING Integrative = QING(Qing);
        assert(Qing == address(War.World().Map().GetQing(Integrative.Waat())));
        if(War.World().Map().Forbidden(address(Integrative.Asset()))) revert ForbiddenToken(address(Integrative.Asset()));
        uint256 Luo = War.Faa(Qing, Lin);
        if(Luo % War.World().Map().Offset() == 0) revert SpawningFailure(Lin);
        Mu = new QING(Luo, Qing, address(War.World().Map().Cho()));
        if(Integrative.owner(msg.sender)) Mu.addOwner(msg.sender);
        Mu.Rename(string.concat(Integrative.Asset().name(), " GWAT"), string.concat(unicode"骨", Integrative.Asset().symbol()));
        War.World().Map().Cho().addOwner(address(Mu));
        Mu.addOwner(address(War.World().Map().Cho()));
        Mu.renounceOwnership(address(this));
        _qings[Qing] = address(Mu);
        _waats[Luo] = Mu;
        addToMap(Luo, address(Mu));
        _addMarketRate(address(Mu), 1 * 10 ** decimals());
        emit NewQing(address(Mu), Qing, Luo);
    }
}