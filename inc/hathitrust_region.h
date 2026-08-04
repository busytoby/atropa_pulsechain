#ifndef HATHITRUST_REGION_H
#define HATHITRUST_REGION_H

#include "hathitrust_alto.h"

typedef enum {
    REGION_UNKNOWN = 0,
    REGION_TITLE_PAGE,
    REGION_CONTENT_BODY,
    REGION_INDEX_BACK
} HtrcStructuralRegionType;

/*
 * Analyzes word density and coordinates to classify page structural region types.
 * Parameters:
 *   page: Pre-parsed page layout.
 * Returns:
 *   The classified region type.
 */
HtrcStructuralRegionType hathitrust_region_classify(const HtrcAltoPage *page);

#endif /* HATHITRUST_REGION_H */
