object "UnificationEngine" {
    code {
        datacopy(0, dataoffset("runtime"), datasize("runtime"))
        return(0, datasize("runtime"))
    }
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))
            
            // unify_terms(bytes32 term_a, bytes32 term_b) -> returns (uint256 success)
            // Selector: 0x76b5a37a
            if iszero(sub(selector, 0x76b5a37a)) {
                let term_a := calldataload(4)
                let term_b := calldataload(36)
                
                let success := unify(term_a, term_b)
                
                mstore(0, success)
                return(0, 32)
            }
            
            // Helper function to resolve variable bindings (dereference)
            function deref(term) -> resolved {
                resolved := term
                // A variable is represented as a pointer to a storage slot.
                // If the storage slot holds a non-zero value, it is bound.
                for {} is_var_cell(resolved) {} {
                    let binding := sload(resolved)
                    if iszero(binding) { break }
                    if eq(binding, resolved) { break } // Self-reference loop safety
                    resolved := binding
                }
            }
            
            // Helper to check if a 256-bit hash represents a variable slot
            function is_var_cell(term) -> res {
                // By convention, variable cells are stored in a specific address space range
                res := lt(term, 0x00000000ffffffffffffffffffffffffffffffffffffffffffffffffffff)
            }
            
            // Unification routine
            function unify(a, b) -> success {
                let deref_a := deref(a)
                let deref_b := deref(b)
                
                if eq(deref_a, deref_b) {
                    success := 1
                    leave
                }
                
                // If A is an unbound variable, bind it to B
                if is_var_cell(deref_a) {
                    sstore(deref_a, deref_b)
                    trail(deref_a)
                    success := 1
                    leave
                }
                
                // If B is an unbound variable, bind it to A
                if is_var_cell(deref_b) {
                    sstore(deref_b, deref_a)
                    trail(deref_b)
                    success := 1
                    leave
                }
                
                // If both are structures, compare their contents
                // We resolve structural terms using SHA256 hashes containing:
                // [Type (32 bytes)][Key (32 bytes)][Left Child (32 bytes)][Right Child (32 bytes)]
                let type_a := sload(deref_a)
                let type_b := sload(deref_b)
                
                if iszero(eq(type_a, type_b)) {
                    success := 0
                    leave
                }
                
                // Unify structural children recursively
                // Left children stored at hash(deref, 1)
                mstore(0, deref_a)
                mstore(32, 1)
                let left_a := sload(keccak256(0, 64))
                
                mstore(0, deref_b)
                mstore(32, 1)
                let left_b := sload(keccak256(0, 64))
                
                success := unify(left_a, left_b)
                
                // Right children stored at hash(deref, 2)
                if success {
                    mstore(0, deref_a)
                    mstore(32, 2)
                    let right_a := sload(keccak256(0, 64))
                    
                    mstore(0, deref_b)
                    mstore(32, 2)
                    let right_b := sload(keccak256(0, 64))
                    
                    success := unify(right_a, right_b)
                }
            }
            
            // Push mutated variable to Trail stack to allow backtracking
            function trail(cell) {
                let trail_ptr := sload(0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF)
                sstore(trail_ptr, cell)
                sstore(0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF, add(trail_ptr, 1))
            }
            
            revert(0, 0)
        }
    }
}
