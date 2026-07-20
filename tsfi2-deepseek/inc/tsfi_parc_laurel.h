#ifndef TSFI_PARC_LAUREL_H
#define TSFI_PARC_LAUREL_H

#include <stdint.h>

typedef struct {
    char sender[32];
    char receiver[32];
    char subject[64];
    char body[128];
} tsfi_parc_email_t;

/* Compose a new email message */
int tsfi_parc_laurel_compose(tsfi_parc_email_t *email, const char *from, const char *to, const char *subj, const char *body_txt);

/* Serialize the composed email into standard PUP network packet payload */
int tsfi_parc_laurel_serialize_pup(const tsfi_parc_email_t *email, uint8_t *buffer, int max_len);

#endif // TSFI_PARC_LAUREL_H
