#ifndef TSFI_PARC_RLE_H
#define TSFI_PARC_RLE_H

/* Compress input buffer using Thomas Knoll-style Run-Length Encoding (RLE) */
int tsfi_parc_rle_compress(const uint8_t *src, int src_len, uint8_t *dest, int max_dest_len);

/* Decompress RLE stream into output buffer */
int tsfi_parc_rle_decompress(const uint8_t *src, int src_len, uint8_t *dest, int max_dest_len);

#endif // TSFI_PARC_RLE_H
