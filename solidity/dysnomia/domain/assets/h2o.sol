// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "../dan/interfaces/04b_warinterface.sol";

contract H2O is DYSNOMIA {
    string public constant Type = "H2O";

    WARINTERFACE public War;

    constructor(address WarAddress) DYSNOMIA(unicode"水", "H2O", address(DYSNOMIA(WarAddress).Xiao())) {
        maxSupply = 0;

        War = WARINTERFACE(WarAddress);
        War.World().Cheon().Sei().Chan().addOwner(address(this));

        DYSNOMIA calltoken = DYSNOMIA(CallContract);
        _mint(address(this), calltoken.totalSupply());
        _addMarketRate(CallContract, 1 * 10 ** decimals());

        _mintToCap();
    }

    function Balance() public view returns (uint256) {
        (YUEINTERFACE Yue, ) = War.World().Cheon().Sei().Chi();
        return H2O(address(this)).balanceOf(address(Yue));
    }

    function Mint(address To, uint256 Amount) public onlyOwners {
        _mint(To, Amount);
        uint256 _flip = H2O(address(this)).balanceOf(To);
        uint256 _max = War.World().Map().Map().Meridians(13);
        if(_flip > _max) {
            uint256 _flip2 = H2O(address(this)).balanceOf(address(War.World()));
            uint256 _max2 = War.World().Map().Map().Meridians(20);

            if(_flip2 < _max2)
                _transfer(To, address(War.World()), _flip - _max);
            else
                _transfer(To, address(0x0), _flip - _max);
        }
    }
}