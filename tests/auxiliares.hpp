#pragma once

#include <unistd.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "grafos/grafos.hpp"

namespace teste {

// Diretorio temporario exclusivo do processo de testes, apagado ao fim do objeto.
class DiretorioTemporario {
public:
    DiretorioTemporario()
        : caminho_(std::filesystem::temp_directory_path() / ("grafos_testes_" + std::to_string(getpid()))) {
        std::filesystem::create_directories(caminho_);
    }
    ~DiretorioTemporario() {
        std::error_code ec;
        std::filesystem::remove_all(caminho_, ec);
    }
    std::string arquivo(const std::string& nome) const { return (caminho_ / nome).string(); }

private:
    std::filesystem::path caminho_;
};

inline std::string ler_arquivo(const std::string& caminho) {
    std::ifstream f(caminho, std::ios::binary);
    std::ostringstream s;
    s << f.rdbuf();
    return s.str();
}

inline constexpr grafos::Representacao REPRESENTACOES[] = {grafos::Representacao::Lista,
                                                           grafos::Representacao::Matriz};

inline const char* nome(grafos::Representacao rep) {
    return rep == grafos::Representacao::Lista ? "lista" : "matriz";
}

inline std::string caminho_dados(const std::string& arquivo) {
    return std::string(GRAFOS_DIR_TESTES_DADOS) + "/" + arquivo;
}

inline grafos::Grafo grafo_de_texto(const std::string& texto, grafos::Representacao rep,
                                    grafos::InfoLeitura* info = nullptr) {
    grafos::ArestasLidas arestas = grafos::parsear_arestas(texto.data(), texto.size());
    return grafos::construir_grafo(arestas, rep, info);
}

// Vizinhos de v (1..n), devolvidos em 1..n na ordem da iteracao.
inline std::vector<int32_t> vizinhos(const grafos::Grafo& g, int32_t v, bool decrescente = false) {
    std::vector<int32_t> r;
    g.visitar([&](const auto& concreto) {
        auto guardar = [&](int32_t w) { r.push_back(w + 1); };
        if (decrescente) {
            concreto.para_cada_vizinho_decrescente(v - 1, guardar);
        } else {
            concreto.para_cada_vizinho(v - 1, guardar);
        }
    });
    return r;
}

// Grafo de teste: arestas em 1..n, podendo conter lacos e repeticoes.
struct GrafoTeste {
    std::string nome;
    int32_t n;
    std::vector<std::pair<int32_t, int32_t>> arestas;

    std::string texto() const {
        std::string s = std::to_string(n) + "\n";
        for (const auto& [u, v] : arestas) s += std::to_string(u) + " " + std::to_string(v) + "\n";
        return s;
    }

    // Adjacencia de referencia, 1..n, sem lacos e sem repeticoes, em ordem crescente.
    std::vector<std::vector<int32_t>> adjacencia() const {
        std::vector<std::set<int32_t>> conj(static_cast<size_t>(n) + 1);
        for (const auto& [u, v] : arestas) {
            if (u == v) continue;
            conj[static_cast<size_t>(u)].insert(v);
            conj[static_cast<size_t>(v)].insert(u);
        }
        std::vector<std::vector<int32_t>> r(static_cast<size_t>(n) + 1);
        for (size_t v = 1; v < conj.size(); ++v) r[v].assign(conj[v].begin(), conj[v].end());
        return r;
    }
};

// Raizes variadas para os testes: 1, n, o meio e algumas sorteadas.
inline std::vector<int32_t> raizes_de_teste(int32_t n, uint64_t semente = 7) {
    std::set<int32_t> r = {1, n, (n + 1) / 2};
    std::mt19937_64 rng(semente + static_cast<uint64_t>(n));
    for (int k = 0; k < 3; ++k) r.insert(static_cast<int32_t>(rng() % static_cast<uint64_t>(n)) + 1);
    return {r.begin(), r.end()};
}

// Grafos pequenos variados com semente fixa: vazios, caminho, ciclo, estrela,
// arvore, completo, desconexos e aleatorios de varias densidades (com lacos e repeticoes).
inline std::vector<GrafoTeste> grafos_de_teste(uint64_t semente = 20260917) {
    std::mt19937_64 rng(semente);
    auto sortear = [&](int32_t limite) { return static_cast<int32_t>(rng() % static_cast<uint64_t>(limite)) + 1; };
    std::vector<GrafoTeste> r;

    r.push_back({"unico", 1, {}});
    r.push_back({"sem_arestas", 10, {}});
    r.push_back({"uma_aresta", 2, {{2, 1}}});

    GrafoTeste caminho{"caminho", 50, {}};
    for (int32_t i = 1; i < caminho.n; ++i) caminho.arestas.push_back({i, i + 1});
    r.push_back(caminho);

    GrafoTeste ciclo{"ciclo", 64, {}};
    for (int32_t i = 1; i < ciclo.n; ++i) ciclo.arestas.push_back({i + 1, i});
    ciclo.arestas.push_back({ciclo.n, 1});
    r.push_back(ciclo);

    GrafoTeste estrela{"estrela", 65, {}};
    for (int32_t i = 2; i <= estrela.n; ++i) estrela.arestas.push_back({i, 1});
    r.push_back(estrela);

    GrafoTeste arvore{"arvore", 130, {}};
    for (int32_t i = 2; i <= arvore.n; ++i) arvore.arestas.push_back({i, sortear(i - 1)});
    std::shuffle(arvore.arestas.begin(), arvore.arestas.end(), rng);
    r.push_back(arvore);

    GrafoTeste completo{"completo", 40, {}};
    for (int32_t i = 1; i <= completo.n; ++i)
        for (int32_t j = i + 1; j <= completo.n; ++j) completo.arestas.push_back({j, i});
    std::shuffle(completo.arestas.begin(), completo.arestas.end(), rng);
    r.push_back(completo);

    GrafoTeste desconexo{"desconexo", 200, {}};
    for (int k = 0; k < 300; ++k) desconexo.arestas.push_back({sortear(80), sortear(80)});
    for (int k = 0; k < 60; ++k) desconexo.arestas.push_back({80 + sortear(70), 80 + sortear(70)});
    for (int32_t i = 160; i < 175; ++i) desconexo.arestas.push_back({i, i + 1});
    r.push_back(desconexo);

    const int32_t tamanhos[] = {2, 3, 17, 63, 64, 65, 100, 129, 200};
    const double densidades[] = {0.005, 0.02, 0.06, 0.2, 0.6};
    for (int32_t n : tamanhos) {
        for (double d : densidades) {
            GrafoTeste g{"aleatorio_n" + std::to_string(n) + "_d" + std::to_string(d), n, {}};
            auto m = static_cast<int64_t>(d * n * (n - 1) / 2) + 1;
            for (int64_t k = 0; k < m; ++k) g.arestas.push_back({sortear(n), sortear(n)});
            // algumas repeticoes explicitas, nos dois sentidos
            for (int64_t k = 0; k < m / 10; ++k) {
                auto [u, v] = g.arestas[static_cast<size_t>(k)];
                g.arestas.push_back({v, u});
            }
            r.push_back(g);
        }
    }
    return r;
}

}  // namespace teste
