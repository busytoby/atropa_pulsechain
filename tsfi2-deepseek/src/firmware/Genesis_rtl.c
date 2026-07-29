#include "Genesis_rtl.h"
void Genesis_eval(Genesis_State *s) {
  if (s->reset) { s->ready=0; s->lfsr=0xB170FADEACE1ULL; }
  else {
    // Hardware Entropy Pulse
    s->lfsr = (s->lfsr << 1) | (((s->lfsr >> 63) ^ (s->lfsr >> 62)) & 1);
    if (s->limb_strobe && s->limb_addr < 1280) s->prime_storage[s->limb_addr] = s->limb_data;
    if (s->cmd_strobe && s->cmd == 0xDA1CE11) {
      for(int i=0; i<1280; i++) {
        s->ichidai_storage[i] = s->lfsr ^ (uint64_t)i;
        s->daiichi_storage[i] = (s->lfsr ^ (uint64_t)i) ^ s->prime_storage[i];
      }
      s->ready = 1;
    } else { s->ready = 0; }
  }
}
