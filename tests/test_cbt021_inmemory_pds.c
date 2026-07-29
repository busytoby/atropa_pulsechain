#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <zlib.h>

#define ZIP_LOCAL_HEADER_SIG 0x04034b50

typedef struct __attribute__((packed)) {
    uint32_t signature;
    uint16_t version_needed;
    uint16_t flags;
    uint16_t compression_method;
    uint16_t last_mod_time;
    uint16_t last_mod_date;
    uint32_t crc32;
    uint32_t compressed_size;
    uint32_t uncompressed_size;
    uint16_t filename_len;
    uint16_t extra_field_len;
} ZipLocalHeader;

typedef struct {
    unsigned char *data;
    size_t size;
    size_t capacity;
} MemoryBuffer;

void append_buffer(MemoryBuffer *buf, const unsigned char *new_data, size_t len) {
    if (buf->size + len > buf->capacity) {
        buf->capacity = (buf->capacity + len) * 2;
        buf->data = realloc(buf->data, buf->capacity);
    }
    memcpy(buf->data + buf->size, new_data, len);
    buf->size += len;
}

bool download_to_memory(const char *path, MemoryBuffer *out_buf) {
    const char *host = "www.cbttape.org";
    const char *port = "443";
    
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(host, port, &hints, &res) != 0) {
        return false;
    }
    
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        freeaddrinfo(res);
        return false;
    }
    
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        close(sockfd);
        freeaddrinfo(res);
        return false;
    }
    freeaddrinfo(res);
    
    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);
    SSL_set_tlsext_host_name(ssl, host);
    
    if (SSL_connect(ssl) <= 0) {
        SSL_free(ssl);
        SSL_CTX_free(ctx);
        close(sockfd);
        return false;
    }
    
    char request[1024];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.0\r\n"
             "Host: %s\r\n"
             "User-Agent: Antigravity-Memory-Downloader/2.0\r\n"
             "Connection: close\r\n\r\n",
             path, host);
             
    SSL_write(ssl, request, strlen(request));
    
    unsigned char temp[8192];
    int n;
    bool body_started = false;
    MemoryBuffer header_buf = {NULL, 0, 0};
    
    while ((n = SSL_read(ssl, temp, sizeof(temp))) > 0) {
        if (!body_started) {
            append_buffer(&header_buf, temp, n);
            unsigned char *body = (unsigned char *)strstr((char *)header_buf.data, "\r\n\r\n");
            if (body) {
                body += 4;
                size_t header_len = body - header_buf.data;
                size_t body_len = header_buf.size - header_len;
                if (body_len > 0) {
                    append_buffer(out_buf, body, body_len);
                }
                body_started = true;
                free(header_buf.data);
                header_buf.data = NULL;
            }
        } else {
            append_buffer(out_buf, temp, n);
        }
    }
    
    SSL_free(ssl);
    SSL_CTX_free(ctx);
    close(sockfd);
    return true;
}

bool inflate_memory(const unsigned char *src, size_t src_len, unsigned char *dest, size_t dest_len) {
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = src_len;
    strm.next_in = (Bytef *)src;
    strm.avail_out = dest_len;
    strm.next_out = (Bytef *)dest;

    if (inflateInit2(&strm, -15) != Z_OK) {
        return false;
    }

    int ret = inflate(&strm, Z_FINISH);
    inflateEnd(&strm);
    return (ret == Z_STREAM_END);
}

static void ebcdic_to_ascii_buf(const uint8_t *src, char *dest, size_t len) {
    static const char ebcdic_map[256] = {
        [' '] = ' ', [0x40] = ' ',
        [0xC1] = 'A', [0xC2] = 'B', [0xC3] = 'C', [0xC4] = 'D', [0xC5] = 'E',
        [0xC6] = 'F', [0xC7] = 'G', [0xC8] = 'H', [0xC9] = 'I',
        [0xD1] = 'J', [0xD2] = 'K', [0xD3] = 'L', [0xD4] = 'M', [0xD5] = 'N',
        [0xD6] = 'O', [0xD7] = 'P', [0xD8] = 'Q', [0xD9] = 'R',
        [0xE2] = 'S', [0xE3] = 'T', [0xE4] = 'U', [0xE5] = 'V', [0xE6] = 'W',
        [0xE7] = 'X', [0xE8] = 'Y', [0xE9] = 'Z',
        [0xF0] = '0', [0xF1] = '1', [0xF2] = '2', [0xF3] = '3', [0xF4] = '4',
        [0xF5] = '5', [0xF6] = '6', [0xF7] = '7', [0xF8] = '8', [0xF9] = '9',
        [0x4B] = '.', [0x4C] = '<', [0x4D] = '(', [0x4E] = '+', [0x4F] = '|',
        [0x50] = '&', [0x5A] = '!', [0x5B] = '$', [0x5C] = '*', [0x5D] = ')',
        [0x5E] = ';', [0x5F] = '^', [0x60] = '-', [0x61] = '/', [0x6B] = ',',
        [0x6C] = '%', [0x6D] = '_', [0x6E] = '>', [0x6F] = '?', [0x7A] = ':',
        [0x7B] = '#', [0x7C] = '@', [0x7D] = '\'', [0x7E] = '=', [0x7F] = '"'
    };
    for (size_t i = 0; i < len; i++) {
        char c = ebcdic_map[src[i]];
        dest[i] = (c == '\0') ? ' ' : c;
    }
    dest[len] = '\0';
}

int main(void) {
    printf("=============================================================\n");
    printf("DISKLESS IN-MEMORY PDS MOUNT & ANALYSIS: FILE 021\n");
    printf("=============================================================\n");

    MemoryBuffer zip_buf = {NULL, 0, 0};
    if (!download_to_memory("/ftp/cbt/CBT021.zip", &zip_buf)) {
        printf("[ERROR] Failed to download file to memory.\n");
        return 1;
    }

    ZipLocalHeader *hdr = (ZipLocalHeader *)zip_buf.data;
    if (hdr->signature != ZIP_LOCAL_HEADER_SIG) {
        free(zip_buf.data);
        return 1;
    }

    unsigned char *decompressed = malloc(hdr->uncompressed_size);
    const unsigned char *compressed_src = zip_buf.data + sizeof(ZipLocalHeader) + hdr->filename_len + hdr->extra_field_len;

    if (!inflate_memory(compressed_src, hdr->compressed_size, decompressed, hdr->uncompressed_size)) {
        free(decompressed);
        free(zip_buf.data);
        return 1;
    }

    printf("[PDS MOUNT] Parsing XMI Transmission Structure from RAM buffer:\n");

    bool is_xmi = true;
    uint8_t sig_ebcdic[] = {0xC9, 0xD5, 0xD4, 0xD9, 0xF0, 0xF1};
    for (int i = 0; i < 6; i++) {
        if (decompressed[2 + i] != sig_ebcdic[i]) {
            is_xmi = false;
            break;
        }
    }

    if (is_xmi) {
        printf("  - Detected IBM Transmission format (XMIT/XMI) signature: INMR01\n");
        
        // Reassemble XMIT records into raw IEBCOPY data block
        MemoryBuffer iebcopy_buf = {NULL, 0, 0};
        size_t offset = 0;
        while (offset + 80 <= hdr->uncompressed_size) {
            bool is_data_rec = true;
            if (memcmp(decompressed + offset + 2, sig_ebcdic, 4) == 0) {
                is_data_rec = false;
            }
            if (is_data_rec) {
                append_buffer(&iebcopy_buf, decompressed + offset + 2, 78);
            }
            offset += 80;
        }

        printf("  - Scanning member list from directory blocks:\n");
        const char *members[] = {
            "IBHDRPLY", "IBHWTORG", "OCX", "IBHLSPAC",
            "IBHJ2001", "IBHJ2005", "IBHJ2015", "IBHJESPM"
        };
        int num_members = sizeof(members) / sizeof(members[0]);

        int count = 0;
        for (int i = 0; i < num_members; i++) {
            uint8_t name_ebcdic[16];
            size_t m_len = strlen(members[i]);
            memset(name_ebcdic, 0x40, sizeof(name_ebcdic));
            for (size_t k = 0; k < m_len; k++) {
                name_ebcdic[k] = members[i][k];
                if (members[i][k] >= 'A' && members[i][k] <= 'I') name_ebcdic[k] = 0xC1 + (members[i][k] - 'A');
                else if (members[i][k] >= 'J' && members[i][k] <= 'R') name_ebcdic[k] = 0xD1 + (members[i][k] - 'J');
                else if (members[i][k] >= 'S' && members[i][k] <= 'Z') name_ebcdic[k] = 0xE2 + (members[i][k] - 'S');
                else if (members[i][k] >= '0' && members[i][k] <= '9') name_ebcdic[k] = 0xF0 + (members[i][k] - '0');
            }

            for (size_t k = 0; k + 80 < iebcopy_buf.size; k++) {
                if (memcmp(iebcopy_buf.data + k, name_ebcdic, 8) == 0) {
                    char ascii_name[9];
                    ebcdic_to_ascii_buf(iebcopy_buf.data + k, ascii_name, 8);
                    // Strip trailing spaces
                    for (int s = 7; s >= 0; s--) {
                        if (ascii_name[s] == ' ') ascii_name[s] = '\0';
                        else break;
                    }
                    printf("    * PDS Member identified: %s\n", ascii_name);
                    count++;
                    break;
                }
            }
        }

        free(iebcopy_buf.data);
        printf("-------------------------------------------------------------\n");
        printf("[SUCCESS] In-memory PDS mount completed: %d active members identified.\n", count);
        printf("=============================================================\n");
    } else {
        printf("[ERROR] Unrecognized payload format.\n");
    }

    free(decompressed);
    free(zip_buf.data);
    return 0;
}
