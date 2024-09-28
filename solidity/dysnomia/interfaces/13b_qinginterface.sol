// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "./01b_dysnomia.sol";
import "./12b_chointerface.sol";
import "./libstrings.sol";
import "./libyai.sol";

interface QING {
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
    function Waat() external view returns (uint256);
    function BouncerDivisor() external view returns (uint16);
    function CoverCharge() external view returns (uint256);
    function NoCROWS() external view returns (bool);
    function VAT() external view returns (LIBYAI);
    function Withdraw(address what, uint256 amount) external;
    function GetUserTokenAddress(address wallet) external view returns(address UserToken);
    function OperatorSendMSG(string memory chatline) external;
    function AllowCROWS(bool _b) external;
    function setBouncerDivisor(uint16 _d) external;
    function setCoverCharge(uint256 _c) external;
    function setStaff(address _a, bool active) external;
    function setGuestlist(address _a) external;
    function Kick(uint64 Soul, string memory Why) external;
    function bouncer(address cBouncer) external view returns (bool);
    function Join(address UserToken) external;
    function Leave() external;
    function Alias(string memory name, address _qing) external;
    function Alias(string memory name) external view returns (address);
    function Connect(string memory name, address _qing) external;
    function Disconnect(string memory name) external;
    function Bounce() external;
    function Admitted(address UserToken) external view returns (bool);
    function YAISendMSG(address UserToken, uint256 Maat, string memory MSG) external;
    function ReactPlayer(uint64 Soul, uint64 Theta) external returns (uint64 Omicron);
}
