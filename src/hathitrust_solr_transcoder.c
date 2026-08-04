#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "hathitrust_solr_transcoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* hathitrust_solr_to_marcxml(
    const char *htid,
    const char *title,
    const char *author,
    const char *pub_date
) {
    if (!htid || !title || !author || !pub_date) return NULL;
    
    size_t cap = 2048;
    char *res = malloc(cap);
    if (!res) return NULL;
    
    snprintf(res, cap,
             "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
             "<record xmlns=\"http://www.loc.gov/MARC21/slim\">\n"
             "  <leader>00000nam a2200000 u 4500</leader>\n"
             "  <controlfield tag=\"001\">%s</controlfield>\n"
             "  <datafield tag=\"245\" ind1=\"0\" ind2=\"0\">\n"
             "    <subfield code=\"a\">%s</subfield>\n"
             "  </datafield>\n"
             "  <datafield tag=\"100\" ind1=\"1\" ind2=\" \">\n"
             "    <subfield code=\"a\">%s</subfield>\n"
             "  </datafield>\n"
             "  <datafield tag=\"260\" ind1=\" \" ind2=\" \">\n"
             "    <subfield code=\"c\">%s</subfield>\n"
             "  </datafield>\n"
             "</record>\n",
             htid, title, author, pub_date);
             
    return res;
}
