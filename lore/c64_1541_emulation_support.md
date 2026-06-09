# Commodore 1541 Disk Drive Emulation Support

Integrating the **Commodore 1541 Disk Drive** as a fully emulated coprocessor in our environment unlocks several capabilities that go far beyond simple file-loading hooks. 

This analysis details the technical features, software, and low-level mechanics we can support by introducing a virtual 1541 drive.

---

## 1. Core Technical Capabilities Enabled

```
   ┌────────────────────────────────────────────────────────┐
   │                       C64 VM                           │
   │   [6510 CPU] <=== Software IEC Serial Bus ===> [VIA]   │
   └───────────────────────────────────┬────────────────────┘
                                       │ (ATN, CLK, DATA)
   ┌───────────────────────────────────▼────────────────────┐
   │                       1541 VM                          │
   │   [6502 CPU] <=== CBM DOS 2.6 ROM ===> [VIA 6522]       │
   │       │                                                │
   │       └───────> [Floppy Controller] ──> [GCR Read]     │
   └────────────────────────────────────────────────────────┘
```

### 1.1 True Dual-CPU Parallel Execution
The 1541 operates its own MOS 6502 CPU at 1 MHz, running concurrently with the computer's 6510 CPU.
* **Support:** We can emulate a parallel 6502 execution thread. When the main C64 CPU issues a command, the drive-side CPU executes its own Kernal/DOS loops, yielding true concurrent execution.
* **Fastloader Kernels:** Enables running custom drive-side fastloader code uploaded to drive RAM (`$0300–$07FF`).

### 1.2 Raw `.D64` Disk Image & GCR Encoding
Commodore floppy disks do not use standard FAT/Sector formats; they use **Group Coded Recording (GCR)** to encode data on magnetic tracks.
* **Support:** We can load raw `.d64` disk image files. The emulated drive controller parses the directory sectors (Track 18), handles GCR bit conversion (translating 4-byte source nibbles to 5-byte physical patterns), and calculates header/data sync marks.

### 1.3 Hardware-Level Copy Protection
Many games from companies like *Synapse*, *Muse*, or *Commodore* implemented custom floppy-disk protection to prevent pirating:
* **Stepper Motor Stepping:** Emulating the head step registers allows the drive-side CPU to move the read head to half-tracks (e.g. Track 18.5) or track ranges outside standard DOS boundaries.
* **Bitrate/Density Changes:** The 1541 has 4 different density zones (different clock speeds for tracks 1–17, 18–24, etc.). Emulating the density selection registers allows running games that verify speed/sync tolerances.

### 1.4 IEC Bus Protocol Verification
Instead of mocking the file transfer outputs, we can run actual bit-level serial loops:
* **Support:** Verify the exact hardware handshake state machine (toggling ATN, CLK, and DATA lines via Virtual VIAs), which is crucial for testing custom fastloader cartridges.

---

## 2. Supported Software & Demos

| Category | Target Mechanics | Technical Requirement |
| :--- | :--- | :--- |
| **Custom DOS Commands** | Direct track/sector access commands (`U1`, `U2`, `B-R`, `B-W`) | Command channel parsing in CBM DOS ROM |
| **Drive-Side Demos** | Music played by vibrating the disk head stepper motor | Precise motor coil selection and step frequency emulation |
| **Commercial Fastloaders** | Epyx Fast Load, JiffyDOS, Mach 5, Final Cartridge | VIA 6522 shift register or exact software timing loops |
| **Copy-Protected Games** | Rapid-fire track checks, deliberate read errors | Accurate read/write head error state registers |
