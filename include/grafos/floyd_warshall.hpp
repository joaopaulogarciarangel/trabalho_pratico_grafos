#pragma once

#include <cstdint>
#include <vector>
#include "grafos/grafo.hpp"

namespace grafos {

struct ResultadoFloydWarshall {
    int32_t n = 0;
    std::vector<double> dist;       // matriz n*n, +inf inalcançavel, -inf indefinido
    std::vector<int32_t> proximo;  // primeiro vertice depois da origem, id 1..n
    bool ciclo_negativo = false;

    double distancia(int32_t origem, int32_t destino) const;
    std::vector<int32_t> caminho(int32_t origem, int32_t destino) const;
    std::vector<int32_t> arvore_origem(int32_t origem) const; // pais, -1 indefinido
};

// DP in-place: O(n^2) memoria para distancias e proximos, O(n^3) tempo.
ResultadoFloydWarshall floyd_warshall(const Grafo& g);
uint64_t estimar_bytes_floyd_warshall(int32_t n);

} // namespace grafos
