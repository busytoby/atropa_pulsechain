#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <assert.h>

#define PAYLOAD_SIZE 64
#define EVENT_LOG_LIMIT 16

typedef enum {
    PREC_ROUTINE = 1,
    PREC_IMMEDIATE = 2,
    PREC_FLASH = 3
} autodin_precedence_t;

typedef enum {
    SEC_UNCLASSIFIED = 1,
    SEC_SECRET = 2,
    SEC_TOP_SECRET = 3
} autodin_security_t;

typedef struct {
    uint64_t sequence;
    autodin_precedence_t precedence;
    autodin_security_t security_level;
    char payload[PAYLOAD_SIZE];
} autodin_packet_t;

// Structured event logs representing telemetry on the first node
typedef struct {
    uint64_t timestamp;
    uint64_t sequence;
    uint8_t event_type;       // 0 = INGEST, 1 = ESCALATION, 2 = SECURITY_ALERT
    uint8_t precedence;
    uint8_t security_level;
} autodin_event_log_t;

typedef struct {
    int rx_fd;
    int tx_fd;
    autodin_security_t reader_clearance;
} autodin_input_node_t;

// Writes events directly to .dat.bin telemetry ledger (Rule 13 compliance)
static bool log_autodin_event(const char *path, const autodin_event_log_t *event) {
    FILE *f = fopen(path, "ab"); // Binary append mode
    if (!f) return false;
    
    size_t written = fwrite(event, sizeof(autodin_event_log_t), 1, f);
    fclose(f);
    return (written == 1);
}

// First Node Execution: Ingest, monitor, and log incoming AUTODIN transactions
static bool autodin_input_node_ingest(autodin_input_node_t *node, const char *log_path, autodin_packet_t *rx_packet, uint64_t simulated_cycles) {
    ssize_t bytes = read(node->rx_fd, rx_packet, sizeof(autodin_packet_t));
    if (bytes != sizeof(autodin_packet_t)) {
        return false;
    }

    // 1. General Ingestion Event Logging
    autodin_event_log_t event;
    event.timestamp = simulated_cycles;
    event.sequence = rx_packet->sequence;
    event.event_type = 0; // INGEST
    event.precedence = (uint8_t)rx_packet->precedence;
    event.security_level = (uint8_t)rx_packet->security_level;
    log_autodin_event(log_path, &event);

    // 2. Precedence Escalation Event Monitoring
    if (rx_packet->precedence == PREC_FLASH) {
        autodin_event_log_t esc_event = event;
        esc_event.event_type = 1; // ESCALATION
        log_autodin_event(log_path, &esc_event);
    }

    // 3. Security Boundary Gate Check & Alert
    if (node->reader_clearance < rx_packet->security_level) {
        autodin_event_log_t alert_event = event;
        alert_event.event_type = 2; // SECURITY_ALERT
        log_autodin_event(log_path, &alert_event);
        return false; // Gated out (blocked at input node boundary)
    }

    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT AUTODIN FIRST NODE MONITORING & EVENT LOGGING SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    const char *log_path = "tests/autodin_events.dat.bin";
    remove(log_path);

    int fds[2];
    bool ok = (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) >= 0);
    assert(ok == true);

    // Initialize first node with SECRET reader clearance
    autodin_input_node_t input_node = {
        .rx_fd = fds[0],
        .tx_fd = fds[0],
        .reader_clearance = SEC_SECRET
    };

    autodin_packet_t pkt_routine = {
        .sequence = 501,
        .precedence = PREC_ROUTINE,
        .security_level = SEC_SECRET,
        .payload = "Standard message data"
    };

    autodin_packet_t pkt_flash_alert = {
        .sequence = 502,
        .precedence = PREC_FLASH,
        .security_level = SEC_TOP_SECRET, // Exceeds SECRET clearance
        .payload = "Critical security command payload"
    };

    // 1. Ingest Routine message (should pass, logs INGEST)
    printf("[TEST] Ingesting Routine transaction...\n");
    fflush(stdout);
    write(fds[1], &pkt_routine, sizeof(autodin_packet_t));
    
    autodin_packet_t rx;
    ok = autodin_input_node_ingest(&input_node, log_path, &rx, 1000);
    assert(ok == true);
    assert(rx.sequence == 501);
    printf("   ✓ Routine message ingested.\n");
    fflush(stdout);

    // 2. Ingest Flash + Top Secret message (should fail gate, logs INGEST, ESCALATION, and SECURITY_ALERT)
    printf("[TEST] Ingesting Flash & Top Secret transaction...\n");
    fflush(stdout);
    write(fds[1], &pkt_flash_alert, sizeof(autodin_packet_t));

    ok = autodin_input_node_ingest(&input_node, log_path, &rx, 1050);
    assert(ok == false); // Blocked
    printf("   ✓ Flash TOP_SECRET message blocked at input node boundary.\n");
    fflush(stdout);

    // 3. Verify logged events in .dat.bin ledger
    printf("[TEST] Parsing and validating event logs from .dat.bin file...\n");
    fflush(stdout);

    FILE *f = fopen(log_path, "rb");
    assert(f != NULL);

    autodin_event_log_t logs[4];
    size_t read_count = fread(logs, sizeof(autodin_event_log_t), 4, f);
    fclose(f);

    assert(read_count == 4); // 1 from Routine, 3 from Flash/TopSecret

    // Log 1: Routine INGEST
    assert(logs[0].sequence == 501 && logs[0].event_type == 0);
    
    // Log 2: Flash/TopSecret INGEST
    assert(logs[1].sequence == 502 && logs[1].event_type == 0);
    
    // Log 3: Flash ESCALATION
    assert(logs[2].sequence == 502 && logs[2].event_type == 1);
    
    // Log 4: Security alert (Clearance violation)
    assert(logs[3].sequence == 502 && logs[3].event_type == 2);

    printf("   ✓ Telemetry verified. All 4 events successfully tracked on the first node.\n");
    fflush(stdout);

    close(fds[0]);
    close(fds[1]);
    remove(log_path);

    printf("=============================================================\n");
    printf("AUTODIN FIRST NODE EVENT LOGGING COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}
