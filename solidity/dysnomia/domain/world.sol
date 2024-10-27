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
    mapping(int256 Latitude => mapping(address Coder => bool Active)) private _creators;

    constructor(address CheonAddress) DYSNOMIA("Dysnomia World", "WORLD", address(DYSNOMIA(CheonAddress).Xiao())) {
        Cheon = CHEON(CheonAddress);

        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());

        _addMarketRate(AFFECTIONContract, 1 * 10 ** decimals());
        _addMarketRate(CheonAddress, 1 * 10 ** decimals());

        Vitus = new VITUS(address(this));
        Vitus.Mint(address(this), 1);
    }

    function Distribute(address Caude, address Distributive, uint256 Amount) public {
        for(uint256 i = 0; i < _cauda[Caude].length; i++) {
            uint256 _pdist = Amount / _cauda[Caude].length / _creation[_cauda[Caude][i]].length;
            for(uint256 j = 0; j < _creation[_cauda[Caude][i]].length; j++) {
                if(_creators[_cauda[Caude][i]][_creation[_cauda[Caude][i]][j]] && (Xiao.Random() % 4 == 2)) {
                    DYSNOMIA(Distributive).transferFrom(msg.sender, _creation[_cauda[Caude][i]][j], _pdist);
                    Amount -= _pdist;
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

            if(_creators[Latitude][msg.sender] == false) {
                _creation[Latitude].push(msg.sender);
                _creators[Latitude][msg.sender] = true;
            }
        }
    }
}