#pragma once

#include <cstdint>
#include <vector>
#include "grafos/grafo.hpp"

namespace grafos {

struct ResultadoBellmanFord {
    int32_t destino = 0;
    std::vector<double> dist;       // +inf: sem caminho; -inf: ciclo negativo afeta o par
    std::vector<int32_t> sucessor; // id 1..n; 0 no destino; -1 se indefinido
    bool ciclo_negativo = false;    // em qualquer componente do grafo
    bool ciclo_afeta_destino = false;
    int32_t rodadas = 0;
};

ResultadoBellmanFord bellman_ford(const Grafo& g, int32_t destino);
std::vector<int32_t> caminho(const ResultadoBellmanFord& r, int32_t origem);

} // namespace grafos
