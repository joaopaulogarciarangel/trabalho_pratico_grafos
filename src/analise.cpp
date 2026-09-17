#include "grafos/busca.hpp"
#include "grafos/componentes.hpp"
#include "grafos/distancia.hpp"
#include "grafos/estatisticas.hpp"

namespace grafos {

Estatisticas estatisticas(const Grafo& g) {
    return g.visitar([](const auto& concreto) { return detalhe::estatisticas(concreto); });
}

Componentes componentes_conexas(const Grafo& g) {
    return g.visitar([](const auto& concreto) { return detalhe::componentes_conexas(concreto); });
}

int32_t distancia(const Grafo& g, int32_t u, int32_t v) {
    return g.visitar([u, v](const auto& concreto) {
        detalhe::validar_vertice(u, concreto.n());
        detalhe::validar_vertice(v, concreto.n());
        return detalhe::distancia(concreto, u - 1, v - 1, nullptr);
    });
}

std::vector<int32_t> caminho_minimo(const Grafo& g, int32_t u, int32_t v) {
    return g.visitar([u, v](const auto& concreto) {
        detalhe::validar_vertice(u, concreto.n());
        detalhe::validar_vertice(v, concreto.n());
        std::vector<int32_t> caminho;
        detalhe::distancia(concreto, u - 1, v - 1, &caminho);
        return caminho;
    });
}

}  // namespace grafos
