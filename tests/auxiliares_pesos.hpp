#pragma once

#include <charconv>
#include <cstdint>
#include <limits>
#include <random>
#include <string>
#include <vector>

#include "auxiliares.hpp"

namespace teste {

constexpr double INF = std::numeric_limits<double>::infinity();

struct ArestaPeso {
    int32_t u, v;  // 1..n
    double peso;
};

// Grafo de teste com pesos: arestas em 1..n, podendo conter lacos e repeticoes.
struct GrafoPesoTeste {
    std::string nome;
    int32_t n;
    std::vector<ArestaPeso> arestas;

    // Pesos escritos na forma mais curta que reconstroi o double (releitura exata).
    std::string texto() const {
        std::string s = std::to_string(n) + "\n";
        char buf[64];
        for (const ArestaPeso& a : arestas) {
            auto r = std::to_chars(buf, buf + sizeof(buf), a.peso);
            s += std::to_string(a.u) + " " + std::to_string(a.v) + " " + std::string(buf, r.ptr) + "\n";
        }
        return s;
    }
};

// Inteiro: pesos 0..3 (forca empates e pesos zero). Real: pesos em (0, 10).
enum class RegimePeso { Inteiro, Real };

inline GrafoPesoTeste com_pesos(const GrafoTeste& g, RegimePeso regime, uint64_t semente) {
    std::mt19937_64 rng(semente);
    GrafoPesoTeste r{g.nome, g.n, {}};
    for (const auto& [u, v] : g.arestas) {
        double p;
        if (regime == RegimePeso::Inteiro) {
            p = static_cast<double>(rng() % 4);
        } else {
            p = static_cast<double>(rng() >> 11) * 0x1.0p-53 * 10.0;
            if (p == 0.0) p = 0.5;
        }
        r.arestas.push_back({u, v, p});
    }
    return r;
}

// Bellman-Ford de referencia (aula 14), sem otimizacoes: dist por indice v-1, INF se nao alcancado.
inline std::vector<double> bellman_ford(const GrafoPesoTeste& g, int32_t origem) {
    std::vector<double> dist(static_cast<size_t>(g.n), INF);
    dist[static_cast<size_t>(origem - 1)] = 0.0;
    for (int32_t i = 1; i < g.n; ++i) {
        for (const ArestaPeso& a : g.arestas) {
            if (a.u == a.v) continue;
            double& du = dist[static_cast<size_t>(a.u - 1)];
            double& dv = dist[static_cast<size_t>(a.v - 1)];
            if (du + a.peso < dv) dv = du + a.peso;
            if (dv + a.peso < du) du = dv + a.peso;
        }
    }
    return dist;
}

}  // namespace teste
