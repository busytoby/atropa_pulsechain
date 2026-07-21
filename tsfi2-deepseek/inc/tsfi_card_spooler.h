#ifndef TSFI_CARD_SPOOLER_H
#define TSFI_CARD_SPOOLER_H

#include <stdint.h>
#include <stddef.h>

#define PUNCH_CARD_COLS 80

typedef struct {
    char card_data[PUNCH_CARD_COLS + 1]; // 80-column card image
    uint32_t card_sequence_num;
} tsfi_punch_card_t;

typedef struct {
    uint32_t cards_processed;
    uint32_t drum_words_written;
    char tape_dat_bin[128]; // Enforces Rule 13 (.DAT.BIN)
} tsfi_card_spooler_summary_t;

/* Initialize and spool a punch card deck to Magnetic Drum and Magnetic Tape Archive */
int tsfi_card_spooler_process_deck(
    const tsfi_punch_card_t *deck,
    size_t card_count,
    tsfi_card_spooler_summary_t *out_summary
);

#endif // TSFI_CARD_SPOOLER_H
