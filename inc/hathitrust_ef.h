#ifndef HATHITRUST_EF_H
#define HATHITRUST_EF_H

#include <stdbool.h>
#include <stddef.h>

/*
 * Structure representing token (word) count and its Part-of-Speech tag.
 */
typedef struct {
    char *token;
    char *pos_tag;
    int count;
} HtrcTokenFeature;

/*
 * Structure representing extracted features of a single page.
 */
typedef struct {
    int page_number;
    int token_count;
    HtrcTokenFeature *tokens;
    int token_capacity;
} HtrcPageFeature;

/*
 * Structure representing a volume's extracted features.
 */
typedef struct {
    char *volume_id;
    char *title;
    char *author;
    char *pub_date;
    char *language;
    
    int page_count;
    HtrcPageFeature *pages;
    int page_capacity;
} HtrcVolumeFeatures;

/*
 * Decompresses and parses an HTRC Extracted Features gzipped JSON file.
 * Parameters:
 *   gz_data: The compressed gzip buffer.
 *   gz_len: The length of the compressed gzip buffer.
 *   volume: Pointer to HtrcVolumeFeatures struct to populate.
 * Returns:
 *   true if parsed successfully, false on decompression or parser error.
 */
bool htrc_parse_extracted_features(const unsigned char *gz_data, size_t gz_len, HtrcVolumeFeatures *volume);

/*
 * Frees all allocated memory in HtrcVolumeFeatures.
 */
void htrc_free_volume_features(HtrcVolumeFeatures *volume);

#endif /* HATHITRUST_EF_H */
