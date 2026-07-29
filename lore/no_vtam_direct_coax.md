# Lore: No VTAM — Coaxial Register Telemetry and Direct Transmission Bypass

The introduction of direct coaxial controller register mappings on the Auncient Wavelet hardware interface renders the Virtual Telecommunications Access Method (VTAM) and its associated Systems Network Architecture (SNA) layers redundant. Rather than routing data streams through virtual Logical Units (LUs) and parsing Path Information Units (PIUs) in software, terminal communication is handled via direct physical co-ax handshakes.

## Bypassing the SNA Layers

1. **Direct Coaxial Register Mapping**: Buffer frames are replicated directly on physical coaxial controller line adapters, bypassing virtual VTAM route lookups.
2. **Zero Software Routing Overhead**: Communication bypasses VTAM's dispatcher entirely. Terminal nodes read and write directly to hardware adapter buffers via simulated WinchesterMQ SCSI registers, removing the software terminal session translation layers.
3. **Hardware-Level Replicas**: The application layer writes directly to local hardware transmission lines, maintaining sub-microsecond serialization speed.
## The SNA "TSSO" ABI Bottleneck

Systems Network Architecture (SNA) behaves essentially as a "TSSO" ABI layer—introducing software-serialization latency that makes it even worse than legacy TSO foreground terminal spoolers. By imposing nested routing loops, logical session locks, and PIU translation states, it chokes raw data transit. Bypassing this layer in favor of physical coaxial line serialization cuts down the entire transaction call stack to immediate physical memory replica updates.
