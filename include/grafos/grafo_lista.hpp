#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "grafos/leitura.hpp"

namespace grafos {

// Lista de adjacencia (vetor de vetores), cada lista ordenada e sem repeticoes.
// Vertices com indice interno 0..n-1.
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

    bool sao_vizinhos(int32_t u, int32_t v) const {
        const std::vector<int32_t>& viz = adjacencia_[static_cast<size_t>(u)];
        return std::binary_search(viz.begin(), viz.end(), v);
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
    std::vector<std::vector<int32_t>> adjacencia_;
    std::vector<int32_t> grau_;
};

// Memoria minima da lista so pelo n (vetores vazios + array de graus): n*(sizeof(vector) + 4).
size_t estimar_bytes_lista_minimo(int32_t n);

}  // namespace grafos
