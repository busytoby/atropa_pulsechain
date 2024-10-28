// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../01_dysnomia_v2.sol";
import "./tang/interfaces/02b_cheoninterface.sol";
import "./assets/vitus.sol";

contract WORLD is DYSNOMIA {
    string public constant Type = "WORLD";

    CHEON public Cheon;
    VITUS public Vitus;

    mapping(int256 Latitude => mapping(int256 Longitude => mapping(address Caude => uint256 Bun))) private _world;
    mapping(address Caude => int256[] Latitudes) private _cauda;
    mapping(int256 Latitude => address[] Coders) private _creation;
    mapping(int256 Latitude => mapping(address Coder => mapping(address Caude => uint256 Buzz))) private _creators;
    mapping(address Caude => mapping(address Distributive => bool Allowed)) private _whitelist;

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

    function Whitelist(address Caude, address Distributive, bool Allow) public {
        QINGINTERFACE Asset = QINGINTERFACE(Caude);
        if((owner(msg.sender)) || (Asset.owner(msg.sender)))
            _whitelist[Caude][Distributive] = Allow;
    }

    function Bun(int256 Latitude, int256 Longitude, address Caude) public view returns (uint256) {
        return _world[Latitude][Longitude][Caude];
    }

    function Buzz(int256 Latitude, address Coder, address Caude) public view returns (uint256) {
        return _creators[Latitude][Coder][Caude];
    }

    function Distribute(address Caude, address Distributive, uint256 Amount) public returns (uint256 Remaining) {
        address[12] memory _l;
        address _c;
        uint256 _pdist;
        uint256 Charge;
        if(_whitelist[Caude][address(0x0)] == false)
            if(_whitelist[Caude][Distributive] == false) return Amount;
            
        for(uint256 i = 0; i < _cauda[Caude].length; i++) {
            _pdist = Amount / (_cauda[Caude].length - i) / _creation[_cauda[Caude][i]].length;
            for(uint256 j = 0; j < _creation[_cauda[Caude][i]].length; j++) {
                _c = _creation[_cauda[Caude][i]][j];
                if(_creators[_cauda[Caude][i]][_c][Caude] == 0) continue;
                for(uint256 k = 0; k < _l.length; k++) {
                    if(_l[k] == _c) _creators[_cauda[Caude][i]][_c][Caude] /= 2;
                    if(k < _l.length - 1) _l[k] = _l[k+1];
                    else _l[k] = _c;
                }
                Charge = _creators[_cauda[Caude][i]][_c][Caude];
                if(Charge == 0) continue;
                if(Charge >= _pdist)
                    Charge = Charge % _pdist;
                DYSNOMIA(Distributive).transferFrom(msg.sender, _c, Charge);
                unchecked {
                    Amount -= Charge;
                    _creators[_cauda[Caude][i]][_c][Caude] -= Charge;
                }
            }
            if(Amount <= 1111111111) return Amount;
        }
        return Amount;
    }

    function Code(int256 Latitude, int256 Longitude, address Cause) public {
        (YUEINTERFACE Chi, ) = Cheon.Sei().Chi();
        (uint256 Charge, uint256 Hypobar, uint256 Epibar) = Cheon.Su(Cause);
        if(Charge == 0) return;
        if(Charge >= _world[Latitude][Longitude][Cause]) {
            _world[Latitude][Longitude][Cause] += Hypobar;
            Vitus.Mint(address(Chi), Epibar);

            if(_creators[Latitude][address(Chi)][Cause] == 0)
                _creation[Latitude].push(address(Chi));
            _creators[Latitude][address(Chi)][Cause] += Charge;
        }
    }
}