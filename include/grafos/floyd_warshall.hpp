#pragma once

#include <cstdint>
#include <vector>
#include "grafos/grafo.hpp"

namespace grafos {

struct ResultadoFloydWarshall {
    int32_t n = 0;
    std::vector<double> dist;       // matriz n*n, +inf inalcançavel, -inf indefinido
    std::vector<int32_t> proximo;  // primeiro vertice depois da origem, id 1..n (vazio na versao so distancias)
    bool ciclo_negativo = false;

    double distancia(int32_t origem, int32_t destino) const;
    std::vector<int32_t> caminho(int32_t origem, int32_t destino) const;
    std::vector<int32_t> arvore_origem(int32_t origem) const; // pais, -1 indefinido
};

// DP in-place: O(n^2) memoria para distancias e proximos, O(n^3) tempo.
ResultadoFloydWarshall floyd_warshall(const Grafo& g);
uint64_t estimar_bytes_floyd_warshall(int32_t n);

// Variante so de distancias (8n^2 bytes, sem a matriz de proximos): mesma recorrencia,
// mesma matriz unica in-place, com o laco k percorrido em blocos BxB (Floyd-Warshall
// em blocos) para reusar cache; os blocos independentes de cada rodada rodam em
// `threads` linhas de execucao (0 = todas). Usada nos estudos com grafos grandes.
ResultadoFloydWarshall floyd_warshall_distancias(const Grafo& g, int threads = 0);
uint64_t estimar_bytes_floyd_warshall_distancias(int32_t n);

} // namespace grafos
