# IBM 380 Punched Card Standard Certification

## 1. Specification Overview
This certification sheet documents compliance with the IBM 80-column punched card standard BCD layout.

## 2. Character and Zone Mappings
Punched card fields represent numeric digit coordinates using the IBM character zone punches:
* **Positive Digits:** Mapped directly to BCD digits `0`–`9`.
* **Negative Digits:** Mapped to alphabetical zone symbols `J`–`R` representing `-1` through `-9`.

## 3. Verified Implementations
* **Card Parser:** [tsfi_s370_punched_card_to_comp3](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_ramac_layout.c)
