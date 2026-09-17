#include "grafos/grafo.hpp"

#include "grafos/erro.hpp"

namespace grafos {

int32_t Grafo::n() const {
    return visitar([](const auto& g) { return g.n(); });
}

int64_t Grafo::m() const {
    return visitar([](const auto& g) { return g.m(); });
}

size_t Grafo::bytes_estrutura() const {
    return visitar([](const auto& g) { return g.bytes_estrutura(); });
}

void Grafo::validar_vertice(int32_t v) const {
    int32_t total = n();
    if (v < 1 || v > total) {
        throw ErroGrafo("vertice " + std::to_string(v) + " fora do intervalo 1.." + std::to_string(total));
    }
}

int32_t Grafo::grau(int32_t v) const {
    validar_vertice(v);
    return visitar([v](const auto& g) { return g.grau(v - 1); });
}

bool Grafo::sao_vizinhos(int32_t u, int32_t v) const {
    validar_vertice(u);
    validar_vertice(v);
    return visitar([u, v](const auto& g) { return g.sao_vizinhos(u - 1, v - 1); });
}

Grafo construir_grafo(const ArestasLidas& arestas, Representacao rep, InfoLeitura* info) {
    Grafo g = rep == Representacao::Lista ? Grafo(GrafoLista::construir(arestas))
                                          : Grafo(GrafoMatriz::construir(arestas));
    if (info != nullptr) {
        info->linhas_aresta = arestas.linhas_aresta;
        info->lacos_ignorados = arestas.lacos_ignorados;
        info->repetidas_ignoradas = static_cast<int64_t>(arestas.origem.size()) - g.m();
    }
    return g;
}

Grafo carregar(const std::string& caminho, Representacao rep, bool direcionado, InfoLeitura* info) {
    if (direcionado) throw ErroGrafo("grafos direcionados ainda nao suportados");
    // A lista temporaria de arestas vive so dentro da lambda; e liberada antes do malloc_trim.
    Grafo g = [&] {
        ArestasLidas arestas = ler_arestas(caminho);
        return construir_grafo(arestas, rep, info);
    }();
    devolver_memoria_ao_sistema();
    return g;
}

}  // namespace grafos
