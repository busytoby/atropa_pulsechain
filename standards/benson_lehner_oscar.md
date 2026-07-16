# Benson-Lehner OSCAR Calibration Standard Certification

## 1. Specification Overview
This certification sheet documents compliance with the Benson-Lehner OSCAR calibration reader standards.

## 2. Calibration Functionality
Analog trace signal amplitudes are calibrated and digitized using linear or polynomial interpolation curves:
* **Segment Interpolation:** Traces mapping across overlay boundaries are mapped via localized intervals.
* **Polynomial Calibration:** Traces mapping through curved sensor bounds are evaluated using order-$N$ polynomial curves:
  $$y = \sum_{i=0}^{N-1} c_i x^i$$

## 3. Verified Implementations
* **Segment Reader:** [tsfi_s370_oscar_reader](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_ramac_layout.c)
* **Polynomial Reader:** [tsfi_s370_oscar_reader_polynomial](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_ramac_layout.c)
