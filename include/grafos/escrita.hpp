#pragma once

#include <string>

#include "grafos/busca.hpp"
#include "grafos/componentes.hpp"
#include "grafos/dijkstra.hpp"
#include "grafos/estatisticas.hpp"

namespace grafos {

// Arquivo texto ASCII: "# <TIPO> raiz=<r>", "# vertice pai nivel ordem" e uma linha
// "vertice pai nivel ordem" por vertice, em ordem de numero (nao alcancados: -1 -1 -1).
void escrever_arvore(const ArvoreBusca& arvore, const std::string& caminho);

// Estatisticas (grau medio com 4 casas, mediana com 1) e componentes, uma por bloco,
// separadas por linha em branco, com os vertices numa unica linha.
void escrever_estatisticas(const Estatisticas& e, const Componentes& c, const std::string& caminho);

// "# Dijkstra origem=<s> fila=<heap|vetor>", "# vertice pai distancia" e uma linha por
// vertice em ordem de numero; distancia na forma mais curta que reconstroi o double,
// "infinito" se nao alcancado (pai -1).
void escrever_dijkstra(const ResultadoDijkstra& r, const std::string& caminho);

}  // namespace grafos
