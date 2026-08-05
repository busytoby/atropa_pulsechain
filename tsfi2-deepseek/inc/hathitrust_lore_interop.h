#ifndef HATHITRUST_LORE_INTEROP_H
#define HATHITRUST_LORE_INTEROP_H

#include <stdbool.h>

typedef struct {
    char *lore_id;
    char *title;
    char *pub_date;
    char *filepath;
} HtrcLoreRecord;

/*
 * Dynamically registers an Auncient lore source path with the interop registry.
 * This registry is thread-safe.
 */
bool hathitrust_lore_interop_register(
    const char *lore_id,
    const char *title,
    const char *pub_date,
    const char *filepath
);

/*
 * Resolves a registered Auncient lore source by ID.
 */
HtrcLoreRecord* hathitrust_lore_interop_lookup(const char *lore_id);

/*
 * Clears the interop registry.
 */
void hathitrust_lore_interop_clear(void);

#endif /* HATHITRUST_LORE_INTEROP_H */
