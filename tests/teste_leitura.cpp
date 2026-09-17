#include <string>

#include "auxiliares.hpp"
#include "doctest.h"

using grafos::ErroGrafo;
using grafos::InfoLeitura;
using grafos::Representacao;

TEST_CASE("11.6: repeticoes e lacos sao ignorados e contados") {
    for (Representacao rep : teste::REPRESENTACOES) {
        CAPTURE(teste::nome(rep));
        InfoLeitura info;
        grafos::Grafo g = grafos::carregar(teste::caminho_dados("repetidas_lacos.txt"), rep, false, &info);
        CHECK(g.representacao() == rep);
        CHECK(g.n() == 3);
        CHECK(g.m() == 1);
        CHECK(info.linhas_aresta == 4);
        CHECK(info.repetidas_ignoradas == 2);
        CHECK(info.lacos_ignorados == 1);
        CHECK(g.grau(1) == 1);
        CHECK(g.grau(2) == 1);
        CHECK(g.grau(3) == 0);
        CHECK(g.sao_vizinhos(1, 2));
        CHECK(g.sao_vizinhos(2, 1));
        CHECK_FALSE(g.sao_vizinhos(3, 3));
        CHECK(teste::vizinhos(g, 3).empty());
    }
}

TEST_CASE("11.6: CRLF, tabulacoes, linhas vazias e terceira coluna") {
    for (Representacao rep : teste::REPRESENTACOES) {
        CAPTURE(teste::nome(rep));
        InfoLeitura info;
        grafos::Grafo g = grafos::carregar(teste::caminho_dados("formato_variado.txt"), rep, false, &info);
        grafos::Grafo fig = grafos::carregar(teste::caminho_dados("figura1.txt"), rep);
        CHECK(g.n() == 5);
        CHECK(g.m() == 5);
        CHECK(info.linhas_aresta == 5);
        CHECK(info.lacos_ignorados == 0);
        CHECK(info.repetidas_ignoradas == 0);
        for (int32_t v = 1; v <= 5; ++v) {
            CAPTURE(v);
            CHECK(teste::vizinhos(g, v) == teste::vizinhos(fig, v));
        }
    }
}

TEST_CASE("leitura: n na linha com numeros extras e arquivo sem quebra de linha final") {
    for (Representacao rep : teste::REPRESENTACOES) {
        grafos::Grafo g = teste::grafo_de_texto("\n\n4 99 abc\n1 4\n  \n3 2", rep);
        CHECK(g.n() == 4);
        CHECK(g.m() == 2);
        CHECK(g.sao_vizinhos(4, 1));
        CHECK(g.sao_vizinhos(2, 3));
    }
}

TEST_CASE("leitura: so a linha de n e vertices isolados") {
    for (Representacao rep : teste::REPRESENTACOES) {
        grafos::Grafo g = teste::grafo_de_texto("7\n", rep);
        CHECK(g.n() == 7);
        CHECK(g.m() == 0);
        for (int32_t v = 1; v <= 7; ++v) CHECK(g.grau(v) == 0);
    }
}

namespace {
void checar_erro(const std::string& texto, const std::string& mensagem) {
    CAPTURE(texto);
    CHECK_THROWS_WITH_AS(grafos::parsear_arestas(texto.data(), texto.size()), mensagem.c_str(), ErroGrafo);
}
}  // namespace

TEST_CASE("11.6: erros com numero de linha") {
    checar_erro("5\n1 2\n0 3\n", "linha 3: vertice 0 fora do intervalo 1..5");
    checar_erro("5\n1 2\n\n3 6\n", "linha 4: vertice 6 fora do intervalo 1..5");
    checar_erro("5\n1 2\n4\n", "linha 3: esperados dois vertices, encontrado apenas um");
    checar_erro("5\n1 x\n", "linha 2: texto invalido 'x'");
    checar_erro("5\n1a 2\n", "linha 2: texto invalido '1a'");
    checar_erro("5\n1 2.0\n", "linha 2: texto invalido '2.0'");
    checar_erro("", "linha 1: n ausente");
    checar_erro("\n\n", "linha 2: n ausente");
    checar_erro("abc\n1 2\n", "linha 1: texto invalido 'abc' (esperado n)");
    checar_erro("0\n", "linha 1: n deve ser positivo (lido 0)");
    checar_erro("\n-3\n", "linha 2: n deve ser positivo (lido -3)");
    checar_erro("2147483648\n", "linha 1: n 2147483648 maior que o limite de int32_t");
    checar_erro("5\n-1 2\n", "linha 2: vertice -1 fora do intervalo 1..5");
    checar_erro("5\r\n1 2\r\n99999999999999999999 2\r\n",
                "linha 3: vertice 99999999999999999999 fora do intervalo 1..5");
}

TEST_CASE("leitura: n no limite de int32_t e aceito pelo parser") {
    std::string texto = "2147483647\n1 2147483647\n";
    grafos::ArestasLidas a = grafos::parsear_arestas(texto.data(), texto.size());
    CHECK(a.n == 2147483647);
    REQUIRE(a.origem.size() == 1);
    CHECK(a.origem[0] == 0);
    CHECK(a.destino[0] == 2147483646);
}

TEST_CASE("carregar: arquivo inexistente e grafo direcionado") {
    CHECK_THROWS_AS(grafos::carregar(teste::caminho_dados("nao_existe.txt"), Representacao::Lista), ErroGrafo);
    CHECK_THROWS_WITH_AS(
        grafos::carregar(teste::caminho_dados("figura1.txt"), Representacao::Lista, true),
        "grafos direcionados ainda nao suportados", ErroGrafo);
}

TEST_CASE("Grafo: vertice fora de 1..n na API publica") {
    grafos::Grafo g = grafos::carregar(teste::caminho_dados("figura1.txt"), Representacao::Lista);
    CHECK_THROWS_WITH_AS(g.grau(0), "vertice 0 fora do intervalo 1..5", ErroGrafo);
    CHECK_THROWS_AS(g.grau(6), ErroGrafo);
    CHECK_THROWS_AS(g.sao_vizinhos(1, 6), ErroGrafo);
}
