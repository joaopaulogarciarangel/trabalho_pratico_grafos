#include <cstdint>
#include <string>

#include "auxiliares.hpp"
#include "doctest.h"

using grafos::ErroGrafo;
using grafos::InfoLeitura;
using grafos::Representacao;

namespace {

const std::string FIGURA1_PESOS = "5\n1 2 0.1\n2 5 0.2\n5 3 5\n3 4 -9.5\n4 5 2.3\n1 5 1\n";

grafos::ArestasLidas parsear(const std::string& texto) {
    return grafos::parsear_arestas(texto.data(), texto.size());
}

}  // namespace

TEST_CASE("P2 6.1: figura 1 do enunciado com pesos (texto e arquivo)") {
    for (Representacao rep : teste::REPRESENTACOES) {
        CAPTURE(teste::nome(rep));
        for (int origem = 0; origem < 2; ++origem) {
            CAPTURE(origem);
            grafos::Grafo g = origem == 0 ? teste::grafo_de_texto(FIGURA1_PESOS, rep)
                                          : grafos::carregar(teste::caminho_dados("figura1_pesos.txt"), rep);
            CHECK(g.ponderado());
            CHECK(g.tem_peso_negativo());
            CHECK(g.m() == 6);
            CHECK(g.peso(1, 2) == 0.1);
            CHECK(g.peso(2, 5) == 0.2);
            CHECK(g.peso(5, 3) == 5.0);
            CHECK(g.peso(3, 4) == -9.5);
            CHECK(g.peso(4, 3) == -9.5);
            CHECK(g.peso(4, 5) == 2.3);
            CHECK(g.peso(5, 1) == 1.0);
        }
    }
    grafos::ArestasLidas a = parsear(FIGURA1_PESOS);
    CHECK(a.ponderado);
    CHECK(a.tem_peso_negativo);
    CHECK(a.peso.size() == a.origem.size());
}

TEST_CASE("P2 6.1: formatos de peso, CRLF, tabulacao e colunas extras") {
    const std::string texto = "4\r\n1 2 7\r\n2\t3\t-0.25\r\n\r\n3 4 1e-3 coluna extra\r\n1 4 .5\r\n";
    for (Representacao rep : teste::REPRESENTACOES) {
        CAPTURE(teste::nome(rep));
        grafos::Grafo g = teste::grafo_de_texto(texto, rep);
        CHECK(g.ponderado());
        CHECK(g.tem_peso_negativo());
        CHECK(g.peso(1, 2) == 7.0);
        CHECK(g.peso(3, 2) == -0.25);
        CHECK(g.peso(3, 4) == 1e-3);
        CHECK(g.peso(4, 1) == 0.5);
    }
}

TEST_CASE("P2 6.1: erros de peso com numero de linha") {
    CHECK_THROWS_WITH_AS(parsear("3\n1 2 1.5\n2 3\n"), "linha 3: peso ausente", ErroGrafo);
    CHECK_THROWS_WITH_AS(parsear("3\n1 2 1.5\n\n2 3 abc\n"), "linha 4: peso invalido 'abc'", ErroGrafo);
    CHECK_THROWS_WITH_AS(parsear("3\n1 2 1.5\n2 3 1.5x\n"), "linha 3: peso invalido '1.5x'", ErroGrafo);
    CHECK_THROWS_WITH_AS(parsear("3\n1 2 1.5\n2 3 nan\n"), "linha 3: peso invalido 'nan'", ErroGrafo);
    CHECK_THROWS_WITH_AS(parsear("3\n1 2 inf\n"), "linha 2: peso invalido 'inf'", ErroGrafo);
    CHECK_THROWS_WITH_AS(parsear("3\n1 2 1.5\n2 3 -1e400\n"), "linha 3: peso invalido '-1e400'", ErroGrafo);
}

TEST_CASE("P2 6.1: aresta repetida fica com o menor peso") {
    const std::string texto = "3\n1 2 5\n2 1 2.5\n1 2 7\n2 3 1\n3 2 4\n";
    for (Representacao rep : teste::REPRESENTACOES) {
        CAPTURE(teste::nome(rep));
        InfoLeitura info;
        grafos::Grafo g = teste::grafo_de_texto(texto, rep, &info);
        CHECK(g.m() == 2);
        CHECK(info.repetidas_ignoradas == 3);
        CHECK(g.peso(1, 2) == 2.5);
        CHECK(g.peso(2, 1) == 2.5);
        CHECK(g.peso(2, 3) == 1.0);
        CHECK(g.peso(3, 2) == 1.0);
    }
}

TEST_CASE("P2 6.1: laco com peso negativo e descartado e nao conta como negativo") {
    const std::string texto = "3\n1 2 1\n3 3 -4\n2 3 0\n";
    grafos::ArestasLidas a = parsear(texto);
    CHECK(a.ponderado);
    CHECK_FALSE(a.tem_peso_negativo);
    CHECK(a.lacos_ignorados == 1);
    for (Representacao rep : teste::REPRESENTACOES) {
        CAPTURE(teste::nome(rep));
        grafos::Grafo g = teste::grafo_de_texto(texto, rep);
        CHECK(g.ponderado());
        CHECK_FALSE(g.tem_peso_negativo());
        CHECK(g.m() == 2);
        CHECK(g.peso(2, 3) == 0.0);
    }
}

TEST_CASE("P2 6.1: arquivo sem peso continua nao ponderado, com pesos 1") {
    for (Representacao rep : teste::REPRESENTACOES) {
        CAPTURE(teste::nome(rep));
        grafos::Grafo fig = grafos::carregar(teste::caminho_dados("figura1.txt"), rep);
        CHECK_FALSE(fig.ponderado());
        CHECK_FALSE(fig.tem_peso_negativo());
        CHECK(fig.peso(1, 2) == 1.0);
        // 1a linha de aresta sem peso: o resto das colunas e ignorado, como na Parte 1.
        grafos::Grafo variado = grafos::carregar(teste::caminho_dados("formato_variado.txt"), rep);
        CHECK_FALSE(variado.ponderado());
        CHECK_FALSE(variado.tem_peso_negativo());
        CHECK(variado.peso(5, 3) == 1.0);
    }
    grafos::ArestasLidas a = parsear("3\n1 2\n2 3 -5\n");
    CHECK_FALSE(a.ponderado);
    CHECK_FALSE(a.tem_peso_negativo);
    CHECK(a.peso.empty());
}

TEST_CASE("P2 6.1: peso de nao vizinhos e de vertice invalido lanca") {
    for (Representacao rep : teste::REPRESENTACOES) {
        grafos::Grafo g = teste::grafo_de_texto(FIGURA1_PESOS, rep);
        CHECK_THROWS_WITH_AS(g.peso(1, 3), "vertices 1 e 3 nao sao vizinhos", ErroGrafo);
        CHECK_THROWS_WITH_AS(g.peso(1, 6), "vertice 6 fora do intervalo 1..5", ErroGrafo);
    }
}

TEST_CASE("P2 6.1: memoria com pesos") {
    // Mesma topologia (figura 1 com pesos: n = 5, 2m = 12 entradas), com e sem a coluna de peso;
    // listas reservadas no tamanho exato.
    std::string sem_peso;
    for (const char* l : {"5", "1 2", "2 5", "5 3", "3 4", "4 5", "1 5"}) sem_peso += std::string(l) + "\n";
    const size_t base = 12 * 4 + 5 * sizeof(std::vector<int32_t>) + 5 * 4;
    grafos::Grafo sem = teste::grafo_de_texto(sem_peso, Representacao::Lista);
    grafos::Grafo com = teste::grafo_de_texto(FIGURA1_PESOS, Representacao::Lista);
    CHECK(sem.bytes_estrutura() == base);
    CHECK(com.bytes_estrutura() == base + 12 * 8 + 5 * sizeof(std::vector<double>));

    CHECK(grafos::estimar_bytes_matriz(1000, true) == grafos::estimar_bytes_matriz(1000, false) + 8 * 1000 * 1000);
    CHECK(grafos::estimar_bytes_matriz(1000) == grafos::estimar_bytes_matriz(1000, false));
    grafos::Grafo matriz = teste::grafo_de_texto(FIGURA1_PESOS, Representacao::Matriz);
    CHECK(matriz.bytes_estrutura() == grafos::estimar_bytes_matriz(5, true));
}
