#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace grafos {

// Nomes dos vertices (rede de colaboracao): arquivo com linhas "id,nome", cortadas na
// primeira virgula. Busca exata por bytes (UTF-8).
class NomesVertices {
public:
    static NomesVertices carregar(const std::string& caminho);

    int32_t id(const std::string& nome) const;  // ErroGrafo se ausente ou ambiguo
    const std::string& nome(int32_t v) const;   // v em 1..tamanho(); "" se o id nao apareceu
    int32_t tamanho() const { return static_cast<int32_t>(nomes_.size()); }  // maior id lido

private:
    std::vector<std::string> nomes_;  // indice v-1
};

}  // namespace grafos
