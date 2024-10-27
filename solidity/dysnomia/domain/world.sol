// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../01_dysnomia_v2.sol";
import "./tang/interfaces/02b_cheoninterface.sol";
import "./interfaces/mapinterface.sol";
import "./assets/vitus.sol";

contract WORLD is DYSNOMIA {
    string public constant Type = "WORLD";

    CHEON public Cheon;
    VITUS public Vitus;

    mapping(int256 Latitude => mapping(int256 Longitude => mapping(address Caude => uint256 Bun))) private _world;
    mapping(address Caude => int256[] Latitudes) private _cauda;
    mapping(int256 Latitude => address[] Coders) private _creation;
    mapping(int256 Latitude => mapping(address Coder => uint256 Buzz)) private _creators;

    constructor(address CheonAddress) DYSNOMIA("Dysnomia World", "WORLD", address(DYSNOMIA(CheonAddress).Xiao())) {
        Cheon = CHEON(CheonAddress);
        Cheon.addOwner(address(this));

        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());

        _addMarketRate(AFFECTIONContract, 1 * 10 ** decimals());
        _addMarketRate(CheonAddress, 1 * 10 ** decimals());

        Vitus = new VITUS(address(this));
        Vitus.Mint(address(this), 1);
    }

    function Bun(int256 Latitude, int256 Longitude, address Caude) public view returns (uint256) {
        return _world[Latitude][Longitude][Caude];
    }

    function Distribute(address Caude, address Distributive, uint256 Amount) public {
        for(uint256 i = 0; i < _cauda[Caude].length; i++) {
            uint256 _pdist = Amount / (_cauda[Caude].length - i) / _creation[_cauda[Caude][i]].length;
            for(uint256 j = 0; j < _creation[_cauda[Caude][i]].length; j++) {
                uint256 Charge = _creators[_cauda[Caude][i]][_creation[_cauda[Caude][i]][j]];
                if(Charge >= _pdist) {
                    DYSNOMIA(Distributive).transferFrom(msg.sender, _creation[_cauda[Caude][i]][j], _pdist);
                    Amount -= _pdist;
                } else {
                    DYSNOMIA(Distributive).transferFrom(msg.sender, _creation[_cauda[Caude][i]][j], _pdist);
                    Amount -= Charge;
                }
            }
        }
    }

    function Code(int256 Latitude, int256 Longitude, address Cause) public {
        (YUEINTERFACE Chi, ) = Cheon.Sei().Chi();
        (uint256 Charge, uint256 Hypobar, uint256 Epibar) = Cheon.Su(Cause);
        if(Charge >= _world[Latitude][Longitude][Cause]) {
            _world[Latitude][Longitude][Cause] += Hypobar;
            Vitus.Mint(address(Chi), Epibar);

            if(_creators[Latitude][address(Chi)] == 0)
                _creation[Latitude].push(address(Chi));
            _creators[Latitude][address(Chi)] += Charge;
        }
    }
}