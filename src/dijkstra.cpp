#include "grafos/dijkstra.hpp"

#include <algorithm>
#include <string>

#include "grafos/busca.hpp"
#include "grafos/erro.hpp"

namespace grafos {

const char* nome(FilaDijkstra fila) {
    return fila == FilaDijkstra::Vetor ? "vetor" : "heap";
}

ResultadoDijkstra dijkstra(const Grafo& g, int32_t origem, FilaDijkstra fila) {
    g.validar_vertice(origem);
    if (g.tem_peso_negativo()) {
        throw ErroGrafo("Dijkstra nao aceita pesos negativos; use Bellman-Ford");
    }
    return g.visitar([&](const auto& concreto) {
        return fila == FilaDijkstra::Vetor ? detalhe::dijkstra<detalhe::FronteiraVetor>(concreto, origem - 1)
                                           : detalhe::dijkstra<detalhe::FronteiraHeap>(concreto, origem - 1);
    });
}

std::vector<int32_t> caminho(const ResultadoDijkstra& r, int32_t destino) {
    detalhe::validar_vertice(destino, static_cast<int32_t>(r.pai.size()));
    std::vector<int32_t> c;
    if (r.pai[static_cast<size_t>(destino - 1)] < 0) return c;
    for (int32_t x = destino; x != 0; x = r.pai[static_cast<size_t>(x - 1)]) c.push_back(x);
    std::reverse(c.begin(), c.end());
    return c;
}

}  // namespace grafos
