#include <fstream>
#include <string>

#include "auxiliares.hpp"
#include "doctest.h"

using grafos::ErroGrafo;
using grafos::NomesVertices;

namespace {

std::string gravar(const teste::DiretorioTemporario& dir, const std::string& nome, const std::string& conteudo) {
    const std::string caminho = dir.arquivo(nome);
    std::ofstream(caminho, std::ios::binary) << conteudo;
    return caminho;
}

}  // namespace

TEST_CASE("P2 6.5: nomes com virgula, UTF-8, CRLF, linhas vazias e id faltando") {
    teste::DiretorioTemporario dir;
    const std::string arquivo = gravar(dir, "nomes.txt",
                                       "1,Edoardo Airoldi\r\n2,Smith, John\n\n3,\xC3\x89va Tardos\n"
                                       "5,Edsger W. Dijkstra\n");
    const NomesVertices nomes = NomesVertices::carregar(arquivo);
    CHECK(nomes.tamanho() == 5);
    CHECK(nomes.nome(1) == "Edoardo Airoldi");
    CHECK(nomes.nome(2) == "Smith, John");
    CHECK(nomes.nome(3) == "\xC3\x89va Tardos");  // "Éva Tardos" em UTF-8
    CHECK(nomes.nome(4).empty());
    CHECK(nomes.nome(5) == "Edsger W. Dijkstra");
    CHECK(nomes.id("\xC3\x89va Tardos") == 3);
    CHECK(nomes.id("Edsger W. Dijkstra") == 5);
    CHECK(nomes.id("Smith, John") == 2);
    CHECK_THROWS_WITH_AS(nomes.id("Eva Tardos"), "nome nao encontrado: Eva Tardos", ErroGrafo);
    CHECK_THROWS_WITH_AS(nomes.nome(0), "id 0 fora do intervalo 1..5", ErroGrafo);
    CHECK_THROWS_WITH_AS(nomes.nome(6), "id 6 fora do intervalo 1..5", ErroGrafo);
}

TEST_CASE("P2 6.5: nome ambiguo") {
    teste::DiretorioTemporario dir;
    const NomesVertices nomes = NomesVertices::carregar(gravar(dir, "amb.txt", "1,Ana\n2,Bia\n3,Ana\n"));
    CHECK(nomes.id("Bia") == 2);
    CHECK_THROWS_WITH_AS(nomes.id("Ana"), "nome ambiguo: Ana", ErroGrafo);
}

TEST_CASE("P2 6.5: erros de formato com numero de linha") {
    teste::DiretorioTemporario dir;
    CHECK_THROWS_WITH_AS(NomesVertices::carregar(gravar(dir, "a.txt", "1,Ana\nBia\n")), "linha 2: virgula ausente",
                         ErroGrafo);
    CHECK_THROWS_WITH_AS(NomesVertices::carregar(gravar(dir, "b.txt", "1,Ana\nx,Bia\n")), "linha 2: id invalido 'x'",
                         ErroGrafo);
    CHECK_THROWS_WITH_AS(NomesVertices::carregar(gravar(dir, "c.txt", "0,Ana\n")), "linha 1: id invalido '0'",
                         ErroGrafo);
    CHECK_THROWS_WITH_AS(NomesVertices::carregar(gravar(dir, "d.txt", "1,Ana\n2,Bia\n1,Caio\n")),
                         "linha 3: id 1 repetido", ErroGrafo);
    CHECK_THROWS_AS(NomesVertices::carregar(dir.arquivo("inexistente.txt")), ErroGrafo);
}
