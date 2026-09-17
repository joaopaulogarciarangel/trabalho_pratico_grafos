#include "grafos/grafo_lista.hpp"

namespace grafos {

GrafoLista GrafoLista::construir(const ArestasLidas& arestas) {
    GrafoLista g;
    g.n_ = arestas.n;
    size_t n = static_cast<size_t>(arestas.n);
    const size_t total = arestas.origem.size();

    // grau_ guarda primeiro o grau bruto (com repeticoes) para reservar cada lista.
    g.grau_.assign(n, 0);
    for (size_t i = 0; i < total; ++i) {
        ++g.grau_[static_cast<size_t>(arestas.origem[i])];
        ++g.grau_[static_cast<size_t>(arestas.destino[i])];
    }
    g.adjacencia_.resize(n);
    for (size_t v = 0; v < n; ++v) g.adjacencia_[v].reserve(static_cast<size_t>(g.grau_[v]));
    for (size_t i = 0; i < total; ++i) {
        int32_t u = arestas.origem[i];
        int32_t v = arestas.destino[i];
        g.adjacencia_[static_cast<size_t>(u)].push_back(v);
        g.adjacencia_[static_cast<size_t>(v)].push_back(u);
    }

    int64_t soma_graus = 0;
    for (size_t v = 0; v < n; ++v) {
        std::vector<int32_t>& viz = g.adjacencia_[v];
        std::sort(viz.begin(), viz.end());
        viz.erase(std::unique(viz.begin(), viz.end()), viz.end());
        if (viz.size() < static_cast<size_t>(g.grau_[v])) viz.shrink_to_fit();
        g.grau_[v] = static_cast<int32_t>(viz.size());
        soma_graus += static_cast<int64_t>(viz.size());
    }
    g.m_ = soma_graus / 2;
    return g;
}

size_t estimar_bytes_lista_minimo(int32_t n) {
    return static_cast<size_t>(n) * (sizeof(std::vector<int32_t>) + 4);
}

size_t GrafoLista::bytes_estrutura() const {
    size_t capacidades = 0;
    for (const std::vector<int32_t>& viz : adjacencia_) capacidades += viz.capacity();
    size_t n = static_cast<size_t>(n_);
    return capacidades * 4 + n * sizeof(std::vector<int32_t>) + 4 * n;
}

}  // namespace grafos
