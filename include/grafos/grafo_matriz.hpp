#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "grafos/leitura.hpp"

namespace grafos {

// Matriz de adjacencia em bitset: um bloco contiguo de n*W palavras de 64 bits,
// W = ceil(n/64). Vertices com indice interno 0..n-1. Se ponderado, pesos_ e um bloco
// denso n*n de double; a existencia da aresta continua vindo do bitset.
class GrafoMatriz {
public:
    static GrafoMatriz construir(const ArestasLidas& arestas);

    GrafoMatriz(const GrafoMatriz&) = delete;
    GrafoMatriz& operator=(const GrafoMatriz&) = delete;
    GrafoMatriz(GrafoMatriz&&) noexcept = default;
    GrafoMatriz& operator=(GrafoMatriz&&) noexcept = default;

    int32_t n() const { return n_; }
    int64_t m() const { return m_; }
    int32_t grau(int32_t v) const { return grau_[static_cast<size_t>(v)]; }
    bool ponderado() const { return ponderado_; }
    bool tem_peso_negativo() const { return tem_peso_negativo_; }

    bool sao_vizinhos(int32_t u, int32_t v) const {
        uint64_t palavra = bits_[static_cast<size_t>(u) * palavras_ + static_cast<size_t>(v >> 6)];
        return (palavra >> (v & 63)) & 1u;
    }

    // Peso da aresta (u, v); exige que sejam vizinhos. 1.0 se nao ponderado.
    double peso(int32_t u, int32_t v) const {
        if (!ponderado_) return 1.0;
        return pesos_[static_cast<size_t>(u) * static_cast<size_t>(n_) + static_cast<size_t>(v)];
    }

    // f(w, peso) para cada vizinho w em ordem crescente; peso 1.0 se nao ponderado.
    template <class F>
    void para_cada_vizinho_com_peso(int32_t v, F&& f) const {
        if (!ponderado_) {
            para_cada_vizinho(v, [&](int32_t w) { f(w, 1.0); });
            return;
        }
        const double* linha = pesos_.data() + static_cast<size_t>(v) * static_cast<size_t>(n_);
        para_cada_vizinho(v, [&](int32_t w) { f(w, linha[w]); });
    }

    template <class F>
    void para_cada_vizinho(int32_t v, F&& f) const {
        const uint64_t* linha = bits_.data() + static_cast<size_t>(v) * palavras_;
        for (size_t w = 0; w < palavras_; ++w) {
            uint64_t x = linha[w];
            while (x != 0) {
                int b = __builtin_ctzll(x);
                f(static_cast<int32_t>(w * 64 + static_cast<size_t>(b)));
                x &= x - 1;
            }
        }
    }

    template <class F>
    void para_cada_vizinho_decrescente(int32_t v, F&& f) const {
        const uint64_t* linha = bits_.data() + static_cast<size_t>(v) * palavras_;
        for (size_t w = palavras_; w-- > 0;) {
            uint64_t x = linha[w];
            while (x != 0) {
                int b = 63 - __builtin_clzll(x);
                f(static_cast<int32_t>(w * 64 + static_cast<size_t>(b)));
                x &= ~(uint64_t{1} << b);
            }
        }
    }

    size_t bytes_estrutura() const;

private:
    GrafoMatriz() = default;

    int32_t n_ = 0;
    int64_t m_ = 0;
    bool ponderado_ = false;
    bool tem_peso_negativo_ = false;
    size_t palavras_ = 0;  // W
    std::vector<uint64_t> bits_;
    std::vector<double> pesos_;
    std::vector<int32_t> grau_;
};

// Memoria teorica da matriz (n*W*8 + 4n, mais 8n^2 se ponderada), para checar
// viabilidade antes de alocar.
size_t estimar_bytes_matriz(int32_t n, bool ponderado = false);

}  // namespace grafos
