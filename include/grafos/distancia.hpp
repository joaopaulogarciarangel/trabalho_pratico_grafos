#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "grafos/grafo.hpp"

namespace grafos {

int32_t distancia(const Grafo& g, int32_t u, int32_t v);                    // -1 = infinito
std::vector<int32_t> caminho_minimo(const Grafo& g, int32_t u, int32_t v);  // vazio se nao houver

namespace detalhe {

// BFS a partir de u com parada antecipada quando v e descoberto (indices internos).
// Se caminho != nullptr, preenche o caminho u..v em ids 1..n (vazio se inalcancavel).
template <class G>
int32_t distancia(const G& g, int32_t u, int32_t v, std::vector<int32_t>* caminho) {
    if (caminho != nullptr) caminho->clear();
    if (u == v) {
        if (caminho != nullptr) caminho->push_back(u + 1);
        return 0;
    }
    const size_t n = static_cast<size_t>(g.n());
    std::vector<int32_t> nivel(n, -1);
    std::vector<int32_t> pai;
    if (caminho != nullptr) pai.assign(n, -1);
    std::vector<int32_t> fila(n);
    size_t inicio = 0;
    size_t fim = 0;
    nivel[static_cast<size_t>(u)] = 0;
    fila[fim++] = u;
    bool descoberto = false;

    while (inicio < fim && !descoberto) {
        const int32_t x = fila[inicio++];
        const int32_t nivel_filho = nivel[static_cast<size_t>(x)] + 1;
        g.para_cada_vizinho(x, [&](int32_t w) {
            const size_t iw = static_cast<size_t>(w);
            if (descoberto || nivel[iw] >= 0) return;
            nivel[iw] = nivel_filho;
            if (caminho != nullptr) pai[iw] = x;
            fila[fim++] = w;
            descoberto = w == v;
        });
    }
    if (!descoberto) return -1;

    if (caminho != nullptr) {
        for (int32_t x = v; x != u; x = pai[static_cast<size_t>(x)]) caminho->push_back(x + 1);
        caminho->push_back(u + 1);
        std::reverse(caminho->begin(), caminho->end());
    }
    return nivel[static_cast<size_t>(v)];
}

}  // namespace detalhe
}  // namespace grafos
