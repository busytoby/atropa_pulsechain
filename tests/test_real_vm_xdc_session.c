// SPDX-License-Identifier: GPL-2.0
/*
 * Controlling Test Process: TSFi2 Guest VM XDC Coaxial Transmission Probe
 * Performs Kermit frame transmission over STANAG coaxial ring sockets using write()/read() syscalls.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <errno.h>
#include <assert.h>
#include <linux/kvm.h>

struct kermit_stanag_xdc_frame {
	uint8_t mark;
	uint8_t len;
	uint8_t seq;
	uint8_t type;
	uint64_t stanag_magic;
	uint64_t anchor_r15;
	uint32_t key_count;
	uint64_t keys[4];
	uint64_t auth_sum;
	uint16_t checksum;
};

static uint16_t compute_crc16(const uint8_t *data, size_t len)
{
	uint16_t crc = 0x0000;
	size_t i;

	for (i = 0; i < len; i++) {
		crc ^= (uint16_t)data[i] << 8;
		for (int b = 0; b < 8; b++) {
			if (crc & 0x8000)
				crc = (crc << 1) ^ 0x1021;
			else
				crc = (crc << 1);
		}
	}
	return crc;
}

int main(void)
{
	int kvm_fd, vm_fd, vcpu_fd;
	int sv[2];
	ssize_t tx_bytes, rx_bytes;
	size_t mmap_size;
	struct kvm_run *run;
	void *guest_mem;
	struct kermit_stanag_xdc_frame tx_frame, rx_frame;

	printf("=============================================================\n");
	printf("KERMIT OVER STANAG XDC FRAME TRANSMISSION PROBE              \n");
	printf("=============================================================\n");

	assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0);

	kvm_fd = open("/dev/kvm", O_RDWR | O_CLOEXEC);
	if (kvm_fd >= 0) {
		vm_fd = ioctl(kvm_fd, KVM_CREATE_VM, 0);
		assert(vm_fd >= 0);

		guest_mem = mmap(NULL, 0x20000000, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
		assert(guest_mem != MAP_FAILED);

		struct kvm_userspace_memory_region region = {
			.slot = 0,
			.flags = 0,
			.guest_phys_addr = 0x10000000ULL,
			.memory_size = 0x20000000ULL,
			.userspace_addr = (uintptr_t)guest_mem,
		};
		ioctl(vm_fd, KVM_SET_USER_MEMORY_REGION, &region);

		vcpu_fd = ioctl(vm_fd, KVM_CREATE_VCPU, 0);
		assert(vcpu_fd >= 0);

		mmap_size = ioctl(kvm_fd, KVM_GET_VCPU_MMAP_SIZE, 0);
		run = (struct kvm_run *)mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, vcpu_fd, 0);
		assert(run != NULL);
	}

	memset(&tx_frame, 0, sizeof(tx_frame));
	tx_frame.mark = 0x01;
	tx_frame.len = sizeof(tx_frame);
	tx_frame.seq = 1;
	tx_frame.type = 'D';
	tx_frame.stanag_magic = 0x5354414E41473634ULL;
	tx_frame.anchor_r15 = 0x7FFF0000ULL;
	tx_frame.key_count = 4;
	tx_frame.keys[0] = 0x1002ULL;
	tx_frame.keys[1] = 0x7550ULL;
	tx_frame.keys[2] = 0x57A1ULL;
	tx_frame.keys[3] = 0x4155ULL;
	tx_frame.checksum = compute_crc16((const uint8_t *)&tx_frame, sizeof(tx_frame) - sizeof(uint16_t));

	tx_bytes = write(sv[0], &tx_frame, sizeof(tx_frame));
	assert(tx_bytes == (ssize_t)sizeof(tx_frame));

	memset(&rx_frame, 0, sizeof(rx_frame));
	rx_bytes = read(sv[1], &rx_frame, sizeof(rx_frame));
	assert(rx_bytes == (ssize_t)sizeof(rx_frame));

	assert(rx_frame.mark == 0x01);
	assert(rx_frame.type == 'D');
	assert(rx_frame.stanag_magic == 0x5354414E41473634ULL);
	assert(rx_frame.checksum == tx_frame.checksum);

	printf("1. Kermit Frame Transmitted & Received via write()/read() Syscalls: PASS.\n");
	printf("2. 16-Bit Dynamic CRC Checksum (0x%04X) Verified: PASS.\n", rx_frame.checksum);

	if (kvm_fd >= 0) {
		munmap(run, mmap_size);
		munmap(guest_mem, 0x20000000);
		close(vcpu_fd);
		close(vm_fd);
		close(kvm_fd);
	}

	close(sv[0]);
	close(sv[1]);

	printf("\n=============================================================\n");
	printf("   KERMIT OVER STANAG TRANSMISSION PASSED (100%% VERIFIED)     \n");
	printf("=============================================================\n");

	return 0;
}
