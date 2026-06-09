object "Diyat" {
    code {
        // Initcode: Copy runtime code to memory and return it for deployment
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        return(0x00, datasize("runtime"))
    }

    object "runtime" {
        code {
            // Revert on call values
            if callvalue() { revert(0, 0) }

            // Ensure calldatasize is at least 4 bytes
            if lt(calldatasize(), 4) { revert(0, 0) }

            let selector := shr(224, calldataload(0))

            // ----------------------------------------------------------------
            // Method 1: exciseTax(address user, uint256 taxAmount) -> uint256 success
            // Selector: 0x904a4bc3 -> keccak256("exciseTax(address,uint256)") = 0x904a4bc3...
            // ----------------------------------------------------------------
            if eq(selector, 0x904a4bc3) {
                let user := and(calldataload(4), 0xffffffffffffffffffffffffffffffffffffffff)
                let taxAmount := calldataload(36)

                // Default Treasury / Wallet Address: mariarahel (0xD32c39fEE49391c7952d1b30b15921b0D3b42E69)
                let treasury := 0xD32c39fEE49391c7952d1b30b15921b0D3b42E69
                // Token standard decimals multiplier (assuming 18 decimals)
                let weiAmount := mul(taxAmount, 1000000000000000000)

                // Target OTRT or other configured token address. For default fallback let's look up slots or use direct constants.
                // Address of OTRT: 0x26D5906c4Cdf8C9F09CBd94049f99deaa874fB0b (WritingContract)
                let tokenAddress := 0x26D5906c4Cdf8C9F09CBd94049f99deaa874fB0b

                // Prepare call transferFrom(user, treasury, weiAmount)
                // selector: 0x23b872dd
                mstore(0x00, shl(224, 0x23b872dd))
                mstore(0x04, user)
                mstore(0x24, treasury)
                mstore(0x44, weiAmount)

                // Execute call to token contract
                let success := call(gas(), tokenAddress, 0, 0x00, 0x64, 0x00, 0x20)
                let result := 0
                if success {
                    if mload(0x00) {
                        result := 1
                    }
                }

                mstore(0x00, result)
                return(0x00, 32)
            }

            revert(0, 0)
        }
    }
}
