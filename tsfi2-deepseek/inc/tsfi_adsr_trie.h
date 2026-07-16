#ifndef TSFI_ADSR_TRIE_H
#define TSFI_ADSR_TRIE_H

#include "tsfi_trie.h"

// 1. Dynamic ADSR Envelope Structures
typedef struct {
    float attack_time;  // Seconds to reach peak amplitude
    float decay_time;   // Seconds to decay to sustain level
    float sustain_level;// Sustain amplitude level [0.0, 1.0]
    float release_time; // Seconds to decay to 0.0 after key release
} TSFiADSR;

// 2. Lexical Trie Dictionary Structure
typedef struct {
    tsfi_trie_node *root;
} TSFiLexicalTrie;

// Initialize ADSR registers with standard values
void tsfi_adsr_init(TSFiADSR *adsr, float a, float d, float s, float r);

// Calculate active amplitude based on current time step
float tsfi_adsr_calculate_amplitude(const TSFiADSR *adsr, float time, float key_release_time);

// Create lexical dictionary trie
TSFiLexicalTrie* tsfi_lexical_trie_create(void);

// Insert valid word to lexical dictionary
void tsfi_lexical_trie_insert(TSFiLexicalTrie *dict, const char *word);

// Check if word is valid in dictionary
int tsfi_lexical_trie_contains(TSFiLexicalTrie *dict, const char *word);

// Destroy lexical dictionary
void tsfi_lexical_trie_destroy(TSFiLexicalTrie *dict);

#endif // TSFI_ADSR_TRIE_H
