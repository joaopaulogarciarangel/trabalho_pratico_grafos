#include "grafos/floyd_warshall.hpp"

#include <cmath>
#include <limits>
#include "grafos/erro.hpp"

namespace grafos {

uint64_t estimar_bytes_floyd_warshall(int32_t n) {
    if (n < 0) throw ErroGrafo("n negativo");
    return static_cast<uint64_t>(n) * static_cast<uint64_t>(n) * (sizeof(double) + sizeof(int32_t));
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
    for (size_t k = 0; k < n; ++k) {
        if (r.dist[k * n + k] >= 0) continue;
        r.ciclo_negativo = true;
        for (size_t i = 0; i < n; ++i) {
            if (r.dist[i * n + k] == INF) continue;
            for (size_t j = 0; j < n; ++j) {
                if (r.dist[k * n + j] == INF) continue;
                r.dist[i * n + j] = -INF;
                r.proximo[i * n + j] = -1;
            }
        }
    }
    return r;
}

double ResultadoFloydWarshall::distancia(int32_t origem, int32_t destino) const {
    if (origem < 1 || destino < 1 || origem > n || destino > n) throw ErroGrafo("vertice invalido");
    return dist[static_cast<size_t>(origem - 1) * static_cast<size_t>(n) + static_cast<size_t>(destino - 1)];
}

std::vector<int32_t> ResultadoFloydWarshall::caminho(int32_t origem, int32_t destino) const {
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
