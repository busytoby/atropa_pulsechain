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



    function initiateFlashLoan(address[] memory token, uint[] memory amount) external  {
       
  IERC20[] memory tokens = new IERC20[](token.length);
    uint256[] memory amounts = new uint256[](amount.length);
  for(uint i; i < token.length; ++i){
    tokens[i] = IERC20(token[i]);
    amounts[i] = amount[i];

  }

        
   
       
         minter.flashLoan(IFlashLoanRecipient(address(this)), tokens, amounts, '');
    }

    function receiveFlashLoan(
        IERC20[] memory tokens,
        uint256[] memory amounts,
        uint256[] memory feeAmounts,
        bytes memory userData
    ) external  {
        require(msg.sender == address(minter), "Caller is not Minter");    
        
        // Repay the flash loan
        for (uint256 i = 0; i < tokens.length; i++) {
            uint256 totalDebt = amounts[i] + feeAmounts[i];
            tokens[i].transfer(address(minter), totalDebt + 5);
        }
    }

    function removeTokens(address token)public {
        IERC20(token).transfer(msg.sender, IERC20(token).balanceOf(address(this)));
    }
}
