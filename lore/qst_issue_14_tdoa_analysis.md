# QST Issue #14 TDOA Localization Analysis: "One Thousand Listening Ears"

This document details the mathematical model and algorithm design for locating a target transmitter using Time Difference of Arrival (TDOA) cross-correlation and 2D spatial grid search.

---

## 1. Mathematical Model of TDOA

Given a spy transmitter at an unknown coordinate $(x_{\text{tx}}, y_{\text{tx}})$ and a set of $N$ receiver stations at known coordinates $(x_i, y_i)$, the signal propagation delay to station $i$ is:
$$\tau_i = \frac{d_i}{c} = \frac{\sqrt{(x_i - x_{\text{tx}})^2 + (y_i - y_{\text{tx}})^2}}{c}$$

Choosing receiver $0$ as the reference station, the Time Difference of Arrival (TDOA) between station $i$ and the reference station is:
$$\Delta \tau_{i,0} = \tau_i - \tau_0 = \frac{\sqrt{(x_i - x_{\text{tx}})^2 + (y_i - y_{\text{tx}})^2} - \sqrt{(x_0 - x_{\text{tx}})^2 + (y_0 - y_{\text{tx}})^2}}{c}$$

---

## 2. Estimating TDOAs via Cross-Correlation

We estimate $\Delta \tau_{i,0}$ by finding the time lag $\tau$ that maximizes the cross-correlation function $R_{i,0}(\tau)$ between the baseband-demodulated signals $y_i(t)$ and $y_0(t)$:
$$R_{i,0}(\tau) = \sum_{n} y_i[n] \cdot y_0[n - \tau]$$

Let $\tau_{\text{peak}, i}$ be the peak index of the cross-correlation:
$$\tau_{\text{peak}, i} = \arg\max_{\tau} R_{i,0}(\tau)$$

The TDOA estimate in seconds is:
$$\Delta \hat{\tau}_{i,0} = \frac{\tau_{\text{peak}, i}}{f_s}$$

---

## 3. 2D Spatial Grid Search Algorithm

To estimate the transmitter's position $(\hat{x}_{\text{tx}}, \hat{y}_{\text{tx}})$, we construct a search grid $(x_g, y_g)$ spanning the region of interest (e.g. $[-100, 100] \times [-100, 100]\text{ km}$).

For each candidate grid point $(x_g, y_g)$, we compute the modeled TDOA:
$$\Delta \tau_{i,0}^{\text{model}}(x_g, y_g) = \frac{\sqrt{(x_i - x_g)^2 + (y_i - y_g)^2} - \sqrt{(x_0 - x_g)^2 + (y_0 - y_g)^2}}{c}$$

We then evaluate a Mean Squared Error (MSE) cost function:
$$E(x_g, y_g) = \sum_{i=1}^{N-1} \left( \Delta \hat{\tau}_{i,0} - \Delta \tau_{i,0}^{\text{model}}(x_g, y_g) \right)^2$$

The grid cell minimizing $E(x_g, y_g)$ yields the localized position:
$$(\hat{x}_{\text{tx}}, \hat{y}_{\text{tx}}) = \arg\min_{(x_g, y_g)} E(x_g, y_g)$$
