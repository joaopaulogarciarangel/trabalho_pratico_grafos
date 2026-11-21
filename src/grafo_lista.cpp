#include "grafos/grafo_lista.hpp"

#include <utility>

namespace grafos {

GrafoLista GrafoLista::construir(const ArestasLidas& arestas) {
    GrafoLista g;
    g.n_ = arestas.n;
    g.ponderado_ = arestas.ponderado;
    g.tem_peso_negativo_ = arestas.tem_peso_negativo;
    g.direcionado_ = arestas.direcionado;
    size_t n = static_cast<size_t>(arestas.n);
    const size_t total = arestas.origem.size();

    // grau_ guarda primeiro o grau bruto (com repeticoes) para reservar cada lista.
    g.grau_.assign(n, 0);
    for (size_t i = 0; i < total; ++i) {
        ++g.grau_[static_cast<size_t>(arestas.origem[i])];
        if (!g.direcionado_) ++g.grau_[static_cast<size_t>(arestas.destino[i])];
    }
    g.adjacencia_.resize(n);
    for (size_t v = 0; v < n; ++v) g.adjacencia_[v].reserve(static_cast<size_t>(g.grau_[v]));
    if (g.ponderado_) {
        g.pesos_.resize(n);
        for (size_t v = 0; v < n; ++v) g.pesos_[v].reserve(static_cast<size_t>(g.grau_[v]));
    }
    for (size_t i = 0; i < total; ++i) {
        int32_t u = arestas.origem[i];
        int32_t v = arestas.destino[i];
        g.adjacencia_[static_cast<size_t>(u)].push_back(v);
        if (!g.direcionado_) g.adjacencia_[static_cast<size_t>(v)].push_back(u);
        if (g.ponderado_) {
            g.pesos_[static_cast<size_t>(u)].push_back(arestas.peso[i]);
            if (!g.direcionado_) g.pesos_[static_cast<size_t>(v)].push_back(arestas.peso[i]);
        }
    }

    int64_t soma_graus = 0;
    std::vector<std::pair<int32_t, double>> pares;  // area de trabalho do caso ponderado
    for (size_t v = 0; v < n; ++v) {
        std::vector<int32_t>& viz = g.adjacencia_[v];
        if (!g.ponderado_) {
            std::sort(viz.begin(), viz.end());
            viz.erase(std::unique(viz.begin(), viz.end()), viz.end());
        } else {
            // Ordena por (vizinho, peso): entre repeticoes, a primeira e a de menor peso.
            std::vector<double>& pes = g.pesos_[v];
            pares.clear();
            for (size_t i = 0; i < viz.size(); ++i) pares.emplace_back(viz[i], pes[i]);
            std::sort(pares.begin(), pares.end());
            viz.clear();
            pes.clear();
            for (const auto& [w, p] : pares) {
                if (!viz.empty() && viz.back() == w) continue;
                viz.push_back(w);
                pes.push_back(p);
            }
            if (pes.size() < static_cast<size_t>(g.grau_[v])) pes.shrink_to_fit();
        }
        if (viz.size() < static_cast<size_t>(g.grau_[v])) viz.shrink_to_fit();
        g.grau_[v] = static_cast<int32_t>(viz.size());
        soma_graus += static_cast<int64_t>(viz.size());
    }
    g.m_ = g.direcionado_ ? soma_graus : soma_graus / 2;
    return g;
}

size_t estimar_bytes_lista_minimo(int32_t n) {
    return static_cast<size_t>(n) * (sizeof(std::vector<int32_t>) + 4);
}

size_t GrafoLista::bytes_estrutura() const {
    size_t capacidades = 0;
    for (const std::vector<int32_t>& viz : adjacencia_) capacidades += viz.capacity();
    size_t n = static_cast<size_t>(n_);
    size_t total = capacidades * 4 + n * sizeof(std::vector<int32_t>) + 4 * n;
    if (ponderado_) {
        size_t capacidades_pesos = 0;
        for (const std::vector<double>& p : pesos_) capacidades_pesos += p.capacity();
        total += capacidades_pesos * 8 + n * sizeof(std::vector<double>);
    }
    return total;
}

}  // namespace grafos
