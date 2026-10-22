#include "grafos/nomes.hpp"

#include <cerrno>
#include <charconv>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string_view>

#include "grafos/erro.hpp"

namespace grafos {
namespace {

[[noreturn]] void erro_linha(int64_t linha, const std::string& mensagem) {
    throw ErroGrafo("linha " + std::to_string(linha) + ": " + mensagem);
}

}  // namespace

NomesVertices NomesVertices::carregar(const std::string& caminho) {
    std::ifstream arquivo(caminho, std::ios::binary);
    if (!arquivo) throw ErroGrafo("nao foi possivel abrir '" + caminho + "': " + std::strerror(errno));
    std::ostringstream conteudo;
    conteudo << arquivo.rdbuf();
    const std::string texto = conteudo.str();

    NomesVertices r;
    std::vector<char> lido;
    int64_t linha = 0;
    size_t ini = 0;
    while (ini < texto.size()) {
        size_t fim = texto.find('\n', ini);
        if (fim == std::string::npos) fim = texto.size();
        ++linha;
        std::string_view l(texto.data() + ini, fim - ini);
        ini = fim + 1;
        if (!l.empty() && l.back() == '\r') l.remove_suffix(1);
        if (l.empty()) continue;

        const size_t virgula = l.find(',');
        if (virgula == std::string_view::npos) erro_linha(linha, "virgula ausente");
        const std::string_view id_texto = l.substr(0, virgula);
        int32_t id = 0;
        auto conv = std::from_chars(id_texto.data(), id_texto.data() + id_texto.size(), id);
        if (conv.ec != std::errc() || conv.ptr != id_texto.data() + id_texto.size() || id < 1) {
            erro_linha(linha, "id invalido '" + std::string(id_texto) + "'");
        }
        const size_t i = static_cast<size_t>(id - 1);
        if (i >= r.nomes_.size()) {
            r.nomes_.resize(i + 1);
            lido.resize(i + 1, 0);
        }
        if (lido[i]) erro_linha(linha, "id " + std::to_string(id) + " repetido");
        lido[i] = 1;
        r.nomes_[i] = std::string(l.substr(virgula + 1));
    }
    return r;
}

int32_t NomesVertices::id(const std::string& nome) const {
    int32_t achado = 0;
    for (size_t i = 0; i < nomes_.size(); ++i) {
        if (nomes_[i] != nome) continue;
        if (achado != 0) throw ErroGrafo("nome ambiguo: " + nome);
        achado = static_cast<int32_t>(i) + 1;
    }
    if (achado == 0) throw ErroGrafo("nome nao encontrado: " + nome);
    return achado;
}

const std::string& NomesVertices::nome(int32_t v) const {
    if (v < 1 || v > tamanho()) {
        throw ErroGrafo("id " + std::to_string(v) + " fora do intervalo 1.." + std::to_string(tamanho()));
    }
    return nomes_[static_cast<size_t>(v - 1)];
}

}  // namespace grafos
