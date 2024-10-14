// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "..//yue.sol";
import "../sky/interfaces/01b_chaninterface.sol";

contract SEI is DYSNOMIA {
    string public constant Type = "SEI";

    CHAN public Chan;

    constructor(address ChanAddress) DYSNOMIA("Dysnomia Sei", "SEI", address(DYSNOMIA(ChanAddress).Xiao())) {
        Chan = CHAN(ChanAddress);
        Chan.addOwner(address(this));
        Chan.Xie().Xia().Mai().Qi().Zuo().Cho().addOwner(address(this));
        addOwner(tx.origin);

        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());

        _addMarketRate(AFFECTIONContract, 1 * 10 ** decimals());
        _addMarketRate(ChanAddress, 1 * 10 ** decimals());

        _mintToCap();
    }

    error NotStarted(address);
    function Chi() public view returns (YUE Yue, LAU UserToken) {        
        if(Chan.Yan(tx.origin) == address(0x0)) revert NotStarted(tx.origin);
        Yue = YUE(Chan.Yan(tx.origin));
        UserToken = LAU(Chan.Xie().Xia().Mai().Qi().Zuo().Cho().GetUserTokenAddress(tx.origin));
    }

    function Start(address LauToken, string calldata YueName, string calldata YueSymbol) public returns (YUE Yue, LAU UserToken) {
        _mintToCap();
        assert((DYSNOMIA(WITHOUTContract).balanceOf(tx.origin) == 0));
        if(Chan.Yan(tx.origin) == address(0x0)) {
            Chan.Xie().Xia().Mai().Qi().Zuo().Cho().Enter(LauToken);
            UserToken = LAU(LauToken);
            Yue = new YUE(YueName, YueSymbol, address(Chan));
            Yue.addOwner(address(Chan));
            Chan.AddYue(tx.origin, address(Yue));
            return (Yue, UserToken);
        }

        (Yue, UserToken) = Chi();
        Yue.Rename(YueName, YueSymbol);
    }
}