#ifndef HATHITRUST_BIBFRAME_H
#define HATHITRUST_BIBFRAME_H

#include <stdbool.h>
#include <stddef.h>

/*
 * Parses a simple MARC XML node and produces a BIB-FRAME RDF representation.
 * Parameters:
 *   title: Title string.
 *   creator: Creator/Author string.
 *   out_rdf: Output buffer for formatting RDF triples.
 *   max_len: Size of output buffer.
 * Returns:
 *   true on success, false on serialization error.
 */
bool hathitrust_to_bibframe_rdf(
    const char *title,
    const char *creator,
    char *out_rdf,
    size_t max_len
);

#endif /* HATHITRUST_BIBFRAME_H */
