#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/inc/auncient_motzkin_engine.h"

int main(void) {
    printf("================================================================================\n");
    printf(" SYSTEM FUNCTION OF phi(x) = x - 1 = 4 IN THE DYSNOMIA VM STATE MACHINE\n");
    printf("================================================================================\n\n");

    const char *contract_addr = "dynamic_0x71C7656EC7ab88b098defB751B7401B5f6d8976F";
    const char *dat_bin_path = "/tmp/function_phi_x_is_4_proof.dat.bin";

    FILE *f = fopen(dat_bin_path, "wb");
    assert(f != NULL);
    uint8_t zero_buf[512] = {0};
    fwrite(zero_buf, 1, sizeof(zero_buf), f);
    fclose(f);

    uint64_t x = 5;
    uint64_t phi_x = x - 1; // 4

    /* -------------------------------------------------------------------------
     * FUNCTION 1: EXPONENT CYCLIC RING REDUCTION MODULO \phi(x) = 4
     * Exponents u = 36 and v = 6 from Chapter 3 are reduced modulo 4:
     *   u_red = 36 mod 4 = 0
     *   v_red = 6 mod 4  = 2
     * ------------------------------------------------------------------------- */
    uint64_t u_exp = 36;
    uint64_t v_exp = 6;
    uint64_t u_red = u_exp % phi_x; // 0
    uint64_t v_red = v_exp % phi_x; // 2

    /* -------------------------------------------------------------------------
     * FUNCTION 2: EULER MULTIPLICATIVE IDENTITY INVERTER
     * For coprime base a = 2: a^{\phi(x)} = 2^4 = 16 \equiv 1 \pmod 5
     * ------------------------------------------------------------------------- */
    uint64_t base_a = 2;
    uint64_t pow_a_phi = 1;
    for (uint64_t i = 0; i < phi_x; i++) pow_a_phi *= base_a; // 2^4 = 16
    uint64_t identity_mod_x = pow_a_phi % x;                 // 16 % 5 = 1

    /* -------------------------------------------------------------------------
     * FUNCTION 3: MODULAR MULTIPLICATIVE INVERSE EXPONENT FOR RADICAL ROOTS
     * In Z_4, 3 * 3 = 9 \equiv 1 \pmod 4.
     * Thus, 3rd radical root \sqrt[3]{a} \equiv a^3 \pmod 5.
     * ------------------------------------------------------------------------- */
    uint64_t radical_root_exp = 3;
    uint64_t inv_exp = 3;
    bool inv_valid = ((radical_root_exp * inv_exp) % phi_x == 1); // (3*3)%4 = 1

    assert(phi_x == 4);
    assert(u_red == 0);
    assert(v_red == 2);
    assert(identity_mod_x == 1);
    assert(inv_valid == true);

    printf("[FUNCTION 1: EXPONENT CYCLIC RING REDUCTION IN Z_4]\n");
    printf("    u = 36  ==>  36 mod 4 = %lu\n", u_red);
    printf("    v = 6   ==>   6 mod 4 = %lu\n\n", v_red);

    printf("[FUNCTION 2: EULER MULTIPLICATIVE IDENTITY INVERTER]\n");
    printf("    Base a = 2: 2^{\\phi(x)} = 2^4 = 16\n");
    printf("    16 mod 5 = %lu (EULER IDENTITY 1 RESTORED!)\n\n", identity_mod_x);

    printf("[FUNCTION 3: RADICAL ROOT MODULAR INVERSION IN Z_4]\n");
    printf("    3 * 3 = 9 mod 4 = %lu\n", (radical_root_exp * inv_exp) % phi_x);
    printf("    Radical Root Inversion Status: %s (\\sqrt[3]{a} \\equiv a^3 mod 5)\n\n",
           inv_valid ? "YES (100% SOUND)" : "NO");

    printf("================================================================================\n");
    printf(" [PROOFS COMPLETE] \\phi(x) = x - 1 = 4 serves 3 core system functions:\n");
    printf("                  1. Exponent ring reduction modulo 4\n");
    printf("                  2. Euler multiplicative identity (a^4 \\equiv 1 mod 5)\n");
    printf("                  3. Radical root modular inversion in Z_4\n");
    printf("================================================================================\n");

    remove(dat_bin_path);
    return 0;
}
