#include <cstdint>
#include <queue>
#include <string>
#include <vector>

#include "auxiliares.hpp"
#include "doctest.h"

using grafos::ArvoreBusca;
using grafos::Representacao;
using grafos::TipoBusca;

namespace {

const std::string FIGURA1 = "5\n1 2\n2 5\n5 3\n4 5\n1 5\n";
const std::string AULA4 = "7\n1 2\n1 3\n1 4\n2 4\n2 6\n3 4\n4 7\n5 6\n5 7\n6 7\n";
const std::string AULA5_BFS = "7\n1 2\n1 3\n1 4\n2 4\n2 7\n3 4\n4 6\n5 6\n5 7\n6 7\n";
const std::string AULA5_DFS = "8\n1 3\n1 4\n2 3\n2 4\n3 4\n3 8\n4 6\n4 7\n5 6\n5 7\n6 7\n";
const std::string DESCONEXO = "8\n1 2\n2 3\n5 6\n";

// Converte a sequencia de visita (vertices 1..n) no vetor ordem.
std::vector<int32_t> ordem_da_sequencia(int32_t n, const std::vector<int32_t>& sequencia) {
    std::vector<int32_t> ordem(static_cast<size_t>(n), -1);
    for (size_t i = 0; i < sequencia.size(); ++i) ordem[static_cast<size_t>(sequencia[i] - 1)] = static_cast<int32_t>(i) + 1;
    return ordem;
}

void checar_arvore(const std::string& grafo, TipoBusca tipo, int32_t raiz, const std::vector<int32_t>& pai,
                   const std::vector<int32_t>& nivel, const std::vector<int32_t>& sequencia) {
    for (Representacao rep : teste::REPRESENTACOES) {
        CAPTURE(teste::nome(rep));
        CAPTURE(raiz);
        grafos::Grafo g = teste::grafo_de_texto(grafo, rep);
        ArvoreBusca a = tipo == TipoBusca::BFS ? grafos::bfs(g, raiz) : grafos::dfs(g, raiz);
        CHECK(a.tipo == tipo);
        CHECK(a.raiz == raiz);
        if (!pai.empty()) CHECK(a.pai == pai);
        CHECK(a.nivel == nivel);
        if (!sequencia.empty()) CHECK(a.ordem == ordem_da_sequencia(g.n(), sequencia));
    }
}

ArvoreBusca bfs_referencia(const std::vector<std::vector<int32_t>>& adj, int32_t n, int32_t raiz) {
    ArvoreBusca a{TipoBusca::BFS, raiz, std::vector<int32_t>(static_cast<size_t>(n), -1),
                  std::vector<int32_t>(static_cast<size_t>(n), -1), std::vector<int32_t>(static_cast<size_t>(n), -1)};
    std::queue<int32_t> fila;
    int32_t contador = 0;
    a.pai[static_cast<size_t>(raiz - 1)] = 0;
    a.nivel[static_cast<size_t>(raiz - 1)] = 0;
    a.ordem[static_cast<size_t>(raiz - 1)] = ++contador;
    fila.push(raiz);
    while (!fila.empty()) {
        int32_t v = fila.front();
        fila.pop();
        for (int32_t w : adj[static_cast<size_t>(v)]) {
            size_t iw = static_cast<size_t>(w - 1);
            if (a.nivel[iw] >= 0) continue;
            a.pai[iw] = v;
            a.nivel[iw] = a.nivel[static_cast<size_t>(v - 1)] + 1;
            a.ordem[iw] = ++contador;
            fila.push(w);
        }
    }
    return a;
}

// DFS recursiva de referencia, visitando vizinhos em ordem crescente.
void dfs_recursiva(const std::vector<std::vector<int32_t>>& adj, ArvoreBusca& a, int32_t& contador, int32_t u,
                   int32_t pai, int32_t profundidade) {
    size_t iu = static_cast<size_t>(u - 1);
    a.pai[iu] = pai;
    a.nivel[iu] = profundidade;
    a.ordem[iu] = ++contador;
    for (int32_t w : adj[static_cast<size_t>(u)]) {
        if (a.nivel[static_cast<size_t>(w - 1)] < 0) dfs_recursiva(adj, a, contador, w, u, profundidade + 1);
    }
}

ArvoreBusca dfs_referencia(const std::vector<std::vector<int32_t>>& adj, int32_t n, int32_t raiz) {
    ArvoreBusca a{TipoBusca::DFS, raiz, std::vector<int32_t>(static_cast<size_t>(n), -1),
                  std::vector<int32_t>(static_cast<size_t>(n), -1), std::vector<int32_t>(static_cast<size_t>(n), -1)};
    int32_t contador = 0;
    dfs_recursiva(adj, a, contador, raiz, 0, 0);
    return a;
}

bool arvores_iguais(const ArvoreBusca& x, const ArvoreBusca& y) {
    return x.tipo == y.tipo && x.raiz == y.raiz && x.pai == y.pai && x.nivel == y.nivel && x.ordem == y.ordem;
}

}  // namespace

TEST_CASE("9.2: arquivo da arvore da Figura 1 (BFS e DFS a partir de 1)") {
    const std::string bfs_esperado = "# BFS raiz=1\n"
                                     "# vertice pai nivel ordem\n"
                                     "1 0 0 1\n"
                                     "2 1 1 2\n"
                                     "3 5 2 4\n"
                                     "4 5 2 5\n"
                                     "5 1 1 3\n";
    const std::string dfs_esperado = "# DFS raiz=1\n"
                                     "# vertice pai nivel ordem\n"
                                     "1 0 0 1\n"
                                     "2 1 1 2\n"
                                     "3 5 3 4\n"
                                     "4 5 3 5\n"
                                     "5 2 2 3\n";
    teste::DiretorioTemporario dir;
    for (Representacao rep : teste::REPRESENTACOES) {
        CAPTURE(teste::nome(rep));
        grafos::Grafo g = grafos::carregar(teste::caminho_dados("figura1.txt"), rep);
        const std::string arquivo_bfs = dir.arquivo(std::string("bfs_") + teste::nome(rep) + ".txt");
        const std::string arquivo_dfs = dir.arquivo(std::string("dfs_") + teste::nome(rep) + ".txt");
        grafos::escrever_arvore(grafos::bfs(g, 1), arquivo_bfs);
        grafos::escrever_arvore(grafos::dfs(g, 1), arquivo_dfs);
        CHECK(teste::ler_arquivo(arquivo_bfs) == bfs_esperado);
        CHECK(teste::ler_arquivo(arquivo_dfs) == dfs_esperado);
    }
}

TEST_CASE("escrever_arvore: caminho invalido lanca") {
    grafos::Grafo g = teste::grafo_de_texto(FIGURA1, Representacao::Lista);
    CHECK_THROWS_AS(grafos::escrever_arvore(grafos::bfs(g, 1), "/diretorio/inexistente/arvore.txt"),
                    grafos::ErroGrafo);
}

TEST_CASE("11.2: BFS dos exemplos da aula 4") {
    checar_arvore(AULA4, TipoBusca::BFS, 1, {0, 1, 1, 1, 6, 2, 4}, {0, 1, 1, 1, 3, 2, 2}, {1, 2, 3, 4, 6, 7, 5});
    checar_arvore(AULA4, TipoBusca::BFS, 2, {}, {1, 0, 2, 1, 2, 1, 2}, {});
    checar_arvore(AULA4, TipoBusca::BFS, 6, {2, 6, 1, 2, 6, 0, 6}, {2, 1, 3, 2, 1, 0, 1}, {6, 2, 5, 7, 1, 4, 3});
}

TEST_CASE("11.3: BFS do exemplo da aula 5") {
    checar_arvore(AULA5_BFS, TipoBusca::BFS, 5, {4, 7, 4, 6, 0, 5, 5}, {3, 2, 3, 2, 0, 1, 1}, {5, 6, 7, 4, 2, 1, 3});
}

TEST_CASE("11.4: DFS do exemplo da aula 5") {
    checar_arvore(AULA5_DFS, TipoBusca::DFS, 1, {0, 3, 1, 2, 6, 4, 5, 3}, {0, 2, 1, 3, 5, 4, 6, 2},
                  {1, 3, 2, 4, 6, 5, 7, 8});
    checar_arvore(AULA5_DFS, TipoBusca::DFS, 4, {4, 3, 1, 0, 6, 4, 5, 3}, {1, 3, 2, 0, 2, 1, 3, 3},
                  {4, 1, 3, 2, 8, 6, 5, 7});
}

TEST_CASE("11.5: BFS no grafo desconexo deixa vertices nao alcancados com -1") {
    checar_arvore(DESCONEXO, TipoBusca::BFS, 1, {0, 1, 2, -1, -1, -1, -1, -1}, {0, 1, 2, -1, -1, -1, -1, -1},
                  {1, 2, 3});
    checar_arvore(DESCONEXO, TipoBusca::DFS, 6, {-1, -1, -1, -1, 6, 0, -1, -1}, {-1, -1, -1, -1, 1, 0, -1, -1},
                  {6, 5});

    teste::DiretorioTemporario dir;
    grafos::Grafo g = teste::grafo_de_texto(DESCONEXO, Representacao::Matriz);
    const std::string arquivo = dir.arquivo("desconexo_bfs.txt");
    grafos::escrever_arvore(grafos::bfs(g, 1), arquivo);
    CHECK(teste::ler_arquivo(arquivo) == "# BFS raiz=1\n# vertice pai nivel ordem\n"
                                         "1 0 0 1\n2 1 1 2\n3 2 2 3\n4 -1 -1 -1\n5 -1 -1 -1\n"
                                         "6 -1 -1 -1\n7 -1 -1 -1\n8 -1 -1 -1\n");
}

TEST_CASE("BFS e DFS: raiz fora de 1..n lanca") {
    for (Representacao rep : teste::REPRESENTACOES) {
        grafos::Grafo g = teste::grafo_de_texto(FIGURA1, rep);
        CHECK_THROWS_WITH_AS(grafos::bfs(g, 0), "vertice 0 fora do intervalo 1..5", grafos::ErroGrafo);
        CHECK_THROWS_AS(grafos::bfs(g, 6), grafos::ErroGrafo);
        CHECK_THROWS_AS(grafos::dfs(g, -1), grafos::ErroGrafo);
        CHECK_THROWS_AS(grafos::dfs(g, 6), grafos::ErroGrafo);
    }
}

TEST_CASE("11.7: BFS e DFS identicas nas duas representacoes e iguais as referencias") {
    int64_t combinacoes = 0;
    for (const teste::GrafoTeste& caso : teste::grafos_de_teste()) {
        CAPTURE(caso.nome);
        const auto adj = caso.adjacencia();
        const std::string texto = caso.texto();
        grafos::Grafo lista = teste::grafo_de_texto(texto, Representacao::Lista);
        grafos::Grafo matriz = teste::grafo_de_texto(texto, Representacao::Matriz);
        for (int32_t raiz : teste::raizes_de_teste(caso.n)) {
            CAPTURE(raiz);
            const ArvoreBusca bfs_l = grafos::bfs(lista, raiz);
            const ArvoreBusca bfs_m = grafos::bfs(matriz, raiz);
            const ArvoreBusca dfs_l = grafos::dfs(lista, raiz);
            const ArvoreBusca dfs_m = grafos::dfs(matriz, raiz);
            CHECK(arvores_iguais(bfs_l, bfs_m));
            CHECK(arvores_iguais(dfs_l, dfs_m));
            CHECK(arvores_iguais(bfs_l, bfs_referencia(adj, caso.n, raiz)));
            CHECK(arvores_iguais(dfs_l, dfs_referencia(adj, caso.n, raiz)));
            ++combinacoes;
        }
    }
    MESSAGE("combinacoes (grafo, raiz) testadas: ", combinacoes);
}
