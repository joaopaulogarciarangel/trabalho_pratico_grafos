#include <algorithm>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "auxiliares.hpp"
#include "doctest.h"

using grafos::Representacao;

TEST_CASE("Figura 1: n, m, graus, adjacencia e ordem dos vizinhos") {
    const std::vector<std::vector<int32_t>> esperado = {{}, {2, 5}, {1, 5}, {5}, {5}, {1, 2, 3, 4}};
    for (Representacao rep : teste::REPRESENTACOES) {
        CAPTURE(teste::nome(rep));
        grafos::Grafo g = grafos::carregar(teste::caminho_dados("figura1.txt"), rep);
        CHECK(g.n() == 5);
        CHECK(g.m() == 5);
        for (int32_t v = 1; v <= 5; ++v) {
            CAPTURE(v);
            CHECK(g.grau(v) == static_cast<int32_t>(esperado[v].size()));
            CHECK(teste::vizinhos(g, v) == esperado[v]);
            std::vector<int32_t> reverso(esperado[v].rbegin(), esperado[v].rend());
            CHECK(teste::vizinhos(g, v, true) == reverso);
            for (int32_t w = 1; w <= 5; ++w) {
                bool viz = std::find(esperado[v].begin(), esperado[v].end(), w) != esperado[v].end();
                CHECK(g.sao_vizinhos(v, w) == viz);
            }
        }
    }
}

TEST_CASE("bytes_estrutura e estimar_bytes_matriz") {
    CHECK(grafos::estimar_bytes_matriz(1) == 8 + 4);
    CHECK(grafos::estimar_bytes_matriz(5) == 5 * 8 + 20);
    CHECK(grafos::estimar_bytes_matriz(64) == 64 * 8 + 256);
    CHECK(grafos::estimar_bytes_matriz(65) == 65 * 2 * 8 + 260);
    CHECK(grafos::estimar_bytes_matriz(375000) == size_t{375000} * 5860 * 8 + 1500000);
    CHECK(grafos::estimar_bytes_matriz(2147483647) ==
          size_t{2147483647} * size_t{33554432} * 8 + size_t{4} * 2147483647);

    grafos::Grafo matriz = grafos::carregar(teste::caminho_dados("figura1.txt"), Representacao::Matriz);
    CHECK(matriz.bytes_estrutura() == grafos::estimar_bytes_matriz(5));

    const size_t tam_vetor = sizeof(std::vector<int32_t>);
    grafos::Grafo lista = grafos::carregar(teste::caminho_dados("figura1.txt"), Representacao::Lista);
    CHECK(lista.bytes_estrutura() == 10 * 4 + 5 * tam_vetor + 5 * 4);

    // Vertice 1 reservou 3 posicoes e ficou com 1: shrink_to_fit deve ter sido aplicado.
    grafos::Grafo rep = grafos::carregar(teste::caminho_dados("repetidas_lacos.txt"), Representacao::Lista);
    CHECK(rep.bytes_estrutura() == 2 * 4 + 3 * tam_vetor + 3 * 4);
}

TEST_CASE("matriz em fronteiras de palavra de 64 bits") {
    const std::string texto = "130\n1 64\n1 65\n64 65\n1 130\n129 130\n65 128\n64 1\n128 129\n";
    for (Representacao rep : teste::REPRESENTACOES) {
        CAPTURE(teste::nome(rep));
        grafos::Grafo g = teste::grafo_de_texto(texto, rep);
        CHECK(g.m() == 7);
        CHECK(teste::vizinhos(g, 1) == std::vector<int32_t>{64, 65, 130});
        CHECK(teste::vizinhos(g, 1, true) == std::vector<int32_t>{130, 65, 64});
        CHECK(teste::vizinhos(g, 65) == std::vector<int32_t>{1, 64, 128});
        CHECK(teste::vizinhos(g, 130) == std::vector<int32_t>{1, 129});
        CHECK(teste::vizinhos(g, 128, true) == std::vector<int32_t>{129, 65});
        CHECK(g.sao_vizinhos(130, 129));
        CHECK_FALSE(g.sao_vizinhos(64, 128));
    }
}

TEST_CASE("lista e matriz identicas em grafos variados") {
    for (const teste::GrafoTeste& caso : teste::grafos_de_teste()) {
        CAPTURE(caso.nome);
        std::set<std::pair<int32_t, int32_t>> distintas;
        int64_t lacos = 0;
        for (auto [u, v] : caso.arestas) {
            if (u == v) {
                ++lacos;
                continue;
            }
            distintas.insert({std::min(u, v), std::max(u, v)});
        }
        const int64_t linhas = static_cast<int64_t>(caso.arestas.size());
        const int64_t repetidas = linhas - lacos - static_cast<int64_t>(distintas.size());

        const std::string texto = caso.texto();
        grafos::InfoLeitura info_l;
        grafos::InfoLeitura info_m;
        grafos::Grafo lista = teste::grafo_de_texto(texto, Representacao::Lista, &info_l);
        grafos::Grafo matriz = teste::grafo_de_texto(texto, Representacao::Matriz, &info_m);

        for (const grafos::InfoLeitura& info : {info_l, info_m}) {
            CHECK(info.linhas_aresta == linhas);
            CHECK(info.lacos_ignorados == lacos);
            CHECK(info.repetidas_ignoradas == repetidas);
        }
        REQUIRE(lista.n() == caso.n);
        REQUIRE(matriz.n() == caso.n);
        CHECK(lista.m() == static_cast<int64_t>(distintas.size()));
        CHECK(matriz.m() == static_cast<int64_t>(distintas.size()));

        bool tudo_igual = true;
        for (int32_t v = 1; v <= caso.n; ++v) {
            std::vector<int32_t> cresc = teste::vizinhos(lista, v);
            std::vector<int32_t> decresc(cresc.rbegin(), cresc.rend());
            tudo_igual = tudo_igual && std::is_sorted(cresc.begin(), cresc.end()) &&
                         std::adjacent_find(cresc.begin(), cresc.end()) == cresc.end() &&
                         cresc == teste::vizinhos(matriz, v) && decresc == teste::vizinhos(lista, v, true) &&
                         decresc == teste::vizinhos(matriz, v, true) &&
                         lista.grau(v) == static_cast<int32_t>(cresc.size()) && matriz.grau(v) == lista.grau(v);
            for (int32_t w = 1; w <= caso.n; ++w) {
                bool esperado = distintas.count({std::min(v, w), std::max(v, w)}) > 0;
                tudo_igual = tudo_igual && lista.sao_vizinhos(v, w) == esperado &&
                             matriz.sao_vizinhos(v, w) == esperado;
            }
        }
        CHECK(tudo_igual);
    }
}
