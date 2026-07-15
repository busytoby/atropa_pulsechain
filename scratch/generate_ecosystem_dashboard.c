#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <stdint.h>

#define MAX_DISCOVERED_TOKENS 512
#define MAX_SWAP_EDGES 1024

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <dirent.h>

bool query_mcp(const char *payload, char *out_buf, size_t out_max) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return false;
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(10042);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sock);
        return false;
    }
    
    send(sock, payload, strlen(payload), 0);
    
    size_t total = 0;
    while (total < out_max - 1) {
        ssize_t n = recv(sock, out_buf + total, out_max - 1 - total, 0);
        if (n <= 0) break;
        total += n;
    }
    out_buf[total] = '\0';
    close(sock);
    return (total > 0);
}

void escape_json_str(char *dest, const char *src) {
    while (*src) {
        if (*src == '\\' && *(src + 1) == 'u') {
            // Keep \uXXXX unicode escape sequences intact
            *dest++ = *src++; // copy '\'
            *dest++ = *src++; // copy 'u'
            for (int i = 0; i < 4 && *src; i++) {
                *dest++ = *src++;
            }
            continue;
        }
        if (*src == '"') {
            *dest++ = '\\';
            *dest++ = '"';
        } else if (*src == '\\') {
            *dest++ = '\\';
            *dest++ = '\\';
        } else if (*src == '\n') {
            *dest++ = '\\';
            *dest++ = 'n';
        } else if (*src == '\r') {
            *dest++ = '\\';
            *dest++ = 'r';
        } else if ((unsigned char)*src >= 32) {
            *dest++ = *src;
        }
        src++;
    }
    *dest = '\0';
}

typedef struct {
    char address[64];
    char symbol[64];
    char name[128];
    uint64_t decimals;
    double price_pls;
    uint64_t last_update;
} RdbmsTokenRow;

typedef struct {
    uint32_t active;
    uint32_t col_count;
    uint32_t count;
    uint32_t capacity;
    RdbmsTokenRow rows[MAX_DISCOVERED_TOKENS];
} LauRdbmsTable;

typedef struct {
    char token0[64];
    char token1[64];
    double price;
} SwapEdge;

const char *find_json_field(const char *json, const char *key) {
    const char *ptr = strstr(json, key);
    if (!ptr) return NULL;
    ptr += strlen(key);
    while (*ptr == ' ' || *ptr == ':' || *ptr == '"' || *ptr == '{' || *ptr == '[') {
        ptr++;
    }
    return ptr;
}

void rehydrate_database_from_file_cache(LauRdbmsTable *rdbms, SwapEdge *edges, int *edge_count) {
    DIR *dir = opendir("/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tmp");
    if (!dir) return;
    
    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if ((strstr(entry->d_name, "dex_pairs_cache_") || strstr(entry->d_name, "dex_cache_")) && strstr(entry->d_name, ".json")) {
            // Check if file is price cache (usually very small, < 50 bytes) or pair cache
            char filepath[512];
            snprintf(filepath, sizeof(filepath), "/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tmp/%s", entry->d_name);
            FILE *f = fopen(filepath, "r");
            if (f) {
                fseek(f, 0, SEEK_END);
                long sz = ftell(f);
                fseek(f, 0, SEEK_SET);
                if (sz < 50) {
                    fclose(f);
                    continue; // Skip single price caches
                }
                
                char *json = malloc(sz + 1);
                if (json) {
                    size_t nr = fread(json, 1, sz, f);
                    json[nr] = '\0';
                    
                    const char *ptr = json;
                    while ((ptr = strstr(ptr, "\"pairAddress\""))) {
                        const char *pair_val = find_json_field(ptr, "\"pairAddress\"");
                        if (!pair_val) break;
                        
                        char pair_addr[64] = {0};
                        const char *end = strchr(pair_val, '"');
                        if (end && (end - pair_val) < 63) {
                            strncpy(pair_addr, pair_val, end - pair_val);
                        }
                        
                        char base_addr[64] = {0};
                        char base_sym[64] = {0};
                        char base_name[128] = {0};
                        const char *base_ptr = strstr(pair_val, "\"baseToken\"");
                        if (base_ptr) {
                            const char *addr_ptr = find_json_field(base_ptr, "\"address\"");
                            if (addr_ptr) {
                                const char *addr_end = strchr(addr_ptr, '"');
                                if (addr_end && (addr_end - addr_ptr) < 63) {
                                    strncpy(base_addr, addr_ptr, addr_end - addr_ptr);
                                }
                            }
                            const char *sym_ptr = find_json_field(base_ptr, "\"symbol\"");
                            if (sym_ptr) {
                                const char *sym_end = strchr(sym_ptr, '"');
                                if (sym_end && (sym_end - sym_ptr) < 63) {
                                    strncpy(base_sym, sym_ptr, sym_end - sym_ptr);
                                }
                            }
                            const char *name_ptr = find_json_field(base_ptr, "\"name\"");
                            if (name_ptr) {
                                const char *name_end = strchr(name_ptr, '"');
                                if (name_end && (name_end - name_ptr) < 127) {
                                    strncpy(base_name, name_ptr, name_end - name_ptr);
                                }
                            }
                        }
                        
                        char quote_addr[64] = {0};
                        char quote_sym[64] = {0};
                        char quote_name[128] = {0};
                        const char *quote_ptr = strstr(pair_val, "\"quoteToken\"");
                        if (quote_ptr) {
                            const char *addr_ptr = find_json_field(quote_ptr, "\"address\"");
                            if (addr_ptr) {
                                const char *addr_end = strchr(addr_ptr, '"');
                                if (addr_end && (addr_end - addr_ptr) < 63) {
                                    strncpy(quote_addr, addr_ptr, addr_end - addr_ptr);
                                }
                            }
                            const char *sym_ptr = find_json_field(quote_ptr, "\"symbol\"");
                            if (sym_ptr) {
                                const char *sym_end = strchr(sym_ptr, '"');
                                if (sym_end && (sym_end - sym_ptr) < 63) {
                                    strncpy(quote_sym, sym_ptr, sym_end - sym_ptr);
                                }
                            }
                            const char *name_ptr = find_json_field(quote_ptr, "\"name\"");
                            if (name_ptr) {
                                const char *name_end = strchr(name_ptr, '"');
                                if (name_end && (name_end - name_ptr) < 127) {
                                    strncpy(quote_name, name_ptr, name_end - name_ptr);
                                }
                            }
                        }
                        
                        double price_native = 0.0;
                        const char *price_ptr = find_json_field(pair_val, "\"priceNative\"");
                        if (price_ptr) {
                            price_native = atof(price_ptr);
                        }
                        
                        if (base_addr[0] && quote_addr[0]) {
                            printf("[PARSER] Parsed pair: %s <-> %s (price: %f)\n", base_addr, quote_addr, price_native);
                            bool base_found = false;
                            for (uint32_t k = 0; k < rdbms->count; k++) {
                                if (strcasecmp(rdbms->rows[k].address, base_addr) == 0) {
                                    base_found = true;
                                    break;
                                }
                            }
                            if (!base_found && rdbms->count < MAX_DISCOVERED_TOKENS) {
                                RdbmsTokenRow *r = &rdbms->rows[rdbms->count++];
                                strcpy(r->address, base_addr);
                                strcpy(r->symbol, base_sym);
                                strcpy(r->name, base_name);
                                r->decimals = 18;
                                r->price_pls = price_native;
                                r->last_update = 1;
                            }
                            
                            bool quote_found = false;
                            for (uint32_t k = 0; k < rdbms->count; k++) {
                                if (strcasecmp(rdbms->rows[k].address, quote_addr) == 0) {
                                    quote_found = true;
                                    break;
                                }
                            }
                            if (!quote_found && rdbms->count < MAX_DISCOVERED_TOKENS) {
                                RdbmsTokenRow *r = &rdbms->rows[rdbms->count++];
                                strcpy(r->address, quote_addr);
                                strcpy(r->symbol, quote_sym);
                                strcpy(r->name, quote_name);
                                r->decimals = 18;
                                r->price_pls = 1.0;
                                r->last_update = 1;
                            }
                            
                            bool edge_found = false;
                            for (int k = 0; k < *edge_count; k++) {
                                if ((strcasecmp(edges[k].token0, base_addr) == 0 && strcasecmp(edges[k].token1, quote_addr) == 0) ||
                                    (strcasecmp(edges[k].token0, quote_addr) == 0 && strcasecmp(edges[k].token1, base_addr) == 0)) {
                                    edge_found = true;
                                    break;
                                }
                            }
                            if (!edge_found && *edge_count < MAX_SWAP_EDGES) {
                                SwapEdge *e = &edges[(*edge_count)++];
                                strcpy(e->token0, base_addr);
                                strcpy(e->token1, quote_addr);
                                e->price = price_native;
                            }
                        }
                        ptr = end;
                    }
                    free(json);
                }
                fclose(f);
            }
        }
    }
    closedir(dir);
}

int main() {
    printf("[DASHBOARD] Parsing solidity/addresses.sol to map constant names...\n");
    FILE *f_sol = fopen("solidity/addresses.sol", "r");
    if (!f_sol) {
        f_sol = fopen("../solidity/addresses.sol", "r");
    }
    
    char line[512];
    char addresses[256][64];
    char var_names[256][128];
    int addr_count = 0;
    
    if (f_sol) {
        while (fgets(line, sizeof(line), f_sol)) {
            char *addr_start = strstr(line, "address(0x");
            if (addr_start) {
                addr_start += 8;
                char *addr_end = strchr(addr_start, ')');
                if (addr_end && (addr_end - addr_start) == 42) {
                    char var_name[128] = "Unknown";
                    char *const_ptr = strstr(line, "constant ");
                    if (const_ptr) {
                        const_ptr += 9;
                        char *space_ptr = strchr(const_ptr, ' ');
                        if (space_ptr) {
                            size_t len = space_ptr - const_ptr;
                            if (len < sizeof(var_name)) {
                                strncpy(var_name, const_ptr, len);
                                var_name[len] = '\0';
                            }
                        }
                    }
                    
                    char clean_addr[64] = "0x";
                    strncat(clean_addr, addr_start + 2, 40);
                    
                    bool dup = false;
                    for (int i = 0; i < addr_count; i++) {
                        if (strcasecmp(addresses[i], clean_addr) == 0) {
                            dup = true;
                            break;
                        }
                    }
                    if (!dup && addr_count < 256) {
                        strcpy(addresses[addr_count], clean_addr);
                        strcpy(var_names[addr_count], var_name);
                        addr_count++;
                    }
                }
            }
        }
        fclose(f_sol);
    }
    
    const char *meta_path = "/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/assets/contract_metadata.dat.bin";
    const char *edges_path = "/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/assets/swap_edges.dat.bin";
    
    LauRdbmsTable rdbms;
    memset(&rdbms, 0, sizeof(rdbms));
    FILE *f_meta = fopen(meta_path, "rb");
    if (f_meta) {
        fread(&rdbms, sizeof(rdbms), 1, f_meta);
        fclose(f_meta);
    }
    
    SwapEdge edges[MAX_SWAP_EDGES];
    int edge_count = 0;
    FILE *f_edges = fopen(edges_path, "rb");
    if (f_edges) {
        uint32_t count = 0;
        if (fread(&count, sizeof(count), 1, f_edges) == 1) {
            edge_count = (int)count;
            if (edge_count > MAX_SWAP_EDGES) edge_count = MAX_SWAP_EDGES;
            fread(edges, sizeof(SwapEdge), edge_count, f_edges);
        }
        fclose(f_edges);
    }
    printf("[DASHBOARD] Loaded %d tokens and %d edges from binary databases.\n", rdbms.count, edge_count);
    
    printf("[DASHBOARD] Rehydrating local binary databases from JSON caches...\n");
    rehydrate_database_from_file_cache(&rdbms, edges, &edge_count);
    
    printf("[DASHBOARD] Rehydrated database: %d tokens and %d edges.\n", rdbms.count, edge_count);
    
    // Write back updated databases to disk
    FILE *f_meta_w = fopen(meta_path, "wb");
    if (f_meta_w) {
        fwrite(&rdbms, sizeof(rdbms), 1, f_meta_w);
        fclose(f_meta_w);
    }
    FILE *f_edges_w = fopen(edges_path, "wb");
    if (f_edges_w) {
        uint32_t count = (uint32_t)edge_count;
        fwrite(&count, sizeof(count), 1, f_edges_w);
        fwrite(edges, sizeof(SwapEdge), edge_count, f_edges_w);
        fclose(f_edges_w);
    }
    
    const char *out_path = "/home/mariarahel/.gemini/antigravity-cli/brain/77cd3e01-149d-4c76-ab63-d64bf3f503e6/atropa_dashboard.html";
    FILE *f_html = fopen(out_path, "w");
    if (!f_html) {
        return 1;
    }
    
    fprintf(f_html, "<!DOCTYPE html>\n<html>\n<head>\n");
    fprintf(f_html, "<meta charset=\"utf-8\">\n");
    fprintf(f_html, "<title>Atropa Ecosystem Knowledge Graph</title>\n");
    fprintf(f_html, "<link href=\"https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;600;700&display=swap\" rel=\"stylesheet\">\n");
    
    fprintf(f_html, "<style>\n");
    fprintf(f_html, "  * { box-sizing: border-box; margin: 0; padding: 0; font-family: 'Outfit', sans-serif; }\n");
    fprintf(f_html, "  body { background: radial-gradient(circle at center, #11141a 0%%, #08090d 100%%); color: #f0f2f5; overflow: hidden; height: 100vh; display: flex; }\n");
    fprintf(f_html, "  #sidebar { width: 380px; background: rgba(13, 16, 22, 0.9); backdrop-filter: blur(16px); border-right: 1px solid rgba(255,255,255,0.08); display: flex; flex-direction: column; padding: 24px; z-index: 10; overflow-y: auto; }\n");
    fprintf(f_html, "  #network-container { flex: 1; height: 100%%; position: relative; }\n");
    fprintf(f_html, "  canvas { width: 100%%; height: 100%%; display: block; background: transparent; cursor: grab; }\n");
    fprintf(f_html, "  canvas:active { cursor: grabbing; }\n");
    fprintf(f_html, "  h1 { font-size: 22px; font-weight: 700; background: linear-gradient(135deg, #00f2fe 0%%, #4facfe 100%%); -webkit-background-clip: text; -webkit-text-fill-color: transparent; margin-bottom: 8px; }\n");
    fprintf(f_html, "  .sub { font-size: 13px; color: #8a92a6; margin-bottom: 24px; }\n");
    fprintf(f_html, "  .card { background: rgba(255,255,255,0.03); border: 1px solid rgba(255,255,255,0.05); border-radius: 12px; padding: 16px; margin-bottom: 16px; }\n");
    fprintf(f_html, "  .card-title { font-size: 11px; text-transform: uppercase; color: #64748b; letter-spacing: 1px; margin-bottom: 8px; font-weight: 600; }\n");
    fprintf(f_html, "  .info-val { font-size: 15px; font-weight: 500; word-break: break-all; color: #e2e8f0; }\n");
    fprintf(f_html, "  .details-box { display: none; }\n");
    fprintf(f_html, "  #search-box { width: 100%%; background: rgba(255,255,255,0.05); border: 1px solid rgba(255,255,255,0.1); border-radius: 8px; padding: 10px; color: #fff; margin-bottom: 16px; font-size: 14px; outline: none; }\n");
    fprintf(f_html, "  #search-box:focus { border-color: #00f2fe; }\n");
    fprintf(f_html, "  .pool-row { display: flex; justify-content: space-between; font-size: 13px; padding: 6px 0; border-bottom: 1px solid rgba(255,255,255,0.05); }\n");
    fprintf(f_html, "  .view-btn { width: 100%%; text-align: left; background: rgba(255,255,255,0.03); border: 1px solid rgba(255,255,255,0.05); border-radius: 6px; padding: 8px 12px; color: #8a92a6; margin-bottom: 8px; cursor: pointer; transition: all 0.2s; font-size: 13px; outline: none; }\n");
    fprintf(f_html, "  .view-btn:hover { background: rgba(255,255,255,0.08); color: #fff; }\n");
    fprintf(f_html, "  .view-btn.active { background: linear-gradient(135deg, rgba(0,242,254,0.1) 0%%, rgba(79,172,254,0.1) 100%%); border-color: #00f2fe; color: #00f2fe; }\n");
    fprintf(f_html, "</style>\n</head>\n<body>\n");
    
    fprintf(f_html, "<div id=\"sidebar\">\n");
    fprintf(f_html, "  <h1>Atropa Ecosystem</h1>\n");
    fprintf(f_html, "  <div class=\"sub\">Interactive Knowledge Graph of LP Connections</div>\n");
    fprintf(f_html, "  <input type=\"text\" id=\"search-box\" placeholder=\"Search Token / Address...\" oninput=\"searchNode()\">\n");
    
    fprintf(f_html, "  <div class=\"card\" id=\"general-stats\">\n");
    fprintf(f_html, "    <div class=\"card-title\">Ecosystem Totals</div>\n");
    fprintf(f_html, "    <div class=\"pool-row\"><span>Known Tokens</span><strong>%d</strong></div>\n", rdbms.count);
    fprintf(f_html, "    <div class=\"pool-row\"><span>Active LP Connections</span><strong>%d</strong></div>\n", edge_count);
    fprintf(f_html, "  </div>\n");
    fprintf(f_html, "  <div class=\"card\" id=\"views-box\">\n");
    fprintf(f_html, "    <div class=\"card-title\">Projection View Styles</div>\n");
    fprintf(f_html, "    <button class=\"view-btn active\" onclick=\"setViewMode('force')\">Force Spring</button>\n");
    fprintf(f_html, "    <button class=\"view-btn\" onclick=\"setViewMode('seed')\">Seed (Circular)</button>\n");
    fprintf(f_html, "    <button class=\"view-btn\" onclick=\"setViewMode('tune')\">Tune (Frequency)</button>\n");
    fprintf(f_html, "    <button class=\"view-btn\" onclick=\"setViewMode('conify')\">Conify (Depth)</button>\n");
    fprintf(f_html, "    <button class=\"view-btn\" onclick=\"setViewMode('bond')\">Bond (Lissajous)</button>\n");
    fprintf(f_html, "    <button class=\"view-btn\" onclick=\"setViewMode('delegate')\">Delegate (Hypotrochoid)</button>\n");
    fprintf(f_html, "  </div>\n");
    
    fprintf(f_html, "  <div class=\"details-box\" id=\"details\">\n");
    fprintf(f_html, "    <div class=\"card\">\n");
    fprintf(f_html, "      <div class=\"card-title\">Selected Token</div>\n");
    fprintf(f_html, "      <div class=\"info-val\" id=\"det-name\" style=\"font-size:18px; font-weight:700; color:#00f2fe;\"></div>\n");
    fprintf(f_html, "      <div class=\"info-val\" id=\"det-sym\" style=\"margin-bottom:8px;\"></div>\n");
    fprintf(f_html, "      <div class=\"card-title\" style=\"margin-top:12px;\">Address</div>\n");
    fprintf(f_html, "      <div class=\"info-val\" id=\"det-addr\" style=\"font-size:11px; font-family:monospace;\"></div>\n");
    fprintf(f_html, "    </div>\n");
    fprintf(f_html, "    <div class=\"card\">\n");
    fprintf(f_html, "      <div class=\"card-title\">LP Connections</div>\n");
    fprintf(f_html, "      <div id=\"det-pools\"></div>\n");
    fprintf(f_html, "    </div>\n");
    fprintf(f_html, "  </div>\n");
    fprintf(f_html, "</div>\n");
    
    fprintf(f_html, "<div id=\"network-container\">\n  <canvas id=\"graph-canvas\"></canvas>\n</div>\n");
    
    fprintf(f_html, "<script>\n");
    fprintf(f_html, "  const rawNodes = [\n");
    for (uint32_t i = 0; i < rdbms.count; i++) {
        const char *var_name = "";
        bool found = false;
        for (int k = 0; k < addr_count; k++) {
            if (strcasecmp(addresses[k], rdbms.rows[i].address) == 0) {
                var_name = var_names[k];
                found = true;
                break;
            }
        }
        if (strcasecmp(rdbms.rows[i].address, "0xA1077a294dDe1B09bB078844df40758a5D0f9a27") == 0) {
            found = true;
            var_name = "WrappedPulse";
        }
        
        if (!found) continue;
        char escaped_sym[128] = {0};
        char escaped_name[256] = {0};
        escape_json_str(escaped_sym, strlen(rdbms.rows[i].symbol) > 0 ? rdbms.rows[i].symbol : "UNKNOWN");
        escape_json_str(escaped_name, rdbms.rows[i].name);
        
        fprintf(f_html, "    { id: \"%s\", label: \"%s\", title: \"%s\", symbol: \"%s\", name: \"%s\", varName: \"%s\" },\n",
                rdbms.rows[i].address, escaped_sym, escaped_name, escaped_sym, escaped_name, var_name);
    }
    fprintf(f_html, "  ];\n\n");
    
    fprintf(f_html, "  const rawEdges = [\n");
    for (int i = 0; i < edge_count; i++) {
        fprintf(f_html, "    { from: \"%s\", to: \"%s\", price: %.8f },\n",
                edges[i].token0, edges[i].token1, edges[i].price);
    }
    fprintf(f_html, "  ];\n\n");
    
    // Custom Force Directed Simulation JS Engine
    fprintf(f_html, "  const activeNodeIds = new Set();\n");
    fprintf(f_html, "  rawEdges.forEach(e => { activeNodeIds.add(e.from.toLowerCase()); activeNodeIds.add(e.to.toLowerCase()); });\n\n");
    
    fprintf(f_html, "  const nodes = [];\n");
    fprintf(f_html, "  const nodesMap = {};\n");
    fprintf(f_html, "  rawNodes.forEach(n => {\n");
    fprintf(f_html, "    if (activeNodeIds.has(n.id.toLowerCase()) || n.id.toLowerCase() === '0xcc78a0acdf847a2c1714d2a925bb4477df5d48a6') {\n");
    fprintf(f_html, "      const isAtropa = n.id.toLowerCase() === '0xcc78a0acdf847a2c1714d2a925bb4477df5d48a6';\n");
    fprintf(f_html, "      const isdOWN = n.id.toLowerCase() === '0x2556f7f8d82ebcdd7b821b0981c38d9da9439cdd';\n");
    fprintf(f_html, "      const node = {\n");
    fprintf(f_html, "        id: n.id,\n");
    fprintf(f_html, "        label: n.label,\n");
    fprintf(f_html, "        symbol: n.symbol,\n");
    fprintf(f_html, "        name: n.name,\n");
    fprintf(f_html, "        varName: n.varName,\n");
    fprintf(f_html, "        x: Math.random() * 800 - 400,\n");
    fprintf(f_html, "        y: Math.random() * 800 - 400,\n");
    fprintf(f_html, "        vx: 0, vy: 0,\n");
    fprintf(f_html, "        radius: isAtropa ? 22 : (isdOWN ? 16 : 12),\n");
    fprintf(f_html, "        color: isAtropa ? '#ff0055' : (isdOWN ? '#00f2fe' : '#4facfe')\n");
    fprintf(f_html, "      };\n");
    fprintf(f_html, "      nodes.push(node);\n");
    fprintf(f_html, "      nodesMap[n.id.toLowerCase()] = node;\n");
    fprintf(f_html, "    }\n");
    fprintf(f_html, "  });\n\n");
    
    fprintf(f_html, "  const links = [];\n");
    fprintf(f_html, "  rawEdges.forEach(e => {\n");
    fprintf(f_html, "    const source = nodesMap[e.from.toLowerCase()];\n");
    fprintf(f_html, "    const target = nodesMap[e.to.toLowerCase()];\n");
    fprintf(f_html, "    if (source && target) {\n");
    fprintf(f_html, "      links.push({ source, target, price: e.price });\n");
    fprintf(f_html, "    }\n");
    fprintf(f_html, "  });\n\n");
    
    fprintf(f_html, "  const canvas = document.getElementById('graph-canvas');\n");
    fprintf(f_html, "  const ctx = canvas.getContext('2d');\n");
    fprintf(f_html, "  let width = canvas.width = canvas.offsetWidth;\n");
    fprintf(f_html, "  let height = canvas.height = canvas.offsetHeight;\n");
    fprintf(f_html, "  let transform = { x: width / 2, y: height / 2, scale: 0.8 };\n\n");
    
    fprintf(f_html, "  window.addEventListener('resize', () => {\n");
    fprintf(f_html, "    width = canvas.width = canvas.offsetWidth;\n");
    fprintf(f_html, "    height = canvas.height = canvas.offsetHeight;\n");
    fprintf(f_html, "  });\n\n");
    
    fprintf(f_html, "  let selectedNode = null;\n");
    fprintf(f_html, "  let draggedNode = null;\n");
    fprintf(f_html, "  let isPanning = false;\n");
    fprintf(f_html, "  let panStart = { x: 0, y: 0 };\n\n");
    
    fprintf(f_html, "  let viewMode = 'force';\n");
    fprintf(f_html, "  function setViewMode(mode) {\n");
    fprintf(f_html, "    viewMode = mode;\n");
    fprintf(f_html, "    document.querySelectorAll('.view-btn').forEach(btn => btn.classList.remove('active'));\n");
    fprintf(f_html, "    const modeBtn = [...document.querySelectorAll('.view-btn')].find(b => b.getAttribute('onclick').includes(mode));\n");
    fprintf(f_html, "    if (modeBtn) modeBtn.classList.add('active');\n\n");
    fprintf(f_html, "    nodes.forEach((n, idx) => {\n");
    fprintf(f_html, "      if (mode === 'seed') {\n");
    fprintf(f_html, "        let ring = Math.floor(idx / 32);\n");
    fprintf(f_html, "        let radius = 100 + ring * 80;\n");
    fprintf(f_html, "        let ringCount = 32;\n");
    fprintf(f_html, "        let angle = ((idx %% ringCount) / ringCount) * 2 * Math.PI;\n");
    fprintf(f_html, "        n.targetX = Math.cos(angle) * radius;\n");
    fprintf(f_html, "        n.targetY = Math.sin(angle) * radius;\n");
    fprintf(f_html, "      } else if (mode === 'tune') {\n");
    fprintf(f_html, "        let rows = 12;\n");
    fprintf(f_html, "        let col = Math.floor(idx / rows);\n");
    fprintf(f_html, "        let totalCols = Math.ceil(nodes.length / rows);\n");
    fprintf(f_html, "        n.targetX = -450 + (col / Math.max(1, totalCols - 1)) * 900;\n");
    fprintf(f_html, "        n.targetY = -250 + (idx %% rows) * 45;\n");
    fprintf(f_html, "      } else if (mode === 'conify') {\n");
    fprintf(f_html, "        let cols = Math.ceil(Math.sqrt(nodes.length));\n");
    fprintf(f_html, "        let col = idx %% cols;\n");
    fprintf(f_html, "        let row = Math.floor(idx / cols);\n");
    fprintf(f_html, "        n.targetX = -420 + col * 55;\n");
    fprintf(f_html, "        n.targetY = -250 + row * 45;\n");
    fprintf(f_html, "      } else if (mode === 'bond') {\n");
    fprintf(f_html, "        let angle = (idx / nodes.length) * 2 * Math.PI;\n");
    fprintf(f_html, "        let offset = (idx %% 3 - 1) * 35;\n");
    fprintf(f_html, "        n.targetX = Math.cos(angle) * 350 + offset * Math.cos(angle);\n");
    fprintf(f_html, "        n.targetY = Math.sin(2 * angle) * 220 + offset * Math.sin(angle);\n");
    fprintf(f_html, "      } else if (mode === 'delegate') {\n");
    fprintf(f_html, "        let angle = (idx / nodes.length) * 6 * Math.PI;\n");
    fprintf(f_html, "        let R = 240, r = 80, d = 130;\n");
    fprintf(f_html, "        let thickness = (idx %% 3 - 1) * 30;\n");
    fprintf(f_html, "        n.targetX = (R - r) * Math.cos(angle) + d * Math.cos(((R - r) / r) * angle) + thickness * Math.cos(angle);\n");
    fprintf(f_html, "        n.targetY = (R - r) * Math.sin(angle) - d * Math.sin(((R - r) / r) * angle) + thickness * Math.sin(angle);\n");
    fprintf(f_html, "      }\n");
    fprintf(f_html, "    });\n");
    fprintf(f_html, "  }\n\n");

    fprintf(f_html, "  function updateSimulation() {\n");
    fprintf(f_html, "    if (viewMode === 'force') {\n");
    fprintf(f_html, "      // Repulsion between all nodes\n");
    fprintf(f_html, "      for (let i = 0; i < nodes.length; i++) {\n");
    fprintf(f_html, "        let n1 = nodes[i];\n");
    fprintf(f_html, "        for (let j = i + 1; j < nodes.length; j++) {\n");
    fprintf(f_html, "          let n2 = nodes[j];\n");
    fprintf(f_html, "          let dx = n2.x - n1.x;\n");
    fprintf(f_html, "          let dy = n2.y - n1.y;\n");
    fprintf(f_html, "          let dist = Math.sqrt(dx*dx + dy*dy) || 1;\n");
    fprintf(f_html, "          if (dist < 350) {\n");
    fprintf(f_html, "            let force = (350 - dist) * 0.12;\n");
    fprintf(f_html, "            let fx = (dx / dist) * force;\n");
    fprintf(f_html, "            let fy = (dy / dist) * force;\n");
    fprintf(f_html, "            if (n1 !== draggedNode) { n1.vx -= fx; n1.vy -= fy; }\n");
    fprintf(f_html, "            if (n2 !== draggedNode) { n2.vx += fx; n2.vy += fy; }\n");
    fprintf(f_html, "          }\n");
    fprintf(f_html, "        }\n");
    fprintf(f_html, "      }\n\n");
    fprintf(f_html, "      // Attraction along links\n");
    fprintf(f_html, "      links.forEach(l => {\n");
    fprintf(f_html, "        let dx = l.target.x - l.source.x;\n");
    fprintf(f_html, "        let dy = l.target.y - l.source.y;\n");
    fprintf(f_html, "        let dist = Math.sqrt(dx*dx + dy*dy) || 1;\n");
    fprintf(f_html, "        let desired = 100;\n");
    fprintf(f_html, "        if (dist > desired) {\n");
    fprintf(f_html, "          let force = (dist - desired) * 0.04;\n");
    fprintf(f_html, "          let fx = (dx / dist) * force;\n");
    fprintf(f_html, "          let fy = (dy / dist) * force;\n");
    fprintf(f_html, "          if (l.source !== draggedNode) { l.source.vx += fx; l.source.vy += fy; }\n");
    fprintf(f_html, "          if (l.target !== draggedNode) { l.target.vx -= fx; l.target.vy -= fy; }\n");
    fprintf(f_html, "        }\n");
    fprintf(f_html, "      });\n\n");
    fprintf(f_html, "      // Gravity towards center and updates\n");
    fprintf(f_html, "      nodes.forEach(n => {\n");
    fprintf(f_html, "        if (n !== draggedNode) {\n");
    fprintf(f_html, "          n.vx -= n.x * 0.015;\n");
    fprintf(f_html, "          n.vy -= n.y * 0.015;\n");
    fprintf(f_html, "          n.x += n.vx;\n");
    fprintf(f_html, "          n.y += n.vy;\n");
    fprintf(f_html, "          n.vx *= 0.75;\n");
    fprintf(f_html, "          n.vy *= 0.75;\n");
    fprintf(f_html, "        }\n");
    fprintf(f_html, "      });\n");
    fprintf(f_html, "    } else {\n");
    fprintf(f_html, "      // Interpolation to target coordinates\n");
    fprintf(f_html, "      nodes.forEach(n => {\n");
    fprintf(f_html, "        if (n !== draggedNode) {\n");
    fprintf(f_html, "          n.x += (n.targetX - n.x) * 0.12;\n");
    fprintf(f_html, "          n.y += (n.targetY - n.y) * 0.12;\n");
    fprintf(f_html, "          n.vx = 0; n.vy = 0;\n");
    fprintf(f_html, "        }\n");
    fprintf(f_html, "      });\n");
    fprintf(f_html, "    }\n");
    fprintf(f_html, "  }\n\n");
    
    fprintf(f_html, "  function draw() {\n");
    fprintf(f_html, "    ctx.clearRect(0, 0, width, height);\n");
    fprintf(f_html, "    ctx.save();\n");
    fprintf(f_html, "    ctx.translate(transform.x, transform.y);\n");
    fprintf(f_html, "    ctx.scale(transform.scale, transform.scale);\n\n");
    
    fprintf(f_html, "    // Draw connections\n");
    fprintf(f_html, "    links.forEach(l => {\n");
    fprintf(f_html, "      ctx.beginPath();\n");
    fprintf(f_html, "      ctx.moveTo(l.source.x, l.source.y);\n");
    fprintf(f_html, "      ctx.lineTo(l.target.x, l.target.y);\n");
    fprintf(f_html, "      ctx.strokeStyle = 'rgba(255, 255, 255, 0.07)';\n");
    fprintf(f_html, "      ctx.lineWidth = 1.5;\n");
    fprintf(f_html, "      ctx.stroke();\n");
    fprintf(f_html, "    });\n\n");
    
    fprintf(f_html, "    // Draw nodes\n");
    fprintf(f_html, "    nodes.forEach(n => {\n");
    fprintf(f_html, "      ctx.beginPath();\n");
    fprintf(f_html, "      ctx.arc(n.x, n.y, n.radius, 0, Math.PI * 2);\n");
    fprintf(f_html, "      const isSel = n === selectedNode;\n");
    fprintf(f_html, "      ctx.fillStyle = n.color;\n");
    fprintf(f_html, "      ctx.shadowBlur = isSel ? 20 : 8;\n");
    fprintf(f_html, "      ctx.shadowColor = n.color;\n");
    fprintf(f_html, "      ctx.fill();\n");
    fprintf(f_html, "      ctx.shadowBlur = 0;\n\n");
    
    fprintf(f_html, "      // Label\n");
    fprintf(f_html, "      ctx.fillStyle = '#ffffff';\n");
    fprintf(f_html, "      ctx.font = isSel ? 'bold 12px Outfit' : '10px Outfit';\n");
    fprintf(f_html, "      ctx.textAlign = 'center';\n");
    fprintf(f_html, "      ctx.fillText(n.label, n.x, n.y - n.radius - 6);\n");
    fprintf(f_html, "    });\n\n");
    
    fprintf(f_html, "    ctx.restore();\n");
    fprintf(f_html, "  }\n\n");
    
    fprintf(f_html, "  function loop() {\n");
    fprintf(f_html, "    updateSimulation();\n");
    fprintf(f_html, "    draw();\n");
    fprintf(f_html, "    requestAnimationFrame(loop);\n");
    fprintf(f_html, "  }\n");
    fprintf(f_html, "  requestAnimationFrame(loop);\n\n");
    
    // Interactive mouse mapping
    fprintf(f_html, "  function getMousePos(e) {\n");
    fprintf(f_html, "    const rect = canvas.getBoundingClientRect();\n");
    fprintf(f_html, "    return {\n");
    fprintf(f_html, "      x: (e.clientX - rect.left - transform.x) / transform.scale,\n");
    fprintf(f_html, "      y: (e.clientY - rect.top - transform.y) / transform.scale\n");
    fprintf(f_html, "    };\n");
    fprintf(f_html, "  }\n\n");
    
    fprintf(f_html, "  canvas.addEventListener('mousedown', e => {\n");
    fprintf(f_html, "    const m = getMousePos(e);\n");
    fprintf(f_html, "    let clickedNode = null;\n");
    fprintf(f_html, "    for (let i = nodes.length - 1; i >= 0; i--) {\n");
    fprintf(f_html, "      let n = nodes[i];\n");
    fprintf(f_html, "      let dist = Math.sqrt((n.x - m.x)**2 + (n.y - m.y)**2);\n");
    fprintf(f_html, "      if (dist < n.radius + 6) {\n");
    fprintf(f_html, "        clickedNode = n;\n");
    fprintf(f_html, "        break;\n");
    fprintf(f_html, "      }\n");
    fprintf(f_html, "    }\n\n");
    
    fprintf(f_html, "    if (clickedNode) {\n");
    fprintf(f_html, "      selectedNode = clickedNode;\n");
    fprintf(f_html, "      draggedNode = clickedNode;\n");
    fprintf(f_html, "      displayDetails(clickedNode);\n");
    fprintf(f_html, "    } else {\n");
    fprintf(f_html, "      isPanning = true;\n");
    fprintf(f_html, "      panStart.x = e.clientX - transform.x;\n");
    fprintf(f_html, "      panStart.y = e.clientY - transform.y;\n");
    fprintf(f_html, "    }\n");
    fprintf(f_html, "  });\n\n");
    
    fprintf(f_html, "  canvas.addEventListener('mousemove', e => {\n");
    fprintf(f_html, "    if (draggedNode) {\n");
    fprintf(f_html, "      const m = getMousePos(e);\n");
    fprintf(f_html, "      draggedNode.x = m.x;\n");
    fprintf(f_html, "      draggedNode.y = m.y;\n");
    fprintf(f_html, "      draggedNode.vx = 0;\n");
    fprintf(f_html, "      draggedNode.vy = 0;\n");
    fprintf(f_html, "    } else if (isPanning) {\n");
    fprintf(f_html, "      transform.x = e.clientX - panStart.x;\n");
    fprintf(f_html, "      transform.y = e.clientY - panStart.y;\n");
    fprintf(f_html, "    }\n");
    fprintf(f_html, "  });\n\n");
    
    fprintf(f_html, "  window.addEventListener('mouseup', () => {\n");
    fprintf(f_html, "    draggedNode = null;\n");
    fprintf(f_html, "    isPanning = false;\n");
    fprintf(f_html, "  });\n\n");
    
    fprintf(f_html, "  canvas.addEventListener('wheel', e => {\n");
    fprintf(f_html, "    e.preventDefault();\n");
    fprintf(f_html, "    const zoom = e.deltaY < 0 ? 1.1 : 0.9;\n");
    fprintf(f_html, "    transform.scale = Math.min(Math.max(transform.scale * zoom, 0.1), 5);\n");
    fprintf(f_html, "  });\n\n");
    
    fprintf(f_html, "  function displayDetails(node) {\n");
    fprintf(f_html, "    document.getElementById('details').style.display = 'block';\n");
    fprintf(f_html, "    document.getElementById('det-name').innerText = node.name || 'Unknown';\n");
    fprintf(f_html, "    document.getElementById('det-sym').innerText = node.symbol || 'UNKNOWN';\n");
    fprintf(f_html, "    document.getElementById('det-addr').innerText = node.id;\n\n");
    
    fprintf(f_html, "    let poolsHtml = '';\n");
    fprintf(f_html, "    links.forEach(l => {\n");
    fprintf(f_html, "      if (l.source.id.toLowerCase() === node.id.toLowerCase()) {\n");
    fprintf(f_html, "        poolsHtml += '<div class=\"pool-row\"><span>' + l.target.symbol + '</span><strong>' + l.price.toFixed(6) + '</strong></div>';\n");
    fprintf(f_html, "      } else if (l.target.id.toLowerCase() === node.id.toLowerCase()) {\n");
    fprintf(f_html, "        poolsHtml += '<div class=\"pool-row\"><span>' + l.source.symbol + '</span><strong>' + (1/l.price).toFixed(6) + '</strong></div>';\n");
    fprintf(f_html, "      }\n");
    fprintf(f_html, "    });\n");
    fprintf(f_html, "    document.getElementById('det-pools').innerHTML = poolsHtml || '<div style=\"color:#64748b;font-size:13px;\">No direct active LPs found</div>';\n");
    fprintf(f_html, "  }\n\n");
    
    fprintf(f_html, "  function searchNode() {\n");
    fprintf(f_html, "    const val = document.getElementById('search-box').value.toLowerCase().trim();\n");
    fprintf(f_html, "    if (val.length === 0) return;\n");
    fprintf(f_html, "    const matched = nodes.find(n => n.id.toLowerCase() === val || n.name.toLowerCase().includes(val) || n.symbol.toLowerCase().includes(val) || (n.varName && n.varName.toLowerCase().includes(val)));\n");
    fprintf(f_html, "    if (matched) {\n");
    fprintf(f_html, "      selectedNode = matched;\n");
    fprintf(f_html, "      displayDetails(matched);\n");
    fprintf(f_html, "      transform.x = width / 2 - matched.x * transform.scale;\n");
    fprintf(f_html, "      transform.y = height / 2 - matched.y * transform.scale;\n");
    fprintf(f_html, "    }\n");
    fprintf(f_html, "  }\n");
    
    fprintf(f_html, "</script>\n</body>\n</html>\n");
    fclose(f_html);
    
    printf("[DASHBOARD] Successfully wrote interactive ecosystem graph to:\n  %s\n", out_path);
    return 0;
}
