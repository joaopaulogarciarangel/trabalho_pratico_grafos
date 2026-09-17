#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "auxiliares.hpp"
#include "doctest.h"

using grafos::Representacao;

namespace {

const std::string DESCONEXO = "8\n1 2\n2 3\n5 6\n";

std::vector<std::vector<int32_t>> listas(const grafos::Componentes& c) {
    std::vector<std::vector<int32_t>> r;
    for (const grafos::Componente& comp : c.lista) {
        CHECK(comp.tamanho == static_cast<int32_t>(comp.vertices.size()));
        r.push_back(comp.vertices);
    }
    return r;
}

grafos::Estatisticas estatisticas_referencia(const teste::GrafoTeste& caso) {
    const auto adj = caso.adjacencia();
    std::vector<int32_t> graus;
    int64_t soma = 0;
    for (int32_t v = 1; v <= caso.n; ++v) {
        graus.push_back(static_cast<int32_t>(adj[static_cast<size_t>(v)].size()));
        soma += graus.back();
    }
    std::sort(graus.begin(), graus.end());
    const size_t n = graus.size();
    double mediana = n % 2 == 1 ? graus[n / 2] : (graus[n / 2 - 1] + graus[n / 2]) / 2.0;
    return {caso.n, soma / 2, graus.front(), graus.back(), static_cast<double>(soma) / caso.n, mediana};
}

std::vector<std::vector<int32_t>> componentes_referencia(const teste::GrafoTeste& caso) {
    const auto adj = caso.adjacencia();
    std::vector<bool> visto(static_cast<size_t>(caso.n) + 1, false);
    std::vector<std::vector<int32_t>> r;
    for (int32_t s = 1; s <= caso.n; ++s) {
        if (visto[static_cast<size_t>(s)]) continue;
        std::vector<int32_t> comp;
        std::vector<int32_t> pilha = {s};
        visto[static_cast<size_t>(s)] = true;
        while (!pilha.empty()) {
            int32_t v = pilha.back();
            pilha.pop_back();
            comp.push_back(v);
            for (int32_t w : adj[static_cast<size_t>(v)]) {
                if (!visto[static_cast<size_t>(w)]) {
                    visto[static_cast<size_t>(w)] = true;
                    pilha.push_back(w);
                }
            }
        }
        std::sort(comp.begin(), comp.end());
        r.push_back(comp);
    }
    std::stable_sort(r.begin(), r.end(), [](const auto& a, const auto& b) { return a.size() > b.size(); });
    return r;
}

bool estatisticas_iguais(const grafos::Estatisticas& a, const grafos::Estatisticas& b) {
    return a.n == b.n && a.m == b.m && a.grau_minimo == b.grau_minimo && a.grau_maximo == b.grau_maximo &&
           a.grau_medio == b.grau_medio && a.mediana_grau == b.mediana_grau;
}

}  // namespace

TEST_CASE("9.1: arquivo de estatisticas da Figura 1") {
    const std::string esperado = "# Estatisticas do grafo\n"
                                 "vertices: 5\n"
                                 "arestas: 5\n"
                                 "grau_minimo: 1\n"
                                 "grau_maximo: 4\n"
                                 "grau_medio: 2.0000\n"
                                 "mediana_grau: 2.0\n"
                                 "\n"
                                 "# Componentes conexas\n"
                                 "num_componentes: 1\n"
                                 "\n"
                                 "componente 1 (tamanho 5):\n"
                                 "1 2 3 4 5\n";
    teste::DiretorioTemporario dir;
    for (Representacao rep : teste::REPRESENTACOES) {
        CAPTURE(teste::nome(rep));
        grafos::Grafo g = grafos::carregar(teste::caminho_dados("figura1.txt"), rep);
        const std::string arquivo = dir.arquivo(std::string("estatisticas_") + teste::nome(rep) + ".txt");
        grafos::escrever_estatisticas(grafos::estatisticas(g), grafos::componentes_conexas(g), arquivo);
        CHECK(teste::ler_arquivo(arquivo) == esperado);
    }
}

TEST_CASE("11.5: estatisticas e componentes do grafo desconexo") {
    const std::string esperado = "# Estatisticas do grafo\n"
                                 "vertices: 8\n"
                                 "arestas: 3\n"
                                 "grau_minimo: 0\n"
                                 "grau_maximo: 2\n"
                                 "grau_medio: 0.7500\n"
                                 "mediana_grau: 1.0\n"
                                 "\n"
                                 "# Componentes conexas\n"
                                 "num_componentes: 5\n"
                                 "\n"
                                 "componente 1 (tamanho 3):\n"
                                 "1 2 3\n"
                                 "\n"
                                 "componente 2 (tamanho 2):\n"
                                 "5 6\n"
                                 "\n"
                                 "componente 3 (tamanho 1):\n"
                                 "4\n"
                                 "\n"
                                 "componente 4 (tamanho 1):\n"
                                 "7\n"
                                 "\n"
                                 "componente 5 (tamanho 1):\n"
                                 "8\n";
    teste::DiretorioTemporario dir;
    for (Representacao rep : teste::REPRESENTACOES) {
        CAPTURE(teste::nome(rep));
        grafos::Grafo g = teste::grafo_de_texto(DESCONEXO, rep);
        const grafos::Estatisticas e = grafos::estatisticas(g);
        CHECK(e.n == 8);
        CHECK(e.m == 3);
        CHECK(e.grau_minimo == 0);
        CHECK(e.grau_maximo == 2);
        CHECK(e.grau_medio == 0.75);
        CHECK(e.mediana_grau == 1.0);
        const grafos::Componentes c = grafos::componentes_conexas(g);
        CHECK(listas(c) == std::vector<std::vector<int32_t>>{{1, 2, 3}, {5, 6}, {4}, {7}, {8}});
        const std::string arquivo = dir.arquivo(std::string("desconexo_") + teste::nome(rep) + ".txt");
        grafos::escrever_estatisticas(e, c, arquivo);
        CHECK(teste::ler_arquivo(arquivo) == esperado);
    }
}

TEST_CASE("estatisticas: mediana com n par e centrais diferentes, arredondamento do grau medio") {
    for (Representacao rep : teste::REPRESENTACOES) {
        CAPTURE(teste::nome(rep));
        // graus 2, 1, 2, 1 -> ordenados 1 1 2 2 -> mediana 1.5
        grafos::Grafo par = teste::grafo_de_texto("4\n1 2\n3 4\n1 3\n", rep);
        grafos::Estatisticas e = grafos::estatisticas(par);
        CHECK(e.mediana_grau == 1.5);
        CHECK(e.grau_medio == 1.5);

        // n = 3, m = 1: grau medio 2/3 -> 0.6667; graus 1 1 0 -> mediana 1
        grafos::Grafo terco = teste::grafo_de_texto("3\n1 2\n", rep);
        grafos::Estatisticas t = grafos::estatisticas(terco);
        CHECK(t.mediana_grau == 1.0);
        CHECK(t.grau_minimo == 0);

        teste::DiretorioTemporario dir;
        const std::string arquivo = dir.arquivo("terco.txt");
        grafos::escrever_estatisticas(t, grafos::componentes_conexas(terco), arquivo);
        const std::string conteudo = teste::ler_arquivo(arquivo);
        CHECK(conteudo.find("grau_medio: 0.6667\nmediana_grau: 1.0\n") != std::string::npos);
        CHECK(conteudo.find("num_componentes: 2\n\ncomponente 1 (tamanho 2):\n1 2\n\ncomponente 2 (tamanho 1):\n3\n") !=
              std::string::npos);

        const std::string arquivo_par = dir.arquivo("par.txt");
        grafos::escrever_estatisticas(e, grafos::componentes_conexas(par), arquivo_par);
        CHECK(teste::ler_arquivo(arquivo_par).find("grau_medio: 1.5000\nmediana_grau: 1.5\n") != std::string::npos);
    }
}

TEST_CASE("estatisticas e componentes: grafo de um vertice") {
    for (Representacao rep : teste::REPRESENTACOES) {
        grafos::Grafo g = teste::grafo_de_texto("1\n", rep);
        grafos::Estatisticas e = grafos::estatisticas(g);
        CHECK(e.grau_minimo == 0);
        CHECK(e.grau_maximo == 0);
        CHECK(e.grau_medio == 0.0);
        CHECK(e.mediana_grau == 0.0);
        CHECK(listas(grafos::componentes_conexas(g)) == std::vector<std::vector<int32_t>>{{1}});
    }
}

TEST_CASE("componentes: muitos vertices isolados (guarda contra custo O(n) por componente)") {
    // n = 200.000 so na lista: a matriz ocuparia ~5 GB.
    std::string texto = "200000\n1 2\n2 3\n199999 200000\n100000 3\n";
    grafos::Grafo g = teste::grafo_de_texto(texto, Representacao::Lista);
    grafos::Componentes c = grafos::componentes_conexas(g);
    // {1,2,3,100000} e {199999,200000} mais 199.994 isolados
    REQUIRE(c.lista.size() == 199996);
    CHECK(c.lista[0].vertices == std::vector<int32_t>{1, 2, 3, 100000});
    CHECK(c.lista[1].vertices == std::vector<int32_t>{199999, 200000});
    CHECK(c.lista[2].vertices == std::vector<int32_t>{4});
    CHECK(c.lista.back().vertices == std::vector<int32_t>{199998});
    grafos::Estatisticas e = grafos::estatisticas(g);
    CHECK(e.mediana_grau == 0.0);
    CHECK(e.grau_maximo == 2);
}

TEST_CASE("11.7: estatisticas e componentes identicas nas duas representacoes e iguais as referencias") {
    for (const teste::GrafoTeste& caso : teste::grafos_de_teste()) {
        CAPTURE(caso.nome);
        const std::string texto = caso.texto();
        grafos::Grafo lista = teste::grafo_de_texto(texto, Representacao::Lista);
        grafos::Grafo matriz = teste::grafo_de_texto(texto, Representacao::Matriz);
        const grafos::Estatisticas ref = estatisticas_referencia(caso);
        CHECK(estatisticas_iguais(grafos::estatisticas(lista), ref));
        CHECK(estatisticas_iguais(grafos::estatisticas(matriz), ref));
        const auto comp_ref = componentes_referencia(caso);
        CHECK(listas(grafos::componentes_conexas(lista)) == comp_ref);
        CHECK(listas(grafos::componentes_conexas(matriz)) == comp_ref);
    }
}
