#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "grafos/grafo.hpp"

namespace grafos {

struct Estatisticas {
    int32_t n;
    int64_t m;
    int32_t grau_minimo, grau_maximo;
    double grau_medio;    // 2m/n
    double mediana_grau;  // n par: media dos dois valores centrais
};

Estatisticas estatisticas(const Grafo& g);

namespace detalhe {

// Minimo, maximo e mediana por histograma de graus: O(n + grau maximo), sem ordenar.
template <class G>
Estatisticas estatisticas(const G& g) {
    Estatisticas e{};
    e.n = g.n();
    e.m = g.m();
    e.grau_medio = 2.0 * static_cast<double>(e.m) / static_cast<double>(e.n);

    int32_t maximo = 0;
    for (int32_t v = 0; v < e.n; ++v) maximo = g.grau(v) > maximo ? g.grau(v) : maximo;
    std::vector<int64_t> histograma(static_cast<size_t>(maximo) + 1, 0);
    for (int32_t v = 0; v < e.n; ++v) ++histograma[static_cast<size_t>(g.grau(v))];

    int32_t minimo = 0;
    while (histograma[static_cast<size_t>(minimo)] == 0) ++minimo;
    e.grau_minimo = minimo;
    e.grau_maximo = maximo;

    // k-esimo grau (0-based) na sequencia ordenada, percorrendo o histograma acumulado.
    auto k_esimo = [&](int64_t k) {
        int64_t acumulado = 0;
        for (size_t d = 0; d < histograma.size(); ++d) {
            acumulado += histograma[d];
            if (acumulado > k) return static_cast<double>(d);
        }
        return static_cast<double>(maximo);
    };
    const int64_t n = e.n;
    e.mediana_grau = n % 2 == 1 ? k_esimo(n / 2) : (k_esimo(n / 2 - 1) + k_esimo(n / 2)) / 2.0;
    return e;
}

}  // namespace detalhe
}  // namespace grafos
