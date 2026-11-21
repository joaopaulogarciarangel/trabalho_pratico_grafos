#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace grafos {

struct InfoLeitura {
    int64_t linhas_aresta = 0;        // linhas de aresta nao vazias, incluindo lacos e repeticoes
    int64_t lacos_ignorados = 0;
    int64_t repetidas_ignoradas = 0;
};

// Lista temporaria de arestas: indices internos 0..n-1, lacos ja descartados,
// repeticoes ainda presentes (sao descartadas na construcao da representacao).
// O arquivo e ponderado se a primeira linha de aresta tiver peso numerico; nesse caso
// peso[i] e o peso da aresta i (vazio se nao ponderado).
struct ArestasLidas {
    int32_t n = 0;
    std::vector<int32_t> origem;
    std::vector<int32_t> destino;
    std::vector<double> peso;
    bool ponderado = false;
    bool tem_peso_negativo = false;  // alguma aresta (nao laco) com peso < 0
    bool direcionado = false;
    int64_t linhas_aresta = 0;
    int64_t lacos_ignorados = 0;
};

ArestasLidas parsear_arestas(const char* dados, size_t tamanho, bool direcionado = false);

// Le o arquivo inteiro para um buffer, parseia e libera o buffer antes de retornar.
ArestasLidas ler_arestas(const std::string& caminho, bool direcionado = false);

// Devolve ao sistema a memoria livre do heap (malloc_trim na glibc).
void devolver_memoria_ao_sistema();

}  // namespace grafos
