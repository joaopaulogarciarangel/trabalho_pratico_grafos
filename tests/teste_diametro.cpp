#include <algorithm>
#include <cstdint>
#include <random>
#include <string>
#include <type_traits>
#include <vector>

#include "auxiliares.hpp"
#include "doctest.h"

using grafos::Diametro;
using grafos::Representacao;

namespace {

// Invariantes comuns a um resultado de diametro, dado o valor verdadeiro.
bool coerente(const grafos::Grafo& g, const Diametro& d, int32_t verdadeiro) {
    const bool limites = d.limite_inferior <= verdadeiro && verdadeiro <= d.limite_superior &&
                         d.valor == d.limite_inferior && d.exato == (d.limite_inferior == d.limite_superior);
    const bool par = d.u >= 1 && d.u <= g.n() && d.v >= 1 && d.v <= g.n() &&
                     grafos::distancia(g, d.u, d.v) == d.limite_inferior;
    return limites && par && d.num_bfs >= 0 && d.segundos >= 0.0;
}

void checar_valor(const std::string& texto, int32_t esperado) {
    for (Representacao rep : teste::REPRESENTACOES) {
        CAPTURE(teste::nome(rep));
        grafos::Grafo g = teste::grafo_de_texto(texto, rep);
        const Diametro ing = grafos::diametro_ingenuo(g);
        const Diametro apr = grafos::diametro_aproximado(g);
        const Diametro exa = grafos::diametro_exato(g);
        CHECK(ing.valor == esperado);
        CHECK(ing.exato);
        CHECK(ing.num_bfs == g.n());
        CHECK(exa.valor == esperado);
        CHECK(exa.exato);
        CHECK_FALSE(exa.tempo_esgotado);
        CHECK(coerente(g, ing, esperado));
        CHECK(coerente(g, apr, esperado));
        CHECK(coerente(g, exa, esperado));
    }
}

std::string texto_de(int32_t n, const std::vector<std::pair<int32_t, int32_t>>& arestas) {
    return teste::GrafoTeste{"", n, arestas}.texto();
}

// Grafos em que o 4-sweep tende a nao fechar os limites, para exercitar o laco do iFUB.
std::vector<teste::GrafoTeste> grafos_dificeis() {
    std::mt19937_64 rng(4242);
    auto sortear = [&](int32_t limite) { return static_cast<int32_t>(rng() % static_cast<uint64_t>(limite)) + 1; };
    std::vector<teste::GrafoTeste> r;

    for (int k = 0; k < 3; ++k) {
        teste::GrafoTeste t{"arvore_aleatoria_" + std::to_string(k), 2000, {}};
        for (int32_t i = 2; i <= t.n; ++i) t.arestas.push_back({i, sortear(i - 1)});
        r.push_back(t);
    }
    for (int k = 0; k < 3; ++k) {
        // pai entre os ultimos 8 vertices: arvores profundas e ramificadas
        teste::GrafoTeste t{"arvore_profunda_" + std::to_string(k), 2000, {}};
        for (int32_t i = 2; i <= t.n; ++i) t.arestas.push_back({i, std::max(1, i - sortear(8))});
        r.push_back(t);
    }

    teste::GrafoTeste lagarta{"lagarta", 0, {}};
    const int32_t espinha = 400;
    int32_t proximo = espinha + 1;
    for (int32_t i = 1; i < espinha; ++i) lagarta.arestas.push_back({i, i + 1});
    for (int32_t i = 1; i <= espinha; ++i) {
        for (int32_t f = sortear(3) - 1; f > 0; --f) lagarta.arestas.push_back({i, proximo++});
    }
    lagarta.n = proximo - 1;
    r.push_back(lagarta);

    for (int k = 0; k < 3; ++k) {
        teste::GrafoTeste t{"ciclo_com_caudas_" + std::to_string(k), 0, {}};
        const int32_t ciclo = 300;
        int32_t prox = ciclo + 1;
        for (int32_t i = 1; i <= ciclo; ++i) t.arestas.push_back({i, i % ciclo + 1});
        for (int c = 0; c < 6; ++c) {
            int32_t anterior = sortear(ciclo);
            for (int32_t tam = sortear(120); tam > 0; --tam) {
                t.arestas.push_back({anterior, prox});
                anterior = prox++;
            }
        }
        t.n = prox - 1;
        r.push_back(t);
    }

    for (int k = 0; k < 3; ++k) {
        teste::GrafoTeste t{"quase_arvore_" + std::to_string(k), 1500, {}};
        for (int32_t i = 2; i <= t.n; ++i) t.arestas.push_back({i, std::max(1, i - sortear(20))});
        for (int e = 0; e < 25; ++e) t.arestas.push_back({sortear(t.n), sortear(t.n)});
        r.push_back(t);
    }

    teste::GrafoTeste varias{"varias_componentes", 0, {}};
    // clique de 60 (diametro 1), caminho de 40 (39), arvore profunda de 500 e isolados
    for (int32_t i = 1; i <= 60; ++i)
        for (int32_t j = i + 1; j <= 60; ++j) varias.arestas.push_back({i, j});
    for (int32_t i = 61; i < 100; ++i) varias.arestas.push_back({i, i + 1});
    for (int32_t i = 102; i <= 600; ++i) varias.arestas.push_back({i, std::max(101, i - sortear(6))});
    varias.n = 620;
    r.push_back(varias);
    return r;
}

}  // namespace

TEST_CASE("11.1 a 11.4: diametro dos exemplos nos tres algoritmos") {
    checar_valor("5\n1 2\n2 5\n5 3\n4 5\n1 5\n", 2);
    checar_valor("7\n1 2\n1 3\n1 4\n2 4\n2 6\n3 4\n4 7\n5 6\n5 7\n6 7\n", 3);
    checar_valor("7\n1 2\n1 3\n1 4\n2 4\n2 7\n3 4\n4 6\n5 6\n5 7\n6 7\n", 3);
    checar_valor("8\n1 3\n1 4\n2 3\n2 4\n3 4\n3 8\n4 6\n4 7\n5 6\n5 7\n6 7\n", 4);
}

TEST_CASE("11.5: diametro do grafo desconexo e par (1,3)") {
    checar_valor("8\n1 2\n2 3\n5 6\n", 2);
    for (Representacao rep : teste::REPRESENTACOES) {
        CAPTURE(teste::nome(rep));
        grafos::Grafo g = teste::grafo_de_texto("8\n1 2\n2 3\n5 6\n", rep);
        for (const Diametro& d : {grafos::diametro_ingenuo(g), grafos::diametro_aproximado(g), grafos::diametro_exato(g)}) {
            const bool par_certo = (d.u == 1 && d.v == 3) || (d.u == 3 && d.v == 1);
            CHECK(par_certo);
        }
    }
}

TEST_CASE("diametro: grafo sem arestas") {
    for (Representacao rep : teste::REPRESENTACOES) {
        CAPTURE(teste::nome(rep));
        grafos::Grafo g = teste::grafo_de_texto("5\n3 3\n", rep);
        for (const Diametro& d : {grafos::diametro_aproximado(g), grafos::diametro_exato(g), grafos::diametro_exato(g, 0.0)}) {
            CHECK(d.valor == 0);
            CHECK(d.limite_inferior == 0);
            CHECK(d.limite_superior == 0);
            CHECK(d.u == 1);
            CHECK(d.v == 1);
            CHECK(d.exato);
            CHECK_FALSE(d.tempo_esgotado);
            CHECK(d.num_bfs == 0);
        }
        const Diametro ing = grafos::diametro_ingenuo(g);
        CHECK(ing.valor == 0);
        CHECK(ing.u == 1);
        CHECK(ing.v == 1);
        CHECK(ing.num_bfs == 5);
    }
}

TEST_CASE("diametro: componente maior com diametro menor nao esconde a menor; componentes pequenas sao puladas") {
    std::vector<std::pair<int32_t, int32_t>> arestas;
    for (int32_t i = 1; i <= 50; ++i)
        for (int32_t j = i + 1; j <= 50; ++j) arestas.push_back({i, j});
    for (int32_t i = 51; i < 70; ++i) arestas.push_back({i, i + 1});  // caminho com 20 vertices
    for (int32_t i = 71; i < 73; ++i) arestas.push_back({i, i + 1});  // caminho com 3 vertices: pulado
    const std::string texto = texto_de(73, arestas);
    checar_valor(texto, 19);
    for (Representacao rep : teste::REPRESENTACOES) {
        grafos::Grafo g = teste::grafo_de_texto(texto, rep);
        const Diametro apr = grafos::diametro_aproximado(g);
        CHECK(apr.exato);
        CHECK(apr.num_bfs == 10);  // 5 BFS no clique + 5 no caminho de 20; o de 3 vertices e pulado
        const bool par_certo = (apr.u == 51 && apr.v == 70) || (apr.u == 70 && apr.v == 51);
        CHECK(par_certo);
    }
}

TEST_CASE("11.7: exato == ingenuo e aproximado contem o ingenuo (grafos pequenos e dificeis)") {
    std::vector<teste::GrafoTeste> casos = teste::grafos_de_teste();
    std::vector<teste::GrafoTeste> dificeis = grafos_dificeis();
    casos.insert(casos.end(), dificeis.begin(), dificeis.end());

    int64_t maior_num_bfs_exato = 0;
    int casos_com_laco_ifub = 0;
    int casos_aproximado_nao_exato = 0;
    int casos_lb_4sweep_abaixo = 0;
    for (const teste::GrafoTeste& caso : casos) {
        CAPTURE(caso.nome);
        const std::string texto = caso.texto();
        grafos::Grafo lista = teste::grafo_de_texto(texto, Representacao::Lista);
        const Diametro ing = grafos::diametro_ingenuo(lista);
        REQUIRE(ing.exato);
        CHECK(coerente(lista, ing, ing.valor));

        for (Representacao rep : teste::REPRESENTACOES) {
            CAPTURE(teste::nome(rep));
            grafos::Grafo g = rep == Representacao::Lista ? teste::grafo_de_texto(texto, rep)
                                                          : teste::grafo_de_texto(texto, Representacao::Matriz);
            const Diametro apr = grafos::diametro_aproximado(g);
            const Diametro exa = grafos::diametro_exato(g);
            CHECK(coerente(g, apr, ing.valor));
            CHECK(coerente(g, exa, ing.valor));
            CHECK(exa.valor == ing.valor);
            CHECK(exa.exato);
            CHECK_FALSE(exa.tempo_esgotado);
            CHECK_FALSE(apr.tempo_esgotado);

            const Diametro zero = grafos::diametro_exato(g, 0.0);
            CHECK(coerente(g, zero, ing.valor));
            CHECK(zero.tempo_esgotado == (g.m() > 0));
            if (zero.exato) CHECK(zero.limite_inferior == zero.limite_superior);

            if (rep == Representacao::Lista) {
                maior_num_bfs_exato = std::max(maior_num_bfs_exato, exa.num_bfs);
                if (exa.num_bfs > apr.num_bfs) ++casos_com_laco_ifub;
                if (!apr.exato) ++casos_aproximado_nao_exato;
                if (apr.limite_inferior < ing.valor) ++casos_lb_4sweep_abaixo;
            } else {
                // A matriz deve reproduzir exatamente o resultado da lista.
                const Diametro apr_l = grafos::diametro_aproximado(lista);
                const Diametro exa_l = grafos::diametro_exato(lista);
                CHECK((apr.limite_inferior == apr_l.limite_inferior && apr.limite_superior == apr_l.limite_superior &&
                       apr.u == apr_l.u && apr.v == apr_l.v && apr.num_bfs == apr_l.num_bfs));
                CHECK((exa.valor == exa_l.valor && exa.u == exa_l.u && exa.v == exa_l.v && exa.num_bfs == exa_l.num_bfs));
            }
        }
    }
    MESSAGE("grafos: ", casos.size(), "; maior num_bfs no exato: ", maior_num_bfs_exato,
            "; casos em que o laco do iFUB rodou: ", casos_com_laco_ifub,
            "; casos com aproximado nao exato: ", casos_aproximado_nao_exato,
            "; casos com lb do 4-sweep abaixo do diametro: ", casos_lb_4sweep_abaixo);
    CHECK(maior_num_bfs_exato > 5);
    CHECK(casos_com_laco_ifub > 0);
}

TEST_CASE("diametro: limites por componente do 4-sweep e do iFUB em milhares de grafos pequenos") {
    std::mt19937_64 rng(777);
    int64_t componentes = 0;
    int64_t lb_4sweep_abaixo = 0;
    int64_t falhas = 0;
    for (int t = 0; t < 12000; ++t) {
        teste::GrafoTeste caso{"pequeno_" + std::to_string(t), static_cast<int32_t>(4 + rng() % 77), {}};
        for (int32_t i = 2; i <= caso.n; ++i) {
            if (rng() % 10 != 0) caso.arestas.push_back({i, static_cast<int32_t>(1 + rng() % static_cast<uint64_t>(i - 1))});
        }
        for (uint64_t e = rng() % static_cast<uint64_t>(caso.n / 3 + 1); e > 0; --e) {
            caso.arestas.push_back({static_cast<int32_t>(1 + rng() % static_cast<uint64_t>(caso.n)),
                                    static_cast<int32_t>(1 + rng() % static_cast<uint64_t>(caso.n))});
        }
        grafos::Grafo lista = teste::grafo_de_texto(caso.texto(), Representacao::Lista);
        const Diametro ing = grafos::diametro_ingenuo(lista);
        const Diametro apr = grafos::diametro_aproximado(lista);
        const Diametro exa = grafos::diametro_exato(lista);
        bool ok = exa.valor == ing.valor && exa.exato && coerente(lista, apr, ing.valor) &&
                  coerente(lista, exa, ing.valor);

        lista.visitar([&](const auto& g) {
            using Tipo = std::decay_t<decltype(g)>;
            grafos::detalhe::BfsAuxiliar<Tipo> bfs(g);
            const grafos::detalhe::Relogio relogio(0.0, false);
            for (const grafos::Componente& comp : grafos::detalhe::componentes_conexas(g).lista) {
                if (comp.tamanho < 2) continue;
                ++componentes;
                int32_t diametro = 0;
                for (int32_t id : comp.vertices) {
                    diametro = std::max(diametro, bfs.executar(id - 1).excentricidade);
                    bfs.limpar();
                }
                const auto s = grafos::detalhe::quatro_varreduras(g, bfs, comp, relogio);
                auto f = s;
                grafos::detalhe::refinar_ifub(bfs, f, relogio, -1);
                if (s.lb < diametro) ++lb_4sweep_abaixo;
                const bool comp_ok = !s.interrompido && s.lb <= diametro && diametro <= s.ub &&
                                     !f.interrompido && f.lb == diametro && f.ub == diametro &&
                                     grafos::distancia(lista, s.u + 1, s.v + 1) == s.lb &&
                                     grafos::distancia(lista, f.u + 1, f.v + 1) == f.lb;
                ok = ok && comp_ok;
            }
        });
        if (!ok) {
            ++falhas;
            if (falhas <= 3) FAIL_CHECK("diametro incorreto em " << caso.nome << ": " << caso.texto());
        }
    }
    MESSAGE("componentes com >= 2 vertices: ", componentes, "; lb do 4-sweep abaixo do diametro: ", lb_4sweep_abaixo);
    CHECK(falhas == 0);
    CHECK(lb_4sweep_abaixo > 0);
}

TEST_CASE("diametro exato: interrupcao por numero de BFS mantem o intervalo valido e informativo") {
    // Componente 1: arvore aleatoria com 3000 vertices + 1500 arestas aleatorias (o iFUB precisa
    // de centenas de BFS). Componente 2: estrela com 1000 vertices (diametro 2, tamanho - 1 = 999).
    std::mt19937_64 rng(2024);
    teste::GrafoTeste caso{"esparso_e_estrela", 4000, {}};
    auto sortear = [&](int32_t limite) { return static_cast<int32_t>(rng() % static_cast<uint64_t>(limite)) + 1; };
    for (int32_t i = 2; i <= 3000; ++i) caso.arestas.push_back({i, sortear(i - 1)});
    for (int k = 0; k < 1500; ++k) caso.arestas.push_back({sortear(3000), sortear(3000)});
    for (int32_t i = 3002; i <= 4000; ++i) caso.arestas.push_back({3001, i});
    const std::string texto = caso.texto();
    const int32_t verdadeiro = grafos::diametro_ingenuo(teste::grafo_de_texto(texto, Representacao::Lista)).valor;

    for (Representacao rep : teste::REPRESENTACOES) {
        CAPTURE(teste::nome(rep));
        grafos::Grafo grafo = teste::grafo_de_texto(texto, rep);
        grafo.visitar([&](const auto& g) {
            const Diametro completo = grafos::detalhe::diametro_exato(g, 1e9, -1);
            REQUIRE(completo.exato);
            REQUIRE(completo.valor == verdadeiro);
            // Fase A: 5 + 5 BFS; fase B: BFS(m2) da arvore e o laco do iFUB.
            REQUIRE(completo.num_bfs > 100);

            const Diametro meio = grafos::detalhe::diametro_exato(g, 1e9, 15);
            CHECK(meio.tempo_esgotado);
            CHECK(meio.num_bfs == 15);
            CHECK(coerente(grafo, meio, verdadeiro));
            CHECK(meio.limite_superior < 999);  // a estrela ja foi varrida: nao entra com tamanho - 1

            // Varredura de limites so na lista (na matriz cada BFS custa O(n^2/64)).
            int64_t verificados = 0;
            bool todos_validos = true;
            const int64_t passo = std::max<int64_t>(1, completo.num_bfs / 30);
            const int64_t ultimo = rep == Representacao::Lista ? completo.num_bfs : 0;
            for (int64_t limite = 0; limite <= ultimo; limite += passo) {
                const Diametro d = grafos::detalhe::diametro_exato(g, 1e9, limite);
                const bool valido = coerente(grafo, d, verdadeiro) && d.num_bfs <= limite &&
                                    d.tempo_esgotado == (limite < completo.num_bfs);
                if (!valido) {
                    CAPTURE(limite);
                    FAIL_CHECK("intervalo invalido apos interrupcao");
                }
                todos_validos = todos_validos && valido;
                ++verificados;
            }
            CHECK(todos_validos);
            MESSAGE(std::string(teste::nome(rep)), ": num_bfs completo = ", completo.num_bfs, "; limites testados = ", verificados,
                    "; intervalo com 15 BFS = [", meio.limite_inferior, ", ", meio.limite_superior, "]");
        });
    }
}
