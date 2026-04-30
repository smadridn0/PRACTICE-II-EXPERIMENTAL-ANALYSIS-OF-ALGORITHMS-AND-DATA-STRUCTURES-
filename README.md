## ⚡ Practice II: DialSort vs Bitonic Sort

---

## 👥 Team Members

| Name | 
|------|
| Matias Cardona Acosta | 
| Simon Madrid Norena | 

---

## 🚀 Execution Instructions

![CLion](https://img.shields.io/badge/CLion-Recommended-black?style=flat-square&logo=clion)

**Requirements:** CLion with C++17 support installed.

**Steps:**
1. Download the ZIP file from the GitHub repository (green button **Code** -> **Download ZIP**)
2. Extract the ZIP file on your computer
3. Open CLion
4. Click **Open** and select the extracted folder
5. CLion will detect the CMakeLists.txt automatically — click **Trust Project**
6. Click the green **Run** button
7. CLion will compile and execute the program automatically

> **Dataset:** The dataset.txt file is included in the ZIP. Once CLion runs the project for the first time it creates the cmake-build-debug folder. Copy dataset.txt into that folder so Benchmark 0 can read it.

> **Interactive Simulation:** Open simulacion.html directly in any modern browser. No server required.

---

## 📂 How the Program Uses the Dataset

The program has 4 benchmarks. Only Benchmark 0 reads the dataset.txt file.
The rest generate their own data in memory to vary n, U and distribution.

| Benchmark | Data Source | Purpose |
|-----------|-------------|---------|
| 🗂️ Benchmark 0 | Reads dataset.txt (10,000,000 records) | Sort real file data |
| ⚙️ Benchmark 1 | Generated in memory | Vary n: 100K to 10M |
| ⚙️ Benchmark 2 | Generated in memory | Vary U: 1K to 50M |
| ⚙️ Benchmark 3 | Generated in memory | Vary n and U together |

> ⚠️ **IMPORTANT:** dataset.txt must be placed in the cmake-build-debug folder. If the file is not found, Benchmark 0 is skipped but the rest run normally.

---

# 📋 Technical Report

## 1. 🔧 Implementation Approach

### 1.1 DialSort

DialSort is a non-comparison sorting algorithm based on the histogram counting technique. It operates in three phases:

- **Phase 1 - Find range:** Scans the array once to find min and max, computing `U = max - min + 1`
- **Phase 2 - Build histogram:** Allocates array `H` of size `U` and counts frequency of each value
- **Phase 3 - Reconstruct:** Reads histogram in order and writes sorted values back to the output array

| Case | Time | Space |
|------|------|-------|
| ✅ Best | O(n + U) | O(n + U) |
| ✅ Average | O(n + U) | O(n + U) |
| ✅ Worst | O(n + U) | O(n + U) |

> DialSort is **stable** and runs in **linear time** when U = O(n). It degrades when U is much greater than n because the histogram becomes very large with mostly empty buckets.

---

### 1.2 Bitonic Sort

Bitonic Sort is a comparison-based sorting network designed for parallel hardware. It works in three phases:

- **Phase 1 - Pad to power of 2:** Pads the array to the next power of 2 using `INT_MAX` as sentinel
- **Phase 2 - Build bitonic sequences:** Recursively sorts the first half ascending and second half descending
- **Phase 3 - Bitonic merge:** Repeatedly compares and swaps elements at fixed distances until fully sorted

| Case | Time | Space |
|------|------|-------|
| ✅ Best | O(n log² n) | O(n log² n) |
| ✅ Average | O(n log² n) | O(n log² n) |
| ✅ Worst | O(n log² n) | O(n log² n) |

> Bitonic Sort is **not stable** but has **predictable performance** regardless of distribution or universe size.

---

## 2. 📈 Performance Measurements

### 2.1 Benchmark 0 - File Dataset (dataset.txt - 10,000,000 records)

| Metric | DialSort | Bitonic Sort |
|--------|:--------:|:-----------:|
| ⏱️ Time (ms) | **80.933** | 14,547.092 |
| 🚀 Throughput (M/s) | **123.559** | 0.687 |
| 🧠 Memory (KB) | 42,968 | 65,536 |
| 📊 Mean value | 499,980.24 | 499,980.24 |
| 📉 Std Deviation | 288,652.91 | 288,652.91 |
| 🏆 Winner | **DialSort** | - |



---

### 2.2 Benchmark 1 - Varying N (U = 1,000,000 fixed)

#### 🎲 Random Uniform

| N | DialSort (ms) | Bitonic (ms) | DialSort (M/s) | Bitonic (M/s) | Dial Mem (KB) | Bitonic Mem (KB) | 🏆 |
|---|:------------:|:-----------:|:--------------:|:-------------:|:-------------:|:----------------:|:--:|
| 100,000 | **3.940** | 61.791 | 25.381 | 1.618 | 4,296 | 512 | Dial |
| 500,000 | **9.976** | 347.011 | 50.123 | 1.441 | 5,859 | 2,048 | Dial |
| 1,000,000 | **14.600** | 716.119 | 68.492 | 1.396 | 7,812 | 4,096 | Dial |
| 5,000,000 | **46.126** | 6,750.496 | 108.398 | 0.741 | 23,437 | 32,768 | Dial |
| 10,000,000 | **90.846** | 14,363.480 | 110.077 | 0.696 | 42,968 | 65,536 | Dial |

#### 📈 Nearly Sorted

| N | DialSort (ms) | Bitonic (ms) | DialSort (M/s) | Bitonic (M/s) | Dial Mem (KB) | Bitonic Mem (KB) | 🏆 |
|---|:------------:|:-----------:|:--------------:|:-------------:|:-------------:|:----------------:|:--:|
| 100,000 | **0.677** | 58.186 | 147.754 | 1.719 | 781 | 512 | Dial |
| 500,000 | **3.608** | 296.781 | 138.566 | 1.685 | 3,906 | 2,048 | Dial |
| 1,000,000 | **7.512** | 637.416 | 133.122 | 1.569 | 7,812 | 4,096 | Dial |
| 5,000,000 | **29.107** | 6,157.689 | 171.780 | 0.812 | 23,437 | 32,768 | Dial |
| 10,000,000 | **66.163** | 13,649.818 | 151.142 | 0.733 | 42,968 | 65,536 | Dial |

#### 🔽 Reverse Sorted

| N | DialSort (ms) | Bitonic (ms) | DialSort (M/s) | Bitonic (M/s) | Dial Mem (KB) | Bitonic Mem (KB) | 🏆 |
|---|:------------:|:-----------:|:--------------:|:-------------:|:-------------:|:----------------:|:--:|
| 100,000 | **0.736** | 52.607 | 135.888 | 1.901 | 781 | 512 | Dial |
| 500,000 | **4.164** | 272.237 | 120.083 | 1.837 | 3,906 | 2,048 | Dial |
| 1,000,000 | **8.759** | 589.357 | 114.166 | 1.697 | 7,812 | 4,096 | Dial |
| 5,000,000 | **29.809** | 6,145.686 | 167.732 | 0.814 | 23,437 | 32,768 | Dial |
| 10,000,000 | **60.243** | 12,690.902 | 165.993 | 0.788 | 42,968 | 65,536 | Dial |

---

### 2.3 Benchmark 2 - Varying U (N = 1,000,000 fixed)

| U | Scenario | DialSort (ms) | Bitonic (ms) | DialSort (M/s) | Bitonic (M/s) | Dial Mem (KB) | Bitonic Mem (KB) | 🏆 |
|---|----------|:------------:|:-----------:|:--------------:|:-------------:|:-------------:|:----------------:|:--:|
| 1,000 | U much less than n | **4.771** | 783.321 | 209.622 | 1.277 | 3,910 | 4,096 | Dial |
| 100,000 | U less than n | **5.862** | 710.094 | 170.596 | 1.408 | 4,296 | 4,096 | Dial |
| 1,000,000 | U equals n | **18.590** | 736.643 | 53.792 | 1.358 | 7,812 | 4,096 | Dial |
| 10,000,000 | U greater than n | **53.477** | 790.788 | 18.700 | 1.265 | 42,968 | 4,096 | Dial |
| 50,000,000 | U much greater than n | **172.179** | 726.685 | 5.808 | 1.376 | 199,218 | 4,096 | Dial |

> ⚠️ As U grows from 1K to 50M, DialSort time increases 36x (4.7ms to 172ms) while Bitonic Sort stays stable around 750ms. DialSort memory grows from 3.9MB to 199MB.

---

### 2.4 Benchmark 3 - Varying N and U Together

| N | U | Scenario | DialSort (ms) | Bitonic (ms) | Dial Mem (KB) | Bitonic Mem (KB) | 🏆 |
|---|---|----------|:------------:|:-----------:|:-------------:|:----------------:|:--:|
| 100,000 | 1,000 | U much less than n | **0.448** | 67.693 | 394 | 512 | Dial |
| 100,000 | 1,000,000 | U much greater than n | **5.403** | 70.624 | 4,296 | 512 | Dial |
| 1,000,000 | 1,000,000 | U equals n | **16.684** | 730.859 | 7,812 | 4,096 | Dial |
| 1,000,000 | 50,000,000 | U much greater than n | **172.692** | 741.643 | 199,218 | 4,096 | Dial |
| 10,000,000 | 1,000,000 | U less than n | **84.745** | 14,533.008 | 42,968 | 65,536 | Dial |
| 10,000,000 | 10,000,000 | U equals n | **364.658** | 14,766.256 | 78,124 | 65,536 | Dial |

---

## 3. ⚖️ Comparison Between Algorithms

### 3.1 Feature Comparison

| Feature | DialSort | Bitonic Sort |
|---------|:--------:|:-----------:|
| Best Case | O(n + U) | O(n log² n) |
| Average Case | O(n + U) | O(n log² n) |
| Worst Case | O(n + U) | O(n log² n) |
| Space | O(n + U) | O(n log² n) |
| Stable | ✅ Yes | ❌ No |
| Parallel-ready | ❌ No | ✅ Yes |
| Integer-only | ✅ Yes | ❌ No |
| Affected by U | ⚠️ Yes | ✅ No |
| Affected by distribution | ✅ No | ✅ No |

### 3.2 🏆 Which Algorithm Performed Better?

**DialSort won in every single test under sequential execution:**

| Test | DialSort | Bitonic Sort | Speedup |
|------|:--------:|:-----------:|:-------:|
| n=10M from file | 80ms | 14,547ms | **179x faster** |
| n=10M Random | 90ms | 14,363ms | **158x faster** |
| n=10M Nearly Sorted | 66ms | 13,649ms | **206x faster** |
| n=10M Reverse Sorted | 60ms | 12,690ms | **211x faster** |
| n=10M, U=10M | 364ms | 14,766ms | **40x faster** |
| U=50M | 172ms | 726ms | **4x faster** |

> ⚠️ **DialSort weakness:** when U grows from 1K to 50M, DialSort slows down 36x and memory grows from 3.9MB to 199MB.

### 3.3 📌 Conclusions

- ✅ **DialSort** is the best choice for sequential execution when `U = O(n)`
- ✅ **Bitonic Sort** is the best choice for GPU or parallel environments
- ✅ **DialSort** is not affected by data distribution
- ⚠️ For very large sparse universes, Bitonic Sort becomes competitive in memory usage

---

## 📊 Big-O Summary

| Algorithm | Best | Average | Worst | Space | Stable | Parallel |
|-----------|:----:|:-------:|:-----:|:-----:|:------:|:--------:|
| ⚡ DialSort | O(n+U) | O(n+U) | O(n+U) | O(n+U) | ✅ | ❌ |
| 🔀 Bitonic Sort | O(n log²n) | O(n log²n) | O(n log²n) | O(n log²n) | ❌ | ✅ |
