#ifndef LAU_REGISTRY_H
#define LAU_REGISTRY_H

#include "lau_memory.h"

// Inserts a metadata block into the global tracking registry.
void lau_registry_insert(LauMetadata *m);

// Removes a metadata block from the global tracking registry.
void lau_registry_remove(LauMetadata *m);

// Returns the head of the registry list (requires locking via lau_registry_lock).
LauMetadata* lau_registry_get_head(void);

// Locking for registry traversal
void lau_registry_lock(void);
void lau_registry_unlock(void);

#endif // LAU_REGISTRY_H
