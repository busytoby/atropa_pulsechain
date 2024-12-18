// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../../../interfaces/01b_dysnomia.sol";
import "./01b_chointerface.sol";
import "../../../interfaces/libstrings.sol";
import "../../interfaces/mapinterface.sol";

interface QINGINTERFACE {
    function maxSupply() external view returns(uint256);
    function Rename(string memory newName, string memory newSymbol) external;
    function GetMarketRate(address _a) external view returns(uint256);
    function Purchase(address _t, uint256 _a) external;
    function Redeem(address _t, uint256 _a) external;
    function name() external view returns (string memory);
    function symbol() external view returns (string memory);
    function decimals() external view returns (uint8);
    function totalSupply() external view returns (uint256);
    function balanceOf(address account) external view returns (uint256);
    function transfer(address to, uint256 value) external returns (bool);
    function allowance(address owner, address spender) external view returns (uint256);
    function approve(address spender, uint256 value) external returns (bool);
    function transferFrom(address from, address to, uint256 value) external returns (bool);
    function addOwner(address newOwner) external;
    function renounceOwnership(address toRemove) external;
    function owner(address cOwner) external view returns (bool);
    function mintToCap() external;
    function Type() external view returns (string memory);
    function Cho() external view returns (CHOINTERFACE);
    function Asset() external view returns (DYSNOMIAINTERFACE);
    function Factory() external view returns (MAPINTERFACE);
    function Waat() external view returns (uint256);
    function Entropy() external view returns (uint64);
    function BouncerDivisor() external view returns (uint16);
    function CoverCharge() external view returns (uint256);
    function NoCROWS() external view returns (bool);
    function GWAT() external view returns (bool);
    function GetQing(uint256 _waat) external view returns (QINGINTERFACE);
    function Withdraw(address what, uint256 amount) external;
    function AllowCROWS(bool _b) external;
    function setBouncerDivisor(uint16 _d) external;
    function setCoverCharge(uint256 _c) external;
    function setStaff(address _a, bool active) external;
    function setGuestlist(address _a) external;
    function removeGuest(address _a) external;
    function bouncer(address cBouncer) external view returns (bool);
    function Join(address UserToken) external;
    function Admitted(address UserToken) external view returns (bool);
    function Chat(LAU UserToken, string memory MSG) external;
}
