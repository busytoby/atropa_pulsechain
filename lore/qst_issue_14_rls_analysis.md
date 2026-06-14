# QST Issue #14 RLS Convergence Analysis: "One Thousand Listening Ears"

This document presents the mathematical model and implementation strategy for replacing the Least Mean Squares (LMS) adaptive filter with a Recursive Least Squares (RLS) filter to accelerate jammer cancellation.

---

## 1. LMS vs. RLS Mathematics

### A. Least Mean Squares (LMS)
The LMS algorithm is a stochastic gradient descent method that minimizes the mean squared error. The weight update equation is:
$$\mathbf{w}[n+1] = \mathbf{w}[n] + \mu \cdot e[n] \cdot \mathbf{x}[n]$$
where $\mu$ is the step-size parameter, $\mathbf{x}[n]$ is the input tap vector, and $e[n] = d[n] - \mathbf{w}^T[n]\mathbf{x}[n]$ is the error.
*   **Limitation**: LMS convergence speed depends heavily on the eigenvalue spread (conditioning) of the input covariance matrix. Under high power variations or pure tone interference, convergence is slow.

### B. Recursive Least Squares (RLS)
The RLS algorithm minimizes a weighted linear least squares cost function, using a forgetting factor $\lambda \in (0, 1]$ to weight recent samples more heavily:
$$J[n] = \sum_{i=1}^{n} \lambda^{n-i} e^2[i]$$
The RLS filter updates the inverse correlation matrix $\mathbf{P}[n] = \mathbf{R}_{xx}^{-1}[n]$ recursively using the matrix inversion lemma:

1.  **Filter Output**:
    $$y[n] = \mathbf{w}^T[n-1] \mathbf{x}[n]$$
2.  **A Priori Error**:
    $$\alpha[n] = d[n] - y[n]$$
3.  **Gain Vector Update**:
    $$\mathbf{k}[n] = \frac{\mathbf{P}[n-1] \mathbf{x}[n]}{\lambda + \mathbf{x}^T[n] \mathbf{P}[n-1] \mathbf{x}[n]}$$
4.  **Inverse Covariance Matrix Update**:
    $$\mathbf{P}[n] = \frac{1}{\lambda} \left( \mathbf{P}[n-1] - \mathbf{k}[n] \mathbf{x}^T[n] \mathbf{P}[n-1] \right)$$
5.  **Weight Vector Update**:
    $$\mathbf{w}[n] = \mathbf{w}[n-1] + \mathbf{k}[n] \alpha[n]$$

*   **Advantage**: RLS offers near-instantaneous convergence (typically $2L$ samples, where $L$ is the number of taps) and is independent of the input signal conditioning.

---

## 2. Implementation Strategy

We will integrate both LMS and RLS filters into the jammer cancellation stage of the spatial combining network:
*   A target transmitter broadcasts the spy signal.
*   A high-power jammer broadcasts an interfering tone.
*   We use the closest station to the jammer as a reference channel.
*   We run both LMS and RLS to cancel the jammer on the combined output.
*   We assert that RLS achieves a correlation $> 0.85$ within the first $0.05$ seconds of the simulation, while LMS fails to converge in that window (correlation $< 0.40$).
