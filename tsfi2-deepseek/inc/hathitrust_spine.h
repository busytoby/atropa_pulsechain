#ifndef HATHITRUST_SPINE_H
#define HATHITRUST_SPINE_H

/*
 * Generates RIB definitions representing the book hardcovers and binding spine hinges
 * dynamically scaled based on book page density parameters.
 * Parameters:
 *   page_count: Total page count inside the book structure.
 * Returns:
 *   Allocated string containing RIB descriptions, or NULL on error.
 *   The caller is responsible for freeing the returned string.
 */
char* hathitrust_generate_book_binding_rib(int page_count);

#endif /* HATHITRUST_SPINE_H */
