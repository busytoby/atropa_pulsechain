// wmq_mount STANAG
int main() {
    // State 1: Initializing Connection
    __builtin_wmq_connect_idx(2);
    __builtin_wmq_peer_idx(6);
    
    // State 2: Exchanging Credentials
    __builtin_wmq_auth_idx(3);
    __builtin_wmq_key_idx(5);
    
    // State 3: Transmission Setup & STANAG SAP QoS Routing
    __builtin_wmq_timeout_idx(4);
    __builtin_wmq_window();
    __builtin_wmq_reg_write(8, 1);  // Map SAP_0x08 active
    __builtin_wmq_reg_write(9, 2);  // Escalate SAP priority level to QoS 2
    __builtin_wmq_reg_write(10, 0); // Enable STANAG Non-ARQ Broadcast Mode
    
    // State 4: Retransmission & Error Control
    __builtin_wmq_retransmit();
    __builtin_wmq_disconnect();
    
    return 4;
}
