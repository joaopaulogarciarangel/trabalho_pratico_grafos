#include "grafos/floyd_warshall.hpp"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <limits>
#include <thread>
#include "grafos/erro.hpp"

namespace grafos {

uint64_t estimar_bytes_floyd_warshall(int32_t n) {
    if (n < 0) throw ErroGrafo("n negativo");
    return static_cast<uint64_t>(n) * static_cast<uint64_t>(n) * (sizeof(double) + sizeof(int32_t));
}

namespace {
// Diagonal negativa: k esta em ciclo negativo; todo par i->k->j fica indefinido (-inf).
void marcar_ciclos_negativos(ResultadoFloydWarshall& r) {
    const size_t n = static_cast<size_t>(r.n);
    const double INF = std::numeric_limits<double>::infinity();
    // Para assim que toda a matriz estiver indefinida (caso comum: ciclo negativo numa
    // componente fortemente conexa). Celulas que ja estouraram para -inf no laco principal
    // contam desde o inicio; sem isso a contagem nunca fecha e este laco custa n^3.
    for (size_t k = 0; k < n; ++k) r.ciclo_negativo = r.ciclo_negativo || r.dist[k * n + k] < 0;
    if (!r.ciclo_negativo) return;
    size_t indefinidos = static_cast<size_t>(std::count(r.dist.begin(), r.dist.end(), -INF));
    for (size_t k = 0; k < n && indefinidos < n * n; ++k) {
        if (!(r.dist[k * n + k] < 0)) continue;
        for (size_t i = 0; i < n; ++i) {
            if (r.dist[i * n + k] == INF) continue;
            for (size_t j = 0; j < n; ++j) {
                if (r.dist[k * n + j] == INF) continue;
                const size_t ij = i * n + j;
                if (r.dist[ij] != -INF) {
                    r.dist[ij] = -INF;
                    ++indefinidos;
                }
                if (!r.proximo.empty()) r.proximo[ij] = -1;
            }
        }
    }
}

// Relaxa d[i][j] com k em [k0,k1), i em [i0,i1), j em [j0,j1): o miolo do Floyd-Warshall.
void relaxar_bloco(double* d, size_t n, size_t i0, size_t i1, size_t j0, size_t j1, size_t k0, size_t k1) {
    const double INF = std::numeric_limits<double>::infinity();
    for (size_t k = k0; k < k1; ++k) {
        const double* linha_k = d + k * n;
        for (size_t i = i0; i < i1; ++i) {
            const double dik = d[i * n + k];
            if (dik == INF) continue;
            double* linha_i = d + i * n;
            for (size_t j = j0; j < j1; ++j) {
                const double nova = dik + linha_k[j];
                linha_i[j] = nova < linha_i[j] ? nova : linha_i[j];
            }
        }
    }
}
} // namespace

uint64_t estimar_bytes_floyd_warshall_distancias(int32_t n) {
    if (n < 0) throw ErroGrafo("n negativo");
    return static_cast<uint64_t>(n) * static_cast<uint64_t>(n) * sizeof(double);
}

ResultadoFloydWarshall floyd_warshall_distancias(const Grafo& g, int threads) {
    ResultadoFloydWarshall r;
    r.n = g.n();
    const size_t n = static_cast<size_t>(r.n);
    const double INF = std::numeric_limits<double>::infinity();
    if (n != 0 && n > r.dist.max_size() / n) throw ErroGrafo("matriz Floyd-Warshall excede limite de memoria");
    r.dist.assign(n * n, INF);
    for (size_t u = 0; u < n; ++u) r.dist[u * n + u] = 0;
    g.visitar([&](const auto& concreto) {
        for (int32_t u = 0; u < r.n; ++u) {
            concreto.para_cada_vizinho_com_peso(u, [&](int32_t v, double w) {
                const size_t ij = static_cast<size_t>(u) * n + static_cast<size_t>(v);
                if (w < r.dist[ij]) r.dist[ij] = w;
            });
        }
    });
    if (threads <= 0) threads = static_cast<int>(std::max(1u, std::thread::hardware_concurrency()));
    const size_t B = 128;
    const size_t nb = (n + B - 1) / B;
    double* d = r.dist.data();
    auto lim = [&](size_t b) { return std::min(n, (b + 1) * B); };
    // Executa tarefa(t) para t em [0, total) distribuindo entre as threads.
    auto em_paralelo = [&](size_t total, const auto& tarefa) {
        std::atomic<size_t> proxima{0};
        auto trabalhador = [&] {
            for (size_t t; (t = proxima.fetch_add(1)) < total;) tarefa(t);
        };
        const int usar = static_cast<int>(std::min<size_t>(static_cast<size_t>(threads), total));
        std::vector<std::thread> pool;
        for (int i = 1; i < usar; ++i) pool.emplace_back(trabalhador);
        trabalhador();
        for (auto& t : pool) t.join();
    };
    for (size_t kb = 0; kb < nb; ++kb) {
        const size_t k0 = kb * B, k1 = lim(kb);
        // Fase 1: bloco diagonal (kb,kb).
        relaxar_bloco(d, n, k0, k1, k0, k1, k0, k1);
        // Fase 2: linha kb e coluna kb (dependem so do bloco diagonal).
        em_paralelo(2 * nb, [&](size_t t) {
            const size_t b = t / 2;
            if (b == kb) return;
            if (t % 2 == 0) relaxar_bloco(d, n, k0, k1, b * B, lim(b), k0, k1);
            else relaxar_bloco(d, n, b * B, lim(b), k0, k1, k0, k1);
        });
        // Fase 3: demais blocos, independentes entre si nesta rodada.
        em_paralelo(nb, [&](size_t ib) {
            if (ib == kb) return;
            for (size_t jb = 0; jb < nb; ++jb) {
                if (jb == kb) continue;
                relaxar_bloco(d, n, ib * B, lim(ib), jb * B, lim(jb), k0, k1);
            }
        });
    }
    marcar_ciclos_negativos(r);
    return r;
}

ResultadoFloydWarshall floyd_warshall(const Grafo& g) {
    ResultadoFloydWarshall r;
    r.n = g.n();
    const size_t n = static_cast<size_t>(r.n);
    const double INF = std::numeric_limits<double>::infinity();
    if (n != 0 && n > r.dist.max_size() / n) throw ErroGrafo("matriz Floyd-Warshall excede limite de memoria");
    r.dist.assign(n * n, INF);
    r.proximo.assign(n * n, -1);
    for (size_t u = 0; u < n; ++u) {
        r.dist[u * n + u] = 0;
        r.proximo[u * n + u] = static_cast<int32_t>(u + 1);
    }
    g.visitar([&](const auto& concreto) {
        for (int32_t u = 0; u < r.n; ++u) {
            concreto.para_cada_vizinho_com_peso(u, [&](int32_t v, double w) {
                const size_t ij = static_cast<size_t>(u) * n + static_cast<size_t>(v);
                if (w < r.dist[ij]) {
                    r.dist[ij] = w;
                    r.proximo[ij] = v + 1;
                }
            });
        }
    });
    for (size_t k = 0; k < n; ++k) {
        for (size_t i = 0; i < n; ++i) {
            const double dik = r.dist[i * n + k];
            if (!std::isfinite(dik)) continue;
            const int32_t primeiro = r.proximo[i * n + k];
            for (size_t j = 0; j < n; ++j) {
                const double dkj = r.dist[k * n + j];
                if (!std::isfinite(dkj)) continue;
                const size_t ij = i * n + j;
                const double nova = dik + dkj;
                if (nova < r.dist[ij]) {
                    r.dist[ij] = nova;
                    r.proximo[ij] = primeiro;
                }
            }
        }
    }
    marcar_ciclos_negativos(r);
    return r;
}

double ResultadoFloydWarshall::distancia(int32_t origem, int32_t destino) const {
    if (origem < 1 || destino < 1 || origem > n || destino > n) throw ErroGrafo("vertice invalido");
    return dist[static_cast<size_t>(origem - 1) * static_cast<size_t>(n) + static_cast<size_t>(destino - 1)];
}

std::vector<int32_t> ResultadoFloydWarshall::caminho(int32_t origem, int32_t destino) const {
    if (proximo.empty()) throw ErroGrafo("Floyd-Warshall calculado so com distancias: sem caminhos");
    if (!std::isfinite(distancia(origem, destino))) return {};
    std::vector<int32_t> path{origem};
    for (int32_t k = 0; k < n && origem != destino; ++k) {
        origem = proximo[static_cast<size_t>(origem - 1) * static_cast<size_t>(n) + static_cast<size_t>(destino - 1)];
        if (origem < 1 || origem > n) throw ErroGrafo("proximos inconsistentes no Floyd-Warshall");
        path.push_back(origem);
    }
    if (origem != destino) throw ErroGrafo("ciclo na reconstrucao Floyd-Warshall");
    return path;
}

std::vector<int32_t> ResultadoFloydWarshall::arvore_origem(int32_t origem) const {
    if (origem < 1 || origem > n) throw ErroGrafo("origem invalida");
    std::vector<int32_t> pai(static_cast<size_t>(n), -1);
    if (distancia(origem, origem) == 0) pai[static_cast<size_t>(origem - 1)] = 0;
    for (int32_t destino = 1; destino <= n; ++destino) {
        if (destino == origem) continue;
        const auto path = caminho(origem, destino);
        if (path.size() >= 2) pai[static_cast<size_t>(destino - 1)] = path[path.size() - 2];
    }
    return pai;
}
} // namespace grafos
