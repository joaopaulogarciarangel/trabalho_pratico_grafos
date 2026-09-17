#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <utility>
#include <vector>

#include "grafos/grafo.hpp"

namespace grafos {

struct Componente {
    int32_t tamanho;
    std::vector<int32_t> vertices;  // 1..n, crescentes
};

struct Componentes {
    std::vector<Componente> lista;  // tamanho decrescente; empate pelo menor vertice
};

Componentes componentes_conexas(const Grafo& g);

namespace detalhe {

// Rotula as componentes (1, 2, ...) na ordem do menor vertice de cada uma.
// Um unico array de rotulos (0 = nao marcado) e uma unica fila reaproveitada por todas as BFS.
template <class G>
int32_t rotular_componentes(const G& g, std::vector<int32_t>& rotulo, std::vector<int32_t>& tamanhos) {
    const int32_t n = g.n();
    rotulo.assign(static_cast<size_t>(n), 0);
    tamanhos.clear();
    std::vector<int32_t> fila(static_cast<size_t>(n));
    int32_t num = 0;
    for (int32_t s = 0; s < n; ++s) {
        if (rotulo[static_cast<size_t>(s)] != 0) continue;
        ++num;
        size_t inicio = 0;
        size_t fim = 0;
        rotulo[static_cast<size_t>(s)] = num;
        fila[fim++] = s;
        while (inicio < fim) {
            const int32_t v = fila[inicio++];
            g.para_cada_vizinho(v, [&](int32_t w) {
                if (rotulo[static_cast<size_t>(w)] == 0) {
                    rotulo[static_cast<size_t>(w)] = num;
                    fila[fim++] = w;
                }
            });
        }
        tamanhos.push_back(static_cast<int32_t>(fim));
    }
    return num;
}

template <class G>
Componentes componentes_conexas(const G& g) {
    std::vector<int32_t> rotulo;
    std::vector<int32_t> tamanhos;
    const int32_t num = rotular_componentes(g, rotulo, tamanhos);

    // Listas por bucket: percorrer os vertices em ordem crescente ja as deixa ordenadas.
    std::vector<Componente> por_rotulo(static_cast<size_t>(num));
    for (size_t c = 0; c < por_rotulo.size(); ++c) {
        por_rotulo[c].tamanho = tamanhos[c];
        por_rotulo[c].vertices.reserve(static_cast<size_t>(tamanhos[c]));
    }
    for (size_t v = 0; v < rotulo.size(); ++v) {
        por_rotulo[static_cast<size_t>(rotulo[v] - 1)].vertices.push_back(static_cast<int32_t>(v) + 1);
    }

    // Rotulos crescem com o menor vertice, entao o desempate e pelo proprio rotulo.
    std::vector<int32_t> indices(static_cast<size_t>(num));
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&](int32_t a, int32_t b) {
        if (tamanhos[static_cast<size_t>(a)] != tamanhos[static_cast<size_t>(b)]) {
            return tamanhos[static_cast<size_t>(a)] > tamanhos[static_cast<size_t>(b)];
        }
        return a < b;
    });

    Componentes r;
    r.lista.reserve(static_cast<size_t>(num));
    for (int32_t c : indices) r.lista.push_back(std::move(por_rotulo[static_cast<size_t>(c)]));
    return r;
}

}  // namespace detalhe
}  // namespace grafos
