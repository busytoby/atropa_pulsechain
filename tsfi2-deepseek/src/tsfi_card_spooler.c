#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_card_spooler.h"
#include "tsfi_minimized_drum_buffer.h"

int tsfi_card_spooler_process_deck(
    const tsfi_punch_card_t *deck,
    size_t card_count,
    tsfi_card_spooler_summary_t *out_summary
) {
    if (!deck || card_count == 0 || !out_summary) return -1;
    memset(out_summary, 0, sizeof(tsfi_card_spooler_summary_t));

    // 1. Spool cards to Magnetic Drum scratchpad memory
    tsfi_minimized_drum_buffer_t drum_buf;
    tsfi_minimized_drum_buffer_init(&drum_buf);

    for (size_t i = 0; i < card_count; i++) {
        // Push sample value derived from card sequence into fast drum memory
        tsfi_minimized_drum_buffer_push(&drum_buf, (float)deck[i].card_sequence_num * 0.1f);
        out_summary->drum_words_written++;
    }

    out_summary->cards_processed = (uint32_t)card_count;

    // 2. Format Magnetic Tape Archive dataset filename strictly enforcing Rule 13 (.DAT.BIN)
    snprintf(out_summary->tape_dat_bin, sizeof(out_summary->tape_dat_bin),
             "PUNCH_CARD_SPOOL_VOL001_%04u.DAT.BIN", (unsigned int)card_count);

    printf("[CARD SPOOLER] Spooled %u Punch Cards -> Drum Words: %u | Tape Archive: %s\n",
           out_summary->cards_processed, out_summary->drum_words_written, out_summary->tape_dat_bin);

    return 0; // Card spooling complete
}
