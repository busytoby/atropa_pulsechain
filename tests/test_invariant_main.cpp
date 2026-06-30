#include <gtest/gtest.h>
#include <cstring>
#include <cstdint>
#include <vector>

// External declaration of the vulnerable function and buffer from main.cpp
extern uint8_t rxpacket[232];
extern void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);

class BufferOverflowSecurityTest : public ::testing::TestWithParam<uint16_t> {};

TEST_P(BufferOverflowSecurityTest, RxPacketBufferBoundsEnforced) {
    // Invariant: OnRxDone must never write beyond the 232-byte rxpacket buffer,
    // regardless of the size parameter from the radio layer.
    
    uint16_t malicious_size = GetParam();
    
    // Create a canary pattern after the buffer to detect overflow
    uint8_t canary[16];
    std::memset(canary, 0xAA, sizeof(canary));
    
    // Create oversized payload
    std::vector<uint8_t> payload(std::max((size_t)malicious_size, (size_t)256), 0xBB);
    
    // Clear rxpacket and set canary
    std::memset(rxpacket, 0, 232);
    uint8_t *canary_ptr = rxpacket + 232;
    std::memcpy(canary_ptr, canary, sizeof(canary));
    
    // Call vulnerable function with adversarial size
    OnRxDone(payload.data(), malicious_size, -80, 10);
    
    // Verify canary is intact (no buffer overflow occurred)
    uint8_t canary_check[16];
    std::memcpy(canary_check, canary_ptr, sizeof(canary_check));
    for (size_t i = 0; i < sizeof(canary); i++) {
        ASSERT_EQ(canary_check[i], 0xAA) 
            << "Buffer overflow detected at offset " << i 
            << " with size parameter: " << malicious_size;
    }
}

INSTANTIATE_TEST_SUITE_P(
    AdversarialPayloads,
    BufferOverflowSecurityTest,
    ::testing::Values(
        232,      // Valid boundary: exact buffer size
        233,      // Boundary overflow: one byte over
        1024,     // Moderate overflow attack
        65535     // Maximum uint16_t: extreme overflow
    )
);

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}