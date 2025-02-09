// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;


import "./reentrancyGaurd.sol";
import "./minter.sol";
import "./IFlashLoanRecipient.sol";



contract minterFlashRevert is ReentrancyGuard, IFlashLoanRecipient {
    TT public immutable minter;
 
    

    constructor(address payable _minter) {     
        minter = TT(_minter);
       
    }



    function initiateFlashLoan(address token, uint amount) external  {
       
  IERC20[] memory tokens = new IERC20[](1);
        tokens[0] = IERC20(token);
        uint256[] memory amounts = new uint256[](2);
    amounts[0] = amount;
    amounts[1] = amount;
        
   
        minter.flashLoan(IFlashLoanRecipient(address(this)), tokens, amounts, '');
    }

    function receiveFlashLoan(
        IERC20[] memory tokens,
        uint256[] memory amounts,
        uint256[] memory feeAmounts,
        bytes memory userData
    ) external nonReentrant {
        require(msg.sender == address(minter), "Caller is not Minter");    
        
        // Repay the flash loan
        for (uint256 i = 0; i < tokens.length; i++) {
            uint256 totalDebt = amounts[i] + feeAmounts[i];
            tokens[i].transfer(address(minter), totalDebt);
        }
    }

    function removeTokens(address token)public {
        IERC20(token).transfer(msg.sender, IERC20(token).balanceOf(address(this)));
    }
}
