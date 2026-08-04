#ifndef HATHITRUST_OSL_IMPRESSION_H
#define HATHITRUST_OSL_IMPRESSION_H

/*
 * Generates an OSL shader representing dynamic letterpress bump-displacement.
 * Indents page coordinates where ink text lies.
 * Parameters:
 *   indent_depth: Physical depth of the letterpress indentation (0.0 to 0.05).
 * Returns:
 *   Allocated string containing the OSL shader code, or NULL on error.
 *   The caller is responsible for freeing the returned string.
 */
char* hathitrust_generate_impression_osl(double indent_depth);

#endif /* HATHITRUST_OSL_IMPRESSION_H */
