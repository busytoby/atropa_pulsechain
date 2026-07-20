#include <string.h>
#include <stdio.h>
#include "tsfi_parc_laurel.h"

int tsfi_parc_laurel_compose(tsfi_parc_email_t *email, const char *from, const char *to, const char *subj, const char *body_txt) {
    if (!email || !from || !to || !subj || !body_txt) return -1;

    strncpy(email->sender, from, sizeof(email->sender) - 1);
    email->sender[sizeof(email->sender) - 1] = '\0';

    strncpy(email->receiver, to, sizeof(email->receiver) - 1);
    email->receiver[sizeof(email->receiver) - 1] = '\0';

    strncpy(email->subject, subj, sizeof(email->subject) - 1);
    email->subject[sizeof(email->subject) - 1] = '\0';

    strncpy(email->body, body_txt, sizeof(email->body) - 1);
    email->body[sizeof(email->body) - 1] = '\0';

    return 0;
}

int tsfi_parc_laurel_serialize_pup(const tsfi_parc_email_t *email, uint8_t *buffer, int max_len) {
    if (!email || !buffer || max_len < 256) return -1;

    // Package the email headers and body into a structured string payload
    int written = snprintf((char *)buffer, max_len,
        "FROM:%s|TO:%s|SUBJ:%s|BODY:%s",
        email->sender, email->receiver, email->subject, email->body);

    if (written < 0 || written >= max_len) {
        return -2; // overflow
    }

    return written;
}
