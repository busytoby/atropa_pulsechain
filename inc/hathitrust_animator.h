#ifndef HATHITRUST_ANIMATOR_H
#define HATHITRUST_ANIMATOR_H

/*
 * Computes frame-by-frame page-turning curl parameters.
 * Parameters:
 *   start_frame: Initial frame index.
 *   end_frame: Ending frame index.
 * Returns:
 *   Allocated string containing sequence of RIB frame descriptions, or NULL on error.
 *   The caller is responsible for freeing the returned string.
 */
char* hathitrust_generate_page_curl_keyframes_rib(int start_frame, int end_frame);

#endif /* HATHITRUST_ANIMATOR_H */
