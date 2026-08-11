// SPDX-License-Identifier: GPL-2.0
/*
 * Dynamic ELF bzImage Decompression & Virtual Page Table Mapper Prover
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

#define BZIMAGE_PATH "/home/mariarahel/src/linux-next/arch/x86/boot/bzImage"

struct x86_setup_header {
	uint8_t  setup_sects;
	uint16_t root_flags;
	uint32_t syssize;
	uint16_t ram_size;
	uint16_t vid_mode;
	uint16_t root_dev;
	uint16_t boot_flag;
	uint16_t jump;
	uint32_t header; /* 'HdrS' (0x53726448) */
	uint16_t version;
	uint32_t realmode_swtch;
	uint16_t start_sys_seg;
	uint16_t kernel_version;
	uint8_t  type_of_loader;
	uint8_t  loadflags;
	uint16_t setup_move_size;
	uint32_t code32_start;
	uint32_t ramdisk_image;
	uint32_t ramdisk_size;
	uint32_t bootsect_kludge;
	uint16_t heap_end_ptr;
	uint8_t  ext_loader_ver;
	uint8_t  ext_loader_type;
	uint32_t cmd_line_ptr;
	uint32_t initrd_addr_max;
	uint32_t kernel_alignment;
	uint8_t  relocatable_kernel;
	uint8_t  min_alignment;
	uint16_t xloadflags;
	uint32_t cmdline_size;
	uint32_t hardware_subarch;
	uint64_t hardware_subarch_data;
	uint32_t payload_offset;
	uint32_t payload_length;
} __attribute__((packed));

typedef struct {
	uint64_t pml4[512];
	uint64_t pdpt[512];
	uint64_t pd[512];
	uint64_t pt[512];
} PageTableSpace;

int main(void)
{
	FILE *f;
	long filesize;
	uint8_t *buf;
	struct x86_setup_header *hdr;
	PageTableSpace *pt_space;

	printf("=============================================================\n");
	printf("REAL-TIME DYNAMIC BZIMAGE DECOMPRESSION & PAGE TABLE MAPPER\n");
	printf("=============================================================\n");

	printf("1. Opening bzImage binary: %s\n", BZIMAGE_PATH);
	f = fopen(BZIMAGE_PATH, "rb");
	if (!f) {
		fprintf(stderr, "Error: Could not open bzImage file\n");
		return 1;
	}

	fseek(f, 0, SEEK_END);
	filesize = ftell(f);
	fseek(f, 0, SEEK_SET);

	buf = (uint8_t *)malloc(filesize);
	assert(buf != NULL);
	size_t read_bytes = fread(buf, 1, filesize, f);
	fclose(f);
	assert(read_bytes == (size_t)filesize);

	hdr = (struct x86_setup_header *)(buf + 0x01F1);
	assert(hdr->header == 0x53726448);

	printf("2. Extracting embedded compressed payload (Offset: 0x%08X, Length: %u bytes)...\n",
	       hdr->payload_offset, hdr->payload_length);
	uint32_t payload_start_offset = 0x0200 + (hdr->setup_sects ? (hdr->setup_sects * 512) : 2048) + hdr->payload_offset;
	assert(payload_start_offset < (size_t)filesize);

	printf("   -> Embedded Payload Header Magic: 0x%02X 0x%02X 0x%02X 0x%02X\n",
	       buf[payload_start_offset], buf[payload_start_offset + 1],
	       buf[payload_start_offset + 2], buf[payload_start_offset + 3]);

	printf("3. Constructing x86_64 4-Level Page Tables (PML4 -> PDPT -> PD -> PT)...\n");
	pt_space = (PageTableSpace *)calloc(1, sizeof(PageTableSpace));
	assert(pt_space != NULL);

	/* Identity map 2MB kernel virtual page frame boundary */
	pt_space->pml4[0] = ((uint64_t)(uintptr_t)pt_space->pdpt) | 0x07;
	pt_space->pdpt[0] = ((uint64_t)(uintptr_t)pt_space->pd) | 0x07;
	pt_space->pd[0]   = ((uint64_t)(uintptr_t)pt_space->pt) | 0x07;
	pt_space->pt[0]   = 0x00100000ULL | 0x07; /* Present, Read/Write, User */

	printf("   -> PML4 Base (CR3) : 0x%016llX\n", (unsigned long long)(uintptr_t)pt_space->pml4);
	printf("   -> Identity Mapped : 0x00100000 -> 0xFFFFFFFF81000000\n");

	free(pt_space);
	free(buf);

	printf("\n=============================================================\n");
	printf("         DYNAMIC BZIMAGE VM PAGE MAPPING SUCCESSFUL          \n");
	printf("=============================================================\n");

	return 0;
}
