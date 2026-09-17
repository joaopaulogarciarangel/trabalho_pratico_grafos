#pragma once

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "grafos/componentes.hpp"
#include "grafos/grafo.hpp"

namespace grafos {

// Diametro de grafo desconexo = maior diametro entre as componentes.
struct Diametro {
    int32_t valor;  // melhor valor (igual ao limite inferior)
    int32_t u, v;   // par de vertices (1..n) que realiza o limite inferior
    int32_t limite_inferior, limite_superior;
    bool exato;           // limite_inferior == limite_superior
    bool tempo_esgotado;  // iFUB interrompido pelo limite de tempo
    int64_t num_bfs;      // quantas BFS foram executadas
    double segundos;
};

Diametro diametro_ingenuo(const Grafo& g);
Diametro diametro_aproximado(const Grafo& g);
Diametro diametro_exato(const Grafo& g, double limite_segundos = 1800.0);

namespace detalhe {

// BFS com area de trabalho reutilizavel: alocada uma vez; ao limpar, desmarca so os
// vertices visitados (percorrendo a propria fila), sem custo O(n) por BFS.
template <class G>
class BfsAuxiliar {
public:
    struct Resultado {
        int32_t excentricidade;
        int32_t mais_distante;  // ultimo vertice descoberto
    };

    explicit BfsAuxiliar(const G& g)
        : g_(g),
          nivel_(static_cast<size_t>(g.n()), -1),
          pai_(static_cast<size_t>(g.n()), -1),
          fila_(static_cast<size_t>(g.n())) {}

    // A area fica marcada ate limpar(): subir(), nivel() e visitados() continuam validos.
    Resultado executar(int32_t origem) {
        ++num_bfs_;
        size_t inicio = 0;
        fim_ = 0;
        nivel_[static_cast<size_t>(origem)] = 0;
        pai_[static_cast<size_t>(origem)] = -1;
        fila_[fim_++] = origem;
        while (inicio < fim_) {
            const int32_t x = fila_[inicio++];
            const int32_t nivel_filho = nivel_[static_cast<size_t>(x)] + 1;
            g_.para_cada_vizinho(x, [&](int32_t w) {
                const size_t iw = static_cast<size_t>(w);
                if (nivel_[iw] < 0) {
                    nivel_[iw] = nivel_filho;
                    pai_[iw] = x;
                    fila_[fim_++] = w;
                }
            });
        }
        const int32_t ultimo = fila_[fim_ - 1];
        return {nivel_[static_cast<size_t>(ultimo)], ultimo};
    }

    // Sobe `passos` pais a partir de v na arvore da ultima BFS.
    int32_t subir(int32_t v, int32_t passos) const {
        for (int32_t k = 0; k < passos; ++k) v = pai_[static_cast<size_t>(v)];
        return v;
    }

    int32_t nivel(int32_t v) const { return nivel_[static_cast<size_t>(v)]; }
    const int32_t* visitados() const { return fila_.data(); }  // em ordem de descoberta
    size_t num_visitados() const { return fim_; }

    void limpar() {
        for (size_t k = 0; k < fim_; ++k) nivel_[static_cast<size_t>(fila_[k])] = -1;
        fim_ = 0;
    }

    int64_t num_bfs() const { return num_bfs_; }

private:
    const G& g_;
    std::vector<int32_t> nivel_;
    std::vector<int32_t> pai_;
    std::vector<int32_t> fila_;
    size_t fim_ = 0;
    int64_t num_bfs_ = 0;
};

// Criterio de parada consultado antes de cada BFS: limite de tempo e, so para testes,
// limite de numero de BFS (negativo = sem limite).
class Relogio {
public:
    Relogio(double limite_segundos, bool com_limite, int64_t limite_bfs = -1)
        : inicio_(std::chrono::steady_clock::now()),
          limite_(limite_segundos),
          com_limite_(com_limite),
          limite_bfs_(limite_bfs) {}

    double segundos() const {
        return std::chrono::duration<double>(std::chrono::steady_clock::now() - inicio_).count();
    }

    bool esgotado(int64_t bfs_feitas) const {
        return (com_limite_ && segundos() >= limite_) || (limite_bfs_ >= 0 && bfs_feitas >= limite_bfs_);
    }

private:
    std::chrono::steady_clock::time_point inicio_;
    double limite_;
    bool com_limite_;
    int64_t limite_bfs_;
};

// Limites de uma componente; par e centro em indices internos.
struct LimitesComponente {
    int32_t lb = -1;
    int32_t u = -1;
    int32_t v = -1;
    int32_t ub = 0;
    int32_t centro = -1;  // m2 do 4-sweep (definido so se o 4-sweep terminou)
    bool interrompido = false;
};

// Registra a excentricidade de `origem`; o par so muda em melhora estrita.
inline void registrar(LimitesComponente& c, int32_t origem, int32_t excentricidade, int32_t mais_distante) {
    if (excentricidade > c.lb) {
        c.lb = excentricidade;
        c.u = origem;
        c.v = mais_distante;
    }
}

// 4-sweep (secao 8.2) sobre a componente. Se interrompido, ub e o menor valor valido
// ja obtido: min(tamanho - 1, 2*ecc dos centros processados).
template <class G>
LimitesComponente quatro_varreduras(const G& g, BfsAuxiliar<G>& bfs, const Componente& comp, const Relogio& relogio) {
    LimitesComponente c;
    c.ub = comp.tamanho - 1;

    // r = maior grau da componente; empate pelo menor id (vertices em ordem crescente).
    int32_t r = comp.vertices[0] - 1;
    for (int32_t id : comp.vertices) {
        if (g.grau(id - 1) > g.grau(r)) r = id - 1;
    }

    typename BfsAuxiliar<G>::Resultado res{};
    auto rodar = [&](int32_t origem, bool centro) {
        if (relogio.esgotado(bfs.num_bfs())) {
            c.interrompido = true;
            return false;
        }
        res = bfs.executar(origem);
        registrar(c, origem, res.excentricidade, res.mais_distante);
        if (centro) c.ub = std::min(c.ub, 2 * res.excentricidade);
        return true;
    };

    if (!rodar(r, true)) return c;
    const int32_t a1 = res.mais_distante;
    bfs.limpar();

    if (!rodar(a1, false)) return c;
    const int32_t m1 = bfs.subir(res.mais_distante, res.excentricidade / 2);  // meio de b1..a1
    bfs.limpar();

    if (!rodar(m1, true)) return c;
    const int32_t a2 = res.mais_distante;
    bfs.limpar();

    if (!rodar(a2, false)) return c;
    const int32_t m2 = bfs.subir(res.mais_distante, res.excentricidade / 2);  // meio de b2..a2
    bfs.limpar();

    if (!rodar(m2, true)) return c;
    bfs.limpar();
    c.centro = m2;
    return c;
}

// Laco do iFUB (Crescenzi et al., 2013) sobre uma componente ja varrida pelo 4-sweep.
// Refaz a BFS(u = m2) para obter as camadas F_i (a fila sai em ordem de nivel).
// Para quando o ub da componente nao supera `lb_global` (nao muda a resposta).
template <class G>
void refinar_ifub(BfsAuxiliar<G>& bfs, LimitesComponente& c, const Relogio& relogio, int32_t lb_global) {
    if (c.centro < 0 || c.ub <= c.lb || c.ub <= lb_global) return;
    if (relogio.esgotado(bfs.num_bfs())) {
        c.interrompido = true;
        return;
    }
    const auto base = bfs.executar(c.centro);
    registrar(c, c.centro, base.excentricidade, base.mais_distante);
    const size_t total = bfs.num_visitados();
    const std::vector<int32_t> ordem(bfs.visitados(), bfs.visitados() + total);
    std::vector<size_t> inicio(static_cast<size_t>(base.excentricidade) + 2, 0);  // F_i = ordem[inicio[i], inicio[i+1])
    for (int32_t x : ordem) ++inicio[static_cast<size_t>(bfs.nivel(x)) + 1];
    for (size_t i = 1; i < inicio.size(); ++i) inicio[i] += inicio[i - 1];
    bfs.limpar();

    int32_t i = base.excentricidade;
    while (c.ub > c.lb && c.ub > lb_global) {
        // B_i: excentricidades de F_i; registrar() mantem c.lb = max(lb, B_i) e o par.
        for (size_t k = inicio[static_cast<size_t>(i)]; k < inicio[static_cast<size_t>(i) + 1]; ++k) {
            if (relogio.esgotado(bfs.num_bfs())) {
                c.interrompido = true;
                return;
            }
            const int32_t x = ordem[k];
            const auto res = bfs.executar(x);
            registrar(c, x, res.excentricidade, res.mais_distante);
            bfs.limpar();
        }
        if (c.lb > 2 * (i - 1)) {
            c.ub = c.lb;
            return;
        }
        c.ub = std::min(c.ub, 2 * (i - 1));  // limites superiores se combinam por minimo
        --i;
    }
}

struct MelhorGlobal {
    int32_t lb = 0;
    int32_t u = -1;
    int32_t v = -1;

    void atualizar(const LimitesComponente& c) {
        if (c.lb > lb) {
            lb = c.lb;
            u = c.u;
            v = c.v;
        }
    }
};

inline Diametro montar_diametro(const MelhorGlobal& melhor, int32_t ub, bool esgotado, int64_t num_bfs,
                                double segundos) {
    Diametro d{};
    d.valor = melhor.lb;
    d.limite_inferior = melhor.lb;
    d.limite_superior = std::max(melhor.lb, ub);
    d.u = melhor.u + 1;
    d.v = melhor.v + 1;
    d.exato = d.limite_inferior == d.limite_superior;
    d.tempo_esgotado = esgotado;
    d.num_bfs = num_bfs;
    d.segundos = segundos;
    return d;
}

// Diametro aproximado: 4-sweep componente a componente, em ordem decrescente de tamanho,
// parando nas que tem tamanho - 1 <= lb atual (e nas seguintes, que sao menores).
template <class G>
Diametro diametro_aproximado(const G& g) {
    const Relogio relogio(0.0, false);
    const Componentes comps = detalhe::componentes_conexas(g);
    BfsAuxiliar<G> bfs(g);
    MelhorGlobal melhor;
    melhor.u = melhor.v = comps.lista[0].vertices[0] - 1;  // sem componente processada: (v, v)
    int32_t ub = 0;
    for (const Componente& comp : comps.lista) {
        if (comp.tamanho - 1 <= melhor.lb) break;
        const LimitesComponente c = quatro_varreduras(g, bfs, comp, relogio);
        melhor.atualizar(c);
        ub = std::max(ub, c.ub);
    }
    return montar_diametro(melhor, ub, false, bfs.num_bfs(), relogio.segundos());
}

// Diametro exato em duas fases. Fase A: 4-sweep em todas as componentes que podem superar
// o lb atual (ordem decrescente de tamanho), dando a cada uma um ub proprio. Fase B: laco
// do iFUB, na mesma ordem, so nas componentes com ub > lb global. Se o limite esgotar,
// o intervalo continua informativo: ub = max(ub das varridas, tamanho - 1 da primeira nao varrida).
template <class G>
Diametro diametro_exato(const G& g, double limite_segundos, int64_t limite_bfs = -1) {
    const Relogio relogio(limite_segundos, true, limite_bfs);
    const Componentes comps = detalhe::componentes_conexas(g);
    BfsAuxiliar<G> bfs(g);
    MelhorGlobal melhor;
    melhor.u = melhor.v = comps.lista[0].vertices[0] - 1;

    std::vector<LimitesComponente> varridas;
    int32_t ub_nao_varridas = 0;
    bool esgotado = false;

    for (size_t k = 0; k < comps.lista.size(); ++k) {  // Fase A
        const Componente& comp = comps.lista[k];
        if (comp.tamanho - 1 <= melhor.lb) break;
        LimitesComponente c = quatro_varreduras(g, bfs, comp, relogio);
        melhor.atualizar(c);
        varridas.push_back(c);
        if (c.interrompido) {
            esgotado = true;
            if (k + 1 < comps.lista.size()) ub_nao_varridas = comps.lista[k + 1].tamanho - 1;
            break;
        }
    }

    for (LimitesComponente& c : varridas) {  // Fase B
        if (esgotado) break;
        refinar_ifub(bfs, c, relogio, melhor.lb);
        melhor.atualizar(c);
        esgotado = c.interrompido;
    }

    int32_t ub = ub_nao_varridas;
    for (const LimitesComponente& c : varridas) ub = std::max(ub, c.ub);
    return montar_diametro(melhor, ub, esgotado, bfs.num_bfs(), relogio.segundos());
}

// BFS a partir de cada vertice; maior excentricidade (ignorando nao alcancados).
template <class G>
Diametro diametro_ingenuo(const G& g) {
    const Relogio relogio(0.0, false);
    BfsAuxiliar<G> bfs(g);
    LimitesComponente c;
    for (int32_t s = 0; s < g.n(); ++s) {
        auto res = bfs.executar(s);
        registrar(c, s, res.excentricidade, res.mais_distante);
        bfs.limpar();
    }
    Diametro d{};
    d.valor = c.lb;
    d.limite_inferior = c.lb;
    d.limite_superior = c.lb;
    d.u = c.u + 1;
    d.v = c.v + 1;
    d.exato = true;
    d.tempo_esgotado = false;
    d.num_bfs = bfs.num_bfs();
    d.segundos = relogio.segundos();
    return d;
}

}  // namespace detalhe
}  // namespace grafos
