#include <string.h>
#include "tsfi_parc_clipboard.h"

void tsfi_parc_clip_init(tsfi_parc_clipboard_t *clip) {
    if (!clip) return;
    memset(clip->buffer, 0, sizeof(clip->buffer));
    clip->length = 0;
}

int tsfi_parc_clip_copy(tsfi_parc_clipboard_t *clip, const char *text, int len) {
    if (!clip || !text || len < 0) return -1;

    int copy_len = len > 255 ? 255 : len;
    strncpy(clip->buffer, text, copy_len);
    clip->buffer[copy_len] = '\0';
    clip->length = copy_len;
    return copy_len;
}

int tsfi_parc_clip_paste(const tsfi_parc_clipboard_t *clip, char *dest, int max_len) {
    if (!clip || !dest || max_len <= 0) return -1;

    int paste_len = clip->length >= max_len ? max_len - 1 : clip->length;
    strncpy(dest, clip->buffer, paste_len);
    dest[paste_len] = '\0';
    return paste_len;
}
