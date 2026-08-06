#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>

#define BUFFER_SIZE 8192

bool stream_qingting(const char *channel_path, const char *out_filename) {
    const char *host = "lhttp.qingting.fm";
    const char *port = "80";
    
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    if (getaddrinfo(host, port, &hints, &res) != 0) {
        fprintf(stderr, "[ERROR] getaddrinfo failed to resolve host %s\n", host);
        return false;
    }
    
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        fprintf(stderr, "[ERROR] socket creation failed\n");
        freeaddrinfo(res);
        return false;
    }
    
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
        fprintf(stderr, "[ERROR] connect to %s failed\n", host);
        close(sockfd);
        freeaddrinfo(res);
        return false;
    }
    freeaddrinfo(res);
    
    char request[1024];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.0\r\n"
             "Host: %s\r\n"
             "User-Agent: Antigravity/2.0\r\n"
             "Connection: close\r\n\r\n",
             channel_path, host);
             
    if (write(sockfd, request, strlen(request)) <= 0) {
        fprintf(stderr, "[ERROR] failed to write request header\n");
        close(sockfd);
        return false;
    }
    
    FILE *fp = NULL;
    if (strcmp(out_filename, "-") == 0) {
        fp = stdout;
    } else {
        fp = fopen(out_filename, "wb");
    }
    
    if (!fp) {
        fprintf(stderr, "[ERROR] could not open output destination %s\n", out_filename);
        close(sockfd);
        return false;
    }
    
    char temp[BUFFER_SIZE];
    int n;
    bool body_started = false;
    char header_buf[16384] = "";
    int header_len = 0;
    
    while ((n = read(sockfd, temp, sizeof(temp))) > 0) {
        if (!body_started) {
            if (header_len + n < (int)sizeof(header_buf)) {
                memcpy(header_buf + header_len, temp, n);
                header_len += n;
                header_buf[header_len] = '\0';
                
                char *body = strstr(header_buf, "\r\n\r\n");
                if (body) {
                    body += 4;
                    int body_offset = body - header_buf;
                    int write_bytes = header_len - body_offset;
                    if (write_bytes > 0) {
                        fwrite(body, 1, write_bytes, fp);
                    }
                    body_started = true;
                }
            } else {
                body_started = true;
                fwrite(temp, 1, n, fp);
            }
        } else {
            fwrite(temp, 1, n, fp);
            fflush(fp);
        }
    }
    
    if (fp != stdout) {
        fclose(fp);
    }
    close(sockfd);
    return true;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Qingting FM C Streamer\n");
        printf("Usage: %s <channel_path> <output_file_or_dash>\n", argv[0]);
        printf("Example (Save stream): %s /live/4973/64k.mp3 radio_out.mp3\n", argv[0]);
        printf("Example (Stdout pipe): %s /live/4973/64k.mp3 - | mpv -\n", argv[0]);
        return 1;
    }
    
    if (stream_qingting(argv[1], argv[2])) {
        return 0;
    } else {
        return 1;
    }
}
