#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

struct Edge { uint32_t u, v; uint16_t cents, pad; };
static_assert(sizeof(Edge) == 12);
static constexpr uint32_t INF = std::numeric_limits<uint32_t>::max() / 4;
using Clock = std::chrono::steady_clock;

struct Result {
    std::vector<uint32_t> d, successor;
    std::vector<uint16_t> successor_weight;
    int passes = 0;
    double seconds = 0;
};

Result bellman(const std::vector<Edge>& edges, uint32_t n, uint32_t dest) {
    Result r;
    r.d.assign(size_t(n) + 1, INF);
    r.successor.assign(size_t(n) + 1, UINT32_MAX);
    r.successor_weight.assign(size_t(n) + 1, 0);
    r.d[dest] = 0;
    auto start = Clock::now();
    for (int pass = 1; pass <= 200; ++pass) {
        uint64_t changes = 0;
        for (const Edge& e : edges) {
            uint32_t dv = r.d[e.v];
            if (dv != INF && dv + e.cents < r.d[e.u]) {
                r.d[e.u] = dv + e.cents;
                r.successor[e.u] = e.v;
                r.successor_weight[e.u] = e.cents;
                ++changes;
            }
        }
        r.passes = pass;
        if (changes == 0) {
            r.seconds = std::chrono::duration<double>(Clock::now() - start).count();
            return r;
        }
    }
    throw std::runtime_error("no convergence within 200 passes");
}

void verify_path(const Result& r, uint32_t src, uint32_t dest, uint32_t n) {
    if (r.d[src] == INF) return;
    uint32_t x = src;
    uint64_t cost = 0;
    for (uint32_t steps = 0; steps <= n; ++steps) {
        if (x == dest) {
            if (cost != r.d[src]) throw std::runtime_error("path cost mismatch");
            std::cerr << "path " << src << "->" << dest << " steps=" << steps
                      << " cents=" << cost << '\n';
            return;
        }
        uint32_t next = r.successor[x];
        if (next > n) throw std::runtime_error("broken successor path");
        cost += r.successor_weight[x];
        x = next;
    }
    throw std::runtime_error("successor cycle");
}

void emit_distance(const std::string& key, uint32_t cents) {
    std::cout << key << ',';
    if (cents == INF) std::cout << "inf\n";
    else std::cout << cents / 100 << '.' << std::setw(2) << std::setfill('0')
                   << cents % 100 << std::setfill(' ') << '\n';
}

int main(int argc, char** argv) {
    try {
        if (argc != 4) throw std::runtime_error("usage: estudo_compacto edges.bin n repetitions");
        std::filesystem::path path(argv[1]);
        uint32_t n = static_cast<uint32_t>(std::stoul(argv[2]));
        int reps = std::stoi(argv[3]);
        if (n < 100 || reps < 1) throw std::runtime_error("invalid n/repetitions");
        uint64_t bytes = std::filesystem::file_size(path);
        if (bytes % sizeof(Edge) != 0) throw std::runtime_error("invalid binary size");
        size_t m = static_cast<size_t>(bytes / sizeof(Edge));
        std::vector<Edge> edges(m);
        FILE* f = std::fopen(path.string().c_str(), "rb");
        if (!f) throw std::runtime_error("cannot open binary file");
        size_t read = std::fread(edges.data(), sizeof(Edge), m, f);
        std::fclose(f);
        if (read != m) throw std::runtime_error("short binary read");
        for (const auto& e : edges)
            if (e.u > n || e.v > n || e.cents > 1000)
                throw std::runtime_error("invalid edge");
        std::cerr << "loaded n=" << n << " m=" << m << " bytes=" << bytes << '\n';
        std::cout << "n," << n << "\nm," << m << "\nrepeticoes," << reps << '\n';
        double total = 0;
        Result first;
        for (int i = 0; i < reps; ++i) {
            Result r = bellman(edges, n, 100);
            verify_path(r, 10, 100, n);
            verify_path(r, 20, 100, n);
            verify_path(r, 30, 100, n);
            if (i == 0) first = std::move(r);
            else for (uint32_t src : {10u, 20u, 30u})
                if (r.d[src] != first.d[src]) throw std::runtime_error("repetition mismatch");
            total += i == 0 ? first.seconds : r.seconds;
            std::cerr << "run " << (i + 1) << '/' << reps << " seconds="
                      << (i == 0 ? first.seconds : r.seconds) << " passes="
                      << (i == 0 ? first.passes : r.passes) << '\n';
        }
        std::cout << std::setprecision(12) << "bellman_destino100_media_s," << total / reps << '\n';
        std::cout << "bellman_destino100_passagens," << first.passes << '\n';
        for (uint32_t src : {10u, 20u, 30u})
            emit_distance("dist_" + std::to_string(src) + "_100", first.d[src]);
        for (uint32_t dest : {10u, 20u, 30u, 40u, 50u}) {
            std::cerr << "matrix destination " << dest << '\n';
            Result r = bellman(edges, n, dest);
            for (uint32_t src : {10u, 20u, 30u, 40u, 50u}) {
                verify_path(r, src, dest, n);
                emit_distance("bf_" + std::to_string(src) + "_" + std::to_string(dest), r.d[src]);
            }
            std::cerr << "matrix destination " << dest << " seconds=" << r.seconds
                      << " passes=" << r.passes << '\n';
        }
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "error: " << e.what() << '\n';
        return 1;
    }
}
