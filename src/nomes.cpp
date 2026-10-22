#include "grafos/nomes.hpp"

#include "grafos/erro.hpp"

namespace grafos {

NomesVertices NomesVertices::carregar(const std::string&) {
    throw ErroGrafo("nao implementado");
}

int32_t NomesVertices::id(const std::string&) const {
    throw ErroGrafo("nao implementado");
}

const std::string& NomesVertices::nome(int32_t) const {
    throw ErroGrafo("nao implementado");
}

}  // namespace grafos
