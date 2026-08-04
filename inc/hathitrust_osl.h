#ifndef HATHITRUST_OSL_H
#define HATHITRUST_OSL_H

/*
 * Generates an OSL (Open Shading Language) source code block representing 
 * a physical paper-aging shader, calibrated with the volume's age.
 * Parameters:
 *   pub_year: The publication year of the book.
 *   target_year: The evaluation year (e.g. current year).
 * Returns:
 *   Allocated string containing the OSL shader code, or NULL on error.
 *   The caller is responsible for freeing the returned string.
 */
char* hathitrust_generate_aging_osl(int pub_year, int target_year);

#endif /* HATHITRUST_OSL_H */
