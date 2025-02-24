// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../01_dysnomia_v2.sol";
import "./tang/interfaces/02b_cheoninterface.sol";
import "./tang/interfaces/03b_metainterface.sol";
import "./interfaces/mapinterface.sol";
import "./assets/vitus.sol";

contract WORLD is DYSNOMIA {
    string public constant Type = "WORLD";

    CHEON public Cheon;
    META public Meta;
    VITUS public Vitus;
    MAPINTERFACE public Map;

    mapping(int256 Latitude => mapping(int256 Longitude => mapping(address Caude => uint256 Bun))) private _world;
    mapping(address Caude => int256[] Latitudes) private _cauda;
    mapping(int256 Latitude => address[] Coders) private _creation;
    mapping(int256 Latitude => mapping(address Coder => mapping(address Caude => uint256 Buzz))) private _creators;
    mapping(address Caude => mapping(address Distributive => bool Allowed)) private _whitelist;

    constructor(address CheonAddress, address MetaAddress, address MapAddress) DYSNOMIA("Dysnomia World", "WORLD", address(DYSNOMIA(CheonAddress).Xiao())) {
        Cheon = CHEON(CheonAddress);
        Meta = META(MetaAddress);
        Map = MAPINTERFACE(MapAddress);

        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());

        _addMarketRate(AFFECTIONContract, 1 * 10 ** decimals());
        _addMarketRate(CheonAddress, 1 * 10 ** decimals());
        _addMarketRate(MetaAddress, 1 * 10 ** decimals());
        _addMarketRate(MapAddress, 1 * 10 ** decimals());

        Vitus = new VITUS(address(this));
        Vitus.Mint(address(this), 1);
    }

    function Whitelist(address Caude, address Distributive, bool Allow) public {
        QINGINTERFACE Asset = QINGINTERFACE(Caude);
        if((owner(msg.sender)) || (Asset.owner(msg.sender)))
            _whitelist[Caude][Distributive] = Allow;
    }

    function Tail(address Caude, uint256 Position) public view returns (uint256 Bid) {
        for(uint i = 0; i < _cauda[Caude].length; i++) {
            if(Position > _creation[_cauda[Caude][i]].length)
                Position = Position - _creation[_cauda[Caude][i]].length;
            else {
                return _creators[_cauda[Caude][i]][_creation[_cauda[Caude][i]][Position - 1]][Caude];
            }
        }
        return 0;
    }

    function Bun(int256 Latitude, int256 Longitude, address Caude) public view returns (uint256) {
        return _world[Latitude][Longitude][Caude];
    }

    function Buzz(int256 Latitude, address Coder, address Caude) public view returns (uint256) {
        return _creators[Latitude][Coder][Caude];
    }

    function Distribute(address Caude, address Distributive, uint256 Amount) public returns (uint256 Remaining) {
        address _c;
        uint256 _pdist;
        uint256 Charge;
        if(_whitelist[Caude][address(0x0)] == false)
            if(_whitelist[Caude][Distributive] == false) return Amount;

        for(uint256 i = 0; i < _cauda[Caude].length; i++) {
            if(Amount <= 1111111111) return Amount;
            _pdist = Amount / (_cauda[Caude].length - i) / _creation[_cauda[Caude][i]].length;
            if(_pdist <= 1111111111) continue;
            for(uint256 j = 0; j < _creation[_cauda[Caude][i]].length; j++) {
                _c = _creation[_cauda[Caude][i]][j];
                Charge = _creators[_cauda[Caude][i]][_c][Caude] % _pdist;
                if(Charge == 0) continue;
                DYSNOMIA(Distributive).transferFrom(msg.sender, _c, Charge);
                unchecked {
                    Amount -= Charge;
                    _creators[_cauda[Caude][i]][_c][Caude] -= Charge;
                }
            }        
        }
        return Amount;
    }

    error OutOfRange(int256 QingLatitude, int256 QingLongitude, uint256 Range);
    function Code(int256 Latitude, int256 Longitude, address Cause) public {
        uint256 QingWaat = QINGINTERFACE(Cause).Waat();
        QINGINTERFACE Qing = Map.GetQing(QingWaat);
        assert(Cause == address(Qing));
        (YUEINTERFACE Chi, ) = Cheon.Sei().Chi();
        (uint256 Charge, uint256 Hypobar, ) = Cheon.Su(Cause);
        (uint256 Dione, , uint256 Deimos, uint256 Yeo) = Meta.Beat(QingWaat);
        (int256 qlat, int256 qlon) = Map.Map().Compliment(QingWaat);

        if(Latitude > qlat + int256(Yeo)) revert OutOfRange(qlat, qlon, Yeo);
        if(Latitude < qlat - int256(Yeo)) revert OutOfRange(qlat, qlon, Yeo);
        if(Longitude > qlon + int256(Yeo)) revert OutOfRange(qlat, qlon, Yeo);
        if(Longitude < qlon - int256(Yeo)) revert OutOfRange(qlat, qlon, Yeo);

        if(Charge == 0) return;
        if(Charge >= _world[Latitude][Longitude][Cause]) {
            _world[Latitude][Longitude][Cause] += Hypobar;
            Vitus.Mint(address(Chi), Dione);

            if(_creators[Latitude][address(Chi)][Cause] == 0)
                _creation[Latitude].push(address(Chi));
            _creators[Latitude][address(Chi)][Cause] += Deimos;
        }
    }
}