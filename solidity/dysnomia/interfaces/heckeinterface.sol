// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;

interface HECKE {
    function Compliment(uint256 Waat) external view returns (int256 Longitude, int256 Latitude);
    function GetMeridian(uint256 Waat) external view returns (uint256 Meridian);
}
