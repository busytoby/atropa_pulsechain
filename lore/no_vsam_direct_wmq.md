# Lore: No VSAM — WinchesterMQ (WMQ) and ABI System Bypass

The integration of direct WinchesterMQ (WMQ) and ABI system register mappings renders the Virtual Storage Access Method (VSAM) redundant. Rather than executing catalog lookups and index allocations in software, dataset references map directly to WinchesterMQ queue addresses.

## Bypassing the VSAM Layer

1. **Direct Queue Mapping**: Dataset requests translate directly into WinchesterMQ address lookups, bypassing legacy VSAM software index paths.
2. **DisplacementShader Synchronization**: When transitions trigger WinchesterMQ (wm) queue operations, the system returns to the `DisplacementShader` to verify that vertex displacement calculations scale in perfect synchronization with register boundary constraints, protecting the EDO-22 wireframe envelope projection from distortion.
3. **Zero Software Serialization Overhead**: Software catalog search loops are completely bypassed, enabling direct buffer copies from virtual storage devices.
