#ifndef HATHITRUST_CAMERA_PATH_H
#define HATHITRUST_CAMERA_PATH_H

#include "hathitrust_alto.h"

/*
 * Generates RIB camera transformations looking sequentially at text line areas.
 * Parameters:
 *   page: Pre-parsed ALTO page layout with coordinates.
 * Returns:
 *   Allocated string containing sequence of RIB camera Translate/Rotate blocks, or NULL on error.
 *   The caller is responsible for freeing the returned string.
 */
char* hathitrust_generate_camera_path_rib(const HtrcAltoPage *page);

#endif /* HATHITRUST_CAMERA_PATH_H */
