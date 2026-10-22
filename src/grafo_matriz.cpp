#include "grafos/grafo_matriz.hpp"

namespace grafos {

size_t estimar_bytes_matriz(int32_t n, bool ponderado) {
    size_t nn = static_cast<size_t>(n);
    size_t palavras = (nn + 63) / 64;
    size_t total = nn * palavras * 8 + 4 * nn;
    if (ponderado) total += 8 * nn * nn;
    return total;
}

GrafoMatriz GrafoMatriz::construir(const ArestasLidas& arestas) {
    GrafoMatriz g;
    g.n_ = arestas.n;
    g.ponderado_ = arestas.ponderado;
    g.tem_peso_negativo_ = arestas.tem_peso_negativo;
    size_t n = static_cast<size_t>(arestas.n);
    g.palavras_ = (n + 63) / 64;
    g.bits_.assign(n * g.palavras_, 0);
    g.grau_.assign(n, 0);
    if (g.ponderado_) g.pesos_.assign(n * n, 0.0);  // so lido onde o bit esta ligado

    const size_t total = arestas.origem.size();
    for (size_t i = 0; i < total; ++i) {
        int32_t u = arestas.origem[i];
        int32_t v = arestas.destino[i];
        uint64_t& palavra_uv = g.bits_[static_cast<size_t>(u) * g.palavras_ + static_cast<size_t>(v >> 6)];
        uint64_t mascara_v = uint64_t{1} << (v & 63);
        const size_t uv = static_cast<size_t>(u) * n + static_cast<size_t>(v);
        const size_t vu = static_cast<size_t>(v) * n + static_cast<size_t>(u);
        // Conta m e graus so quando o bit e ligado pela primeira vez: repeticoes somem aqui.
        if ((palavra_uv & mascara_v) == 0) {
            palavra_uv |= mascara_v;
            g.bits_[static_cast<size_t>(v) * g.palavras_ + static_cast<size_t>(u >> 6)] |= uint64_t{1} << (u & 63);
            ++g.grau_[static_cast<size_t>(u)];
            ++g.grau_[static_cast<size_t>(v)];
            ++g.m_;
            if (g.ponderado_) g.pesos_[uv] = g.pesos_[vu] = arestas.peso[i];
        } else if (g.ponderado_ && arestas.peso[i] < g.pesos_[uv]) {  // repetida: fica o menor peso
            g.pesos_[uv] = g.pesos_[vu] = arestas.peso[i];
        }
    }
    return g;
}

size_t GrafoMatriz::bytes_estrutura() const {
    return estimar_bytes_matriz(n_, ponderado_);
}

}  // namespace grafos
