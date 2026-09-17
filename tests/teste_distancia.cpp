#include <cstdint>
#include <string>
#include <vector>

#include "auxiliares.hpp"
#include "doctest.h"

using grafos::Representacao;

namespace {

bool caminho_valido(const grafos::Grafo& g, const std::vector<int32_t>& caminho, int32_t u, int32_t v,
                    int32_t dist) {
    if (dist < 0) return caminho.empty();
    if (caminho.size() != static_cast<size_t>(dist) + 1) return false;
    if (caminho.front() != u || caminho.back() != v) return false;
    for (size_t i = 1; i < caminho.size(); ++i) {
        if (!g.sao_vizinhos(caminho[i - 1], caminho[i])) return false;
    }
    return true;
}

}  // namespace

TEST_CASE("11.1: distancia(3,4) = 2 na Figura 1") {
    for (Representacao rep : teste::REPRESENTACOES) {
        CAPTURE(teste::nome(rep));
        grafos::Grafo g = grafos::carregar(teste::caminho_dados("figura1.txt"), rep);
        CHECK(grafos::distancia(g, 3, 4) == 2);
        CHECK(grafos::caminho_minimo(g, 3, 4) == std::vector<int32_t>{3, 5, 4});
        CHECK(grafos::distancia(g, 1, 1) == 0);
        CHECK(grafos::caminho_minimo(g, 1, 1) == std::vector<int32_t>{1});
        CHECK(grafos::distancia(g, 1, 5) == 1);
        CHECK(grafos::caminho_minimo(g, 5, 1) == std::vector<int32_t>{5, 1});
    }
}

TEST_CASE("11.5: distancias no grafo desconexo") {
    for (Representacao rep : teste::REPRESENTACOES) {
        CAPTURE(teste::nome(rep));
        grafos::Grafo g = teste::grafo_de_texto("8\n1 2\n2 3\n5 6\n", rep);
        CHECK(grafos::distancia(g, 1, 5) == -1);
        CHECK(grafos::caminho_minimo(g, 1, 5).empty());
        CHECK(grafos::distancia(g, 1, 3) == 2);
        CHECK(grafos::caminho_minimo(g, 1, 3) == std::vector<int32_t>{1, 2, 3});
        CHECK(grafos::distancia(g, 4, 7) == -1);
        CHECK(grafos::distancia(g, 4, 4) == 0);
    }
}

TEST_CASE("distancia: vertices fora de 1..n lancam") {
    for (Representacao rep : teste::REPRESENTACOES) {
        grafos::Grafo g = teste::grafo_de_texto("5\n1 2\n", rep);
        CHECK_THROWS_WITH_AS(grafos::distancia(g, 0, 1), "vertice 0 fora do intervalo 1..5", grafos::ErroGrafo);
        CHECK_THROWS_AS(grafos::distancia(g, 1, 6), grafos::ErroGrafo);
        CHECK_THROWS_AS(grafos::caminho_minimo(g, 6, 1), grafos::ErroGrafo);
        CHECK_THROWS_AS(grafos::caminho_minimo(g, 1, 0), grafos::ErroGrafo);
    }
}

TEST_CASE("11.7: distancia = nivel da BFS, caminho valido, identica nas duas representacoes") {
    int64_t pares = 0;
    for (const teste::GrafoTeste& caso : teste::grafos_de_teste()) {
        CAPTURE(caso.nome);
        const std::string texto = caso.texto();
        grafos::Grafo lista = teste::grafo_de_texto(texto, Representacao::Lista);
        grafos::Grafo matriz = teste::grafo_de_texto(texto, Representacao::Matriz);
        // Todos os destinos em grafos pequenos; nos maiores, um em cada 7 (inclui 1 e n via raizes).
        const int32_t passo = caso.n <= 70 ? 1 : 7;
        for (int32_t u : teste::raizes_de_teste(caso.n)) {
            const grafos::ArvoreBusca arvore = grafos::bfs(lista, u);
            bool tudo_certo = true;
            for (int32_t v = 1; v <= caso.n; v += (v == 1 ? 1 : passo)) {
                const int32_t d_l = grafos::distancia(lista, u, v);
                const int32_t d_m = grafos::distancia(matriz, u, v);
                const std::vector<int32_t> c_l = grafos::caminho_minimo(lista, u, v);
                const std::vector<int32_t> c_m = grafos::caminho_minimo(matriz, u, v);
                const bool ok = d_l == arvore.nivel[static_cast<size_t>(v - 1)] && d_l == d_m && c_l == c_m &&
                                caminho_valido(lista, c_l, u, v, d_l);
                if (!ok) {
                    CAPTURE(u);
                    CAPTURE(v);
                    FAIL_CHECK("distancia/caminho incorreto");
                }
                tudo_certo = tudo_certo && ok;
                ++pares;
            }
            CHECK(tudo_certo);
        }
    }
    MESSAGE("pares (u, v) testados: ", pares);
}
