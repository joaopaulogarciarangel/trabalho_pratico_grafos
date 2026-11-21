#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <iomanip>
#include <limits>
#include <stdexcept>
#include <string>

#include "grafos/grafos.hpp"
#include "grafos/sistema.hpp"

namespace {
double segundos(std::chrono::steady_clock::time_point inicio) {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - inicio).count();
}

void linha(const std::string& chave, double valor) { std::cout << chave << ',' << valor << '\n'; }
void linha(const std::string& chave, const std::string& valor) { std::cout << chave << ',' << valor << '\n'; }

std::string numero(double x) {
    if (std::isinf(x)) return x < 0 ? "-inf" : "inf";
    return grafos::formatar_curto(x);
}
}

int main(int argc, char** argv) {
    try {
        std::cout << std::setprecision(12);
        if (argc < 2 || argc > 4) {
            std::cerr << "uso: estudos_parte3 <grafo.txt> [repeticoes=10] [max_trabalho=1e10]\n";
            return 2;
        }
        const std::string arquivo = argv[1];
        const int repeticoes = argc >= 3 ? std::stoi(argv[2]) : 10;
        const double max_trabalho = argc >= 4 ? std::stod(argv[3]) : 1e10;
        if (repeticoes <= 0 || !(max_trabalho > 0)) throw std::invalid_argument("parametros invalidos");
        const auto cab = grafos::ler_cabecalho(arquivo);
        const int64_t disponivel = grafos::meminfo_bytes("MemAvailable");
        const uint64_t bytes_floyd = grafos::estimar_bytes_floyd_warshall(cab.n);
        const long double cubo = static_cast<long double>(cab.n) * cab.n * cab.n;
        const bool floyd_viavel = (disponivel < 0 || bytes_floyd < static_cast<uint64_t>(0.8 * disponivel)) &&
                                   cubo <= max_trabalho;
        grafos::InfoLeitura info;
        const auto g = grafos::carregar(arquivo, grafos::Representacao::Lista, true, &info);
        linha("arquivo", std::filesystem::path(arquivo).filename().string());
        linha("n", static_cast<double>(g.n()));
        linha("m", static_cast<double>(g.m()));
        linha("peso_negativo", g.tem_peso_negativo() ? "sim" : "nao");
        linha("repeticoes", static_cast<double>(repeticoes));
        linha("memoria_disponivel_bytes", static_cast<double>(disponivel));
        linha("floyd_bytes_estimados", static_cast<double>(bytes_floyd));
        linha("floyd_operacoes_cubicas", static_cast<double>(cubo));

        if (g.n() >= 100) {
            double tempo = 0;
            grafos::ResultadoBellmanFord primeiro;
            for (int k = 0; k < repeticoes; ++k) {
                const auto inicio = std::chrono::steady_clock::now();
                auto atual = grafos::bellman_ford(g, 100);
                tempo += segundos(inicio);
                if (k == 0) primeiro = std::move(atual);
                else for (int v : {10, 20, 30})
                    if (numero(atual.dist[static_cast<size_t>(v - 1)]) !=
                        numero(primeiro.dist[static_cast<size_t>(v - 1)]))
                        throw std::runtime_error("Bellman-Ford inconsistente entre rodadas");
            }
            linha("bellman_destino100_media_s", tempo / repeticoes);
            linha("ciclo_negativo", primeiro.ciclo_negativo ? "sim" : "nao");
            linha("ciclo_afeta_100", primeiro.ciclo_afeta_destino ? "sim" : "nao");
            for (int v : {10, 20, 30})
                linha("dist_" + std::to_string(v) + "_100", numero(primeiro.dist[static_cast<size_t>(v - 1)]));
        }

        if (g.n() >= 50) {
            for (int v : {10, 20, 30, 40, 50}) {
                const auto r = grafos::bellman_ford(g, v);
                for (int u : {10, 20, 30, 40, 50})
                    linha("bf_" + std::to_string(u) + "_" + std::to_string(v),
                          numero(r.dist[static_cast<size_t>(u - 1)]));
            }
        }

        if (floyd_viavel) {
            double tempo = 0;
            grafos::ResultadoFloydWarshall primeiro;
            for (int k = 0; k < repeticoes; ++k) {
                const auto inicio = std::chrono::steady_clock::now();
                auto atual = grafos::floyd_warshall(g);
                tempo += segundos(inicio);
                if (k == 0) primeiro = std::move(atual);
                else if (g.n() >= 50)
                    for (int u : {10, 20, 30, 40, 50})
                        for (int v : {10, 20, 30, 40, 50})
                            if (numero(atual.distancia(u, v)) != numero(primeiro.distancia(u, v)))
                                throw std::runtime_error("Floyd-Warshall inconsistente entre rodadas");
            }
            linha("floyd_media_s", tempo / repeticoes);
            linha("floyd_ciclo_negativo", primeiro.ciclo_negativo ? "sim" : "nao");
            if (g.n() >= 50)
                for (int u : {10, 20, 30, 40, 50})
                    for (int v : {10, 20, 30, 40, 50})
                        linha("floyd_" + std::to_string(u) + "_" + std::to_string(v),
                              numero(primeiro.distancia(u, v)));
        } else {
            linha("floyd_status", "inviavel_por_memoria_ou_tempo");
        }

        const long double trabalho_bf = static_cast<long double>(g.n()) * g.n() * (g.m() + g.n());
        if (trabalho_bf <= max_trabalho) {
            double tempo = 0;
            volatile double checksum = 0;
            for (int k = 0; k < repeticoes; ++k) {
                const auto inicio = std::chrono::steady_clock::now();
                for (int32_t destino = 1; destino <= g.n(); ++destino) {
                    const auto r = grafos::bellman_ford(g, destino);
                    const double d = r.dist[0];
                    if (std::isfinite(d)) checksum += d;
                }
                tempo += segundos(inicio);
            }
            linha("bellman_todos_pares_media_s", tempo / repeticoes);
            linha("bellman_todos_pares_checksum", checksum);
        } else {
            linha("bellman_todos_pares_status", "inviavel_por_tempo");
        }
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "erro: " << e.what() << '\n';
        return 1;
    }
}
