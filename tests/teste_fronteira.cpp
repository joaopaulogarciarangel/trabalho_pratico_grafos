#include <cstdint>
#include <iterator>
#include <random>
#include <set>
#include <type_traits>
#include <utility>
#include <vector>

#include "auxiliares_pesos.hpp"
#include "doctest.h"

using grafos::detalhe::FronteiraHeap;
using grafos::detalhe::FronteiraVetor;

namespace {

// Sequencia aleatoria de inserir / diminuir_chave / extrair_minimo comparada com um
// std::set<(chave, id)> de referencia. Chaves inteiras pequenas forcam empates (desempate pelo id).
template <class F>
void checar_fronteira(uint64_t semente) {
    const int32_t n = 300;
    std::vector<double> dist(static_cast<size_t>(n), teste::INF);
    std::vector<char> ja_inserido(static_cast<size_t>(n), 0);
    F fronteira(n, dist);
    std::set<std::pair<double, int32_t>> ref;
    std::mt19937_64 rng(semente);

    int64_t divergencias = 0;
    int64_t extracoes = 0;
    auto extrair_e_comparar = [&]() {
        const int32_t esperado = ref.begin()->second;
        ref.erase(ref.begin());
        const int32_t obtido = fronteira.extrair_minimo();
        ++extracoes;
        if (obtido != esperado) ++divergencias;
    };
    auto conferir_estado = [&]() {
        if (fronteira.vazia() != ref.empty()) ++divergencias;
        if constexpr (std::is_same_v<F, FronteiraHeap>) {
            if (!fronteira.invariantes_ok()) ++divergencias;
        }
    };

    for (int passo = 0; passo < 20000; ++passo) {
        const uint64_t op = rng() % 3;
        if (op == 0) {
            const auto v = static_cast<int32_t>(rng() % static_cast<uint64_t>(n));
            if (ja_inserido[static_cast<size_t>(v)]) continue;
            ja_inserido[static_cast<size_t>(v)] = 1;
            dist[static_cast<size_t>(v)] = static_cast<double>(rng() % 50);
            ref.insert({dist[static_cast<size_t>(v)], v});
            fronteira.inserir(v);
        } else if (op == 1 && !ref.empty()) {
            auto it = std::next(ref.begin(), static_cast<long>(rng() % ref.size()));
            const auto [chave, v] = *it;
            ref.erase(it);
            const double nova = chave - 1.0 - static_cast<double>(rng() % 3);
            dist[static_cast<size_t>(v)] = nova;
            ref.insert({nova, v});
            fronteira.diminuir_chave(v);
        } else if (op == 2 && !ref.empty()) {
            extrair_e_comparar();
        }
        conferir_estado();
    }
    while (!ref.empty()) {
        extrair_e_comparar();
        conferir_estado();
    }
    MESSAGE("extracoes conferidas: ", extracoes);
    CHECK(divergencias == 0);
    CHECK(fronteira.vazia());
}

}  // namespace

TEST_CASE("P2 6.2: fronteira vetor contra referencia") {
    checar_fronteira<FronteiraVetor>(1);
    checar_fronteira<FronteiraVetor>(2);
}

TEST_CASE("P2 6.2: fronteira heap contra referencia, com invariantes a cada operacao") {
    checar_fronteira<FronteiraHeap>(1);
    checar_fronteira<FronteiraHeap>(2);
}

TEST_CASE("P2 6.2: fronteiras declaram o tipo de fila") {
    CHECK(FronteiraVetor::tipo == grafos::FilaDijkstra::Vetor);
    CHECK(FronteiraHeap::tipo == grafos::FilaDijkstra::Heap);
}
