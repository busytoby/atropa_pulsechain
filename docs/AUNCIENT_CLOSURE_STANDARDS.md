# Auncient Closure Strategy Language Standards

This document establishes the standards for the **Closure** strategy language used to configure and execute co-processor transactions within the Dysnomia VM.

---

## 1. Syntax and Layout Specification
The **Closure** language uses a declarative Lisp-based syntax to encapsulate job parameters, dataset definitions, and execution logic within a single self-contained structure:

*   **Metadata Map**: Predeclared at the top of the strategy file using the metadata reader macro `^{...}` to define job controls.
*   **Variable Declarations**: Defined globally using `def` bindings to declare parameter constants.
*   **Main Entry Point**: Defined using the `defn -main []` block to encapsulate procedural register transactions.

---

## 2. JCL to Closure Language Mapping

| Legacy JCL Card | Closure Language Equivalent | Description |
| :--- | :--- | :--- |
| `//JOBNAME JOB` | `:wmq-job "JOBNAME"` | Defines the job identifier. |
| `//STEP EXEC PGM=IKFCBL00` | `:wmq-compiler "FOLKLORE"` | Identifies the compiler program pass. |
| `//STEP EXEC PGM=GOSTINT` | `:wmq-mount "GOSTINT"` | Specifies the active program execution target. |
| `//DD DSN=DATASET` | `:wmq-params {:key "VAL"}` | Maps data definitions directly to metadata map keys. |

---

## 3. Compiler Security Auditing
The compiler validates 100% of the JCL parameters parsed from the **Closure** metadata map before emitting VM bytecode:

1.  **Presence Verification**: Asserts that `:wmq-job`, `:wmq-compiler`, `:wmq-mount`, `:wmq-author`, `:tin`, and `:ssn` are present in the strategy namespace.
2.  **Format Constraints**:
    *   `TIN` must be exactly 9 numeric digits.
    *   `SSN` must be exactly 9 numeric digits.
3.  **Audit Abort**: Trigger active compilation aborts if a non-compliant parameter layout is parsed, preventing out-of-bounds register execution.

---

## 4. Standard Strategy Example
Below is the standard representation of the [`gost_intrusion.strategy`](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/dysnomia/domain/strategies/gost_intrusion.strategy) file:

```lisp
^{:wmq-job "ESEVJOB"
  :wmq-compiler "FOLKLORE"
  :wmq-mount "GOSTINT"
  :wmq-author "ADVERSARY"
  :wmq-params {:tin 950000000 :ssn 050051122 :gost-key "KGBKEY01"}}

(def ws-ssn-tin "050051122")
(def ws-gost-key "KGBKEY01")

(defn -main []
  (wmq-reg-write 1 950000000)
  (wmq-reg-write 2 050051122))
```
