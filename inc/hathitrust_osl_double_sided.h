#ifndef HATHITRUST_OSL_DOUBLE_SIDED_H
#define HATHITRUST_OSL_DOUBLE_SIDED_H

/*
 * Generates an OSL shader that dynamically switches textures 
 * between the front and back of a double-sided page mesh based on normal facing.
 * Parameters:
 *   front_tex: URL/path to the front page texture.
 *   back_tex: URL/path to the back page texture.
 * Returns:
 *   Allocated string containing the OSL shader code, or NULL on error.
 *   The caller is responsible for freeing the returned string.
 */
char* hathitrust_generate_double_sided_osl(const char *front_tex, const char *back_tex);

#endif /* HATHITRUST_OSL_DOUBLE_SIDED_H */
