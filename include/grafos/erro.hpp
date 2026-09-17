#pragma once

#include <stdexcept>

namespace grafos {

// Erro reportado pela biblioteca (arquivo invalido, vertice fora do intervalo etc.).
class ErroGrafo : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

}  // namespace grafos
