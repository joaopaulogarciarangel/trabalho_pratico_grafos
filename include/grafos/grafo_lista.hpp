#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "grafos/leitura.hpp"

namespace grafos {

// Lista de adjacencia (vetor de vetores), cada lista ordenada e sem repeticoes.
// Vertices com indice interno 0..n-1. Se ponderado, pesos_[v][i] e o peso da aresta
// ate adjacencia_[v][i] (vetores paralelos, alocados so em grafo ponderado).
class GrafoLista {
public:
    static GrafoLista construir(const ArestasLidas& arestas);

    GrafoLista(const GrafoLista&) = delete;
    GrafoLista& operator=(const GrafoLista&) = delete;
    GrafoLista(GrafoLista&&) noexcept = default;
    GrafoLista& operator=(GrafoLista&&) noexcept = default;

    int32_t n() const { return n_; }
    int64_t m() const { return m_; }
    int32_t grau(int32_t v) const { return grau_[static_cast<size_t>(v)]; }
    bool ponderado() const { return ponderado_; }
    bool tem_peso_negativo() const { return tem_peso_negativo_; }
    bool direcionado() const { return direcionado_; }

    bool sao_vizinhos(int32_t u, int32_t v) const {
        const std::vector<int32_t>& viz = adjacencia_[static_cast<size_t>(u)];
        return std::binary_search(viz.begin(), viz.end(), v);
    }

    // Peso da aresta (u, v); exige que sejam vizinhos. 1.0 se nao ponderado.
    double peso(int32_t u, int32_t v) const {
        if (!ponderado_) return 1.0;
        const std::vector<int32_t>& viz = adjacencia_[static_cast<size_t>(u)];
        auto it = std::lower_bound(viz.begin(), viz.end(), v);
        return pesos_[static_cast<size_t>(u)][static_cast<size_t>(it - viz.begin())];
    }

    // f(w, peso) para cada vizinho w em ordem crescente; peso 1.0 se nao ponderado.
    template <class F>
    void para_cada_vizinho_com_peso(int32_t v, F&& f) const {
        const std::vector<int32_t>& viz = adjacencia_[static_cast<size_t>(v)];
        if (!ponderado_) {
            for (int32_t w : viz) f(w, 1.0);
            return;
        }
        const std::vector<double>& p = pesos_[static_cast<size_t>(v)];
        for (size_t i = 0; i < viz.size(); ++i) f(viz[i], p[i]);
    }

    template <class F>
    void para_cada_vizinho(int32_t v, F&& f) const {
        for (int32_t w : adjacencia_[static_cast<size_t>(v)]) f(w);
    }

    template <class F>
    void para_cada_vizinho_decrescente(int32_t v, F&& f) const {
        const std::vector<int32_t>& viz = adjacencia_[static_cast<size_t>(v)];
        for (auto it = viz.rbegin(); it != viz.rend(); ++it) f(*it);
    }

    size_t bytes_estrutura() const;

private:
    GrafoLista() = default;

    int32_t n_ = 0;
    int64_t m_ = 0;
    bool ponderado_ = false;
    bool tem_peso_negativo_ = false;
    bool direcionado_ = false;
    std::vector<std::vector<int32_t>> adjacencia_;
    std::vector<std::vector<double>> pesos_;
    std::vector<int32_t> grau_;
};

// Memoria minima da lista so pelo n (vetores vazios + array de graus): n*(sizeof(vector) + 4).
size_t estimar_bytes_lista_minimo(int32_t n);

}  // namespace grafos
