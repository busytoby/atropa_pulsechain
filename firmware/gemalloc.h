#pragma once
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <sys/random.h>

#define DLE 16
#define ALIGN(size) (((size) + 15) & ~15)

struct dm_blk_hdr {
  size_t size;
  bool cleared;
  int reuse;
  int seq;
  struct dm_blk_hdr *n;
  struct dm_blk_hdr *p;
};

#define DM_HDR_SIZE ALIGN(sizeof(struct dm_blk_hdr))

struct dm_blk_hdr* HEAD = NULL;
void* tail = NULL;
int seq_ctr = 0;

bool rand_seeded = false;
void ___gem(void* ptr, size_t size, char set) {
  if(!ptr || size == 0) return;

  if(set != DLE) {
    for(size_t i = 0; i < size; i++) ((char*)ptr)[i] = set;
    return;
  }

  uint64_t* dptr = (uint64_t*)ptr;
  size_t len = size / sizeof(uint64_t);
  size_t rem = size % sizeof(uint64_t);
  uint64_t r;

  if(!rand_seeded) {
    unsigned int seed;
    getrandom(&seed, sizeof(seed), 0);
    srandom(seed);
    rand_seeded = true;
  }

  for(size_t i = 0; i < len; i++) {
    if(set == DLE) {
      getrandom(&r, sizeof(r), 0); 
      dptr[i] = r;
    } else dptr[i] = set;
  }

  if(rem>0) {
    getrandom(&r, sizeof(r), 0);
    unsigned char* bptr = (unsigned char*)(dptr + len);
    for(size_t i = 0; i < rem; i++) bptr[i] = bptr[i] ^ (unsigned char)(r >> (i*8));
  }
}

void printgem(void* ptr) {
  if(!ptr) {
    printf("NULL GEM\n");
    return;
  }
  struct dm_blk_hdr *blk = (struct dm_blk_hdr*)ptr - 1;
  unsigned char* bytes = (unsigned char*)ptr;
  printf("GEM/%d %s: [", blk->size, blk->cleared ? "(FREE)" : "");
  for(size_t i = 0; i < blk->size; i++) printf("%02X", (unsigned int)bytes[i]);
  printf("]\n");
}

void* jgemalloc(size_t size, char set) {
  if(size == 0) return NULL;
  size = ALIGN(size);

  struct dm_blk_hdr *hptr = (struct dm_blk_hdr*)tail;

  while(hptr) {
    if(hptr->cleared && hptr->size >= size && hptr->size <= size + 40) {
      ___gem(hptr+1, hptr->size, set);
      hptr->cleared = false;
      hptr->reuse++;
      hptr->seq = ++seq_ctr;
      return (void*)(hptr+1);
    }
    hptr = hptr->p;
  }

  size_t block_size = size + DM_HDR_SIZE;
  struct dm_blk_hdr* newblk = (struct dm_blk_hdr*)sbrk(block_size);
  assert(newblk != (void*)-1);
  ___gem(newblk+1, block_size, set);

  newblk->size = size;
  newblk->cleared = false;
  newblk->reuse = 0;
  newblk->seq = ++seq_ctr;
  newblk->n = NULL;
  newblk->p = tail;

  if(tail) ((struct dm_blk_hdr*)tail)->n = newblk;
  tail = newblk;
  if(!HEAD) HEAD = newblk;
  return (void*)(newblk+1);
}

void* gemalloc(size_t size) {
  void* ptr = jgemalloc(size, DLE);
  //___gem(ptr, size, DLE);
  return ptr;
}

void gemfree(void* ptr) {
  if(!ptr) return;
  struct dm_blk_hdr* blk = (struct dm_blk_hdr*)ptr - 1;
  assert(!blk->cleared);
  blk->cleared = true;
  ___gem(ptr, blk->size, DLE);
}

void* gemrealloc(void* ptr, size_t size) {
  if(!ptr) return jgemalloc(size, 0);
  struct dm_blk_hdr* blk = (struct dm_blk_hdr*)ptr - 1;

  size = ALIGN(size);
  if(size == blk->size) return ptr;

  if(size > blk->size) {
    void* newblkptr = jgemalloc(size, 0);
    assert(newblkptr != NULL);
    memcpy(newblkptr, ptr, blk->size);
    gemfree(ptr);
    return newblkptr;
  }

  size_t sizediff = blk->size - size;
  if(sizediff >= (DM_HDR_SIZE + 16)) {
    struct dm_blk_hdr* splitblk = (struct dm_blk_hdr*)((char*)ptr + size);

    splitblk->size = sizediff - DM_HDR_SIZE;
    splitblk->cleared = true;
    splitblk->reuse = 0;
    splitblk->seq = ++seq_ctr;
    ___gem(splitblk+1, splitblk->size, 0);

    splitblk->p = blk;
    splitblk->n = blk->n;

    if(blk->n) blk->n->p = splitblk;
    else tail = splitblk;

    blk->n = splitblk;
    blk->size = size;
  }

  return ptr;
}

void printgemallocstats() {
  struct dm_blk_hdr *hptr = HEAD;
  int total = 0;
  int cleared = 0;
  int total_alloc_size = 0;
  int total_cleared_size = 0;
  int total_reuse = 0;

  while(hptr) {
    if(hptr->cleared) {
      cleared++;
      total_cleared_size += hptr->size;
    }
    total++;
    total_alloc_size += hptr->size;
    total_reuse += hptr->reuse;

    hptr = hptr->n;
  }

  printf("Total Alloc: %d\nTotal Alloc Bytes: %d\nTotal Cleared: %d\nReuse After Free: %d\nTotal Cleared Bytes: %d\n", 
    total, total_alloc_size, cleared, total_reuse, total_cleared_size);
}
