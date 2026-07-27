# The Doctrine of Federal Licenseability

In the legal and technical development of the **Auncient** Dysnomia VM, all operations are governed by a strict mandate: **complete isolation from external dependencies**. To qualify for a federal license, the platform must remain entirely self-contained.

### 1. The Hazard of External Contamination
In standard software architectures, developers routinely link third-party libraries, dynamic modules, and external package managers to accelerate development. Under the federal licensing template, this practice is classified as **contamination**:
* **License Collapse**: Introducing a single external module can invoke restrictive copyleft licenses (such as GPL/AGPL). This would legally require the system to expose its proprietary register structures, disqualifying it from federal compliance.
* **Security Vector**: External code introduces unverified execution paths, breaking the absolute audit boundary maintained by the system-wide gate.

### 2. The Four-Library Constraint
To establish a legally defensible and auditable boundary, the simulation loop is restricted to four standard libraries:
1. `glibc` (Standard C Library)
2. `libm` (Standard Math Library)
3. `librt` (Realtime Extensions)
4. `libcrypto` (Standard Hashing Library for FNV-1a DNA generation)

By developing exclusively in pure ISO C11 with these libraries, the code contains no proprietary or copyleft third-party code. The entire platform remains clean, deterministic, and fully eligible for national verification.
