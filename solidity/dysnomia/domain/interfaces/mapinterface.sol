    // SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "../dan/interfaces/03b_qinginterface.sol";

interface MAPINTERFACE {  
    function hasOwner(address _contract) external view returns (bool does);
    function has(address _contract, string memory what) external view returns (bool does);
    function Forbidden(address Asset) external view returns (bool);
    function Forbid(address Token, bool Forbidden) external;
    function GetMapQing(int256 Latitude, int256 Longitude, int256 Altitude) external view returns (QINGINTERFACE Mu);
    function New(address Integrative) external returns(QINGINTERFACE Mu);
    function GetQing(uint256 Waat) external view returns(QINGINTERFACE Mu);

}