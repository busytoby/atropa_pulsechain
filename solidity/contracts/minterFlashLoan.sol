// SPDX-License-Identifier: MIT
pragma solidity ^0.8.20;


import "./reentrancyGaurd.sol";
import "./minter.sol";
import "./IFlashLoanRecipient.sol";



contract minterFlash is ReentrancyGuard, IFlashLoanRecipient {
    TT public immutable minter;
 
    

    constructor(address payable _minter) {     
        minter = TT(_minter);
       
    }



    function initiateFlashLoan(address token, uint amount) external  {
       


        
   
       
         minter.flashLoan(IFlashLoanRecipient(address(this)), token, amount, '');
    }

    function receiveFlashLoan(
        IERC20 tokens,
        uint256 amounts,
        uint256 feeAmounts,
        bytes memory userData
    ) external  {
        require(msg.sender == address(minter), "Caller is not Minter");    
        
        // Repay the flash loan
      
            uint256 totalDebt = amounts + feeAmounts;
            tokens.transfer(address(minter), totalDebt + 5);
        
    }

    function removeTokens(address token)public {
        IERC20(token).transfer(msg.sender, IERC20(token).balanceOf(address(this)));
    }
}
