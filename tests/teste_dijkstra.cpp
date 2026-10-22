#include <cstdint>
#include <string>
#include <vector>

#include "auxiliares_pesos.hpp"
#include "doctest.h"

using grafos::ErroGrafo;
using grafos::FilaDijkstra;
using grafos::Representacao;
using grafos::ResultadoDijkstra;
using teste::INF;

namespace {

constexpr FilaDijkstra FILAS[] = {FilaDijkstra::Vetor, FilaDijkstra::Heap};

const char* const MSG_NEGATIVO = "a biblioteca ainda nao implementa caminhos minimos com pesos negativos";

// Aula 8, vertices a..g = 1..7.
const std::string AULA8 = "7\n1 2 1\n1 4 4\n1 5 2\n2 3 4\n2 4 2\n3 4 1\n3 7 2\n4 5 2\n4 6 2\n5 6 3\n6 7 3\n";
const std::string FIGURA1_POSITIVA = "5\n1 2 0.1\n2 5 0.2\n5 3 5\n3 4 9.5\n4 5 2.3\n1 5 1\n";
const std::string FIGURA1_NEGATIVA = "5\n1 2 0.1\n2 5 0.2\n5 3 5\n3 4 -9.5\n4 5 2.3\n1 5 1\n";
const std::string DESCONEXO = "6\n1 2 2\n2 3 1.5\n4 5 1\n";

bool iguais(const ResultadoDijkstra& a, const ResultadoDijkstra& b) {
    return a.origem == b.origem && a.dist == b.dist && a.pai == b.pai && a.ordem == b.ordem &&
           a.explorados == b.explorados;
}

// Roda detalhe::dijkstra com a fronteira F sobre a representacao concreta de g.
template <class F>
ResultadoDijkstra dijkstra_limitado(const grafos::Grafo& g, int32_t origem, int64_t max_explorados) {
    return g.visitar(
        [&](const auto& c) { return grafos::detalhe::dijkstra<F>(c, origem - 1, max_explorados); });
}

}  // namespace

TEST_CASE("P2 6.3: exemplo da aula 8 (tabela a..g)") {
    for (Representacao rep : teste::REPRESENTACOES) {
        grafos::Grafo g = teste::grafo_de_texto(AULA8, rep);
        for (FilaDijkstra fila : FILAS) {
            CAPTURE(teste::nome(rep));
            CAPTURE(grafos::nome(fila));
            ResultadoDijkstra r = grafos::dijkstra(g, 1, fila);
            CHECK(r.fila == fila);
            CHECK(r.origem == 1);
            CHECK(r.dist == std::vector<double>{0, 1, 4, 3, 2, 5, 6});
            CHECK(r.pai == std::vector<int32_t>{0, 1, 4, 2, 1, 5, 3});
            CHECK(r.ordem == std::vector<int32_t>{1, 2, 5, 4, 3, 6, 7});  // a, b, e, d, c, f, g
            CHECK(r.explorados == 7);
            CHECK(grafos::caminho(r, 7) == std::vector<int32_t>{1, 2, 4, 3, 7});
        }
    }
}

TEST_CASE("P2 6.3: figura 1 do enunciado com pesos positivos") {
    const double d2 = 0.0 + 0.1;
    const double d5 = d2 + 0.2;
    const double d4 = d5 + 2.3;
    const double d3 = d5 + 5.0;
    for (Representacao rep : teste::REPRESENTACOES) {
        grafos::Grafo g = teste::grafo_de_texto(FIGURA1_POSITIVA, rep);
        for (FilaDijkstra fila : FILAS) {
            CAPTURE(teste::nome(rep));
            CAPTURE(grafos::nome(fila));
            ResultadoDijkstra r = grafos::dijkstra(g, 1, fila);
            CHECK(r.dist == std::vector<double>{0.0, d2, d3, d4, d5});
            CHECK(r.pai == std::vector<int32_t>{0, 1, 5, 5, 2});
            CHECK(grafos::caminho(r, 3) == std::vector<int32_t>{1, 2, 5, 3});
            CHECK(grafos::caminho(r, 1) == std::vector<int32_t>{1});
        }
    }
}

TEST_CASE("P2 6.3: pesos negativos sao recusados") {
    for (Representacao rep : teste::REPRESENTACOES) {
        grafos::Grafo texto = teste::grafo_de_texto(FIGURA1_NEGATIVA, rep);
        grafos::Grafo arquivo = grafos::carregar(teste::caminho_dados("figura1_pesos.txt"), rep);
        for (FilaDijkstra fila : FILAS) {
            CAPTURE(teste::nome(rep));
            CAPTURE(grafos::nome(fila));
            CHECK_THROWS_WITH_AS(grafos::dijkstra(texto, 1, fila), MSG_NEGATIVO, ErroGrafo);
            CHECK_THROWS_WITH_AS(grafos::dijkstra(arquivo, 2, fila), MSG_NEGATIVO, ErroGrafo);
        }
    }
}

TEST_CASE("P2 6.3: arestas de peso zero") {
    for (Representacao rep : teste::REPRESENTACOES) {
        grafos::Grafo g = teste::grafo_de_texto("4\n1 2 0\n2 3 0\n1 3 1\n3 4 0\n", rep);
        for (FilaDijkstra fila : FILAS) {
            CAPTURE(teste::nome(rep));
            CAPTURE(grafos::nome(fila));
            ResultadoDijkstra r = grafos::dijkstra(g, 1, fila);
            CHECK(r.dist == std::vector<double>{0, 0, 0, 0});
            CHECK(r.pai == std::vector<int32_t>{0, 1, 2, 3});
            CHECK(r.ordem == std::vector<int32_t>{1, 2, 3, 4});
        }
    }
}

TEST_CASE("P2 6.3: grafo desconexo") {
    for (Representacao rep : teste::REPRESENTACOES) {
        grafos::Grafo g = teste::grafo_de_texto(DESCONEXO, rep);
        for (FilaDijkstra fila : FILAS) {
            CAPTURE(teste::nome(rep));
            CAPTURE(grafos::nome(fila));
            ResultadoDijkstra r = grafos::dijkstra(g, 1, fila);
            CHECK(r.dist == std::vector<double>{0, 2, 3.5, INF, INF, INF});
            CHECK(r.pai == std::vector<int32_t>{0, 1, 2, -1, -1, -1});
            CHECK(r.ordem == std::vector<int32_t>{1, 2, 3, -1, -1, -1});
            CHECK(r.explorados == 3);
            CHECK(grafos::caminho(r, 4).empty());
            CHECK(grafos::caminho(r, 1) == std::vector<int32_t>{1});
            CHECK(grafos::caminho(r, 3) == std::vector<int32_t>{1, 2, 3});
        }
    }
}

TEST_CASE("P2 6.3: grafo sem peso da a mesma distancia que a BFS") {
    for (const teste::GrafoTeste& caso : teste::grafos_de_teste()) {
        CAPTURE(caso.nome);
        grafos::Grafo g = teste::grafo_de_texto(caso.texto(), Representacao::Lista);
        int64_t divergencias = 0;
        for (int32_t raiz : teste::raizes_de_teste(caso.n)) {
            const grafos::ArvoreBusca bfs = grafos::bfs(g, raiz);
            for (FilaDijkstra fila : FILAS) {
                const ResultadoDijkstra r = grafos::dijkstra(g, raiz, fila);
                for (size_t i = 0; i < r.dist.size(); ++i) {
                    const double esperado = bfs.nivel[i] < 0 ? INF : static_cast<double>(bfs.nivel[i]);
                    if (r.dist[i] != esperado) ++divergencias;
                }
            }
        }
        CHECK(divergencias == 0);
    }
}

TEST_CASE("P2 6.3: origem e destino fora de 1..n lancam") {
    for (Representacao rep : teste::REPRESENTACOES) {
        grafos::Grafo g = teste::grafo_de_texto(FIGURA1_POSITIVA, rep);
        for (FilaDijkstra fila : FILAS) {
            CHECK_THROWS_WITH_AS(grafos::dijkstra(g, 0, fila), "vertice 0 fora do intervalo 1..5", ErroGrafo);
            CHECK_THROWS_AS(grafos::dijkstra(g, 6, fila), ErroGrafo);
        }
        ResultadoDijkstra r = grafos::dijkstra(g, 1);
        CHECK_THROWS_WITH_AS(grafos::caminho(r, 0), "vertice 0 fora do intervalo 1..5", ErroGrafo);
        CHECK_THROWS_AS(grafos::caminho(r, 6), ErroGrafo);
    }
}

TEST_CASE("P2 6.3: max_explorados interrompe a busca") {
    grafos::Grafo g = teste::grafo_de_texto(AULA8, Representacao::Lista);
    for (int vez = 0; vez < 2; ++vez) {
        ResultadoDijkstra r = vez == 0 ? dijkstra_limitado<grafos::detalhe::FronteiraVetor>(g, 1, 3)
                                       : dijkstra_limitado<grafos::detalhe::FronteiraHeap>(g, 1, 3);
        CAPTURE(vez);
        CHECK(r.explorados == 3);
        CHECK(r.ordem == std::vector<int32_t>{1, 2, -1, -1, 3, -1, -1});  // a, b, e
    }
    grafos::Grafo d = teste::grafo_de_texto(DESCONEXO, Representacao::Lista);
    CHECK(dijkstra_limitado<grafos::detalhe::FronteiraHeap>(d, 1, -1).explorados == 3);
    CHECK(dijkstra_limitado<grafos::detalhe::FronteiraVetor>(d, 4, -1).explorados == 2);
}

TEST_CASE("P2 6.3: arquivo do Dijkstra") {
    teste::DiretorioTemporario dir;
    grafos::Grafo g = teste::grafo_de_texto(FIGURA1_POSITIVA, Representacao::Lista);
    const std::string heap = dir.arquivo("dijkstra_heap.txt");
    grafos::escrever_dijkstra(grafos::dijkstra(g, 1, FilaDijkstra::Heap), heap);
    CHECK(teste::ler_arquivo(heap) == "# Dijkstra origem=1 fila=heap\n"
                                      "# vertice pai distancia\n"
                                      "1 0 0\n"
                                      "2 1 0.1\n"
                                      "3 5 5.3\n"
                                      "4 5 2.5999999999999996\n"
                                      "5 2 0.30000000000000004\n");

    grafos::Grafo d = teste::grafo_de_texto(DESCONEXO, Representacao::Matriz);
    const std::string vetor = dir.arquivo("dijkstra_vetor.txt");
    grafos::escrever_dijkstra(grafos::dijkstra(d, 2, FilaDijkstra::Vetor), vetor);
    CHECK(teste::ler_arquivo(vetor) == "# Dijkstra origem=2 fila=vetor\n"
                                       "# vertice pai distancia\n"
                                       "1 2 2\n"
                                       "2 0 0\n"
                                       "3 2 1.5\n"
                                       "4 -1 infinito\n"
                                       "5 -1 infinito\n"
                                       "6 -1 infinito\n");
}

TEST_CASE("P2 6.4: aleatorios - vetor = heap, lista = matriz, = Bellman-Ford, caminhos validos") {
    int64_t combinacoes = 0;
    for (teste::RegimePeso regime : {teste::RegimePeso::Inteiro, teste::RegimePeso::Real}) {
        const bool inteiro = regime == teste::RegimePeso::Inteiro;
        uint64_t semente = inteiro ? 100 : 200;
        for (const teste::GrafoTeste& base : teste::grafos_de_teste()) {
            const teste::GrafoPesoTeste caso = teste::com_pesos(base, regime, semente++);
            CAPTURE(caso.nome);
            CAPTURE(inteiro);
            const std::string texto = caso.texto();
            grafos::Grafo lista = teste::grafo_de_texto(texto, Representacao::Lista);
            grafos::Grafo matriz = teste::grafo_de_texto(texto, Representacao::Matriz);
            for (int32_t raiz : teste::raizes_de_teste(caso.n)) {
                CAPTURE(raiz);
                const ResultadoDijkstra lv = grafos::dijkstra(lista, raiz, FilaDijkstra::Vetor);
                const ResultadoDijkstra lh = grafos::dijkstra(lista, raiz, FilaDijkstra::Heap);
                const ResultadoDijkstra mv = grafos::dijkstra(matriz, raiz, FilaDijkstra::Vetor);
                const ResultadoDijkstra mh = grafos::dijkstra(matriz, raiz, FilaDijkstra::Heap);
                CHECK(iguais(lv, lh));
                CHECK(iguais(lv, mv));
                CHECK(iguais(lv, mh));

                const std::vector<double> ref = teste::bellman_ford(caso, raiz);
                int64_t erros_dist = 0;
                int64_t erros_caminho = 0;
                for (int32_t v = 1; v <= caso.n; ++v) {
                    const double d = lh.dist[static_cast<size_t>(v - 1)];
                    const double e = ref[static_cast<size_t>(v - 1)];
                    if (inteiro || e == INF) {
                        if (d != e) ++erros_dist;
                    } else if (!(d == doctest::Approx(e).epsilon(1e-12))) {
                        ++erros_dist;
                    }
                    const std::vector<int32_t> c = grafos::caminho(lh, v);
                    if (d == INF) {
                        if (!c.empty()) ++erros_caminho;
                        continue;
                    }
                    if (c.empty() || c.front() != raiz || c.back() != v) {
                        ++erros_caminho;
                        continue;
                    }
                    // Soma na mesma ordem do algoritmo (a partir de 0): igualdade exata.
                    double soma = 0.0;
                    for (size_t k = 1; k < c.size(); ++k) {
                        if (!lista.sao_vizinhos(c[k - 1], c[k])) {
                            ++erros_caminho;
                            break;
                        }
                        soma += lista.peso(c[k - 1], c[k]);
                    }
                    if (soma != d) ++erros_caminho;
                }
                CHECK(erros_dist == 0);
                CHECK(erros_caminho == 0);
                ++combinacoes;
            }
        }
    }
    MESSAGE("combinacoes (grafo, pesos, raiz) testadas: ", combinacoes);
}
