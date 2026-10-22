#include "grafos/dijkstra.hpp"

namespace grafos {

const char* nome(FilaDijkstra fila) {
    return fila == FilaDijkstra::Vetor ? "vetor" : "heap";
}

ResultadoDijkstra dijkstra(const Grafo&, int32_t, FilaDijkstra) {
    detalhe::nao_implementado();
}

std::vector<int32_t> caminho(const ResultadoDijkstra&, int32_t) {
    detalhe::nao_implementado();
}

}  // namespace grafos
