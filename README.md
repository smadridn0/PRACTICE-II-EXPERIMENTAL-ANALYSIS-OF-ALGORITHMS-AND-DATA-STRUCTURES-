---

## Execution Instructions

### Requirements
- g++ compiler with C++17 support

### Compile
```bash
g++ -std=c++17 -O2 -o sorter main.cpp
```

### Run
```bash
./sorter          # Linux / Mac
sorter.exe        # Windows
```

### Interactive Simulation
Open `simulacion.html` directly in any modern browser. No server required.

---

# Technical Report

## 1. Implementation Approach

### 1.1 DialSort

DialSort is a non-comparison sorting algorithm based on the histogram (counting) technique. It operates in three phases:

**Phase 1 — Find range:** Scans the array once to find min and max, computing `U = max - min + 1`.

**Phase 2 — Build histogram:** Allocates array `H[U]` and counts frequency of each value: `H[a[i] - min]++`.

**Phase 3 — Reconstruct:** Reads histogram in order and writes sorted values back to the output array.

| Case | Time | Space |
|------|------|-------|
| Best | O(n + U) | O(n + U) |
| Average | O(n + U) | O(n + U) |
| Worst | O(n + U) | O(n + U) |

DialSort is **stable** and runs in **linear time** when U = O(n). It degrades when U >> n because the histogram becomes very large with mostly empty buckets.

---

### 1.2 Bitonic Sort

Bitonic Sort is a comparison-based sorting network designed for parallel hardware. It works in three phases:

**Phase 1 — Pad to power of 2:** Pads the array to the next power of 2 using `INT_MAX` as sentinel.

**Phase 2 — Build bitonic sequences:** Recursively sorts the first half ascending and second half descending, creating a bitonic sequence.

**Phase 3 — Bitonic merge:** Repeatedly compares and swaps elements at fixed distances until fully sorted.

| Case | Time | Space |
|------|------|-------|
| Best | O(n log^2 n) | O(n log^2 n) |
| Average | O(n log^2 n) | O(n log^2 n) |
| Worst | O(n log^2 n) | O(n log^2 n) |

Bitonic Sort is **not stable** but has **predictable performance** regardless of distribution or universe size.

---

## 2. Performance Measurements

### 2.1 Benchmark 1 — Varying N (U = 1,000,000 fixed)

#### Random Uniform

| N | DialSort (ms) | Bitonic (ms) | DialSort (M/s) | Bitonic (M/s) | Memory Dial (KB) | Memory Bitonic (KB) | Winner |
|---|--------------|-------------|----------------|---------------|------------------|---------------------|--------|
| 100,000 | 4.427 | 76.772 | 22.587 | 1.303 | 4,296 | 512 | DialSort |
| 500,000 | 11.977 | 380.770 | 41.746 | 1.313 | 5,859 | 2,048 | DialSort |
| 1,000,000 | 17.433 | 804.703 | 57.362 | 1.243 | 7,812 | 4,096 | DialSort |
| 5,000,000 | 56.592 | 7,507.507 | 88.351 | 0.666 | 23,437 | 32,768 | DialSort |
| 10,000,000 | 103.641 | 16,009.749 | 96.486 | 0.625 | 42,968 | 65,536 | DialSort |

#### Nearly Sorted

| N | DialSort (ms) | Bitonic (ms) | DialSort (M/s) | Bitonic (M/s) | Memory Dial (KB) | Memory Bitonic (KB) | Winner |
|---|--------------|-------------|----------------|---------------|------------------|---------------------|--------|
| 100,000 | 0.727 | 59.504 | 137.533 | 1.681 | 781 | 512 | DialSort |
| 500,000 | 5.429 | 329.817 | 92.091 | 1.516 | 3,906 | 2,048 | DialSort |
| 1,000,000 | 7.535 | 782.397 | 132.716 | 1.278 | 7,812 | 4,096 | DialSort |
| 5,000,000 | 37.497 | 6,619.146 | 133.343 | 0.755 | 23,437 | 32,768 | DialSort |
| 10,000,000 | 79.775 | 14,193.053 | 125.352 | 0.705 | 42,968 | 65,536 | DialSort |

#### Reverse Sorted

| N | DialSort (ms) | Bitonic (ms) | DialSort (M/s) | Bitonic (M/s) | Memory Dial (KB) | Memory Bitonic (KB) | Winner |
|---|--------------|-------------|----------------|---------------|------------------|---------------------|--------|
| 100,000 | 0.800 | 63.794 | 125.000 | 1.568 | 781 | 512 | DialSort |
| 500,000 | 4.370 | 420.930 | 114.411 | 1.188 | 3,906 | 2,048 | DialSort |
| 1,000,000 | 9.783 | 817.846 | 102.214 | 1.223 | 7,812 | 4,096 | DialSort |
| 5,000,000 | 32.927 | 9,479.630 | 151.851 | 0.527 | 23,437 | 32,768 | DialSort |
| 10,000,000 | 109.119 | 20,311.965 | 91.643 | 0.492 | 42,968 | 65,536 | DialSort |

---

### 2.2 Benchmark 2 — Varying U (N = 1,000,000 fixed)

> Shows how universe size U directly impacts DialSort performance.

| U | Scenario | DialSort (ms) | Bitonic (ms) | DialSort (M/s) | Bitonic (M/s) | Dial Mem (KB) | Bitonic Mem (KB) | Winner |
|---|----------|--------------|-------------|----------------|---------------|---------------|------------------|--------|
| 1,000 | U << n | 6.905 | 1,090.344 | 144.823 | 0.917 | 3,910 | 4,096 | DialSort |
| 100,000 | U < n | 8.560 | 991.436 | 116.822 | 1.009 | 4,296 | 4,096 | DialSort |
| 1,000,000 | U = n | 19.152 | 993.523 | 52.214 | 1.007 | 7,812 | 4,096 | DialSort |
| 10,000,000 | U > n | 114.301 | 1,062.717 | 8.749 | 0.941 | 42,968 | 4,096 | DialSort |
| 50,000,000 | U >> n | 319.005 | 1,266.708 | 3.135 | 0.789 | 199,218 | 4,096 | DialSort |

**Key observation:** As U grows from 1K to 50M, DialSort time increases 46x (6.9ms to 319ms) while Bitonic Sort stays stable around ~1,000ms. DialSort memory also explodes from 3.9MB to 199MB.

---

### 2.3 Benchmark 3 — Varying N and U Together

| N | U | Scenario | DialSort (ms) | Bitonic (ms) | Dial Mem (KB) | Bitonic Mem (KB) | Winner |
|---|---|----------|--------------|-------------|---------------|------------------|--------|
| 100,000 | 1,000 | U << n | 0.484 | 82.212 | 394 | 512 | DialSort |
| 100,000 | 1,000,000 | U >> n | 7.493 | 148.641 | 4,296 | 512 | DialSort |
| 1,000,000 | 1,000,000 | U = n | 39.525 | 1,371.951 | 7,812 | 4,096 | DialSort |
| 1,000,000 | 50,000,000 | U >> n | 273.252 | 1,325.791 | 199,218 | 4,096 | DialSort |
| 10,000,000 | 1,000,000 | U < n | 118.885 | 17,979.648 | 42,968 | 65,536 | DialSort |
| 10,000,000 | 10,000,000 | U = n | 341.294 | 15,979.707 | 78,124 | 65,536 | DialSort |

---

## 3. Comparison Between Algorithms

### 3.1 Feature Comparison

| Feature | DialSort | Bitonic Sort |
|---------|----------|-------------|
| Best Case | O(n + U) | O(n log^2 n) |
| Average Case | O(n + U) | O(n log^2 n) |
| Worst Case | O(n + U) | O(n log^2 n) |
| Space | O(n + U) | O(n log^2 n) |
| Stable | Yes | No |
| Parallel-ready | No | Yes |
| Integer-only | Yes | No |
| Affected by U | Yes | No |
| Affected by distribution | No | No |

### 3.2 Which Algorithm Performed Better?

**DialSort won in every single test** under sequential execution:

- At n=10M, U=1M: DialSort **103ms** vs Bitonic **16,009ms** — DialSort is **154x faster**
- At n=10M, U=10M: DialSort **341ms** vs Bitonic **15,979ms** — DialSort is **46x faster**
- Even at U=50M (extreme sparse universe): DialSort **319ms** vs Bitonic **1,266ms** — DialSort still wins

However the results also reveal DialSort's weakness clearly: when U grows from 1K to 50M, DialSort slows down 46x and its memory usage grows from 3.9MB to 199MB.

**Conclusions:**
- **DialSort** is the best choice for sequential execution when U = O(n) (bounded integer universe)
- **Bitonic Sort** is the best choice for GPU/parallel environments, or when sorting non-integer data types
- For very large sparse universes (U >> n), Bitonic Sort becomes competitive in both speed and memory
