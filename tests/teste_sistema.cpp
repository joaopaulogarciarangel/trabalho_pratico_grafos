#include <algorithm>
#include <cstdint>
#include <fstream>
#include <limits>
#include <random>
#include <set>
#include <string>
#include <vector>

#include "auxiliares.hpp"
#include "doctest.h"
#include "grafos/sistema.hpp"

namespace {

// Reimplementacao direta do sorteio com rejeicao, para conferir a da biblioteca.
std::vector<int32_t> sorteio_referencia(int32_t n, int32_t k, uint64_t semente) {
    std::mt19937_64 gerador(semente);
    const uint64_t nn = static_cast<uint64_t>(n);
    const uint64_t descarte = (std::numeric_limits<uint64_t>::max() % nn + 1) % nn;  // 2^64 mod n
    std::set<int32_t> vistos;
    std::vector<int32_t> r;
    while (static_cast<int32_t>(r.size()) < std::min(k, n)) {
        uint64_t x = gerador();
        if (x < descarte) continue;
        int32_t v = static_cast<int32_t>(x % nn) + 1;
        if (vistos.insert(v).second) r.push_back(v);
    }
    return r;
}

void gravar(const std::string& caminho, const std::string& conteudo) {
    std::ofstream f(caminho, std::ios::binary);
    f << conteudo;
}

}  // namespace

TEST_CASE("sortear_vertices: distintos, no intervalo, reprodutiveis e iguais a referencia") {
    for (int32_t n : {1, 2, 7, 50, 99, 100, 101, 1000, 1024, 65536, 1000003}) {
        CAPTURE(n);
        const std::vector<int32_t> s = grafos::sortear_vertices(n, 100, 42);
        CHECK(s.size() == static_cast<size_t>(std::min(n, 100)));
        std::set<int32_t> distintos(s.begin(), s.end());
        CHECK(distintos.size() == s.size());
        CHECK(*distintos.begin() >= 1);
        CHECK(*distintos.rbegin() <= n);
        CHECK(s == grafos::sortear_vertices(n, 100, 42));
        CHECK(s == sorteio_referencia(n, 100, 42));
    }
    CHECK(grafos::sortear_vertices(1000, 100, 42) != grafos::sortear_vertices(1000, 100, 43));
}

TEST_CASE("sortear_vertices: valores fixos para a semente 42 (n comum e potencia de 2)") {
    const std::vector<int32_t> n1000 = grafos::sortear_vertices(1000, 100, 42);
    const std::vector<int32_t> n1024 = grafos::sortear_vertices(1024, 100, 42);
    CHECK(std::vector<int32_t>(n1000.begin(), n1000.begin() + 5) == std::vector<int32_t>{407, 825, 451, 663, 382});
    CHECK(std::vector<int32_t>(n1024.begin(), n1024.begin() + 5) == std::vector<int32_t>{727, 681, 779, 847, 342});
}

TEST_CASE("ler_num_vertices: so a primeira linha nao vazia") {
    teste::DiretorioTemporario dir;
    CHECK(grafos::ler_num_vertices(teste::caminho_dados("figura1.txt")) == 5);
    CHECK(grafos::ler_num_vertices(teste::caminho_dados("formato_variado.txt")) == 5);

    const std::string extras = dir.arquivo("extras.txt");
    gravar(extras, "\n \t\r\n  7 3 x\n1 2\n");
    CHECK(grafos::ler_num_vertices(extras) == 7);

    const std::string sem_quebra = dir.arquivo("sem_quebra.txt");
    gravar(sem_quebra, "12");
    CHECK(grafos::ler_num_vertices(sem_quebra) == 12);

    // primeira linha nao vazia depois de um bloco de leitura (4096 bytes)
    const std::string longe = dir.arquivo("longe.txt");
    gravar(longe, std::string(5000, '\n') + "33\n1 2\n");
    CHECK(grafos::ler_num_vertices(longe) == 33);

    const std::string vazio = dir.arquivo("vazio.txt");
    gravar(vazio, "");
    CHECK_THROWS_WITH_AS(grafos::ler_num_vertices(vazio), "linha 1: n ausente", grafos::ErroGrafo);

    const std::string invalido = dir.arquivo("invalido.txt");
    gravar(invalido, "\nabc\n1 2\n");
    CHECK_THROWS_WITH_AS(grafos::ler_num_vertices(invalido), "linha 2: texto invalido 'abc' (esperado n)",
                         grafos::ErroGrafo);
    CHECK_THROWS_AS(grafos::ler_num_vertices(dir.arquivo("nao_existe.txt")), grafos::ErroGrafo);
}

TEST_CASE("viabilidade da matriz e da lista e leituras de /proc") {
    const grafos::Viabilidade pequena = grafos::checar_viabilidade_matriz(1000);
    CHECK(pequena.viavel);
    CHECK(pequena.bytes_estimados == grafos::estimar_bytes_matriz(1000));
    CHECK(pequena.bytes_disponiveis > 0);
    CHECK(pequena.bytes_limite == static_cast<int64_t>(0.8 * static_cast<double>(pequena.bytes_disponiveis)));

    const grafos::Viabilidade enorme = grafos::checar_viabilidade_matriz(std::numeric_limits<int32_t>::max());
    CHECK_FALSE(enorme.viavel);

    const grafos::Viabilidade lista_pequena = grafos::checar_viabilidade_lista(1000000);
    CHECK(lista_pequena.viavel);
    CHECK(lista_pequena.bytes_estimados == size_t{1000000} * (sizeof(std::vector<int32_t>) + 4));
    CHECK(grafos::estimar_bytes_lista_minimo(10) == 10 * (sizeof(std::vector<int32_t>) + 4));
    CHECK_FALSE(grafos::checar_viabilidade_lista(2000000000).viavel);  // ~52 GB

    CHECK(grafos::memoria_residente_bytes() > 0);
    CHECK(grafos::meminfo_bytes("MemTotal") > 0);
    CHECK(grafos::meminfo_bytes("CampoInexistente") == -1);
    CHECK_FALSE(grafos::modelo_cpu().empty());
    CHECK(grafos::versao_compilador().find("g++") == 0);
    CHECK(grafos::data_hora_atual().size() == 19);
}

TEST_CASE("formatacao numerica independente de locale") {
    CHECK(grafos::formatar_cientifico(0.00123456789) == "1.234567890e-03");
    CHECK(grafos::formatar_cientifico(0.0) == "0.000000000e+00");
    CHECK(grafos::formatar_fixo(2.0 / 3.0, 4) == "0.6667");
    CHECK(grafos::formatar_fixo(1.25, 1) == "1.2");  // arredondamento para o par mais proximo (valor exato 1.25)
}
