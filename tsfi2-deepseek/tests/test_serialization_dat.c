#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define MAX_STATE_RECORDS 100

// Constant configuration parameters of the single Red/Black 2-chip system per DAT file
typedef struct {
    uint32_t write_lock_key;               // NCR 304 Tape write-protection key
    uint32_t block_density_selector;       // NCR 304 tape density configuration key
    uint32_t telemetry_block_size_validation;  // NCR 304 hardware block size validator
    uint32_t sort_key_offset_pointer;      // NCR 304 comparator sorting offset pointer
    uint32_t base_index_modulus;           // G-15D Auto-modulo register limits
    uint32_t key_mask_filter;              // NCR 304 Comparator Key filter
    uint32_t memory_guard_boundary;        // G-15D physical drum protected guard boundary
    uint32_t timing_cycle_interrupt_mask;  // G-15D Mainframe timing interrupt mask
    uint32_t segment_buffer_frame_size;    // NCR 304 multi-record segment buffer size
    uint32_t overflow_intercept_target_address; // G-15D redirection pointer for math errors
    uint16_t telemetry_signal_warning_level; // NCR 304 tape read signal warning threshold
    uint8_t config_padding[6];             // Struct alignment padding to ensure 8-byte boundary
} DualRailSystemConfig;

// Mainframe DAT file header block - serialized exactly ONCE per DAT file
typedef struct {
    uint32_t dat_magic_signature;          // Magic number identifying the DAT file (e.g. 0x44415421 - "DAT!")
    uint32_t dat_version;                  // Version of the DAT layout schema
    DualRailSystemConfig hardware_config;  // Single nested dual-rail system configuration per DAT
} VMStateHeader;

// Dynamic telemetry trace record structure - written sequentially for each step execution
typedef struct {
    // 1. Core Stack Fields
    uint32_t step;
    uint32_t pc;
    uint32_t sp;
    uint32_t asp;
    uint32_t halted;

    // 2. Debug & Diagnostics Fields
    uint32_t debug_step_mode;          // G-15D Single-step execution toggle
    uint32_t debug_call_stack_depth;   // Diagnostic call stack depth tracking
    uint64_t debug_gas_remaining;      // Remaining gas budget
    char debug_current_contract[64];   // Target contract name boundary

    // 3. Mainframe Dynamic Registers
    uint64_t sector_rotation_offset;   // G-15D drum timing sector
    uint32_t channel_parity_key;       // NCR 304 tape merge channel parity
    uint32_t tape_sequence_flag;       // NCR 304 sequence flag
    uint32_t overflow_intercept_pc;    // G-15D Overflow track pointer
    uint64_t running_witness_accum;    // NCR 304 witness checksum accumulator
    uint32_t base_index_offset;        // G-15D Auto-increment offset register
    uint32_t trailer_label_signature;  // NCR 304 Trailer Label EOF signature (e.g. 0x454F4621)
    uint32_t record_parity_checksum;   // NCR 304 hardware record parity check
    uint32_t next_instruction_prefetch_address; // G-15D next instruction prefetch pointer
    uint32_t sign_inversion_flag;          // G-15D Arithmetic sign inversion toggle
    uint32_t merge_sequence_validator;     // NCR 304 Mainframe merge sequence counter
    uint32_t transaction_group_id;         // NCR 304 segment group identifier
    uint32_t program_relocation_base;      // G-15D relocatable trace address pointer
    uint32_t tape_search_mode_flag;        // G-15D tape scanning search mode toggle
    uint32_t block_error_warning_indicator;  // NCR 304 hardware signal degradation flags
    uint32_t accumulator_sign_status;      // G-15D hardware sign flag mapping
    uint32_t parity_error_code_register;   // NCR 304 hardware diagnostics warning status
    uint64_t transaction_sort_key;     // NCR 304 Mainframe Key Sort Field
    uint64_t search_target_key;        // G-15D Mainframe Search comparison key
    uint8_t double_precision_flag;    // G-15D Double-precision execution mode
    uint8_t telemetry_retry_count;    // NCR 304 telemetry read retry count
    uint8_t input_source_channel;     // G-15D paper tape input stream channel ID
    uint8_t direction;                 // G-15D Bidirectional step direction (0=FWD, 1=BWD)
    uint8_t record_padding[4];         // Padding to align dynamic section to 8-byte boundary

    // 4. Auncient Yul WinchesterMQ Virtual Hardware Registers
    uint64_t base_reg;                 // Base value B used in exponentiation
    uint64_t channel_reg;              // Channel = Base^Signal mod MotzkinPrime
    uint64_t signal_reg;               // Signal carrier wave register
    uint64_t pole_reg;                 // Pole = Base^Secret mod MotzkinPrime
    uint64_t secret_reg;               // Secret exponent register
    uint64_t foundation_reg;           // Foundation = Base^Identity mod MotzkinPrime
    uint64_t identity_reg;             // Identity symmetry mapping boundary
    uint64_t element_reg;              // Element spacing layout modulus
    uint64_t chin_reg;                 // Chin bottom clamp boundary
    uint64_t monopole_reg;             // Monopole = Chin^Identity mod MotzkinPrime
    uint64_t dynamo_reg;               // Dynamo = Base^Signal mod Element
    uint64_t delegate_reg;             // Delegate user identity mapping
} VMStateRecord;

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT SERIALIZATION: RAILS VM STATE TRACE TO DAT.BIN\n");
    printf("=============================================================\n");

    const char *out_path = "assets/logic_trace.dat.bin";

    // Setup exactly ONE constant config header per DAT file
    VMStateHeader header = {
        .dat_magic_signature = 0x44415421, // "DAT!"
        .dat_version = 1,
        .hardware_config = {
            .write_lock_key = 0x19571001,
            .block_density_selector = 0x0200,
            .telemetry_block_size_validation = 3,
            .sort_key_offset_pointer = 0x24,
            .base_index_modulus = 0x0010,
            .key_mask_filter = 0xF0F00000,
            .memory_guard_boundary = 0xFFFF0000,
            .timing_cycle_interrupt_mask = 0xF0F0,
            .segment_buffer_frame_size = 0x1000,
            .overflow_intercept_target_address = 0x1100,
            .telemetry_signal_warning_level = 0x0A,
            .config_padding = {0}
        }
    };

    // Simulate VM state transitions containing only dynamic registers
    VMStateRecord trace[3] = {
        {
            .step = 0, .pc = 0, .sp = 4352, .asp = 36384, .halted = 0,
            .debug_step_mode = 0, .debug_call_stack_depth = 1, .debug_gas_remaining = 100000,
            .debug_current_contract = "OnChainDatabaseManager",
            .sector_rotation_offset = 0x1000, .channel_parity_key = 0xAA01, .tape_sequence_flag = 1,
            .overflow_intercept_pc = 0x00FF, .running_witness_accum = 0xBAADF00D12345678ULL,
            .base_index_offset = 0x0004, .trailer_label_signature = 0x454F4621,
            .record_parity_checksum = 0x55AAAA55, .next_instruction_prefetch_address = 0x0005,
            .sign_inversion_flag = 0, .merge_sequence_validator = 0x0001,
            .transaction_group_id = 0x100, .program_relocation_base = 0x1000,
            .tape_search_mode_flag = 0, .block_error_warning_indicator = 0,
            .accumulator_sign_status = 1, .parity_error_code_register = 0x00,
            .transaction_sort_key = 0x1111222233334444ULL, .search_target_key = 0x9999888877776666ULL,
            .double_precision_flag = 1, .telemetry_retry_count = 0, .input_source_channel = 1,
            .direction = 0, .record_padding = {0},
            .base_reg = 0x02, .channel_reg = 0x100, .signal_reg = 0x08,
            .pole_reg = 0x400, .secret_reg = 0x0A, .foundation_reg = 0x200,
            .identity_reg = 0x09, .element_reg = 0x50, .chin_reg = 0x30,
            .monopole_reg = 0x900, .dynamo_reg = 0x20, .delegate_reg = 0xABCDEF
        },
        {
            .step = 1, .pc = 5, .sp = 4384, .asp = 36384, .halted = 0,
            .debug_step_mode = 0, .debug_call_stack_depth = 1, .debug_gas_remaining = 99800,
            .debug_current_contract = "OnChainDatabaseManager",
            .sector_rotation_offset = 0x2000, .channel_parity_key = 0xAA02, .tape_sequence_flag = 2,
            .overflow_intercept_pc = 0x00FF, .running_witness_accum = 0xBAADF00D87654321ULL,
            .base_index_offset = 0x0008, .trailer_label_signature = 0x454F4621,
            .record_parity_checksum = 0x55AAAA55, .next_instruction_prefetch_address = 0x000D,
            .sign_inversion_flag = 0, .merge_sequence_validator = 0x0002,
            .transaction_group_id = 0x100, .program_relocation_base = 0x1000,
            .tape_search_mode_flag = 0, .block_error_warning_indicator = 0,
            .accumulator_sign_status = 1, .parity_error_code_register = 0x00,
            .transaction_sort_key = 0x1111222233334445ULL, .search_target_key = 0x9999888877776666ULL,
            .double_precision_flag = 1, .telemetry_retry_count = 0, .input_source_channel = 1,
            .direction = 0, .record_padding = {0},
            .base_reg = 0x02, .channel_reg = 0x100, .signal_reg = 0x08,
            .pole_reg = 0x400, .secret_reg = 0x0A, .foundation_reg = 0x200,
            .identity_reg = 0x09, .element_reg = 0x50, .chin_reg = 0x30,
            .monopole_reg = 0x900, .dynamo_reg = 0x20, .delegate_reg = 0xABCDEF
        },
        {
            .step = 2, .pc = 13, .sp = 4352, .asp = 36384, .halted = 1,
            .debug_step_mode = 1, .debug_call_stack_depth = 1, .debug_gas_remaining = 95000,
            .debug_current_contract = "OnChainDatabaseManager",
            .sector_rotation_offset = 0x3000, .channel_parity_key = 0xAA03, .tape_sequence_flag = 3,
            .overflow_intercept_pc = 0x00FF, .running_witness_accum = 0xBAADF00DAAAAAAAAULL,
            .base_index_offset = 0x000C, .trailer_label_signature = 0x454F4621,
            .record_parity_checksum = 0x55AAAA55, .next_instruction_prefetch_address = 0x0000,
            .sign_inversion_flag = 1, .merge_sequence_validator = 0x0003,
            .transaction_group_id = 0x100, .program_relocation_base = 0x1000,
            .tape_search_mode_flag = 1, .block_error_warning_indicator = 1,
            .accumulator_sign_status = 2, .parity_error_code_register = 0xAA01,
            .transaction_sort_key = 0x1111222233334446ULL, .search_target_key = 0x9999888877776666ULL,
            .double_precision_flag = 2, .telemetry_retry_count = 1, .input_source_channel = 1,
            .direction = 1, .record_padding = {0},
            .base_reg = 0x02, .channel_reg = 0x100, .signal_reg = 0x08,
            .pole_reg = 0x400, .secret_reg = 0x0A, .foundation_reg = 0x200,
            .identity_reg = 0x09, .element_reg = 0x50, .chin_reg = 0x30,
            .monopole_reg = 0x900, .dynamo_reg = 0x20, .delegate_reg = 0xABCDEF
        }
    };
    uint32_t record_count = 3;

    // Verify sizes
    assert(sizeof(VMStateHeader) % 8 == 0);
    assert(sizeof(VMStateRecord) % 8 == 0);
    printf("Header Record size: %zu bytes (1 per DAT file)\n", sizeof(VMStateHeader));
    printf("Trace Record size: %zu bytes (per step)\n", sizeof(VMStateRecord));

    printf("Serializing header and %u trace records to %s...\n", record_count, out_path);

    FILE *f = fopen(out_path, "wb");
    if (!f) {
        out_path = "../assets/logic_trace.dat.bin";
        f = fopen(out_path, "wb");
        if (!f) {
            out_path = "logic_trace.dat.bin";
            f = fopen(out_path, "wb");
            if (!f) {
                printf("Error: Could not open output file for writing.\n");
                return 1;
            }
        }
    }

    // Write constant header first
    fwrite(&header, sizeof(VMStateHeader), 1, f);

    // Write record count prefix
    fwrite(&record_count, sizeof(uint32_t), 1, f);

    // Write trace entries
    for (uint32_t i = 0; i < record_count; i++) {
        fwrite(&trace[i], sizeof(VMStateRecord), 1, f);
    }
    fclose(f);

    printf("✓ Successfully persisted trace to %s\n", out_path);

    // Read back verification
    printf("Reading back trace from %s for verification...\n", out_path);
    f = fopen(out_path, "rb");
    if (!f) {
        printf("Error: Could not open file for verification reading.\n");
        return 1;
    }

    VMStateHeader read_header;
    size_t items_read = fread(&read_header, sizeof(VMStateHeader), 1, f);
    assert(items_read == 1);
    assert(read_header.dat_magic_signature == header.dat_magic_signature);
    assert(read_header.dat_version == header.dat_version);
    assert(read_header.hardware_config.write_lock_key == header.hardware_config.write_lock_key);
    assert(read_header.hardware_config.block_density_selector == header.hardware_config.block_density_selector);
    assert(read_header.hardware_config.overflow_intercept_target_address == header.hardware_config.overflow_intercept_target_address);

    uint32_t read_count = 0;
    items_read = fread(&read_count, sizeof(uint32_t), 1, f);
    assert(items_read == 1);
    assert(read_count == record_count);

    for (uint32_t i = 0; i < read_count; i++) {
        VMStateRecord read_rec;
        items_read = fread(&read_rec, sizeof(VMStateRecord), 1, f);
        assert(items_read == 1);

        printf("  Step[%u]: PC=%u, SP=%u, ASP=%u, Halted=%u, Gas=%llu, Contract=%s, SectorRotationOffset=0x%lx, ParityKey=0x%x, TapeSequence=%u, OverflowTrap=0x%x, Witness=0x%llx, AutoInc=0x%x, EOF=0x%x, Checksum=0x%x, Precision=%u, Retries=%u, Stream=%u, Prefetch=0x%x, SignFlip=%u, Guard=0x%x, SortKeyPtr=0x%x, MergeSequence=0x%x, GroupID=0x%x, RelocBase=0x%x, SearchMode=%u, OverflowTarget=0x%x, ErrorWarning=%u, SignStatus=%u, ParityCode=0x%x, SortKey=0x%llx, SearchKey=0x%llx, BaseReg=0x%llx, ChannelReg=0x%llx, SignalReg=0x%llx, PoleReg=0x%llx, SecretReg=0x%llx, FoundationReg=0x%llx, IdentityReg=0x%llx, ElementReg=0x%llx, ChinReg=0x%llx, MonopoleReg=0x%llx, DynamoReg=0x%llx, DelegateReg=0x%llx, Direction=%s\n",
               read_rec.step, read_rec.pc, read_rec.sp, read_rec.asp, read_rec.halted,
               (unsigned long long)read_rec.debug_gas_remaining, read_rec.debug_current_contract,
               (unsigned long)read_rec.sector_rotation_offset, read_rec.channel_parity_key, 
               read_rec.tape_sequence_flag, read_rec.overflow_intercept_pc,
               (unsigned long long)read_rec.running_witness_accum, read_rec.base_index_offset,
               read_rec.trailer_label_signature, read_rec.record_parity_checksum,
               read_rec.double_precision_flag, read_rec.telemetry_retry_count, read_rec.input_source_channel,
               read_rec.next_instruction_prefetch_address, read_rec.sign_inversion_flag,
               read_header.hardware_config.memory_guard_boundary, read_header.hardware_config.sort_key_offset_pointer,
               read_rec.merge_sequence_validator, read_rec.transaction_group_id, read_rec.program_relocation_base,
               read_rec.tape_search_mode_flag, read_header.hardware_config.overflow_intercept_target_address,
               read_rec.block_error_warning_indicator, read_rec.accumulator_sign_status,
               read_rec.parity_error_code_register,
               (unsigned long long)read_rec.transaction_sort_key, (unsigned long long)read_rec.search_target_key,
               (unsigned long long)read_rec.base_reg, (unsigned long long)read_rec.channel_reg,
               (unsigned long long)read_rec.signal_reg, (unsigned long long)read_rec.pole_reg,
               (unsigned long long)read_rec.secret_reg, (unsigned long long)read_rec.foundation_reg,
               (unsigned long long)read_rec.identity_reg, (unsigned long long)read_rec.element_reg,
               (unsigned long long)read_rec.chin_reg, (unsigned long long)read_rec.monopole_reg,
               (unsigned long long)read_rec.dynamo_reg, (unsigned long long)read_rec.delegate_reg,
               read_rec.direction == 0 ? "FORWARD" : "BACKWARD");

        assert(read_rec.step == trace[i].step);
        assert(read_rec.pc == trace[i].pc);
        assert(read_rec.sp == trace[i].sp);
        assert(read_rec.asp == trace[i].asp);
        assert(read_rec.halted == trace[i].halted);
        assert(read_rec.debug_step_mode == trace[i].debug_step_mode);
        assert(read_rec.debug_call_stack_depth == trace[i].debug_call_stack_depth);
        assert(read_rec.debug_gas_remaining == trace[i].debug_gas_remaining);
        assert(strcmp(read_rec.debug_current_contract, trace[i].debug_current_contract) == 0);
        assert(read_rec.sector_rotation_offset == trace[i].sector_rotation_offset);
        assert(read_rec.channel_parity_key == trace[i].channel_parity_key);
        assert(read_rec.tape_sequence_flag == trace[i].tape_sequence_flag);
        assert(read_rec.overflow_intercept_pc == trace[i].overflow_intercept_pc);
        assert(read_rec.running_witness_accum == trace[i].running_witness_accum);
        assert(read_rec.base_index_offset == trace[i].base_index_offset);
        assert(read_rec.trailer_label_signature == trace[i].trailer_label_signature);
        assert(read_rec.record_parity_checksum == trace[i].record_parity_checksum);
        assert(read_rec.double_precision_flag == trace[i].double_precision_flag);
        assert(read_rec.telemetry_retry_count == trace[i].telemetry_retry_count);
        assert(read_rec.input_source_channel == trace[i].input_source_channel);
        assert(read_rec.next_instruction_prefetch_address == trace[i].next_instruction_prefetch_address);
        assert(read_rec.sign_inversion_flag == trace[i].sign_inversion_flag);
        assert(read_rec.merge_sequence_validator == trace[i].merge_sequence_validator);
        assert(read_rec.transaction_group_id == trace[i].transaction_group_id);
        assert(read_rec.program_relocation_base == trace[i].program_relocation_base);
        assert(read_rec.tape_search_mode_flag == trace[i].tape_search_mode_flag);
        assert(read_rec.block_error_warning_indicator == trace[i].block_error_warning_indicator);
        assert(read_rec.accumulator_sign_status == trace[i].accumulator_sign_status);
        assert(read_rec.parity_error_code_register == trace[i].parity_error_code_register);
        assert(read_rec.transaction_sort_key == trace[i].transaction_sort_key);
        assert(read_rec.search_target_key == trace[i].search_target_key);
        assert(read_rec.base_reg == trace[i].base_reg);
        assert(read_rec.channel_reg == trace[i].channel_reg);
        assert(read_rec.signal_reg == trace[i].signal_reg);
        assert(read_rec.pole_reg == trace[i].pole_reg);
        assert(read_rec.secret_reg == trace[i].secret_reg);
        assert(read_rec.foundation_reg == trace[i].foundation_reg);
        assert(read_rec.identity_reg == trace[i].identity_reg);
        assert(read_rec.element_reg == trace[i].element_reg);
        assert(read_rec.chin_reg == trace[i].chin_reg);
        assert(read_rec.monopole_reg == trace[i].monopole_reg);
        assert(read_rec.dynamo_reg == trace[i].dynamo_reg);
        assert(read_rec.delegate_reg == trace[i].delegate_reg);
        assert(read_rec.direction == trace[i].direction);
    }
    fclose(f);

    printf("✓ Read-back verification complete. Saturated logic paths verified successfully.\n");
    printf("=============================================================\n");
    return 0;
}
