int main() {
    __builtin_wmq_connect_idx(2);
    __builtin_wmq_auth_idx(3);
    __builtin_wmq_timeout_idx(4);
    __builtin_wmq_key_idx(5);
    __builtin_wmq_disconnect();
    return 42;
}
