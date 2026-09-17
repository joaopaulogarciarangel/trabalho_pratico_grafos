#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "grafos/erro.hpp"
#include "grafos/grafo.hpp"

namespace grafos {

enum class TipoBusca { BFS, DFS };

struct ArvoreBusca {
    TipoBusca tipo;
    int32_t raiz;               // 1..n
    std::vector<int32_t> pai;   // indice interno; valor = id 1..n do pai; 0 para a raiz; -1 se nao alcancado
    std::vector<int32_t> nivel; // 0 para a raiz; -1 se nao alcancado
    std::vector<int32_t> ordem; // posicao 1, 2, 3... na sequencia da busca; -1 se nao alcancado
};

ArvoreBusca bfs(const Grafo& g, int32_t raiz);
ArvoreBusca dfs(const Grafo& g, int32_t raiz);

namespace detalhe {

inline void validar_vertice(int32_t v, int32_t n) {
    if (v < 1 || v > n) {
        throw ErroGrafo("vertice " + std::to_string(v) + " fora do intervalo 1.." + std::to_string(n));
    }
}

inline ArvoreBusca arvore_desmarcada(TipoBusca tipo, int32_t raiz, int32_t n) {
    ArvoreBusca a;
    a.tipo = tipo;
    a.raiz = raiz + 1;
    a.pai.assign(static_cast<size_t>(n), -1);
    a.nivel.assign(static_cast<size_t>(n), -1);
    a.ordem.assign(static_cast<size_t>(n), -1);
    return a;
}

// BFS da aula: o vertice e marcado ao ser descoberto (nivel >= 0 significa marcado).
// Fila em array de tamanho n com dois indices. raiz em indice interno.
template <class G>
ArvoreBusca bfs(const G& g, int32_t raiz) {
    const int32_t n = g.n();
    ArvoreBusca a = arvore_desmarcada(TipoBusca::BFS, raiz, n);
    std::vector<int32_t> fila(static_cast<size_t>(n));
    size_t inicio = 0;
    size_t fim = 0;
    int32_t contador = 1;

    a.pai[static_cast<size_t>(raiz)] = 0;
    a.nivel[static_cast<size_t>(raiz)] = 0;
    a.ordem[static_cast<size_t>(raiz)] = contador++;
    fila[fim++] = raiz;

    while (inicio < fim) {
        const int32_t v = fila[inicio++];
        const int32_t nivel_filho = a.nivel[static_cast<size_t>(v)] + 1;
        g.para_cada_vizinho(v, [&](int32_t w) {
            const size_t iw = static_cast<size_t>(w);
            if (a.nivel[iw] < 0) {  // descoberto agora
                a.pai[iw] = v + 1;
                a.nivel[iw] = nivel_filho;
                a.ordem[iw] = contador++;
                fila[fim++] = w;
            }
        });
    }
    return a;
}

// DFS iterativa dos slides: pilha de pares (vertice, pai) e marca = explorado
// (nivel >= 0), feita ao remover da pilha. Vizinhos empilhados em ordem decrescente
// para que o menor fique no topo; vizinhos ja marcados nao sao empilhados.
template <class G>
ArvoreBusca dfs(const G& g, int32_t raiz) {
    const int32_t n = g.n();
    ArvoreBusca a = arvore_desmarcada(TipoBusca::DFS, raiz, n);
    std::vector<std::pair<int32_t, int32_t>> pilha;
    pilha.emplace_back(raiz, -1);
    int32_t contador = 1;

    while (!pilha.empty()) {
        const auto [u, p] = pilha.back();
        pilha.pop_back();
        const size_t iu = static_cast<size_t>(u);
        if (a.nivel[iu] >= 0) continue;  // ja explorado

        if (p < 0) {
            a.pai[iu] = 0;
            a.nivel[iu] = 0;
        } else {
            a.pai[iu] = p + 1;
            a.nivel[iu] = a.nivel[static_cast<size_t>(p)] + 1;
        }
        a.ordem[iu] = contador++;
        g.para_cada_vizinho_decrescente(u, [&](int32_t w) {
            if (a.nivel[static_cast<size_t>(w)] < 0) pilha.emplace_back(w, u);
        });
    }
    return a;
}

}  // namespace detalhe
}  // namespace grafos
