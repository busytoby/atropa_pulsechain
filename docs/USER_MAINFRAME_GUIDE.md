# Mainframe Subsystem Integration & Execution Guide

This document describes how to compile, execute, and monitor batch jobs, transmission vectors, and Quadtree index databases within the simulated **Auncient** Dysnomia VM environment.

## 1. Batch Job Compilation & Spooling (HASP / JCL)
To compile the Clojure-style strategy closure programs and execute them within the batch runner environment:
```bash
make test-auncient-tsv-wmq-integration
```
If the JCL card audit fails validation (such as missing a verified SSN or TIN parameter in the program header), execution aborts. Refer to the diagnostic log below to resolve issues:
*   **Verification Log**: `/tmp/hasp_spool_audit.log`

## 2. Quadtree KSDS Database Operations
To execute the standardized `.dat.bin` Quadtree database format tests, validating spatial indexing and Alternate Index (AIX) pointers:
```bash
make test-auncient-quadtree-ksds
```

## 3. HathiTrust Catalog Exporting
To parse raw HathiTrust TSV catalog files and export them to EBCDIC/UTF-8 dual-encoded Quadtree KSDS slices:
```bash
make test-hathitrust-hathifile
```
Latency checks are instrumented automatically. If any row distribution takes longer than 1000 ns, performance warning alerts are written directly to standard error.
