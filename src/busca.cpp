#include "grafos/busca.hpp"

namespace grafos {

ArvoreBusca bfs(const Grafo& g, int32_t raiz) {
    return g.visitar([raiz](const auto& concreto) {
        detalhe::validar_vertice(raiz, concreto.n());
        return detalhe::bfs(concreto, raiz - 1);
    });
}

ArvoreBusca dfs(const Grafo& g, int32_t raiz) {
    return g.visitar([raiz](const auto& concreto) {
        detalhe::validar_vertice(raiz, concreto.n());
        return detalhe::dfs(concreto, raiz - 1);
    });
}

}  // namespace grafos
