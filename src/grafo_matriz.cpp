#include "grafos/grafo_matriz.hpp"

namespace grafos {

size_t estimar_bytes_matriz(int32_t n) {
    size_t nn = static_cast<size_t>(n);
    size_t palavras = (nn + 63) / 64;
    return nn * palavras * 8 + 4 * nn;
}

GrafoMatriz GrafoMatriz::construir(const ArestasLidas& arestas) {
    GrafoMatriz g;
    g.n_ = arestas.n;
    size_t n = static_cast<size_t>(arestas.n);
    g.palavras_ = (n + 63) / 64;
    g.bits_.assign(n * g.palavras_, 0);
    g.grau_.assign(n, 0);

    const size_t total = arestas.origem.size();
    for (size_t i = 0; i < total; ++i) {
        int32_t u = arestas.origem[i];
        int32_t v = arestas.destino[i];
        uint64_t& palavra_uv = g.bits_[static_cast<size_t>(u) * g.palavras_ + static_cast<size_t>(v >> 6)];
        uint64_t mascara_v = uint64_t{1} << (v & 63);
        // Conta m e graus so quando o bit e ligado pela primeira vez: repeticoes somem aqui.
        if ((palavra_uv & mascara_v) == 0) {
            palavra_uv |= mascara_v;
            g.bits_[static_cast<size_t>(v) * g.palavras_ + static_cast<size_t>(u >> 6)] |= uint64_t{1} << (u & 63);
            ++g.grau_[static_cast<size_t>(u)];
            ++g.grau_[static_cast<size_t>(v)];
            ++g.m_;
        }
    }
    return g;
}

size_t GrafoMatriz::bytes_estrutura() const {
    return estimar_bytes_matriz(n_);
}

}  // namespace grafos
