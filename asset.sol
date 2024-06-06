// SPDX-License-Identifier: Sharia
pragma solidity ^0.8.21;
import "@openzeppelin/contracts/token/ERC20/IERC20.sol";

interface PLSXLP is IERC20 {
    function sync() external;
    function token0() external view returns(address);
    function token1() external view returns(address);
}

abstract contract Asset {
     function Sync(address LPA) public returns (bool) {
        PLSXLP LPContract = PLSXLP(LPA);

        try LPContract.sync() {
            return true;
        } catch {
            return false;
        }
    }

/*
    function IsPLP(address payee) public view returns (bool) {
        PLSXLP LPContract = PLSXLP(payee);
        try LPContract.token0() {
            if(LPContract.token0() == address(this)) return true;
        } catch { return false; }
        try LPContract.token1() {
            if(LPContract.token1() == address(this)) return true;
        } catch { return false; }
        return false;
    }
*/
}
