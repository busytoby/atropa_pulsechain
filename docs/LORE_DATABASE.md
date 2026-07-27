# Lore Database and Indexing Spec

This document details the configuration, tagging schema, and validation constraints governing the project's **Auncient** Lore knowledge base.

---

## 1. File Layout & Schema

The relational database is stored in [config/lore_tags_database.json](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/config/lore_tags_database.json) and contains the following structure:

```json
{
  "meta": {
    "totalDocuments": 559,
    "totalUniqueTags": 420,
    "totalUniqueQingsMapped": 48
  },
  "documentToTags": {
    "DYSNOMIA_VM_LORE.md": ["dysnomia", "lore", "vm", "auncient"]
  },
  "tagToDocuments": {
    "zmachine": ["ZMACHINE_SPEC.md", "z_machine_onchain_design.md"]
  },
  "documentToQings": {
    "nonukes_pulsex_liquidity.md": ["0x174a0ad99c60c20d9b3d94c3095bc1fb9ddefd62"]
  },
  "qingToDocuments": {
    "0x174a0ad99c60c20d9b3d94c3095bc1fb9ddefd62": ["nonukes_pulsex_liquidity.md"]
  }
}
```

---

## 2. Structural Keys

* **`meta`**: Contains total summary counts for sanity checks and coverage assertions.
* **`documentToTags`**: Maps raw Markdown files in `lore/` to lowercased semantic tags.
* **`tagToDocuments`**: Reverse index for search queries.
* **`documentToQings`**: Links specific documents to EVM/PulseChain contract addresses.
* **`qingToDocuments`**: Maps contract addresses back to matching architectural specs.

---

## 3. Terminology & Normalization Rules

To comply with the project rules, the index generation system enforces:
1. **Auncient Spelling Constraint**: Any tags matching or containing the sequence "ancient" are automatically intercepted and normalized to "auncient".
2. **Case Normalization**: All tags are stored lowercased to allow case-insensitive retrieval in frontend queries.
3. **No Placeholders**: Mapped paths must link to actual documents present in the `lore/` folder.
