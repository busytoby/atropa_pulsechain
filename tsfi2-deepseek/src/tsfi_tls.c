#include "tsfi_tls.h"
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <immintrin.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

// SHA-384 / HKDF Architecture
static const uint64_t K_SHA384[80] = {
    0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc,
    0x3956c25bf348b538, 0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118,
    0xd807aa98a3030242, 0x12835b0145706fbe, 0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2,
    0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 0xc19bf174cf692694,
    0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65,
    0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5,
    0x983e5152ee66dfab, 0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4,
    0xc6e00bf33da88fc2, 0xd5a79147930aa725, 0x06ca6351e003826f, 0x142929670a0e6e70,
    0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 0x53380d139d95b3df,
    0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b,
    0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30,
    0xd192e819d6ef5218, 0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8,
    0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8,
    0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3,
    0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec,
    0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b,
    0xca273eceea26619c, 0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178,
    0x06f067aa72176fba, 0x0a637dc5a2c898a6, 0x113f9804bef90dae, 0x1b710b35131c471b,
    0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c,
    0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817
};

typedef struct {
    uint64_t state[8];
    uint64_t count[2];
    uint8_t buffer[128];
} tsfi_sha384_context;

#define ROTR64(x, n) (((x) >> (n)) | ((x) << (64 - (n))))
#define Ch(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define Maj(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define S0(x) (ROTR64(x, 28) ^ ROTR64(x, 34) ^ ROTR64(x, 39))
#define S1(x) (ROTR64(x, 14) ^ ROTR64(x, 18) ^ ROTR64(x, 41))
#define s0(x) (ROTR64(x, 1) ^ ROTR64(x, 8) ^ ((x) >> 7))
#define s1(x) (ROTR64(x, 19) ^ ROTR64(x, 61) ^ ((x) >> 6))

void tsfi_sha384_compress(tsfi_sha384_context *ctx, const uint8_t *buf) {
    uint64_t a, b, c, d, e, f, g, h, t1, t2, w[80];
    for (int i = 0; i < 16; i++) {
        w[i] = ((uint64_t)buf[i*8] << 56) | ((uint64_t)buf[i*8+1] << 48) |
               ((uint64_t)buf[i*8+2] << 40) | ((uint64_t)buf[i*8+3] << 32) |
               ((uint64_t)buf[i*8+4] << 24) | ((uint64_t)buf[i*8+5] << 16) |
               ((uint64_t)buf[i*8+6] << 8) | buf[i*8+7];
    }
    for (int i = 16; i < 80; i++) w[i] = s1(w[i-2]) + w[i-7] + s0(w[i-15]) + w[i-16];
    a = ctx->state[0]; b = ctx->state[1]; c = ctx->state[2]; d = ctx->state[3];
    e = ctx->state[4]; f = ctx->state[5]; g = ctx->state[6]; h = ctx->state[7];
    for (int i = 0; i < 80; i++) {
        t1 = h + S1(e) + Ch(e, f, g) + K_SHA384[i] + w[i];
        t2 = S0(a) + Maj(a, b, c);
        h = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }
    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

void tsfi_sha384_init(tsfi_sha384_context *ctx) {
    ctx->state[0] = 0xcbbb9d5dc1059ed8ULL; ctx->state[1] = 0x629a292a367cd507ULL;
    ctx->state[2] = 0x9159015a3070dd17ULL; ctx->state[3] = 0x152fecd8f70e5939ULL;
    ctx->state[4] = 0x67332667ffc00b31ULL; ctx->state[5] = 0x8eb44a8768581511ULL;
    ctx->state[6] = 0xdb0c2e0d64f98fa7ULL; ctx->state[7] = 0x47b5481dbefa4fa4ULL;
    ctx->count[0] = 0; ctx->count[1] = 0;
}

void tsfi_sha384_update(tsfi_sha384_context *ctx, const uint8_t *in, size_t len) {
    uint32_t left = (uint32_t)((ctx->count[0] >> 3) & 0x7F);
    uint32_t fill = 128 - left;
    ctx->count[0] += (uint64_t)len << 3;
    if (ctx->count[0] < (uint64_t)len << 3) ctx->count[1]++;
    if (left && len >= fill) {
        memcpy(ctx->buffer + left, in, fill);
        tsfi_sha384_compress(ctx, ctx->buffer);
        in += fill; len -= fill; left = 0;
    }
    while (len >= 128) { tsfi_sha384_compress(ctx, in); in += 128; len -= 128; }
    if (len) memcpy(ctx->buffer + left, in, len);
}

void tsfi_sha384_final(tsfi_sha384_context *ctx, uint8_t *out) {
    uint8_t msglen[16];
    uint64_t high = ctx->count[1], low = ctx->count[0];
    for(int i=0; i<8; i++) { msglen[i] = (high >> (56-i*8)) & 0xFF; msglen[i+8] = (low >> (56-i*8)) & 0xFF; }
    uint32_t left = (uint32_t)((ctx->count[0] >> 3) & 0x7F);
    uint32_t padlen = (left < 112) ? (112 - left) : (240 - left);
    static const uint8_t padding[256] = {0x80};
    tsfi_sha384_update(ctx, padding, padlen);
    tsfi_sha384_update(ctx, msglen, 16);
    for (int i = 0; i < 6; i++) {
        uint64_t s = ctx->state[i];
        out[i*8] = (s >> 56) & 0xFF; out[i*8+1] = (s >> 48) & 0xFF;
        out[i*8+2] = (s >> 40) & 0xFF; out[i*8+3] = (s >> 32) & 0xFF;
        out[i*8+4] = (s >> 24) & 0xFF; out[i*8+5] = (s >> 16) & 0xFF;
        out[i*8+6] = (s >> 8) & 0xFF; out[i*8+7] = s & 0xFF;
    }
}

// HMAC-SHA-384
static void tsfi_hmac_sha384(const uint8_t *key, size_t key_len, const uint8_t *data, size_t data_len, uint8_t *out) {
    tsfi_sha384_context ctx; uint8_t k_ipad[128]={0}, k_opad[128]={0};
    if (key_len > 128) { tsfi_sha384_init(&ctx); tsfi_sha384_update(&ctx, key, key_len); tsfi_sha384_final(&ctx, k_ipad); key_len = 48; }
    memcpy(k_ipad, key, key_len); memcpy(k_opad, key, key_len);
    for (int i = 0; i < 128; i++) { k_ipad[i] ^= 0x36; k_opad[i] ^= 0x5c; }
    tsfi_sha384_init(&ctx); tsfi_sha384_update(&ctx, k_ipad, 128); tsfi_sha384_update(&ctx, data, data_len);
    uint8_t inner[48]; tsfi_sha384_final(&ctx, inner);
    tsfi_sha384_init(&ctx); tsfi_sha384_update(&ctx, k_opad, 128); tsfi_sha384_update(&ctx, inner, 48); tsfi_sha384_final(&ctx, out);
}

static void tsfi_hkdf_extract(const uint8_t *salt, size_t slen, const uint8_t *ikm, size_t ilen, uint8_t *prk) {
    if(!salt){ static const uint8_t zs[48]={0}; salt=zs; slen=48; }
    tsfi_hmac_sha384(salt, slen, ikm, ilen, prk);
}

static void tsfi_hkdf_expand_label(const uint8_t *secret, const char *label, const uint8_t *ctx_buf, size_t ctx_len, size_t len, uint8_t *out) {
    uint8_t lbl[256]; size_t llen = strlen(label);
    lbl[0] = (len >> 8) & 0xFF; lbl[1] = len & 0xFF; lbl[2] = 6 + llen;
    memcpy(lbl + 3, "tls13 ", 6); memcpy(lbl + 9, label, llen);
    lbl[9 + llen] = ctx_len; if (ctx_len > 0) memcpy(lbl + 10 + llen, ctx_buf, ctx_len);
    size_t tlen = 10 + llen + ctx_len; uint8_t t[48 + 256 + 1]; memcpy(t, lbl, tlen); t[tlen] = 0x01;
    uint8_t mac[48]; tsfi_hmac_sha384(secret, 48, t, tlen + 1, mac); memcpy(out, mac, len);
}

// --- X25519 Math (TweetNaCl-style) ---
typedef int64_t fe[16];
static void car25519(fe out) {
    for (int i = 0; i < 16; i++) {
        out[i] += (1LL << 16);
        int64_t c = out[i] >> 16;
        out[(i + 1) % 16] += c - 1;
        if (i == 15) out[0] += 37 * (c - 1);
        out[i] -= (c << 16);
    }
}
static void sel25519(fe p, fe q, int b) {
    int64_t t, c = ~(b - 1);
    for (int i = 0; i < 16; i++) { t = c & (p[i] ^ q[i]); p[i] ^= t; q[i] ^= t; }
}
static void pack25519(uint8_t *o, fe n) {
    int i, j, b; fe m, t;
    for (i = 0; i < 16; i++) t[i] = n[i];
    for (j = 0; j < 2; j++) {
        car25519(t);
        m[0] = t[0] - 0xffed; for (i = 1; i < 15; i++) m[i] = t[i] - 0xffff; m[15] = t[15] - 0x7fff;
        for (i = 0; i < 16; i++) {
            int64_t c = m[i] >> 16;
            m[(i + 1) % 16] += c;
            m[i] -= (c << 16);
        }
        b = !(m[15] >> 16);
        if (b) for (i = 0; i < 16; i++) t[i] = m[i];
    }
    for (i = 0; i < 16; i++) { o[2 * i] = t[i] & 0xff; o[2 * i + 1] = t[i] >> 8; }
}
static void unpack25519(fe o, const uint8_t *n) {
    for (int i = 0; i < 16; i++) o[i] = n[2 * i] + ((int64_t)n[2 * i + 1] << 8);
    o[15] &= 0x7fff;
}
static void A(fe o, fe a, fe b) { for (int i = 0; i < 16; i++) o[i] = a[i] + b[i]; }
static void Z(fe o, fe a, fe b) { for (int i = 0; i < 16; i++) o[i] = a[i] - b[i]; }
static void M(fe o, fe a, fe b) {
    int64_t t[31] = {0};
    for (int i = 0; i < 16; i++) for (int j = 0; j < 16; j++) t[i + j] += a[i] * b[j];
    for (int i = 0; i < 15; i++) t[i] += 38 * t[i + 16];
    for (int i = 0; i < 16; i++) o[i] = t[i];
    car25519(o); car25519(o);
}
static void S(fe o, fe a) { M(o, a, a); }
static void inv25519(fe o, fe i) {
    fe c; for (int a = 0; a < 16; a++) c[a] = i[a];
    for (int a = 253; a >= 0; a--) { S(c, c); if (a != 2 && a != 4) M(c, c, i); }
    for (int a = 0; a < 16; a++) o[a] = c[a];
}
void tsfi_x25519_ladder(const uint8_t *scalar, const uint8_t *point, uint8_t *out) {
    fe x1, x2 = {1}, z2 = {0}, x3, z3 = {1};
    uint8_t s[32]; for (int i = 0; i < 32; i++) s[i] = scalar[i];
    s[0] &= 248; s[31] &= 127; s[31] |= 64;
    unpack25519(x1, point); for (int i = 0; i < 16; i++) x3[i] = x1[i];
    for (int i = 254; i >= 0; i--) {
        int b = (s[i / 8] >> (i % 8)) & 1;
        sel25519(x2, x3, b); sel25519(z2, z3, b);
        fe a, b_fe, c, d, e, f, g, h, i_fe, j, k, l;
        A(a, x2, z2); S(b_fe, a); Z(c, x2, z2); S(d, c); Z(e, b_fe, d);
        M(x2, b_fe, d);
        M(f, e, (fe){121665}); A(g, d, f); M(z2, e, g);
        Z(h, x3, z3); A(i_fe, x3, z3); M(j, h, a); M(k, i_fe, c);
        Z(l, j, k); S(z3, l); M(z3, z3, x1);
        A(l, j, k); S(x3, l);
        sel25519(x2, x3, b); sel25519(z2, z3, b);
    }
    fe inv_z2; inv25519(inv_z2, z2); M(x2, x2, inv_z2); pack25519(out, x2);
}

// --- VAES ---
static inline __m128i aes_exp(__m128i k, __m128i g) { k=_mm_xor_si128(k,_mm_slli_si128(k,4)); k=_mm_xor_si128(k,_mm_slli_si128(k,4)); k=_mm_xor_si128(k,_mm_slli_si128(k,4)); return _mm_xor_si128(k,_mm_shuffle_epi32(g,0xff)); }
static void tsfi_aes256_expand(const uint8_t *key, __m512i *ks) {
    __m128i k1=_mm_loadu_si128((void*)key), k2=_mm_loadu_si128((void*)(key+16)), t1=k1, t2=k2, r[15];
    r[0]=k1; r[1]=k2; 
    t1=aes_exp(t1,_mm_aeskeygenassist_si128(t2,0x01)); r[2]=t1; t2=aes_exp(t2,_mm_aeskeygenassist_si128(t1,0x00)); r[3]=t2;
    t1=aes_exp(t1,_mm_aeskeygenassist_si128(t2,0x02)); r[4]=t1; t2=aes_exp(t2,_mm_aeskeygenassist_si128(t1,0x00)); r[5]=t2;
    t1=aes_exp(t1,_mm_aeskeygenassist_si128(t2,0x04)); r[6]=t1; t2=aes_exp(t2,_mm_aeskeygenassist_si128(t1,0x00)); r[7]=t2;
    t1=aes_exp(t1,_mm_aeskeygenassist_si128(t2,0x08)); r[8]=t1; t2=aes_exp(t2,_mm_aeskeygenassist_si128(t1,0x00)); r[9]=t2;
    t1=aes_exp(t1,_mm_aeskeygenassist_si128(t2,0x10)); r[10]=t1; t2=aes_exp(t2,_mm_aeskeygenassist_si128(t1,0x00)); r[11]=t2;
    t1=aes_exp(t1,_mm_aeskeygenassist_si128(t2,0x20)); r[12]=t1; t2=aes_exp(t2,_mm_aeskeygenassist_si128(t1,0x00)); r[13]=t2;
    r[14]=aes_exp(t1,_mm_aeskeygenassist_si128(t2,0x40));
    for(int i=0;i<15;i++) ks[i]=_mm512_broadcast_i32x4(r[i]);
}

static void tsfi_aes256_gcm_vaes(const uint8_t *in, uint8_t *out, size_t blks, const __m512i *ks, __m512i *iv) {
    for (size_t i = 0; i < blks; i += 4) {
        __m512i pt = _mm512_loadu_si512((void*)(in + i * 16));
        __m512i st = _mm512_xor_si512(*iv, ks[0]);
        for (int r = 1; r < 14; r++) st = _mm512_aesenc_epi128(st, ks[r]);
        st = _mm512_aesenclast_epi128(st, ks[14]);
        _mm512_storeu_si512((void*)(out + i * 16), _mm512_xor_si512(pt, st));
        // Simple counter increment
        uint64_t *c = (uint64_t*)iv; c[1]+=4; c[3]+=4; c[5]+=4; c[7]+=4;
    }
}

// --- TLS 1.3 State Machine ---
void tsfi_tls_init(TsfiTlsContext *ctx, int fd) { 
    memset(ctx, 0, sizeof(*ctx)); ctx->socket_fd = fd; 
}

static inline void w16(uint8_t **p, uint16_t v) { *(*p)++ = v >> 8; *(*p)++ = v & 0xFF; }

bool tsfi_tls_handshake(TsfiTlsContext *ctx, const char *host) {
    uint8_t ch[1024]={0}, *p=ch;
    *p++=0x16; w16(&p,0x0301); uint8_t *rl=p; p+=2; // Record
    *p++=0x01; uint8_t *hl=p; p+=3; // ClientHello
    w16(&p,0x0303); // Ver
    struct timeval tv; gettimeofday(&tv,0); srand(tv.tv_usec);
    for(int i=0;i<32;i++) {
        ctx->client_random[i]=rand();
    }
    memcpy(p,ctx->client_random,32); p+=32;
    *p++=32; for(int i=0;i<32;i++) *p++=rand(); // SID
    w16(&p,2); w16(&p,0x1302); // Cipher
    *p++=1; *p++=0; // Comp
    uint8_t *el=p; p+=2; // Ext
    w16(&p,0); w16(&p,strlen(host)+5); w16(&p,strlen(host)+3); *p++=0; w16(&p,strlen(host)); memcpy(p,host,strlen(host)); p+=strlen(host); // SNI
    w16(&p,10); w16(&p,4); w16(&p,2); w16(&p,0x001d); // Groups
    w16(&p,13); w16(&p,4); w16(&p,2); w16(&p,0x0403); // Sig
    w16(&p,43); w16(&p,3); *p++=2; w16(&p,0x0304); // Ver
    w16(&p,51); w16(&p,38); w16(&p,36); w16(&p,0x001d); w16(&p,32); // KeyShare
    uint8_t epriv[32], epub[32]; for(int i=0;i<32;i++) epriv[i]=rand();
    epriv[0]&=248; epriv[31]&=127; epriv[31]|=64; tsfi_x25519_ladder(epriv, (uint8_t[]){9}, epub);
    memcpy(p,epub,32); p+=32;
    size_t ex=p-el-2; el[0]=ex>>8; el[1]=ex&0xFF;
    size_t hs=p-hl-3; hl[0]=hs>>16; hl[1]=hs>>8; hl[2]=hs&0xFF;
    size_t rc=p-rl-2; rl[0]=rc>>8; rl[1]=rc&0xFF;
    if(write(ctx->socket_fd,ch,p-ch)<0) return false;
    uint8_t sh[2048]; ssize_t n=read(ctx->socket_fd,sh,2048); if(n<40) return false;
    // Extract server public key from KeyShare (naive offset scan)
    uint8_t spub[32]; int found=0;
    for(int i=0;i<n-36;i++) if(sh[i]==0x00 && sh[i+1]==0x33 && sh[i+4]==0x00 && sh[i+5]==0x1d) { memcpy(spub,sh+i+8,32); found=1; break; }
    if(!found) return false;
    uint8_t ss[32]; tsfi_x25519_ladder(epriv, spub, ss);
    // Key Derivation
    uint8_t es[48], des[48], hs_sec[48], hsh[48]; tsfi_sha384_context sctx;
    tsfi_hkdf_extract(0,0,(uint8_t[]){0},48,es);
    tsfi_sha384_init(&sctx); tsfi_sha384_final(&sctx,hsh);
    tsfi_hkdf_expand_label(es,"derived",hsh,48,48,des);
    tsfi_hkdf_extract(des,48,ss,32,hs_sec);
    tsfi_sha384_init(&sctx); tsfi_sha384_update(&sctx,ch,p-ch); tsfi_sha384_update(&sctx,sh,n); tsfi_sha384_final(&sctx,hsh);
    tsfi_hkdf_expand_label(hs_sec,"c hs traffic",hsh,48,48,ctx->client_write_key);
    tsfi_hkdf_expand_label(hs_sec,"s hs traffic",hsh,48,48,ctx->server_write_key);
    tsfi_hkdf_expand_label(ctx->client_write_key,"iv",0,0,12,ctx->client_write_iv);
    tsfi_hkdf_expand_label(ctx->server_write_key,"iv",0,0,12,ctx->server_write_iv);
    return true;
}

ssize_t tsfi_tls_write(TsfiTlsContext *ctx, const void *b, size_t l) {
    uint8_t *rec, *pt;
    if (posix_memalign((void**)&rec, 64, 17408) != 0) return -1;
    if (posix_memalign((void**)&pt, 64, 17408) != 0) { free(rec); return -1; }
    memset(rec, 0, 17408); memset(pt, 0, 17408);
    rec[0] = 0x17; rec[1] = 0x03; rec[2] = 0x03;
    uint16_t rl = l + 1 + 16; rec[3] = rl >> 8; rec[4] = rl & 0xFF;
    memcpy(pt, b, l); pt[l] = 0x17;
    __m512i ks[15], iv = _mm512_broadcast_i32x4(_mm_loadu_si128((void*)ctx->client_write_iv));
    tsfi_aes256_expand(ctx->client_write_key, ks);
    tsfi_aes256_gcm_vaes(pt, rec + 5, (l + 63) / 16, ks, &iv);
    printf("[TLS] Writing %u encrypted bytes (record length %u)\n", (unsigned int)(rl + 5), (unsigned int)rl);
    ssize_t ret = write(ctx->socket_fd, rec, rl + 5);
    free(rec); free(pt);
    return ret;
}

ssize_t tsfi_tls_read(TsfiTlsContext *ctx, void *b, size_t m) {
    uint8_t h[5]; 
    ssize_t hr = read(ctx->socket_fd, h, 5);
    if (hr < 5) {
        printf("[TLS] Failed to read record header (got %zd bytes)\n", hr);
        return -1;
    }
    printf("[TLS] Record Type: 0x%02x, Ver: 0x%02x%02x, Len: %u\n", h[0], h[1], h[2], (h[3] << 8) | h[4]);
    uint16_t rl = (h[3] << 8) | h[4];
    uint8_t *ct, *pt;
    if (posix_memalign((void**)&ct, 64, 17408) != 0) return -1;
    if (posix_memalign((void**)&pt, 64, 17408) != 0) { free(ct); return -1; }
    memset(ct, 0, 17408); memset(pt, 0, 17408);
    ssize_t dr = read(ctx->socket_fd, ct, rl);
    if (dr < rl) {
        printf("[TLS] Failed to read full record data (got %zd of %u bytes)\n", dr, rl);
        free(ct); free(pt); return -1;
    }
    __m512i ks[15], iv = _mm512_broadcast_i32x4(_mm_loadu_si128((void*)ctx->server_write_iv));
    tsfi_aes256_expand(ctx->server_write_key, ks);
    tsfi_aes256_gcm_vaes(ct, pt, (rl + 63) / 16, ks, &iv);
    size_t pl = (rl > 17) ? rl - 17 : 0; if (pl > m) pl = m;
    memcpy(b, pt, pl);
    free(ct); free(pt);
    return (ssize_t)pl;
}

void tsfi_tls_close(TsfiTlsContext *ctx) { (void)ctx; }
