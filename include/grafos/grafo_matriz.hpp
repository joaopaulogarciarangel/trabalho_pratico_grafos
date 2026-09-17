#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "grafos/leitura.hpp"

namespace grafos {

// Matriz de adjacencia em bitset: um bloco contiguo de n*W palavras de 64 bits,
// W = ceil(n/64). Vertices com indice interno 0..n-1.
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

    bool sao_vizinhos(int32_t u, int32_t v) const {
        uint64_t palavra = bits_[static_cast<size_t>(u) * palavras_ + static_cast<size_t>(v >> 6)];
        return (palavra >> (v & 63)) & 1u;
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
    size_t palavras_ = 0;  // W
    std::vector<uint64_t> bits_;
    std::vector<int32_t> grau_;
};

// Memoria teorica da matriz (n*W*8 + 4n), para checar viabilidade antes de alocar.
size_t estimar_bytes_matriz(int32_t n);

}  // namespace grafos
