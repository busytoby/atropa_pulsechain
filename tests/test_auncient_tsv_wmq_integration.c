// wmq_mount STANAG
int main() {
    // Phase 1: Set base and secret to derive public pole
    __builtin_wmq_reg_write(1, 5);      // Base = 5
    __builtin_wmq_reg_write(2, 7);      // Secret = 7
    __builtin_wmq_reg_write(3, 953467); // MotzkinPrime = 953467
    
    // Trigger polarization to compute public pole
    __builtin_wmq_reg_write(4, 1);      // Execute Polarize
    __builtin_wmq_reg_read(5);          // Read computed Pole (returned in register EAX)
    
    // Phase 2: Exchange public poles over STANAG interface
    __builtin_wmq_connect_idx(2);
    __builtin_wmq_poke(0, 78125);       // Poke my public key to offset 0
    __builtin_wmq_peek_idx(0);          // Peek peer public key (returned in EAX)
    
    // Phase 3: Set Base to peer_pole (201308) to derive shared secret
    __builtin_wmq_reg_write(1, 201308);
    __builtin_wmq_reg_write(4, 1);      // Execute Polarize again
    __builtin_wmq_reg_read(5);          // Shared Secret derived
    
    __builtin_wmq_disconnect();
    return 899025;
}
