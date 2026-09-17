#include "grafos/diametro.hpp"

namespace grafos {

Diametro diametro_ingenuo(const Grafo& g) {
    return g.visitar([](const auto& concreto) { return detalhe::diametro_ingenuo(concreto); });
}

Diametro diametro_aproximado(const Grafo& g) {
    return g.visitar([](const auto& concreto) { return detalhe::diametro_aproximado(concreto); });
}

Diametro diametro_exato(const Grafo& g, double limite_segundos) {
    return g.visitar([limite_segundos](const auto& concreto) {
        return detalhe::diametro_exato(concreto, limite_segundos);
    });
}

}  // namespace grafos
