int main() {
    // State 1: Initializing Connection
    __builtin_wmq_connect_idx(2);
    __builtin_wmq_peer_idx(6);
    
    // State 2: Exchanging Credentials
    __builtin_wmq_auth_idx(3);
    __builtin_wmq_key_idx(5);
    
    // State 3: Transmission Setup
    __builtin_wmq_timeout_idx(4);
    __builtin_wmq_window();
    
    // State 4: Retransmission & Error Control
    __builtin_wmq_retransmit();
    __builtin_wmq_disconnect();
    
    return 4;
}
