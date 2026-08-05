#ifndef HATHITRUST_WINDOW_H
#define HATHITRUST_WINDOW_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Initializes and opens a Vulkan/Wayland window display interface for reading HathiTrust
 * books with interactive key controls.
 * Parameters:
 *   book_title: Title of the book.
 *   total_pages: Total pages in the book.
 * Returns:
 *   true on successful initialization and display, false on window error.
 */
bool hathitrust_open_reader_window(const char *book_title, int total_pages);

/*
 * Evaluates raw SCSI/ZMM keyboard event registers for the window.
 * keycode 32 is 'd'/'D' (page turn forward) and 30 is 'a'/'A' (page turn backward).
 */
int hathitrust_process_scsi_key_event(uint32_t keycode);

#endif /* HATHITRUST_WINDOW_H */
