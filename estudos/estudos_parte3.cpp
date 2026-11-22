// Estudos da Parte 3 (enunciado, itens 1-4) sobre um grafo dirigido com pesos.
//
//   estudos_parte3 <grafo.txt> [--repeticoes 10] [--fw-repeticoes 10] [--bf-todos-repeticoes 0]
//                  [--bf-todos-amostra 0] [--threads 0]
//
// Saida: CSV chave,valor em stdout. Carga do grafo e preparacao do Bellman-Ford (montagem
// dos arcos de entrada e deteccao global de ciclo negativo) ficam fora do cronometro das
// consultas; a preparacao e medida e registrada a parte (bellman_preparacao_s).
//  1. Bellman-Ford ate 100: distancias de 10, 20, 30 e tempo medio de --repeticoes chamadas.
//  2. Pares de {10,...,50}: Floyd-Warshall (so distancias, 8n^2 bytes) quando cabe em 85% da
//     memoria disponivel; conferido com 5 chamadas de Bellman-Ford (bf_u_v).
//  3. Tempo medio do Floyd-Warshall em --fw-repeticoes rodadas.
//  4. Bellman-Ford para todos os pares (n chamadas): medido por --bf-todos-repeticoes rodadas
//     completas, ou estimado por n x media de --bf-todos-amostra destinos sorteados.
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

#include "grafos/grafos.hpp"
#include "grafos/sistema.hpp"

namespace {
using Relogio = std::chrono::steady_clock;

double segundos(Relogio::time_point inicio) {
    return std::chrono::duration<double>(Relogio::now() - inicio).count();
}

void linha(const std::string& chave, double valor) { std::cout << chave << ',' << valor << std::endl; }
void linha(const std::string& chave, const std::string& valor) { std::cout << chave << ',' << valor << std::endl; }

std::string numero(double x) {
    if (std::isinf(x)) return x < 0 ? "-inf" : "inf";
    return grafos::formatar_curto(x);
}

bool iguais(double a, double b) {  // Floyd e Bellman-Ford somam em ordens diferentes
    if (std::isinf(a) || std::isinf(b)) return a == b;
    return std::fabs(a - b) <= 1e-9 * (1 + std::fabs(a));
}

struct Opcoes {
    std::string arquivo;
    int repeticoes = 10;
    int fw_repeticoes = 10;
    int bf_todos_repeticoes = 0;
    int bf_todos_amostra = 0;
    int threads = 0;
};

Opcoes ler_opcoes(int argc, char** argv) {
    Opcoes o;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        auto inteiro = [&](int& destino) {
            if (i + 1 >= argc) throw std::invalid_argument("falta valor para " + a);
            destino = std::stoi(argv[++i]);
            if (destino < 0) throw std::invalid_argument("valor negativo para " + a);
        };
        if (a == "--repeticoes") inteiro(o.repeticoes);
        else if (a == "--fw-repeticoes") inteiro(o.fw_repeticoes);
        else if (a == "--bf-todos-repeticoes") inteiro(o.bf_todos_repeticoes);
        else if (a == "--bf-todos-amostra") inteiro(o.bf_todos_amostra);
        else if (a == "--threads") inteiro(o.threads);
        else if (!a.empty() && a[0] != '-' && o.arquivo.empty()) o.arquivo = a;
        else throw std::invalid_argument("argumento desconhecido: " + a);
    }
    if (o.arquivo.empty() || o.repeticoes == 0) throw std::invalid_argument("uso");
    return o;
}
}  // namespace

int main(int argc, char** argv) {
    Opcoes o;
    try {
        o = ler_opcoes(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "erro: " << e.what() << "\nuso: estudos_parte3 <grafo.txt> [--repeticoes 10] [--fw-repeticoes 10]"
                     " [--bf-todos-repeticoes 0] [--bf-todos-amostra 0] [--threads 0]\n";
        return 2;
    }
    try {
        std::cout << std::setprecision(12);
        const int conjunto[] = {10, 20, 30, 40, 50};

        auto inicio = Relogio::now();
        const auto g = grafos::carregar(o.arquivo, grafos::Representacao::Lista, true);
        const double carga = segundos(inicio);
        const int32_t n = g.n();
        linha("arquivo", std::filesystem::path(o.arquivo).filename().string());
        linha("n", static_cast<double>(n));
        linha("m", static_cast<double>(g.m()));
        linha("peso_negativo", g.tem_peso_negativo() ? "sim" : "nao");
        linha("carga_s", carga);
        linha("cpu", grafos::modelo_cpu());
        linha("compilador", grafos::versao_compilador());
        linha("data", grafos::data_hora_atual());

        inicio = Relogio::now();
        const grafos::BellmanFord bf(g);
        linha("bellman_preparacao_s", segundos(inicio));
        linha("ciclo_negativo", bf.ciclo_negativo() ? "sim" : "nao");

        // Estudo 1
        if (n >= 100) {
            double tempo = 0;
            grafos::ResultadoBellmanFord primeiro;
            for (int k = 0; k < o.repeticoes; ++k) {
                inicio = Relogio::now();
                auto atual = bf.executar(100);
                tempo += segundos(inicio);
                if (k == 0) primeiro = std::move(atual);
                else if (atual.dist != primeiro.dist) throw std::runtime_error("Bellman-Ford inconsistente entre rodadas");
            }
            linha("repeticoes", static_cast<double>(o.repeticoes));
            linha("bellman_destino100_media_s", tempo / o.repeticoes);
            linha("bellman_destino100_rodadas", static_cast<double>(primeiro.rodadas));
            linha("ciclo_afeta_100", primeiro.ciclo_afeta_destino ? "sim" : "nao");
            for (int v : {10, 20, 30}) {
                const std::string par = std::to_string(v) + "_100";
                linha("dist_" + par, numero(primeiro.dist[static_cast<size_t>(v - 1)]));
                const auto c = grafos::caminho(primeiro, v);
                std::string texto;
                for (int32_t x : c) texto += (texto.empty() ? "" : " ") + std::to_string(x);
                if (!c.empty()) linha("caminho_" + par, texto);
            }
        }

        // Estudo 2 (referencia Bellman-Ford): 5 chamadas, uma por destino
        double bf5[5][5] = {};
        if (n >= 50) {
            for (int j = 0; j < 5; ++j) {
                const auto r = bf.executar(conjunto[j]);
                for (int i = 0; i < 5; ++i) {
                    bf5[i][j] = r.dist[static_cast<size_t>(conjunto[i] - 1)];
                    linha("bf_" + std::to_string(conjunto[i]) + "_" + std::to_string(conjunto[j]), numero(bf5[i][j]));
                }
            }
        }

        // Estudos 2 e 3: Floyd-Warshall
        const uint64_t bytes_fw = grafos::estimar_bytes_floyd_warshall_distancias(n);
        const int64_t disponivel = grafos::meminfo_bytes("MemAvailable");
        linha("memoria_disponivel_bytes", static_cast<double>(disponivel));
        linha("floyd_bytes", static_cast<double>(bytes_fw));
        const bool fw_cabe = disponivel > 0 && static_cast<double>(bytes_fw) <= 0.85 * static_cast<double>(disponivel);
        if (o.fw_repeticoes == 0) {
            linha("floyd_status", "nao_pedido");
        } else if (!fw_cabe) {
            linha("floyd_status", "nao_cabe_na_memoria");
        } else {
            double tempo = 0;
            grafos::ResultadoFloydWarshall primeiro;
            for (int k = 0; k < o.fw_repeticoes; ++k) {
                inicio = Relogio::now();
                auto atual = grafos::floyd_warshall_distancias(g, o.threads);
                const double s = segundos(inicio);
                tempo += s;
                linha("floyd_rodada_" + std::to_string(k + 1) + "_s", s);
                if (k == 0) primeiro = std::move(atual);
            }
            linha("floyd_status", "executado");
            linha("floyd_repeticoes", static_cast<double>(o.fw_repeticoes));
            linha("floyd_media_s", tempo / o.fw_repeticoes);
            linha("floyd_ciclo_negativo", primeiro.ciclo_negativo ? "sim" : "nao");
            if (primeiro.ciclo_negativo != bf.ciclo_negativo()) throw std::runtime_error("Floyd e Bellman-Ford divergem no ciclo");
            if (n >= 50) {
                bool confere = true;
                for (int i = 0; i < 5; ++i)
                    for (int j = 0; j < 5; ++j) {
                        const double d = primeiro.distancia(conjunto[i], conjunto[j]);
                        linha("floyd_" + std::to_string(conjunto[i]) + "_" + std::to_string(conjunto[j]), numero(d));
                        confere = confere && iguais(d, bf5[i][j]);
                    }
                linha("conferencia_floyd_bf", confere ? "ok" : "DIVERGENTE");
                if (!confere) throw std::runtime_error("Floyd e Bellman-Ford divergem nos pares");
            }
        }

        // Estudo 4: Bellman-Ford para todos os pares (n consultas)
        if (o.bf_todos_repeticoes > 0) {
            double tempo = 0;
            volatile double checksum = 0;
            for (int k = 0; k < o.bf_todos_repeticoes; ++k) {
                inicio = Relogio::now();
                for (int32_t destino = 1; destino <= n; ++destino) {
                    const auto r = bf.executar(destino);
                    checksum = checksum + (std::isfinite(r.dist[0]) ? r.dist[0] : 0.0);
                }
                const double s = segundos(inicio);
                tempo += s;
                linha("bf_todos_rodada_" + std::to_string(k + 1) + "_s", s);
            }
            linha("bf_todos_status", "executado");
            linha("bf_todos_repeticoes", static_cast<double>(o.bf_todos_repeticoes));
            linha("bf_todos_media_s", tempo / o.bf_todos_repeticoes);
        } else if (o.bf_todos_amostra > 0) {
            const auto destinos = grafos::sortear_vertices(n, o.bf_todos_amostra, 242);
            inicio = Relogio::now();
            for (int32_t destino : destinos) (void)bf.executar(destino);
            const double s = segundos(inicio);
            linha("bf_todos_status", "estimado_por_amostra");
            linha("bf_todos_amostra", static_cast<double>(destinos.size()));
            linha("bf_todos_media_por_destino_s", s / static_cast<double>(destinos.size()));
            linha("bf_todos_estimado_s", s / static_cast<double>(destinos.size()) * n);
        } else {
            linha("bf_todos_status", "nao_pedido");
        }
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "erro: " << e.what() << '\n';
        return 1;
    }
}
