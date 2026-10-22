#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

#include "grafos/grafo.hpp"

namespace grafos {

// Estrutura que guarda as estimativas de distancia e extrai o minimo.
enum class FilaDijkstra { Vetor, Heap };

struct ResultadoDijkstra {
    FilaDijkstra fila;
    int32_t origem;              // 1..n
    std::vector<double> dist;    // indice interno; +infinito se nao alcancado
    std::vector<int32_t> pai;    // id 1..n do pai; 0 para a origem; -1 se nao alcancado
    std::vector<int32_t> ordem;  // posicao 1, 2, ... na exploracao (entrada em S); -1 se nao explorado
    int64_t explorados;          // vertices retirados da fronteira
};

// Caminhos minimos a partir de origem (1..n). Lanca ErroGrafo se o grafo tiver peso negativo.
ResultadoDijkstra dijkstra(const Grafo& g, int32_t origem, FilaDijkstra fila = FilaDijkstra::Heap);

// Caminho origem..destino em ids 1..n, subindo pelos pais; vazio se destino nao foi alcancado.
std::vector<int32_t> caminho(const ResultadoDijkstra& r, int32_t destino);

const char* nome(FilaDijkstra fila);  // "vetor" ou "heap"

namespace detalhe {

// Fronteira do Dijkstra: vertices descobertos e ainda nao explorados (V - S com dist finita).
// As chaves sao lidas do vetor dist do algoritmo; ordem por (dist, id), para que as duas
// fronteiras extraiam exatamente a mesma sequencia. Interface comum:
//   inserir(v)         v acabou de ser descoberto
//   diminuir_chave(v)  dist[v] diminuiu
//   vazia()
//   extrair_minimo()   remove e devolve o vertice de menor (dist, id)

// Vetor de estimativas, como na aula 8: chave_[v] = dist[v] se v esta na fronteira e
// +infinito caso contrario (nao descoberto ou ja explorado). extrair_minimo varre os n
// vertices (O(n), O(n^2) no total); inserir e diminuir_chave sao O(1).
class FronteiraVetor {
public:
    static constexpr FilaDijkstra tipo = FilaDijkstra::Vetor;

    FronteiraVetor(int32_t n, const std::vector<double>& dist)
        : dist_(dist), chave_(static_cast<size_t>(n), std::numeric_limits<double>::infinity()) {}

    void inserir(int32_t v) {
        chave_[static_cast<size_t>(v)] = dist_[static_cast<size_t>(v)];
        ++quantidade_;
    }

    void diminuir_chave(int32_t v) { chave_[static_cast<size_t>(v)] = dist_[static_cast<size_t>(v)]; }

    bool vazia() const { return quantidade_ == 0; }

    int32_t extrair_minimo() {
        // Varredura crescente com '<' estrito: no empate fica o menor id. Toda chave da
        // fronteira e finita, entao a primeira encontrada ja substitui o infinito inicial.
        size_t melhor = 0;
        double menor = std::numeric_limits<double>::infinity();
        const double* chave = chave_.data();
        const size_t n = chave_.size();
        for (size_t v = 0; v < n; ++v) {
            if (chave[v] < menor) {
                menor = chave[v];
                melhor = v;
            }
        }
        chave_[melhor] = std::numeric_limits<double>::infinity();
        --quantidade_;
        return static_cast<int32_t>(melhor);
    }

private:
    const std::vector<double>& dist_;
    std::vector<double> chave_;
    int64_t quantidade_ = 0;
};

// Heap binario minimo em vetor (aula 9, com indices a partir de 0: pai (i-1)/2, filhos
// 2i+1 e 2i+2), indexado: pos_[v] = posicao de v em heap_ (-1 se fora). O indice permite
// achar v em O(1) e fazer diminuir_chave subindo em O(log n); std::priority_queue nao
// permite modificar chaves. So os vertices descobertos entram no heap.
class FronteiraHeap {
public:
    static constexpr FilaDijkstra tipo = FilaDijkstra::Heap;

    FronteiraHeap(int32_t n, const std::vector<double>& dist) : dist_(dist), pos_(static_cast<size_t>(n), -1) {}

    void inserir(int32_t v) {
        heap_.push_back(v);
        subir(heap_.size() - 1);
    }

    void diminuir_chave(int32_t v) { subir(static_cast<size_t>(pos_[static_cast<size_t>(v)])); }

    bool vazia() const { return heap_.empty(); }

    int32_t extrair_minimo() {
        const int32_t topo = heap_.front();
        const int32_t ultimo = heap_.back();
        heap_.pop_back();
        pos_[static_cast<size_t>(topo)] = -1;
        if (!heap_.empty()) {
            heap_[0] = ultimo;
            descer(0);
        }
        return topo;
    }

    // So para testes: propriedade de heap e pos_ consistente com heap_.
    bool invariantes_ok() const {
        size_t marcados = 0;
        for (int32_t p : pos_) marcados += p >= 0 ? 1 : 0;
        if (marcados != heap_.size()) return false;
        for (size_t i = 0; i < heap_.size(); ++i) {
            if (pos_[static_cast<size_t>(heap_[i])] != static_cast<int32_t>(i)) return false;
            if (i > 0 && menor(heap_[i], heap_[(i - 1) / 2])) return false;
        }
        return true;
    }

private:
    bool menor(int32_t a, int32_t b) const {
        const double da = dist_[static_cast<size_t>(a)];
        const double db = dist_[static_cast<size_t>(b)];
        return da < db || (da == db && a < b);
    }

    void colocar(size_t i, int32_t v) {
        heap_[i] = v;
        pos_[static_cast<size_t>(v)] = static_cast<int32_t>(i);
    }

    void subir(size_t i) {
        const int32_t v = heap_[i];
        while (i > 0) {
            const size_t p = (i - 1) / 2;
            if (!menor(v, heap_[p])) break;
            colocar(i, heap_[p]);
            i = p;
        }
        colocar(i, v);
    }

    void descer(size_t i) {
        const int32_t v = heap_[i];
        const size_t n = heap_.size();
        while (true) {
            size_t f = 2 * i + 1;
            if (f >= n) break;
            if (f + 1 < n && menor(heap_[f + 1], heap_[f])) ++f;
            if (!menor(heap_[f], v)) break;
            colocar(i, heap_[f]);
            i = f;
        }
        colocar(i, v);
    }

    const std::vector<double>& dist_;
    std::vector<int32_t> heap_;
    std::vector<int32_t> pos_;
};

// Dijkstra da aula 8, escrito uma unica vez para qualquer representacao G e fronteira F.
// S (explorados) e marcado por ordem >= 0. Relaxamento estrito e vizinhos em ordem
// crescente: com o desempate por (dist, id), dist/pai/ordem nao dependem da fronteira.
// origem em indice interno; max_explorados = -1 sem limite (limite so para estimar tempo).
template <class F, class G>
ResultadoDijkstra dijkstra(const G& g, int32_t origem, int64_t max_explorados = -1) {
    const int32_t n = g.n();
    const size_t nn = static_cast<size_t>(n);
    constexpr double INF = std::numeric_limits<double>::infinity();
    ResultadoDijkstra r;
    r.fila = F::tipo;
    r.origem = origem + 1;
    r.dist.assign(nn, INF);
    r.pai.assign(nn, -1);
    r.ordem.assign(nn, -1);
    r.explorados = 0;

    F fronteira(n, r.dist);
    r.dist[static_cast<size_t>(origem)] = 0.0;
    r.pai[static_cast<size_t>(origem)] = 0;
    fronteira.inserir(origem);

    while (!fronteira.vazia() && r.explorados != max_explorados) {
        const int32_t u = fronteira.extrair_minimo();
        r.ordem[static_cast<size_t>(u)] = static_cast<int32_t>(++r.explorados);
        const double du = r.dist[static_cast<size_t>(u)];
        g.para_cada_vizinho_com_peso(u, [&](int32_t v, double w) {
            const size_t iv = static_cast<size_t>(v);
            if (r.ordem[iv] >= 0) return;  // ja explorado
            const double nova = du + w;
            if (nova < r.dist[iv]) {
                const bool descoberto_agora = r.dist[iv] == INF;
                r.dist[iv] = nova;
                r.pai[iv] = u + 1;
                if (descoberto_agora) {
                    fronteira.inserir(v);
                } else {
                    fronteira.diminuir_chave(v);
                }
            }
        });
    }
    return r;
}

}  // namespace detalhe
}  // namespace grafos
