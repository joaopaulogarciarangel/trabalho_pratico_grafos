#pragma once

#include <cstdint>
#include <utility>
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

// Bellman-Ford preparado: monta uma vez os arcos de entrada/saida e a deteccao global de
// ciclo negativo (que nao dependem do destino); cada executar(destino) faz so o algoritmo.
// Assim, varias consultas (ex.: todos os pares) nao pagam de novo a copia do grafo.
class BellmanFord {
public:
    explicit BellmanFord(const Grafo& g);

    ResultadoBellmanFord executar(int32_t destino) const;  // destino em 1..n
    int32_t n() const { return n_; }
    bool ciclo_negativo() const { return ciclo_global_; }

private:
    using Arco = std::pair<int32_t, double>;
    int32_t n_ = 0;
    std::vector<std::vector<Arco>> entrada_;
    bool ciclo_global_ = false;
    bool fortemente_conexo_ = false;  // so calculado se houver ciclo negativo
};

// Atalho para uma consulta: prepara e executa.
ResultadoBellmanFord bellman_ford(const Grafo& g, int32_t destino);
std::vector<int32_t> caminho(const ResultadoBellmanFord& r, int32_t origem);

} // namespace grafos
