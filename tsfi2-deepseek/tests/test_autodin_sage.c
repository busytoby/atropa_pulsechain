#include "tsfi_autodin_sage.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

int main(void) {
    printf("[INFO] Starting AUTODIN SAGE Transaction Compliance Test...\n");
    
    // Test 1: Latching relay snaps
    tsfi_autodin_relay relay;
    tsfi_autodin_relay_init(&relay);
    assert(relay.contacts_closed == false);
    
    // Exert flux on Set coil
    int rc = tsfi_autodin_relay_latch(&relay, 120, 0);
    assert(rc == 0);
    assert(relay.contacts_closed == false); // Not latched yet (flux: 120 <= 200)
    
    rc = tsfi_autodin_relay_latch(&relay, 100, 0);
    assert(rc == 0);
    assert(relay.contacts_closed == true); // snapped/latched! (flux: 220 > 200)
    
    // Exert flux on Reset coil
    rc = tsfi_autodin_relay_latch(&relay, 0, 250);
    assert(rc == 0);
    assert(relay.contacts_closed == false); // Reset snaps back!
    
    // Test 2: SAGE PLL Phase-Locked synchronization loop
    tsfi_sage_pll_fabric pll;
    memset(&pll, 0, sizeof(tsfi_sage_pll_fabric));
    pll.feedback_phase = 100;
    
    // Run sync loop towards target external phase 100
    rc = tsfi_sage_pll_sync(&pll, 100);
    assert(rc == 0); // Locked! (error voltage = 0)
    
    // Test 3: CICS transaction routing checks
    tsfi_autodin_manager mgr;
    memset(&mgr, 0, sizeof(tsfi_autodin_manager));
    mgr.system_unstable = false;
    mgr.pll = pll;
    
    tsfi_sage_cics_io cics_tx;
    cics_tx.tx_id = 7001;
    cics_tx.queue_id = 5;
    
    rc = tsfi_sage_cics_io_route(&cics_tx, &mgr);
    assert(rc == 0);
    assert(mgr.active_tx_id == 7001);
    
    // Check stability rejection
    mgr.system_unstable = true;
    rc = tsfi_sage_cics_io_route(&cics_tx, &mgr);
    assert(rc == -2);
    
    // Test 4: WinchesterMQ SCSI local loopback handshake
    int sv[2];
    rc = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
    assert(rc == 0);
    
    tsfi_winchester_scsi scsi;
    memset(&scsi, 0, sizeof(tsfi_winchester_scsi));
    
    // Attempt keycode 32 (D/d)
    rc = tsfi_winchester_scsi_handshake(&scsi, sv[0], 32);
    assert(rc == 1); // Handshake step 1 in progress (REQ line high, sent register payload)
    assert(scsi.req_line == true);
    
    // Simulating target node loopback echo ACK
    uint8_t read_payload = 0;
    rc = recv(sv[1], &read_payload, 1, 0);
    assert(rc == 1 && read_payload == 32);
    
    rc = send(sv[1], &read_payload, 1, 0);
    assert(rc == 1);
    
    // Complete the handshake
    rc = tsfi_winchester_scsi_handshake(&scsi, sv[0], 32);
    assert(rc == 0); // Handshake successfully complete!
    assert(scsi.ack_line == true);
    
    close(sv[0]);
    close(sv[1]);
    
    // Test 5: SAGE Radar Coordinate Filtering
    int32_t station_x[3] = {1000, 1010, 990};
    int32_t station_y[3] = {5000, 5020, 4980};
    int32_t filt_x = 0;
    int32_t filt_y = 0;
    
    rc = tsfi_sage_filter_tracks(&filt_x, &filt_y, station_x, station_y, 3);
    assert(rc == 0);
    assert(filt_x == 1000);
    assert(filt_y == 5000);
    
    // Test 6: SAGE Duplex Processor Sync and Failover
    tsfi_sage_duplex duplex;
    duplex.active_cpu_id = 1;
    duplex.standby_cpu_id = 2;
    duplex.last_sync_time = 0;
    duplex.standby_active = false;
    
    rc = tsfi_sage_duplex_sync(&duplex, true);
    assert(rc == 0);
    assert(duplex.last_sync_time == 1);
    
    rc = tsfi_sage_duplex_sync(&duplex, false); // Failover!
    assert(rc == 1);
    assert(duplex.standby_active == true);
    assert(duplex.active_cpu_id == 2);
    
    // Test 7: AUTODIN Precedence Message Switch Scheduling
    char switch_buf[128];
    rc = tsfi_autodin_schedule_message(AUTODIN_PRECEDENCE_FLASH, "IMPENDING FLIGHT DETECTED", switch_buf);
    assert(rc == 0);
    assert(strcmp(switch_buf, "[FLASH] IMPENDING FLIGHT DETECTED") == 0);
    
    // Test 8: SAGE Magnetic Drum Memory Writes and Reads
    tsfi_sage_drum drum;
    memset(&drum, 0, sizeof(tsfi_sage_drum));
    
    uint8_t write_data[32] = "DRUM_SECTOR_DATA";
    rc = tsfi_sage_drum_write(&drum, 3, 12, write_data, 16);
    assert(rc == 0);
    assert(drum.head_position == 12);
    
    uint8_t read_data[32] = "";
    rc = tsfi_sage_drum_read(&drum, 3, 12, read_data, 16);
    assert(rc == 0);
    assert(memcmp(read_data, "DRUM_SECTOR_DATA", 16) == 0);
    
    // Test 9: AUTODIN Routing Indicator Lookup Search
    tsfi_autodin_route route_table[2] = {
        {"RUA1", 101},
        {"RUA2", 102}
    };
    uint32_t chan = 0;
    rc = tsfi_autodin_find_route(route_table, 2, "RUA2", &chan);
    assert(rc == 0);
    assert(chan == 102);
    
    rc = tsfi_autodin_find_route(route_table, 2, "UNKNOWN", &chan);
    assert(rc == -2);
    
    // Test 10: SAGE Marginal Checking (Vacuum Tube diagnostics)
    tsfi_sage_marginal_unit tube_unit;
    tube_unit.baseline_voltage_mv = 6000;
    tube_unit.applied_voltage_mv = 6000;
    tube_unit.tube_emission_percent = 80;
    
    rc = tsfi_sage_marginal_check(&tube_unit, 0); // Normal range
    assert(rc == 0);
    assert(tube_unit.tube_emission_percent == 80);
    
    rc = tsfi_sage_marginal_check(&tube_unit, 1500); // 7500 mv (Over-voltage excursion)
    assert(rc == 0);
    assert(tube_unit.tube_emission_percent == 65);
    
    rc = tsfi_sage_marginal_check(&tube_unit, 1500); // 7500 mv again (exceeds tolerance, wear snap)
    assert(rc == 1); // Triggers warning (emission 50 < 60)
    assert(tube_unit.tube_emission_percent == 50);
    
    // Test 11: AUTODIN Preemption Queue Controls
    tsfi_autodin_preempt_channel p_chan;
    p_chan.channel_busy = true;
    p_chan.current_precedence = AUTODIN_PRECEDENCE_ROUTINE;
    p_chan.suspended_tx_id = 0;
    
    bool preempt = false;
    bool reject = false;
    rc = tsfi_autodin_preempt_check(&p_chan, 8001, AUTODIN_PRECEDENCE_FLASH, &preempt, &reject);
    assert(rc == 0);
    assert(preempt == true && reject == false); // Flash preempts routine
    assert(p_chan.suspended_tx_id == 8001);
    
    rc = tsfi_autodin_preempt_check(&p_chan, 8002, AUTODIN_PRECEDENCE_ROUTINE, &preempt, &reject);
    assert(rc == 0);
    assert(preempt == false && reject == true); // Routine rejected under current active Flash
    
    // Test 12: SAGE Light Gun operator input coordinate resolution
    tsfi_sage_light_gun gun;
    gun.target_x = 105;
    gun.target_y = 205;
    gun.trigger_pulled = true;
    
    int32_t track_x[3] = {10, 107, 300};
    int32_t track_y[3] = {20, 204, 400};
    int selected_idx = -1;
    
    rc = tsfi_sage_light_gun_select(&gun, track_x, track_y, 3, &selected_idx);
    assert(rc == 0);
    assert(selected_idx == 1); // Selected closest target track
    
    // Test 13: AUTODIN Tape Journaling writes
    const char *journal_path = "tmp/autodin_tape.log";
    int j_fd = open(journal_path, O_RDWR | O_CREAT | O_TRUNC, 0644);
    assert(j_fd >= 0);
    
    tsfi_autodin_journal journal;
    journal.journal_fd = j_fd;
    journal.last_journal_lsn = 0;
    
    rc = tsfi_autodin_journal_write(&journal, 1205, "TAPE_JOURNAL_BLOCK");
    assert(rc == 0);
    assert(journal.last_journal_lsn == 1205);
    
    close(j_fd);
    unlink(journal_path);
    
    printf("[SUCCESS] AUTODIN SAGE Transaction Compliance Test Passed!\n");
    return 0;
}
