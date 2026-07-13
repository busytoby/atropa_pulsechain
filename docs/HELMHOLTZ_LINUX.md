# Helmholtz Linux: The Minimalist Auncient VM Distribution

Helmholtz Linux is an ultra-lightweight, single-purpose virtual Linux distribution designed specifically to execute headless tasks, cryptographic audits, and coordinate transformations under the **Auncient** ZMM VM.

## 1. System Overview & Footprint
Unlike standard embedded distributions (like BusyBox or TinyCore), Helmholtz Linux completely dispenses with shell utilities (`/bin/sh`, coreutils), command-line parsers, and multi-user environments. 

* **The Init Daemon (`zmm_init`):** A single statically-compiled C binary placed at `/sbin/init` (or `/init`) that acts as the absolute PID 1 process.
* **Footprint size:** ~30KB (excluding standard Linux kernel size).
* **Environment:** Executed within an isolated user-namespace container (`unshare -m -r`) or booted directly via VM boot manager selectors.

## 2. Directory Structure Skeleton
The root filesystem (`rootfs.img`) maps to the following minimal layout:
```
/
├── bin/          (Empty)
├── sbin/
│   └── init      (Statically compiled zmm_init binary)
├── dev/          (Dynamic mount for virtual PTY and SCSI device nodes)
├── proc/         (Mount point for procfs kernel states)
├── sys/          (Mount point for sysfs hardware states)
├── etc/          (Configuration files)
└── tmp/          (Scratch space)
```

## 3. System Boot & Initialization Sequence
When the boot manager (`tsfi_boot_manager.c`) redirects execution to the kernel:

1. **Kernel Mount Stage:** The kernel loads `/sbin/init` as the first process.
2. **Virtual Filesystem Mounts:** `zmm_init` mounts `/proc` and `/sys` to establish system interfaces.
3. **Helmholtz Operator Initialization:** Instantiates the `HelmholtzListOperator` mapping, binding coordinate structures and initial hashes.
4. **System Audit (`execute_system_audit`):** Runs cryptographic audits using the `lau_audit.h` framework.
5. **Headless Execution & Shutdown:** Performs scheduled mathematical transformations, pipes logs to host SCSI buffers, and triggers a clean guest shutdown via `reboot(RB_POWER_OFF)`.

## 4. Storage & Integration
* **Permanent Disk:** The root image is stored at `assets/zmm_rootfs.img` (automatically ignored by git).
* **SCSI Trap Mapping:** Read/write operations to the filesystem route via **LUN 2** directly to the host image file.
* **Host Sharing:** Both the guest Linux system and the host ZMM VM can dynamically modify files within `zmm_rootfs.img` via the WinchesterMQ SCSI protocol.

## 5. Verification & Tests
* **C Simulation Test:** Run `tests/test_auncient_diagnostics_c` to verify early boot parameter parsing.
* **Container Boot Test:** Run `tests/test_auncient_boot_transform_c` to execute the full unshare boot sequence of `zmm_init`.
