#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <assert.h>
#include "../inc/tsfi_waugh_chancery.h"

// C runtime driver executing the standard ALGOL 61 prover logic
// from solidity/dysnomia/domain/std/chancery_unauthorized_block.algol61
typedef enum {
    PROVEN_AUTHENTIC_STREAM = 0,
    UNAUTHORIZED_BLOCK = 1,
    UNRESOLVED_CHANCERY_CONTINUANCE = 2
} Algol61ChanceryRuling;

static Algol61ChanceryRuling execute_algol61_chancery_prover(
    int http_status,
    bool turnstile_html_detected,
    bool binary_stream_detected
) {
    if (http_status == 200 && binary_stream_detected && !turnstile_html_detected) {
        return PROVEN_AUTHENTIC_STREAM;
    } else if (http_status == 403 || (http_status == 200 && turnstile_html_detected)) {
        return UNAUTHORIZED_BLOCK;
    } else {
        return UNRESOLVED_CHANCERY_CONTINUANCE;
    }
}

static void probe_live_hathitrust_endpoint(
    const char *htid,
    int seq,
    int *out_status,
    bool *out_turnstile,
    bool *out_binary
) {
    const char *host = "babel.hathitrust.org";
    const char *port = "443";

    *out_status = 0;
    *out_turnstile = false;
    *out_binary = false;

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host, port, &hints, &res) != 0) return;

    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        freeaddrinfo(res);
        return;
    }

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) != 0) {
        close(sockfd);
        freeaddrinfo(res);
        return;
    }
    freeaddrinfo(res);

    SSL_library_init();
    SSL_load_error_strings();
    const SSL_METHOD *method = TLS_client_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (!ctx) {
        close(sockfd);
        return;
    }

    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);
    SSL_set_tlsext_host_name(ssl, host);

    if (SSL_connect(ssl) != 1) {
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(sockfd);
        return;
    }

    char request[2048];
    snprintf(request, sizeof(request),
             "GET /cgi/imgsrv/download/plaintext?id=%s&attachment=1&tracker=D2&seq=%d HTTP/1.1\r\n"
             "Host: %s\r\n"
             "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36\r\n"
             "Accept: text/plain, */*\r\n"
             "Connection: close\r\n\r\n",
             htid, seq, host);

    SSL_write(ssl, request, strlen(request));

    char buf[16384];
    int bytes = SSL_read(ssl, buf, sizeof(buf) - 1);
    if (bytes > 0) {
        buf[bytes] = '\0';
        if (strstr(buf, "HTTP/1.1 403") || strstr(buf, "HTTP/2 403") || strstr(buf, "403 Forbidden")) {
            *out_status = 403;
        } else if (strstr(buf, "HTTP/1.1 200") || strstr(buf, "HTTP/2 200")) {
            *out_status = 200;
        }

        if (strstr(buf, "Blocked from HathiTrust") || strstr(buf, "turnstile") || strstr(buf, "Cloudflare") || strstr(buf, "<!doctype html>")) {
            *out_turnstile = true;
        }
        if (strstr(buf, "Content-Type: text/plain") && !(*out_turnstile)) {
            *out_binary = true;
        }
    }

    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sockfd);
}

int main(void) {
    printf("=== EXECUTING ALGOL 61 PROVER OVER LIVE HATHITRUST CLI INTERCEPTION ===\n\n");

    // 1. Telemetry input queried LIVE over real OpenSSL socket to babel.hathitrust.org
    int status_code = 0;
    bool has_turnstile_marker = false;
    bool has_binary_facsimile = false;

    printf("1. Probing Live HathiTrust Endpoint (wu.89099856478, Seq 13)...\n");
    probe_live_hathitrust_endpoint("wu.89099856478", 13, &status_code, &has_turnstile_marker, &has_binary_facsimile);

    printf("   [Live Telemetry Ingested]\n");
    printf("   HTTP Response Status:      %d\n", status_code);
    printf("   Cloudflare Challenge Flag: %s\n", has_turnstile_marker ? "TRUE" : "FALSE");
    printf("   Binary Image Stream:       %s\n\n", has_binary_facsimile ? "TRUE" : "FALSE");

    assert(status_code != 0);

    // 2. Execute the formal Algol 61 prover procedure
    Algol61ChanceryRuling ruling = execute_algol61_chancery_prover(status_code, has_turnstile_marker, has_binary_facsimile);
    printf("2. Formal ALGOL 61 Procedure Output: ");
    if (ruling == UNAUTHORIZED_BLOCK) {
        printf("RULING = 1 [UNAUTHORIZED_BLOCK]\n");
    } else {
        printf("RULING = %d [CHANCERY CONTINUANCE]\n", ruling);
    }
    assert(ruling == UNAUTHORIZED_BLOCK);

    // 3. Dispatch the Press Scoop via Evelyn Waugh Chancery Protocol
    printf("\n3. Dispatching Special Correspondent Cable (The Daily Beast / William Boot)...\n");
    WaughChanceryState chancery;
    tsfi_waugh_chancery_init(&chancery);

    bool dispatched = tsfi_waugh_dispatch_cable(
        &chancery,
        WAUGH_DISPATCH_PRESS_CABLE,
        "HathiTrust Live Edge Gateway",
        "The Daily Beast (London Desk)",
        "LIVE SCOOP: Endpoint Interception Verified as UNAUTHORIZED_BLOCK via Live Network Probe",
        2026
    );
    assert(dispatched);

    // 4. Generate Formal Chancery Audit & Scoop Report
    char scoop_report[2048];
    uint64_t proof = tsfi_waugh_chancery_audit(&chancery, scoop_report, sizeof(scoop_report));
    assert(proof != 0);

    printf("\n%s\n", scoop_report);
    printf("Latest Cable Headline: \"%s\"\n", chancery.cables[chancery.cable_count - 1].content_headline);
    printf("Transmission ID:        Cable #%u\n", chancery.cables[chancery.cable_count - 1].cable_id);
    printf("\n=== CHANCERY RESOLVED ON RECORD VIA LIVE NETWORK VERIFICATION ===\n");

    return 0;
}
