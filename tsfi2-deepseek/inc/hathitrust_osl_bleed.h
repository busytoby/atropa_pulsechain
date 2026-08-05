#ifndef HATHITRUST_OSL_BLEED_H
#define HATHITRUST_OSL_BLEED_H

/*
 * Generates an OSL (Open Shading Language) shader representing 
 * typographic ink-bleed, letterpress indentation, and paper fiber noise.
 * Parameters:
 *   bleed_factor: Size/intensity of the simulated ink bleed (0.0 to 1.0).
 *   bump_scale: Depth of the letterpress indentation (0.0 to 0.1).
 * Returns:
 *   Allocated string containing the OSL shader code, or NULL on error.
 *   The caller is responsible for freeing the returned string.
 */
char* hathitrust_generate_bleed_osl(double bleed_factor, double bump_scale);

#endif /* HATHITRUST_OSL_BLEED_H */
