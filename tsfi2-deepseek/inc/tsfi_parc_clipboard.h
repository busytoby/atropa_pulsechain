#ifndef TSFI_PARC_CLIPBOARD_H
#define TSFI_PARC_CLIPBOARD_H

typedef struct {
    char buffer[256];
    int length;
} tsfi_parc_clipboard_t;

/* Initialize empty clipboard */
void tsfi_parc_clip_init(tsfi_parc_clipboard_t *clip);

/* Copy text range into clipboard buffer */
int tsfi_parc_clip_copy(tsfi_parc_clipboard_t *clip, const char *text, int len);

/* Paste text range from clipboard into destination buffer */
int tsfi_parc_clip_paste(const tsfi_parc_clipboard_t *clip, char *dest, int max_len);

#endif // TSFI_PARC_CLIPBOARD_H
