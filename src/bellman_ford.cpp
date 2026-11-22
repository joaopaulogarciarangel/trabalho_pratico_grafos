#include "grafos/bellman_ford.hpp"

#include <algorithm>
#include <cmath>
#include <deque>
#include <limits>
#include <string>
#include <utility>
#include "grafos/erro.hpp"

namespace grafos {
namespace {
using Arco = std::pair<int32_t, double>;

// Super-origem com custo zero para todos os vertices: detecta ciclos em
// qualquer componente, inclusive as que nao alcancam o destino consultado.
bool ciclo_predecessor_negativo(const std::vector<int32_t>& pred, const std::vector<double>& pred_peso) {
    const int32_t n = static_cast<int32_t>(pred.size());
    std::vector<int32_t> estado(static_cast<size_t>(n), 0);
    for (int32_t inicio = 0; inicio < n; ++inicio) {
        if (estado[static_cast<size_t>(inicio)] != 0) continue;
        const int32_t marca = inicio + 1;
        int32_t v = inicio;
        while (v >= 0 && estado[static_cast<size_t>(v)] == 0) {
            estado[static_cast<size_t>(v)] = marca;
            v = pred[static_cast<size_t>(v)];
        }
        if (v >= 0 && estado[static_cast<size_t>(v)] == marca) {
            double soma = 0;
            const int32_t primeiro = v;
            do {
                soma += pred_peso[static_cast<size_t>(v)];
                v = pred[static_cast<size_t>(v)];
            } while (v != primeiro);
            if (soma < 0) return true;
        }
        v = inicio;
        while (v >= 0 && estado[static_cast<size_t>(v)] == marca) {
            estado[static_cast<size_t>(v)] = -1;
            v = pred[static_cast<size_t>(v)];
        }
    }
    return false;
}

bool ciclo_global(const std::vector<std::vector<Arco>>& saida, bool tem_negativo) {
    if (!tem_negativo) return false;
    const int32_t n = static_cast<int32_t>(saida.size());
    std::vector<double> d(static_cast<size_t>(n), 0.0);
    std::vector<int32_t> pred(static_cast<size_t>(n), -1);
    std::vector<double> pred_peso(static_cast<size_t>(n), 0);
    std::vector<uint8_t> na_fila(static_cast<size_t>(n), 1);
    std::deque<int32_t> fila;
    for (int32_t v = 0; v < n; ++v) fila.push_back(v);
    uint64_t relaxacoes = 0;
    while (!fila.empty()) {
        const int32_t u = fila.front();
        fila.pop_front();
        na_fila[static_cast<size_t>(u)] = 0;
        for (const auto& [v, w] : saida[static_cast<size_t>(u)]) {
            const double nova = d[static_cast<size_t>(u)] + w;
            if (nova < d[static_cast<size_t>(v)]) {
                d[static_cast<size_t>(v)] = nova;
                pred[static_cast<size_t>(v)] = u;
                pred_peso[static_cast<size_t>(v)] = w;
                if (!na_fila[static_cast<size_t>(v)]) {
                    na_fila[static_cast<size_t>(v)] = 1;
                    fila.push_back(v);
                }
                if (++relaxacoes % 1024 == 0 && ciclo_predecessor_negativo(pred, pred_peso)) return true;
            }
        }
    }
    return false;
}

bool conexo_forte(const std::vector<std::vector<Arco>>& entrada,
                  const std::vector<std::vector<Arco>>& saida, int32_t raiz) {
    const int32_t n = static_cast<int32_t>(entrada.size());
    for (const auto* adj : {&entrada, &saida}) {
        std::vector<uint8_t> visto(static_cast<size_t>(n), 0);
        std::vector<int32_t> fila{raiz};
        visto[static_cast<size_t>(raiz)] = 1;
        for (size_t i = 0; i < fila.size(); ++i) {
            for (const auto& [v, w] : (*adj)[static_cast<size_t>(fila[i])]) {
                (void)w;
                if (!visto[static_cast<size_t>(v)]) {
                    visto[static_cast<size_t>(v)] = 1;
                    fila.push_back(v);
                }
            }
        }
        if (fila.size() != static_cast<size_t>(n)) return false;
    }
    return true;
}
} // namespace

BellmanFord::BellmanFord(const Grafo& g) : n_(g.n()), entrada_(static_cast<size_t>(g.n())) {
    // Preparacao independente do destino: feita uma vez, fora do cronometro das consultas.
    std::vector<std::vector<Arco>> saida(static_cast<size_t>(n_));
    g.visitar([&](const auto& concreto) {
        for (int32_t u = 0; u < n_; ++u) {
            concreto.para_cada_vizinho_com_peso(u, [&](int32_t v, double w) {
                entrada_[static_cast<size_t>(v)].emplace_back(u, w);
                saida[static_cast<size_t>(u)].emplace_back(v, w);
            });
        }
    });
    ciclo_global_ = ciclo_global(saida, g.tem_peso_negativo());
    // Forte conexidade nao depende da raiz; so interessa se houver ciclo negativo.
    if (ciclo_global_ && n_ > 0) fortemente_conexo_ = conexo_forte(entrada_, saida, 0);
}

ResultadoBellmanFord bellman_ford(const Grafo& g, int32_t destino) {
    g.validar_vertice(destino);
    return BellmanFord(g).executar(destino);
}

ResultadoBellmanFord BellmanFord::executar(int32_t destino) const {
    if (destino < 1 || destino > n_) {
        throw ErroGrafo("vertice " + std::to_string(destino) + " fora do intervalo 1.." + std::to_string(n_));
    }
    const int32_t n = n_;
    const auto& entrada = entrada_;
    const double INF = std::numeric_limits<double>::infinity();
    ResultadoBellmanFord r;
    r.destino = destino;
    r.dist.assign(static_cast<size_t>(n), INF);
    r.sucessor.assign(static_cast<size_t>(n), -1);
    r.dist[static_cast<size_t>(destino - 1)] = 0;
    r.sucessor[static_cast<size_t>(destino - 1)] = 0;
    r.ciclo_negativo = ciclo_global_;
    if (r.ciclo_negativo && fortemente_conexo_) {
        r.ciclo_afeta_destino = true;
        std::fill(r.dist.begin(), r.dist.end(), -INF);
        std::fill(r.sucessor.begin(), r.sucessor.end(), -1);
        return r;
    }
    // Aula 14: memoria com vetor M[v] (r.dist) em vez de M[i,v]; tempo com
    // vertices alterados na rodada anterior e parada sem novas melhorias.
    std::vector<int32_t> ativos{destino - 1};
    std::vector<uint8_t> marcado(static_cast<size_t>(n), 0);
    for (int32_t rodada = 0; rodada < n - 1 && !ativos.empty(); ++rodada) {
        std::vector<int32_t> proximos;
        for (int32_t v : ativos) {
            const double dv = r.dist[static_cast<size_t>(v)];
            for (const auto& [u, w] : entrada[static_cast<size_t>(v)]) {
                const double nova = dv + w;
                if (nova < r.dist[static_cast<size_t>(u)]) {
                    r.dist[static_cast<size_t>(u)] = nova;
                    r.sucessor[static_cast<size_t>(u)] = v + 1;
                    if (!marcado[static_cast<size_t>(u)]) {
                        marcado[static_cast<size_t>(u)] = 1;
                        proximos.push_back(u);
                    }
                }
            }
        }
        for (int32_t v : proximos) marcado[static_cast<size_t>(v)] = 0;
        ativos.swap(proximos);
        ++r.rodadas;
    }
    std::vector<int32_t> afetados;
    for (int32_t v : ativos) {
        for (const auto& [u, w] : entrada[static_cast<size_t>(v)]) {
            if (r.dist[static_cast<size_t>(v)] + w < r.dist[static_cast<size_t>(u)] &&
                !marcado[static_cast<size_t>(u)]) {
                marcado[static_cast<size_t>(u)] = 1;
                afetados.push_back(u);
            }
        }
    }
    r.ciclo_afeta_destino = !afetados.empty();
    for (size_t i = 0; i < afetados.size(); ++i) {
        const int32_t v = afetados[i];
        for (const auto& [u, w] : entrada[static_cast<size_t>(v)]) {
            (void)w;
            if (!marcado[static_cast<size_t>(u)]) {
                marcado[static_cast<size_t>(u)] = 1;
                afetados.push_back(u);
            }
        }
    }
    for (int32_t v : afetados) {
        r.dist[static_cast<size_t>(v)] = -INF;
        r.sucessor[static_cast<size_t>(v)] = -1;
    }
    r.ciclo_negativo = r.ciclo_afeta_destino || r.ciclo_negativo;
    return r;
}

std::vector<int32_t> caminho(const ResultadoBellmanFord& r, int32_t origem) {
    if (origem < 1 || origem > static_cast<int32_t>(r.dist.size())) throw ErroGrafo("origem invalida");
    if (!std::isfinite(r.dist[static_cast<size_t>(origem - 1)])) return {};
    std::vector<int32_t> path;
    int32_t u = origem;
    for (size_t i = 0; i <= r.dist.size(); ++i) {
        path.push_back(u);
        if (u == r.destino) return path;
        u = r.sucessor[static_cast<size_t>(u - 1)];
        if (u < 1) break;
    }
    throw ErroGrafo("sucessores inconsistentes no Bellman-Ford");
}
} // namespace grafos
