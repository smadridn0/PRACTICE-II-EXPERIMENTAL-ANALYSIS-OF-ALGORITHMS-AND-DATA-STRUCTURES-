#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <random>
#include <cmath>
#include <numeric>
#include <iomanip>
#include <string>
#include <climits>
#include <functional>

// ============================================================
//  UTILITIES
// ============================================================

// Calculates universe size U = max - min + 1
// Returns false if U exceeds the maximum allowed size
static std::pair<bool, long long> universe_size(int mn, int mx) {
    long long U = (long long)mx - mn + 1;
    const long long MAX_U = 100000000LL;
    return {U <= MAX_U, U};
}

// Reads dataset.txt line by line
// First line: number of records
// Following lines: one integer per line
std::vector<int> load_dataset(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "[ERROR] Cannot open file: " << path << "\n";
        std::cerr << "        Make sure dataset.txt is in the same folder.\n";
        return {};
    }
    size_t n;
    f >> n;                          // read total number of records
    std::vector<int> v;
    v.reserve(n);                    // reserve memory in advance
    int x;
    while (f >> x) v.push_back(x);  // read each number line by line
    f.close();
    std::cout << "  [FILE] Loaded " << v.size() << " records from \"" << path << "\"\n";
    return v;
}

// Generates n random integers in range [0, U-1]
std::vector<int> generate_random(size_t n, int U) {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(0, U - 1);
    std::vector<int> v(n);
    for (auto& x : v) x = dist(rng);
    return v;
}

// Generates n nearly sorted integers with 5% random swaps
std::vector<int> generate_nearly_sorted(size_t n, int U) {
    std::vector<int> v(n);
    std::mt19937 rng(42);
    for (size_t i = 0; i < n; ++i) v[i] = (int)(i % U);
    int swaps = (int)(n * 0.05);
    std::uniform_int_distribution<int> idist(0, (int)n - 1);
    for (int i = 0; i < swaps; ++i) std::swap(v[idist(rng)], v[idist(rng)]);
    return v;
}

// Generates n integers in reverse order
std::vector<int> generate_reverse(size_t n, int U) {
    std::vector<int> v(n);
    for (size_t i = 0; i < n; ++i) v[i] = (int)((n - 1 - i) % U);
    return v;
}

// ============================================================
//  DIALSORT
// ============================================================

static bool dialsort(std::vector<int>& a) {
    const size_t n = a.size();
    if (n <= 1) return true;

    // Phase 1: find min and max to compute universe size U
    int mn = a[0], mx = a[0];
    for (size_t i = 1; i < n; ++i) {
        if (a[i] < mn) mn = a[i];
        if (a[i] > mx) mx = a[i];
    }

    // Check that U does not exceed maximum allowed size
    auto [ok, U64] = universe_size(mn, mx);
    if (!ok) { std::cerr << "[WARN] dialsort: U > MAX_U\n"; return false; }

    // Phase 2: build histogram H where H[value - min] = frequency
    const size_t U = static_cast<size_t>(U64);
    std::vector<int> H(U, 0);
    for (size_t i = 0; i < n; ++i)
        H[static_cast<size_t>(a[i] - mn)]++;

    // Phase 3: reconstruct sorted array by reading histogram in order
    size_t out = 0;
    for (size_t y = 0; y < U; ++y) {
        const int val = static_cast<int>(y) + mn;
        for (int c = H[y]; c > 0; --c)
            a[out++] = val;
    }
    return true;
}

// ============================================================
//  BITONIC SORT
// ============================================================

// Performs the bitonic merge step
// Compares elements at distance k and swaps if needed
static void bitonic_merge(std::vector<int>& a, int lo, int cnt, bool ascending) {
    if (cnt <= 1) return;
    int k = cnt / 2;
    for (int i = lo; i < lo + k; ++i)
        if ((a[i] > a[i + k]) == ascending)
            std::swap(a[i], a[i + k]);
    // Recursively merge both halves
    bitonic_merge(a, lo,     k, ascending);
    bitonic_merge(a, lo + k, k, ascending);
}

// Recursively builds bitonic sequences
// First half sorted ascending, second half descending
static void bitonic_sort_rec(std::vector<int>& a, int lo, int cnt, bool ascending) {
    if (cnt <= 1) return;
    int k = cnt / 2;
    bitonic_sort_rec(a, lo,     k, true);   // sort first half ascending
    bitonic_sort_rec(a, lo + k, k, false);  // sort second half descending
    bitonic_merge(a, lo, cnt, ascending);   // merge into sorted sequence
}

// Main bitonic sort function
// Pads array to next power of 2, sorts, then removes padding
static void bitonic_sort(std::vector<int>& a) {
    size_t n = a.size();
    if (n <= 1) return;

    // Pad to next power of 2 using INT_MAX as sentinel
    size_t pw = 1;
    while (pw < n) pw <<= 1;
    a.resize(pw, INT_MAX);

    bitonic_sort_rec(a, 0, (int)pw, true);

    // Remove padding — restore original size
    a.resize(n);
}

// ============================================================
//  BENCHMARK
// ============================================================

// Stores all benchmark metrics for one algorithm run
struct BenchResult {
    double time_ms;       // execution time in milliseconds
    double mean;          // mean value of sorted array
    double stddev;        // standard deviation
    double throughput;    // elements sorted per second
    size_t memory_bytes;  // estimated memory usage in bytes
};

// Runs DialSort on a copy of data and measures all metrics
BenchResult benchmark_dialsort(std::vector<int> data) {
    size_t n = data.size();
    int mn = *std::min_element(data.begin(), data.end());
    int mx = *std::max_element(data.begin(), data.end());
    auto [ok, U64] = universe_size(mn, mx);

    // Memory = histogram array (U ints) + input array (n ints)
    size_t mem = (ok ? (size_t)U64 : 0) * sizeof(int) + n * sizeof(int);

    // Measure execution time
    auto t0 = std::chrono::high_resolution_clock::now();
    dialsort(data);
    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    // Calculate mean and standard deviation
    double sum = 0; for (auto x : data) sum += x;
    double mean = sum / n;
    double var = 0; for (auto x : data) var += ((double)x - mean)*((double)x - mean);
    return {ms, mean, std::sqrt(var/n), (double)n/(ms/1000.0), mem};
}

// Runs Bitonic Sort on a copy of data and measures all metrics
BenchResult benchmark_bitonic(std::vector<int> data) {
    size_t n = data.size();

    // Memory = padded array (next power of 2 ints)
    size_t pw = 1; while (pw < n) pw <<= 1;
    size_t mem = pw * sizeof(int);

    // Measure execution time
    auto t0 = std::chrono::high_resolution_clock::now();
    bitonic_sort(data);
    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    // Calculate mean and standard deviation
    double sum = 0; for (auto x : data) sum += x;
    double mean = sum / n;
    double var = 0; for (auto x : data) var += ((double)x - mean)*((double)x - mean);
    return {ms, mean, std::sqrt(var/n), (double)n/(ms/1000.0), mem};
}

// ============================================================
//  ASCII VISUALIZATION
// ============================================================

// Prints a separator line of repeated characters
void print_separator(char c = '=', int w = 76) {
    for (int i = 0; i < w; ++i) std::cout << c;
    std::cout << '\n';
}

// Shows DialSort step by step on a small sample of 10 elements
void visualize_dialsort(std::vector<int> sample) {
    std::cout << "\n  [DIALSORT - STEP BY STEP VISUALIZATION (10 elements)]\n";
    print_separator('-');

    // Step 1: find min, max and universe size
    int mn = sample[0], mx = sample[0];
    for (auto x : sample) { mn = std::min(mn,x); mx = std::max(mx,x); }
    long long U = (long long)mx - mn + 1;
    std::cout << "  STEP 1 - Find range:\n";
    std::cout << "           min=" << mn << "  max=" << mx << "  U=" << U << "\n\n";

    // Step 2: show original input array
    std::cout << "  STEP 2 - Input array:\n           [ ";
    for (auto x : sample) std::cout << x << " ";
    std::cout << "]\n\n";

    // Step 3: build and show histogram
    std::vector<int> H(U, 0);
    for (auto x : sample) H[x - mn]++;
    std::cout << "  STEP 3 - Histogram H[value-min]=count:\n           ";
    for (long long y = 0; y < U; ++y)
        if (H[y] > 0) std::cout << "[" << (y+mn) << "]=" << H[y] << "  ";
    std::cout << "\n\n";

    // Step 4: reconstruct and show sorted output
    std::vector<int> out;
    for (long long y = 0; y < U; ++y)
        for (int c = 0; c < H[y]; ++c)
            out.push_back((int)y + mn);
    std::cout << "  STEP 4 - Sorted output:\n           [ ";
    for (auto x : out) std::cout << x << " ";
    std::cout << "]\n";
    print_separator('-');
}

// Shows Bitonic Sort step by step on a small sample of 8 elements
void visualize_bitonic(std::vector<int> sample) {
    while (sample.size() > 8) sample.pop_back();

    // Pad to next power of 2
    size_t pw = 1;
    while (pw < sample.size()) pw <<= 1;
    sample.resize(pw, 9999);

    std::cout << "\n  [BITONIC SORT - STEP BY STEP VISUALIZATION (8 elements)]\n";
    print_separator('-');

    // Step 1: show padded array
    std::cout << "  STEP 1 - Pad to next power of 2 (n=" << pw << "):\n           [ ";
    for (auto x : sample)
        std::cout << (x == 9999 ? "PAD" : std::to_string(x)) << " ";
    std::cout << "]\n\n";

    // Step 2: explain the merge stages
    std::cout << "  STEP 2 - Build bitonic sequences:\n";
    std::cout << "           Stage k=1: compare/swap pairs     (distance=1)\n";
    std::cout << "           Stage k=2: merge size-4 sequences (distance=2)\n";
    std::cout << "           Stage k=3: final merge to sorted  (distance=4)\n\n";

    // Step 3: show sorted result
    bitonic_sort_rec(sample, 0, (int)pw, true);
    std::cout << "  STEP 3 - Sorted result:\n           [ ";
    for (auto x : sample)
        if (x != 9999) std::cout << x << " ";
    std::cout << "]\n";
    print_separator('-');
}

// Prints a formatted results table comparing both algorithms
void print_results_table(const std::string& label, size_t n, int U,
                         const BenchResult& rd, const BenchResult& rb) {
    std::cout << "\n";
    print_separator();

    // Header: dataset name, size and universe
    std::cout << "  DATASET: " << label << " | N=" << n << " | U=" << U << "\n";
    print_separator();

    // Column titles
    std::cout << std::left
              << std::setw(26) << "Metric"
              << std::setw(22) << "DialSort"
              << std::setw(22) << "BitonicSort" << "\n";
    print_separator('-');

    // Rows with metrics
    std::cout << std::left << std::setw(26) << "Time (ms)"
              << std::fixed << std::setprecision(3)
              << std::setw(22) << rd.time_ms
              << std::setw(22) << rb.time_ms << "\n";
    std::cout << std::left << std::setw(26) << "Mean value"
              << std::fixed << std::setprecision(2)
              << std::setw(22) << rd.mean
              << std::setw(22) << rb.mean << "\n";
    std::cout << std::left << std::setw(26) << "Std Deviation"
              << std::fixed << std::setprecision(2)
              << std::setw(22) << rd.stddev
              << std::setw(22) << rb.stddev << "\n";
    std::cout << std::left << std::setw(26) << "Throughput (M elem/s)"
              << std::fixed << std::setprecision(3)
              << std::setw(22) << rd.throughput/1e6
              << std::setw(22) << rb.throughput/1e6 << "\n";
    std::cout << std::left << std::setw(26) << "Memory (KB)"
              << std::setw(22) << rd.memory_bytes/1024
              << std::setw(22) << rb.memory_bytes/1024 << "\n";

    // Winner
    print_separator('-');
    std::cout << "  WINNER (speed): "
              << (rd.time_ms < rb.time_ms ? "DialSort" : "BitonicSort") << "\n";
    print_separator();
}

// Prints Big-O complexity table for both algorithms
void print_complexity() {
    std::cout << "\n";
    print_separator('*');
    std::cout << "  BIG-O COMPLEXITY ANALYSIS\n";
    print_separator('*');
    std::cout << std::left
              << std::setw(16) << "Algorithm"
              << std::setw(18) << "Best"
              << std::setw(18) << "Average"
              << std::setw(18) << "Worst"
              << "Space\n";
    print_separator('-');
    std::cout << std::left
              << std::setw(16) << "DialSort"
              << std::setw(18) << "O(n + U)"
              << std::setw(18) << "O(n + U)"
              << std::setw(18) << "O(n + U)"
              << "O(n + U)\n";
    std::cout << std::left
              << std::setw(16) << "BitonicSort"
              << std::setw(18) << "O(n log^2 n)"
              << std::setw(18) << "O(n log^2 n)"
              << std::setw(18) << "O(n log^2 n)"
              << "O(n log^2 n)\n";
    print_separator('*');
    std::cout << "  U = universe size = max - min + 1\n";
    std::cout << "  DialSort is LINEAR when U = O(n).\n";
    std::cout << "  DialSort DEGRADES when U >> n (sparse universe).\n";
    std::cout << "  BitonicSort is comparison-based and parallel-friendly.\n";
    print_separator('*');
}

// ============================================================
//  MAIN
// ============================================================

int main() {
    print_separator('*');
    std::cout << "  DATA STRUCTURES AND ALGORITHMS\n";
    std::cout << "  PRACTICE II: DialSort vs Bitonic Sort\n";
    print_separator('*');

    // Show step-by-step visualization for both algorithms
    std::vector<int> sample = {42, 7, 19, 3, 55, 27, 11, 3, 42, 8};
    visualize_dialsort(sample);
    visualize_bitonic(sample);

    // Show Big-O complexity table
    print_complexity();

    // BENCHMARK 0: read and sort data from dataset.txt
    print_separator('*');
    std::cout << "  BENCHMARK 0: FROM FILE (dataset.txt)\n";
    print_separator('*');

    std::vector<int> file_data = load_dataset("dataset.txt");
    if (!file_data.empty()) {
        int mn = *std::min_element(file_data.begin(), file_data.end());
        int mx = *std::max_element(file_data.begin(), file_data.end());
        int file_U = mx - mn + 1;
        std::cout << "  Records: " << file_data.size()
                  << " | Min: " << mn
                  << " | Max: " << mx
                  << " | U: " << file_U << "\n";
        BenchResult rd = benchmark_dialsort(file_data);
        BenchResult rb = benchmark_bitonic(file_data);
        print_results_table("dataset.txt", file_data.size(), file_U, rd, rb);
    }

    // BENCHMARK 1: vary n from 100K to 10M with fixed U = 1,000,000
    // Tests three data distributions: Random, Nearly Sorted, Reverse Sorted
    print_separator('*');
    std::cout << "  BENCHMARK 1: VARYING N (U = 1,000,000 fixed)\n";
    std::cout << "  Distributions: Random | Nearly Sorted | Reverse Sorted\n";
    print_separator('*');

    struct DistSpec {
        std::string name;
        std::function<std::vector<int>(size_t, int)> gen;
    };

    std::vector<DistSpec> dists = {
        {"Random Uniform", [](size_t n, int U){ return generate_random(n, U); }},
        {"Nearly Sorted",  [](size_t n, int U){ return generate_nearly_sorted(n, U); }},
        {"Reverse Sorted", [](size_t n, int U){ return generate_reverse(n, U); }},
    };

    std::vector<size_t> sizes = {100000, 500000, 1000000, 5000000, 10000000};
    int fixed_U = 1000000;

    for (auto& [dname, gen] : dists) {
        std::cout << "\n  -- Distribution: " << dname << " --\n";
        for (size_t n : sizes) {
            std::cout << "  Generating n=" << n << " U=" << fixed_U << "...\n";
            std::vector<int> data = gen(n, fixed_U);
            BenchResult rd = benchmark_dialsort(data);
            BenchResult rb = benchmark_bitonic(data);
            print_results_table(dname, n, fixed_U, rd, rb);
        }
    }

    // BENCHMARK 2: vary U from 1K to 50M with fixed N = 1,000,000
    // Shows how universe size directly affects DialSort performance
    print_separator('*');
    std::cout << "  BENCHMARK 2: VARYING U (N = 1,000,000 fixed)\n";
    std::cout << "  Shows how universe size affects DialSort performance\n";
    print_separator('*');

    size_t fixed_N = 1000000;
    struct USpec { std::string label; int U; };
    std::vector<USpec> universes = {
        {"U small  (U=1K,   U<<n)", 1000},
        {"U medium (U=100K, U<n)",  100000},
        {"U equal  (U=1M,   U=n)",  1000000},
        {"U large  (U=10M,  U>n)",  10000000},
        {"U xlarge (U=50M,  U>>n)", 50000000},
    };

    for (auto& [ulabel, U] : universes) {
        std::cout << "\n  -- " << ulabel << " --\n";
        std::vector<int> data = generate_random(fixed_N, U);
        BenchResult rd = benchmark_dialsort(data);
        BenchResult rb = benchmark_bitonic(data);
        print_results_table("Random Uniform", fixed_N, U, rd, rb);
    }

    // BENCHMARK 3: vary both N and U together
    // Shows combined effect of input size and universe size
    print_separator('*');
    std::cout << "  BENCHMARK 3: VARYING N AND U TOGETHER\n";
    print_separator('*');

    struct ComboSpec { size_t n; int U; std::string label; };
    std::vector<ComboSpec> combos = {
        {100000,    1000,     "n=100K,  U=1K   (U<<n, DialSort favored)"},
        {100000,    1000000,  "n=100K,  U=1M   (U>>n, Bitonic favored)"},
        {1000000,   1000000,  "n=1M,    U=1M   (U=n,  DialSort favored)"},
        {1000000,   50000000, "n=1M,    U=50M  (U>>n, Bitonic favored)"},
        {10000000,  1000000,  "n=10M,   U=1M   (U<n,  DialSort favored)"},
        {10000000,  10000000, "n=10M,   U=10M  (U=n,  DialSort favored)"},
    };

    for (auto& [n, U, lbl] : combos) {
        std::cout << "\n  -- " << lbl << " --\n";
        std::vector<int> data = generate_random(n, U);
        BenchResult rd = benchmark_dialsort(data);
        BenchResult rb = benchmark_bitonic(data);
        print_results_table(lbl, n, U, rd, rb);
    }

    std::cout << "\n  All benchmarks complete.\n";
    print_separator('*');
    return 0;
}
