# Mainframe standards & Database Management Documentation

This document covers the specifications, APIs, and design details for the mainframe database transactions, cryptographic systems, and standard I/O interfaces implemented under Volume 24 and Datamation Volume 24 (1978) standards.

---

## 1. Two-Phase Commit (2PC) Protocol Coordinator (Scenario 140)
The 2PC coordinator manages atomic distributed transactions across multiple mounted **2-3 Tree Nodes** in the mainframe architecture. It ensures that all node partitions either cooperatively prepare and commit their writes or safely abort to original snapshots if consensus fails.

### API Architecture
* [tsfi_2pc_init](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_mainframe_v370.h#L526): Initializes the distributed coordinator context.
* [tsfi_2pc_join](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_mainframe_v370.h#L527): Registers a target node key to participate in the transaction.
* [tsfi_2pc_prepare](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_mainframe_v370.h#L528): Dispatches prepare requests to all participating nodes.
* [tsfi_2pc_commit](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_mainframe_v370.h#L529): Commits changes on all nodes if consensus is achieved.
* [tsfi_2pc_abort](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_mainframe_v370.h#L530): Aborts the transaction and rolls back all nodes.

---

## 2. IBM System/38 Single-Level Store and Access Path Manager (Scenario 141)
Emulates the 1978 System/38 architecture where physical database files and logical relational views (Logical Files) are accessed as objects within a unified 64-bit flat virtual address space rather than physical file system paths.

### Key Invariants
* **Single-Level Store:** Checks for address collisions globally when registering database objects.
* **Access Path Sorting:** Querying a Logical File projects the raw records from the underlying Physical File, automatically sorting the outputs by key value using secondary index pointers.

### API Architecture
* [tsfi_s38_store_init](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_mainframe_v370.h#L556): Initializes System/38 store context.
* [tsfi_s38_create_object](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_mainframe_v370.h#L557): Registers a physical or logical database object at a virtual address.
* [tsfi_s38_insert_physical](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_mainframe_v370.h#L558): Appends records into the physical store table.
* [tsfi_s38_query_logical_path](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_mainframe_v370.h#L559): Resolves dynamic sorting paths for logical views.

---

## 3. IBM 3848 Cryptographic Subsystem (Scenario 142)
Simulates the block-oriented encryption coprocessor used to secure network payload communications over mainframe teleprocessing networks.

### Security Invariants
* **Supervisor State Lockout:** Cryptographic block operations are locked and return privilege violations unless the processor runs in Supervisor State.
* **Feistel Network:** Payload blocks of 64 bits are encrypted using a 2-round Feistel network with keys derived from a loaded 64-bit Master Key.

### API Architecture
* [tsfi_crypto_init](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_mainframe_v370.h#L567): Initializes cryptoprocessor context.
* [tsfi_crypto_load_master_key](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_mainframe_v370.h#L568): Configures the master crypto key.
* [tsfi_crypto_encrypt](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_mainframe_v370.h#L569): Encrypts a 64-bit block.
* [tsfi_crypto_decrypt](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_mainframe_v370.h#L570): Decrypts a 64-bit block.

---

## 4. NBS FIPS PUB 60 Coaxial I/O Channel Interface (Scenario 143)
Provides standard channel-to-control-unit interfacing (OEMI architecture) allowing peripheral hardware units to communicate with the mainframe CPU channel.

### Coaxial System Integration
* Adds `COAX_EVENT_CHANNEL_IO` events to the [TSFiCoaxialObserver](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_coaxial_observer.h#L22-L29).
* Captures standard `BUS OUT` command handshakes (SEEK, WRITE, READ, SENSE) and translates them into coaxial signals that are propagated directly to the system's non-preferential accumulator models.

---

## 5. NBS FIPS PUB 54 Computer Output Microform (COM) Spool Formatter (Scenario 144)
Formats print spools and database outputs directly onto virtual microfiche frame grids.
* **Fiche Grid Limits:** Automatically wraps rows and advances page frame indices every 4 records.
* **Index Resolution:** Generates index frames referencing record start keys for rapid microfiche readers lookup.

---

## 6. NBS FIPS PUB 48 Personal Identification Authenticator (Scenario 145)
Restricts virtual terminal access through badge readers and PIN checks.
* **Token Mapping:** Supports registering user badges by mapping verified LAU token addresses as badge IDs, and PKI keys as secure sign-on PINs.

---

## 7. NBS FIPS PUB 62 Magnetic Tape Subsystem Interface (Scenario 146)
Provides movement and access control logic for 9-track magnetic tape units.
* **Command Codes:** Simulates rewinding (`0x0F`), block spacing (`0x3F`), and backspacing (`0x27`).
* **DAT Address Translation:** Integrates with the System/370 segment and page tables to translate target virtual destinations to real memory before loading tape blocks.

---

## 8. NBS FIPS PUB 63 Rotating Mass Storage Subsystem Interface (Scenario 147)
Standardizes disk controller command sequences and status reporting rules.
* **OEMI disk controls:** Handles head recalibration (`0x03`), sector writes/reads (`0x01`/`0x02`), and cylinder seeks (`0x07`).
* **Safety boundaries:** Restricts head seeks to valid cylinders (< 45), returning Seek Check status on out-of-bounds requests.

---

## 9. NBS FIPS PUB 68 Minimal BASIC Interpreter (Scenario 148)
Emulates the minimal BASIC environment on timesharing terminals.
* **Instruction set:** Implements core commands `LET`, `PRINT`, and `END`.
* **Execution tracking:** Supports line numbering and state checks.

---

## 10. NBS FIPS PUB 69 Numeric Value Parser/Formatter (Scenario 149)
Standardizes numeric string notations for information interchange.
* **Notation checks:** Validates sign characters (`+` or `-`), fractional segments, and scientific exponents.
* **Normalization:** Strips trailing fractional zeros to enforce consistent database schemas.

---

## 11. NBS FIPS PUB 79 Magnetic Tape Label Parser/Validator (Scenario 150)
Standardizes label block structures for tape interchange files.
* **Volume/Header processing:** Parses standard 80-character `VOL1`, `HDR1`, and `EOF1` EBCDIC/ASCII records.
* **Metadata checks:** Extracts and validates File Identifiers, serial tracking numbers, and block counts to ensure boundary security.

---

## 12. NBS FIPS PUB 81 Block Cipher Modes of Operation (Scenario 151)
Extends block ciphers with standard security feedback chaining loops.
* **ECB/CBC modes:** Supports independent block encryption (Electronic Codebook) and chained feedback (Cipher Block Chaining).
* **Feedback loops:** Incorporates Initialization Vectors (IV) to prevent block pattern leaks over teleprocessing lines.

---

## 13. NBS FIPS PUB 94 Power Disturbance Monitor (Scenario 152)
Establishes standardized facility auditing for electrical power transients and fluctuations.
* **Disturbance classifications:** Identifies power quality events including sags (drops > 10%), surges (increases > 10%), and transient voltage spikes.
* **Write protection integration:** Automatically signals unsafe power environments to halt active writing hardware pathways to prevent memory failures.

---

## 14. NBS FIPS PUB 73 Application Processing Controls Audit (Scenario 153)
Standardizes record validation and audit logging requirements for mainframe software applications.
* **Input boundary validations:** Verifies key parameters (non-empty strings, non-negative transaction amounts).
* **Security auditing:** Automates tracking of validation attempts, parsing outcomes, and processing integrity violations.

---

## 15. NBS FIPS PUB 38 Automated Documentation Compliance Auditor (Scenario 154)
Enforces structured federal documentation guidelines across project specification files.
* **Structural auditing:** Scans documentation layouts for mandatory FIPS PUB 38 phase headers.
* **Score evaluation:** Reports a completeness percentage based on the presence of Functional Requirements, System/Subsystem Specifications, Program Specifications, and User Manual sections.

---

## 16. NBS FIPS PUB 1-1 Character Set Translation (Scenario 155)
Standardizes character mappings for compatibility during database exports.
* **Alphanumeric translation:** Converts standard 7-bit ASCII characters to EBCDIC byte patterns and back.

---

## 17. NBS FIPS PUB 16-1 Serial Bit Sequence & Parity Checker (Scenario 156)
Standardizes sequencing and transmission parity checks for peripheral communications.
* **LSB first mapping:** Serializes data bytes starting with the Least Significant Bit.
* **Parity calculation:** Automatically attaches and validates parity bits for synchronous (even parity) or asynchronous (odd parity) transmissions.

---

## 18. NBS FIPS PUB 41 Access Control Security Levels (Scenario 157)
Maintains dataset classification security boundaries for federal applications.
* **Role-based check:** Validates operations (read, write, audit) against verified user authorization levels (Owner, Operator, Auditor).

---

## 19. NBS FIPS PUB 113 Computer Data Authentication (Scenario 158)
Implements standard cryptographic data integrity validations.
* **Data Authentication Algorithm:** Emulates generating secure 8-byte Message Authentication Codes (MAC) using the DES cipher in Cipher Block Chaining (CBC) mode with zero IV configuration.

---

## 20. NBS FIPS PUB 112 Password Security Validator (Scenario 160)
Standardizes federal authentication policy guidelines.
* **Format enforcement:** Asserts password length is at least 6 characters.
* **Complexity scoring:** Awards validation points based on lowercase, uppercase, digit, and special symbol class representation.

---

## 21. NBS FIPS PUB 127 SQL Syntax Validator (Scenario 163)
Validates query formatting structures against database standards.
* **Verb check:** Asserts that query statement prefixes conform to standard DML commands (`SELECT`, `INSERT`, `UPDATE`, `DELETE`).

---

## 22. NBS FIPS Peripheral-Level Pre-Validation Filter (Scenario 164)
Integrates formatting validations at the device interface boundary to optimize bus load.
* **Format interception:** Analyzes data parameters (passwords, numeric formatting, SQL structures) directly inside device adapters.
* **Load reduction:** Drops non-compliant frames immediately and sets error flags, preventing unnecessary host interrupts and WinchesterMQ network traffic.

---

## 23. NBS FIPS PUB 54 Computer Output Microform Layout Generator (Scenario 165)
Formats microform data catalog layouts.
* **Grid index calculation:** Formats headers and maps document page indexes to standard 24x and 48x microfiche coordinate grids.

---

## 24. NBS FIPS PUB 55 Geographic Location Code Validator (Scenario 166)
Resolves geographic codes in database records.
* **Geographical mapping:** Decodes standard 5-digit locational numeric identifiers to resolve named populated divisions and cities.

---

## 25. NBS FIPS PUB 4-1 / 58-1 Date/Time Validator (Scenario 167)
Validates chronological fields in database entries.
* **Date validation:** Asserts format matches standard YYYYMMDD with logical month/day checking.
* **Time validation:** Asserts format matches standard HHMMSS with logical hour/minute/second checking.

---

## 26. NBS FIPS PUB 31 Risk Analysis Calculator (Scenario 168)
Quantifies security risk exposures for facility environments.
* **ALE calculation:** Emulates calculations for Annual Loss Expectancy based on asset value, threat frequency, and impact multipliers.

---

## 27. NBS FIPS PUB 30 Software Summary Descriptors (Scenario 169)
Validates software registry metadata cards.
* **Metadata validation:** Checks fields for program title and verifies status descriptors against compliant codes (`ACT`, `DEV`, `HIS`).

---

## 28. NBS FIPS PUB 105 Software Documentation Scorer (Scenario 170)
Assesses project lifecycle documentation status against federal standards.
* **Checks checklist masks:** Scores documentation maturity levels based on phase progression flags for Requirements (`0x01`), Design (`0x02`), Testing (`0x04`), and Operations (`0x08`).

---

## 29. NBS FIPS PUB 86 Real-Time Event Latency Monitor (Scenario 171)
Evaluates scheduling performance constraints for determinism.
* **Checks delays:** Compares measured event execution delays against strict deadlines.
* **Escalates priority:** Automatically calculates task priority adjustments if deadlines are missed to preserve real-time system performance.

---

## 30. NBS FIPS PUB 19-2 Data Code Dictionary Validator (Scenario 172)
Validates registered data elements and code files against federal exchange rules.
* **Data Element Checks:** Audits data code formats and validates categorization markers representing Entities (`ENT`), Attributes (`ATT`), and Representations (`REP`).



















