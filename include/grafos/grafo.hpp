#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <variant>

#include "grafos/grafo_lista.hpp"
#include "grafos/grafo_matriz.hpp"
#include "grafos/leitura.hpp"

namespace grafos {

enum class Representacao { Lista, Matriz };

// Grafo com representacao escolhida em tempo de execucao. A API publica usa
// vertices 1..n; as classes concretas usam indices internos 0..n-1.
// Algoritmos fazem um unico visitar() e rodam a versao template sobre o tipo concreto.
class Grafo {
public:
    explicit Grafo(GrafoLista g) : dados_(std::move(g)) {}
    explicit Grafo(GrafoMatriz g) : dados_(std::move(g)) {}

    Representacao representacao() const {
        return std::holds_alternative<GrafoLista>(dados_) ? Representacao::Lista : Representacao::Matriz;
    }

    int32_t n() const;
    int64_t m() const;
    size_t bytes_estrutura() const;
    int32_t grau(int32_t v) const;                  // v em 1..n
    bool sao_vizinhos(int32_t u, int32_t v) const;  // u, v em 1..n

    template <class F>
    decltype(auto) visitar(F&& f) const {
        return std::visit(std::forward<F>(f), dados_);
    }

    // Lanca ErroGrafo se v nao estiver em 1..n.
    void validar_vertice(int32_t v) const;

private:
    std::variant<GrafoLista, GrafoMatriz> dados_;
};

Grafo construir_grafo(const ArestasLidas& arestas, Representacao rep, InfoLeitura* info = nullptr);

Grafo carregar(const std::string& caminho, Representacao rep, bool direcionado = false,
               InfoLeitura* info = nullptr);

}  // namespace grafos
