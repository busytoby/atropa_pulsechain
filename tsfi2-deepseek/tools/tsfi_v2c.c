#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    (void)argc; (void)argv;
    
    FILE *h = fopen("src/firmware/Genesis_rtl.h", "w");
    FILE *c = fopen("src/firmware/Genesis_rtl.c", "w");
    
    if (!h || !c) return 1;

    fprintf(h, "#ifndef GENESIS_RTL_H\n#define GENESIS_RTL_H\n#include <stdint.h>\n#include <stdbool.h>\n");
    fprintf(h, "typedef struct {\n");
    fprintf(h, "  bool clk, reset, limb_strobe, cmd_strobe, ready;\n");
    fprintf(h, "  uint32_t cmd;\n");
    fprintf(h, "  uint16_t limb_addr;\n");
    fprintf(h, "  uint8_t  target_select;\n");
    fprintf(h, "  uint64_t limb_data, lfsr;\n");
    fprintf(h, "  uint64_t prime_storage[1280];\n");
    fprintf(h, "  uint64_t ichidai_storage[1280];\n");
    fprintf(h, "  uint64_t daiichi_storage[1280];\n");
    fprintf(h, "} Genesis_State;\n");
    fprintf(h, "void Genesis_eval(Genesis_State *s);\n#endif\n");
    fclose(h);

    fprintf(c, "#include \"Genesis_rtl.h\"\nvoid Genesis_eval(Genesis_State *s) {\n");
    fprintf(c, "  if (s->reset) { s->ready=0; s->lfsr=0xB170FADEACE1ULL; }\n");
    fprintf(c, "  else {\n");
    fprintf(c, "    // Hardware Entropy Pulse\n");
    fprintf(c, "    s->lfsr = (s->lfsr << 1) | (((s->lfsr >> 63) ^ (s->lfsr >> 62)) & 1);\n");
    fprintf(c, "    if (s->limb_strobe && s->limb_addr < 1280) s->prime_storage[s->limb_addr] = s->limb_data;\n");
    fprintf(c, "    if (s->cmd_strobe && s->cmd == 0xDA1CE11) {\n");
    fprintf(c, "      for(int i=0; i<1280; i++) {\n");
    fprintf(c, "        s->ichidai_storage[i] = s->lfsr ^ (uint64_t)i;\n");
    fprintf(c, "        s->daiichi_storage[i] = (s->lfsr ^ (uint64_t)i) ^ s->prime_storage[i];\n");
    fprintf(c, "      }\n");
    fprintf(c, "      s->ready = 1;\n    } else { s->ready = 0; }\n  }\n}\n");
    fclose(c);

    return 0;
}
