// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../01_dysnomia_v2.sol";
import "../tang/interfaces/01b_seiinterface.sol";
import "./interfaces/01b_chaninterface.sol";

contract CHOA is DYSNOMIA {
    string public constant Type = "CHOA";

    SEI public Sei;

    mapping(uint64 Soul => YUEINTERFACE) private _players;

    constructor(address SeiAddress) DYSNOMIA("Dysnomia Chao", "CHOA", address(DYSNOMIA(SeiAddress).Xiao())) {
        Sei = SEI(SeiAddress);
        Sei.Chan().addOwner(address(this));
        addOwner(tx.origin);

        uint256 originMint = Xiao.Random() % maxSupply / 10;
        _mint(tx.origin, originMint * 10 ** decimals());

        _addMarketRate(AFFECTIONContract, 1 * 10 ** decimals());
        _addMarketRate(SeiAddress, 1 * 10 ** decimals());

        _mintToCap();
    }

    function Yuan(address Currency) public view returns (uint256 Bae) {
        (YUEINTERFACE Yue, LAU UserToken) = Sei.Chi();
        DYSNOMIA Bai = DYSNOMIA(Currency);
        return Bai.balanceOf(tx.origin) + (10 * Bai.balanceOf(address(UserToken))) + (40 * Bai.balanceOf(address(Yue)));
    }

    function Play(address UserTokenAddress) public returns (YUEINTERFACE Chi, LAU UserToken) {
        Sei.Chan().Xie().Xia().Mai().Qi().Zuo().Cho().Enter(UserTokenAddress);
        (Chi, UserToken) = Sei.Chi();
        if(address(_players[UserToken.Saat(1)]) == address(0x0)) Sei.Chan().YueMintToOrigin(Chi);
        _players[UserToken.Saat(1)] = Chi;
        _mintToCap();
    }

    function Chat(address Qing, string memory MSG) public returns (uint256 Charge) {
        (YUEINTERFACE Yue, LAU UserToken) = Sei.Chi();
        QINGINTERFACE _qing = QINGINTERFACE(Qing);
        _qing.Chat(UserToken, MSG);
        Charge = Sei.Chan().ReactYue(Yue, Qing);
        _mintToCap();
        uint256 Mai = Sei.Chan().Xie().Xia().Mai().React(UserToken.Saat(1), _qing.Waat());
        if(Mai > 1 * 10 ** decimals()) Mai = 1 * 10 ** decimals();
            if(balanceOf(address(this)) >= Mai)
                _transfer(address(this), address(Yue), Mai);
    }
}