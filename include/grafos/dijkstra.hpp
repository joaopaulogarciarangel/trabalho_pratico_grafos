#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

#include "grafos/erro.hpp"
#include "grafos/grafo.hpp"

namespace grafos {

// Estrutura que guarda as estimativas de distancia e extrai o minimo.
enum class FilaDijkstra { Vetor, Heap };

struct ResultadoDijkstra {
    FilaDijkstra fila;
    int32_t origem;            // 1..n
    std::vector<double> dist;  // indice interno; +infinito se nao alcancado
    std::vector<int32_t> pai;  // id 1..n do pai; 0 para a origem; -1 se nao alcancado
    std::vector<int32_t> ordem;  // posicao 1, 2, ... na exploracao (entrada em S); -1 se nao explorado
    int64_t explorados;        // vertices retirados da fronteira
};

// Caminhos minimos a partir de origem (1..n). Lanca ErroGrafo se o grafo tiver peso negativo.
ResultadoDijkstra dijkstra(const Grafo& g, int32_t origem, FilaDijkstra fila = FilaDijkstra::Heap);

// Caminho origem..destino em ids 1..n, subindo pelos pais; vazio se destino nao foi alcancado.
std::vector<int32_t> caminho(const ResultadoDijkstra& r, int32_t destino);

const char* nome(FilaDijkstra fila);  // "vetor" ou "heap"

namespace detalhe {

[[noreturn]] inline void nao_implementado() { throw ErroGrafo("nao implementado"); }

// Fronteira do Dijkstra (vertices descobertos e ainda nao explorados). As chaves sao lidas
// do vetor dist do algoritmo; ordem por (dist, id). Interface comum:
//   inserir(v), diminuir_chave(v), vazia(), extrair_minimo().

// Vetor de marcas: extrair_minimo varre os n vertices (O(n)); diminuir_chave e O(1).
class FronteiraVetor {
public:
    static constexpr FilaDijkstra tipo = FilaDijkstra::Vetor;

    FronteiraVetor(int32_t n, const std::vector<double>& dist) : dist_(dist), na_fronteira_(static_cast<size_t>(n), 0) {}

    void inserir(int32_t) { nao_implementado(); }
    void diminuir_chave(int32_t) { nao_implementado(); }
    bool vazia() const { return quantidade_ == 0; }
    int32_t extrair_minimo() { nao_implementado(); }

private:
    const std::vector<double>& dist_;
    std::vector<char> na_fronteira_;
    int64_t quantidade_ = 0;
};

// Heap binario minimo em vetor, indexado: pos_[v] = posicao de v em heap_ (-1 se fora).
// inserir, diminuir_chave e extrair_minimo em O(log n).
class FronteiraHeap {
public:
    static constexpr FilaDijkstra tipo = FilaDijkstra::Heap;

    FronteiraHeap(int32_t n, const std::vector<double>& dist) : dist_(dist), pos_(static_cast<size_t>(n), -1) {}

    void inserir(int32_t) { nao_implementado(); }
    void diminuir_chave(int32_t) { nao_implementado(); }
    bool vazia() const { return heap_.empty(); }
    int32_t extrair_minimo() { nao_implementado(); }

    // So para testes: propriedade de heap e pos_ consistente.
    bool invariantes_ok() const { nao_implementado(); }

private:
    const std::vector<double>& dist_;
    std::vector<int32_t> heap_;
    std::vector<int32_t> pos_;
};

// Dijkstra da aula 8, escrito uma unica vez para qualquer representacao G e fronteira F.
// origem em indice interno; max_explorados = -1 sem limite (limite so para estimar tempo).
template <class F, class G>
ResultadoDijkstra dijkstra(const G&, int32_t, int64_t = -1) {
    nao_implementado();
}

}  // namespace detalhe
}  // namespace grafos
