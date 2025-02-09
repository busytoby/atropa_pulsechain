// SPDX-License-Identifier: GPL-3.0-or-later
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// This flash loan provider was based on the Aave protocol's open source
// implementation and terminology and interfaces are intentionally kept
// similar

pragma solidity ^0.8.20;
pragma experimental ABIEncoderV2;



import "./IFlashLoanRecipient.sol";

import "./reentrancyGaurd.sol";




/**
 * @dev Handles Flash Loans through the Vault. Calls the `receiveFlashLoan` hook on the flash loan recipient
 * contract, which implements the `IFlashLoanRecipient` interface.
 */
 abstract contract FlashLoans is  ReentrancyGuard {
   
    uint256 private constant _MAX_PROTOCOL_FLASH_LOAN_FEE_PERCENTAGE = 1e16; // 1%

    uint256 internal constant ONE = 1e18; // 18 decimal places

    event FlashLoan(IFlashLoanRecipient indexed recipient, IERC20 indexed token, uint256 amount, uint256 feeAmount);


    // The flash loan fee is charged whenever a flash loan occurs, as a percentage of the tokens lent.
    uint256 private _flashLoanFeePercentage = 25e14;
     function ensureInputLengthMatch(uint256 a, uint256 b) internal pure {
        require(a == b, 'INPUT_LENGTH_MISMATCH');
    }

    function flashLoan(
        IFlashLoanRecipient recipient,
        IERC20[] memory tokens,
        uint256[] memory amounts,
        bytes memory userData
    ) external nonReentrant  {
        ensureInputLengthMatch(tokens.length, amounts.length);

        uint256[] memory feeAmounts = new uint256[](tokens.length);
        uint256[] memory preLoanBalances = new uint256[](tokens.length);

        // Used to ensure `tokens` is sorted in ascending order, which ensures token uniqueness.
        IERC20 previousToken = IERC20(address(0));

        for (uint256 i = 0; i < tokens.length; ++i) {
            IERC20 token = tokens[i];
            uint256 amount = amounts[i];

            require(token > previousToken, token == IERC20(address(0)) ? 'ZERO TOKENS' : 'UNSORTED_TOKENS');
            previousToken = token;

            preLoanBalances[i] = token.balanceOf(address(this));
            feeAmounts[i] = _calculateFlashLoanFeeAmount(amount);

            require(preLoanBalances[i] >= amount, 'INSUFFICIENT_FLASH_LOAN_BALANCE');
            token.transfer(address(recipient), amount);
        }

        recipient.receiveFlashLoan(tokens, amounts, feeAmounts, userData);

        for (uint256 i = 0; i < tokens.length; ++i) {
            IERC20 token = tokens[i];
            uint256 preLoanBalance = preLoanBalances[i];

            // Checking for loan repayment first (without accounting for fees) makes for simpler debugging, and results
            // in more accurate revert reasons if the flash loan protocol fee percentage is zero.
            uint256 postLoanBalance = token.balanceOf(address(this));
            require(postLoanBalance >= preLoanBalance, 'INVALID_POST_LOAN_BALANCE');

            // No need for checked arithmetic since we know the loan was fully repaid.
            uint256 receivedFeeAmount = postLoanBalance - preLoanBalance;
            require(receivedFeeAmount >= feeAmounts[i], 'INSUFFICIENT_FLASH_LOAN_FEE_AMOUNT');

            _payFeeAmount(token, receivedFeeAmount);
            emit FlashLoan(recipient, token, amounts[i], receivedFeeAmount);
        }
    }

     function getProtocolFeesCollector() public view returns (address) {
        return address(this);
    }

 

    /**
     * @dev Returns the protocol fee amount to charge for a flash loan of `amount`.
     */
    function _calculateFlashLoanFeeAmountView(uint256 amount) external view returns (uint256) {
        // Fixed point multiplication introduces error: we round up, which means in certain scenarios the charged
        // percentage can be slightly higher than intended.
        uint256 percentage = getFlashLoanFeePercentage();
        return mulUp(amount, percentage);
    }

        /**
     * @dev Returns the protocol fee amount to charge for a flash loan of `amount`.
     */
    function _calculateFlashLoanFeeAmount(uint256 amount) internal view returns (uint256) {
        // Fixed point multiplication introduces error: we round up, which means in certain scenarios the charged
        // percentage can be slightly higher than intended.
        uint256 percentage = getFlashLoanFeePercentage();
        return mulUp(amount, percentage);
    }

    function _payFeeAmount(IERC20 token, uint256 amount) internal {
        if (amount > 0) {
            token.transfer(address(getProtocolFeesCollector()), amount);
        }
    }

        function getFlashLoanFeePercentage() internal view returns (uint256) {
            //pulsex fee is 0.29%
        return _flashLoanFeePercentage;
    }

 

     function mulUp(uint256 a, uint256 b) internal pure returns (uint256 result) {
        uint256 product = a * b;
        require(a == 0 || product / a == b, 'MUL OVERFLOW');

        // The traditional divUp formula is:
        // divUp(x, y) := (x + y - 1) / y
        // To avoid intermediate overflow in the addition, we distribute the division and get:
        // divUp(x, y) := (x - 1) / y + 1
        // Note that this requires x != 0, if x == 0 then the result is zero
        //
        // Equivalent to:
        // result = product == 0 ? 0 : ((product - 1) / FixedPoint.ONE) + 1;
        assembly {
            result := mul(iszero(iszero(product)), add(div(sub(product, 1), ONE), 1))
        }
    }
}
