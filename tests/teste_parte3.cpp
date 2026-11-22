#include <cmath>
#include <limits>
#include <random>
#include <sstream>
#include <string>

#include "auxiliares.hpp"
#include "doctest.h"

namespace {
grafos::Grafo dirigido(const std::string& texto, grafos::Representacao rep) {
    auto a = grafos::parsear_arestas(texto.data(), texto.size(), true);
    return grafos::construir_grafo(a, rep);
}
}

TEST_CASE("P3: orientacao, repeticoes e lacos em ambas representacoes") {
    const std::string texto = "4\n1 2 5\n2 3 -2\n2 3 3\n3 4 1\n1 4 10\n4 4 0\n";
    for (auto rep : teste::REPRESENTACOES) {
        auto g = dirigido(texto, rep);
        CHECK(g.direcionado());
        CHECK(g.m() == 5);
        CHECK(g.grau(4) == 1);
        CHECK(g.sao_vizinhos(1, 2));
        CHECK_FALSE(g.sao_vizinhos(2, 1));
        CHECK(g.peso(2, 3) == -2);
        CHECK(g.sao_vizinhos(4, 4));
        CHECK(grafos::bfs(g, 4).nivel[0] == -1);
        CHECK(grafos::dfs(g, 4).nivel[0] == -1);
        CHECK(grafos::bfs(g, 1).nivel[3] == 1);
        CHECK_THROWS_AS(grafos::dijkstra(g, 1), grafos::ErroGrafo);

        auto b = grafos::bellman_ford(g, 4);
        CHECK_FALSE(b.ciclo_negativo);
        CHECK(b.dist[0] == 4);
        CHECK(b.dist[1] == -1);
        CHECK(b.dist[2] == 1);
        CHECK(b.dist[3] == 0);
        CHECK((grafos::caminho(b, 1) == std::vector<int32_t>{1, 2, 3, 4}));

        auto f = grafos::floyd_warshall(g);
        CHECK_FALSE(f.ciclo_negativo);
        CHECK(f.distancia(1, 4) == 4);
        CHECK(std::isinf(f.distancia(4, 1)));
        CHECK((f.caminho(1, 4) == std::vector<int32_t>{1, 2, 3, 4}));
        CHECK((f.arvore_origem(1) == std::vector<int32_t>{0, 1, 2, 3}));
    }
}

TEST_CASE("P3: ciclos negativos globais e pares afetados") {
    const std::string texto = "5\n1 2 1\n2 3 -3\n3 2 1\n3 4 2\n5 5 -1\n";
    for (auto rep : teste::REPRESENTACOES) {
        auto g = dirigido(texto, rep);
        auto b4 = grafos::bellman_ford(g, 4);
        CHECK(b4.ciclo_negativo);
        CHECK(b4.ciclo_afeta_destino);
        for (int v : {1, 2, 3}) CHECK(b4.dist[static_cast<size_t>(v - 1)] == -std::numeric_limits<double>::infinity());
        CHECK(b4.dist[3] == 0);
        CHECK(std::isinf(b4.dist[4]));
        CHECK(grafos::caminho(b4, 1).empty());
        auto b1 = grafos::bellman_ford(g, 1);
        CHECK(b1.ciclo_negativo);
        CHECK_FALSE(b1.ciclo_afeta_destino);

        auto f = grafos::floyd_warshall(g);
        CHECK(f.ciclo_negativo);
        CHECK(f.distancia(1, 4) == -std::numeric_limits<double>::infinity());
        CHECK(f.distancia(5, 5) == -std::numeric_limits<double>::infinity());
        CHECK(f.distancia(4, 1) == std::numeric_limits<double>::infinity());
        CHECK(f.caminho(1, 4).empty());
    }
}

TEST_CASE("P3: Dijkstra em arcos positivos e Bellman-Ford/Floyd concordam") {
    const std::string texto = "5\n1 2 2\n2 3 4\n1 3 9\n3 4 0\n4 5 1\n";
    for (auto rep : teste::REPRESENTACOES) {
        auto g = dirigido(texto, rep);
        auto heap = grafos::dijkstra(g, 1, grafos::FilaDijkstra::Heap);
        auto vetor = grafos::dijkstra(g, 1, grafos::FilaDijkstra::Vetor);
        auto f = grafos::floyd_warshall(g);
        for (int32_t v = 1; v <= 5; ++v) {
            CHECK(heap.dist[static_cast<size_t>(v - 1)] == vetor.dist[static_cast<size_t>(v - 1)]);
            CHECK(heap.dist[static_cast<size_t>(v - 1)] == f.distancia(1, v));
            auto b = grafos::bellman_ford(g, v);
            CHECK(b.dist[0] == f.distancia(1, v));
        }
        CHECK((grafos::caminho(heap, 5) == std::vector<int32_t>{1, 2, 3, 4, 5}));
    }
}

TEST_CASE("P3: 100 grafos dirigidos aleatorios contra Floyd de tres matrizes") {
    std::mt19937 rng(20261121);
    const double INF = std::numeric_limits<double>::infinity();
    for (int caso = 0; caso < 100; ++caso) {
        const int n = 2 + static_cast<int>(rng() % 6);
        std::ostringstream texto;
        texto << n << '\n';
        std::vector<double> ref(static_cast<size_t>(n*n), INF);
        for (int i = 0; i < n; ++i) ref[static_cast<size_t>(i*n+i)] = 0;
        for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
            if (rng() % 4 == 0) {
                const int w = static_cast<int>(rng() % 9) - 3;
                texto << i+1 << ' ' << j+1 << ' ' << w << '\n';
                auto& d = ref[static_cast<size_t>(i*n+j)];
                if (w < d) d = w;
            }
        }
        for (int k = 0; k < n; ++k) {
            auto nova = ref; // referencia independente: cada rodada le matriz anterior
            for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j) {
                if (ref[static_cast<size_t>(i*n+k)] == INF || ref[static_cast<size_t>(k*n+j)] == INF) continue;
                const double via = ref[static_cast<size_t>(i*n+k)] + ref[static_cast<size_t>(k*n+j)];
                if (via < nova[static_cast<size_t>(i*n+j)]) nova[static_cast<size_t>(i*n+j)] = via;
            }
            ref.swap(nova);
        }
        auto esperado = ref;
        for (int k = 0; k < n; ++k) if (ref[static_cast<size_t>(k*n+k)] < 0)
            for (int i = 0; i < n; ++i) for (int j = 0; j < n; ++j)
                if (ref[static_cast<size_t>(i*n+k)] != INF && ref[static_cast<size_t>(k*n+j)] != INF)
                    esperado[static_cast<size_t>(i*n+j)] = -INF;

        for (auto rep : teste::REPRESENTACOES) {
            CAPTURE(caso);
            CAPTURE(teste::nome(rep));
            auto g = dirigido(texto.str(), rep);
            auto f = grafos::floyd_warshall(g);
            auto fd = grafos::floyd_warshall_distancias(g, 2);
            const grafos::BellmanFord bf(g);
            CHECK(fd.proximo.empty());
            CHECK_THROWS_AS(fd.caminho(1, 1), grafos::ErroGrafo);
            bool ciclo = false;
            for (int k = 0; k < n; ++k) ciclo |= ref[static_cast<size_t>(k*n+k)] < 0;
            CHECK(f.ciclo_negativo == ciclo);
            for (int j = 0; j < n; ++j) {
                auto b = grafos::bellman_ford(g, j+1);
                CHECK(b.ciclo_negativo == ciclo);
                for (int i = 0; i < n; ++i) {
                    const double d = esperado[static_cast<size_t>(i*n+j)];
                    CAPTURE(i);
                    CAPTURE(j);
                    CHECK(f.distancia(i+1,j+1) == d);
                    CHECK(fd.distancia(i+1,j+1) == d);
                    CHECK(bf.executar(j+1).dist[static_cast<size_t>(i)] == d);
                    CHECK(b.dist[static_cast<size_t>(i)] == d);
                    if (std::isfinite(d)) {
                        const auto path = f.caminho(i+1,j+1);
                        REQUIRE(!path.empty());
                        CHECK(path.front() == i+1);
                        CHECK(path.back() == j+1);
                        double peso = 0;
                        for (size_t h = 1; h < path.size(); ++h) peso += g.peso(path[h-1],path[h]);
                        CHECK(peso == d);
                        const auto path_b = grafos::caminho(b, i+1);
                        REQUIRE(!path_b.empty());
                        double peso_b = 0;
                        for (size_t h = 1; h < path_b.size(); ++h) peso_b += g.peso(path_b[h-1],path_b[h]);
                        CHECK(peso_b == d);
                    }
                }
            }
        }
    }
}

TEST_CASE("P3: Floyd-Warshall em blocos (n > bloco) igual ao classico e ao Bellman-Ford preparado") {
    std::mt19937 rng(242);
    for (int variante = 0; variante < 3; ++variante) {
        // 0: pesos >= 0; 1: negativos sem ciclo (arcos so de i para j > i); 2: com ciclo negativo isolado
        const int n = 300 + variante * 37;
        std::ostringstream texto;
        texto << n << '\n';
        for (int e = 0; e < 4 * n; ++e) {
            int u = 1 + static_cast<int>(rng() % static_cast<uint32_t>(n));
            int v = 1 + static_cast<int>(rng() % static_cast<uint32_t>(n));
            int w = static_cast<int>(rng() % 20);
            if (variante >= 1) {
                if (u == v) continue;
                if (u > v) std::swap(u, v);
                w -= 5;
            }
            texto << u << ' ' << v << ' ' << w << '\n';
        }
        if (variante == 2) texto << "7 8 -3\n8 7 1\n";
        CAPTURE(variante);
        auto g = dirigido(texto.str(), grafos::Representacao::Lista);
        const auto f = grafos::floyd_warshall(g);
        const auto fd = grafos::floyd_warshall_distancias(g, 3);
        const grafos::BellmanFord bf(g);
        CHECK(fd.ciclo_negativo == (variante == 2));
        CHECK(f.ciclo_negativo == fd.ciclo_negativo);
        CHECK(f.dist == fd.dist);
        for (int t : {1, 50, 129, 257, n}) {
            const auto b = bf.executar(t);
            CHECK(b.ciclo_negativo == fd.ciclo_negativo);
            for (int s = 1; s <= n; ++s) {
                CAPTURE(t);
                CAPTURE(s);
                CHECK(b.dist[static_cast<size_t>(s - 1)] == fd.distancia(s, t));
            }
        }
    }
}
