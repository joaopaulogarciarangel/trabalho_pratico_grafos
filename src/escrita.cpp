#include "grafos/escrita.hpp"

#include <cerrno>
#include <charconv>
#include <cstdio>
#include <cstring>
#include <memory>
#include <string_view>

#include "grafos/erro.hpp"

namespace grafos {
namespace {

// Escrita bufferizada independente de locale (inteiros via std::to_chars).
class Escritor {
public:
    explicit Escritor(const std::string& caminho)
        : caminho_(caminho), arquivo_(std::fopen(caminho.c_str(), "wb")), buffer_(new char[kTamanho]) {
        if (arquivo_ == nullptr) {
            throw ErroGrafo("nao foi possivel criar '" + caminho + "': " + std::strerror(errno));
        }
    }

    ~Escritor() {
        if (arquivo_ != nullptr) std::fclose(arquivo_);
    }

    Escritor(const Escritor&) = delete;
    Escritor& operator=(const Escritor&) = delete;

    void texto(std::string_view s) {
        if (usado_ + s.size() > kTamanho) descarregar();
        if (s.size() > kTamanho) {
            gravar(s.data(), s.size());
            return;
        }
        std::memcpy(buffer_.get() + usado_, s.data(), s.size());
        usado_ += s.size();
    }

    void caractere(char c) {
        if (usado_ == kTamanho) descarregar();
        buffer_[usado_++] = c;
    }

    void inteiro(long long x) {
        if (usado_ + 24 > kTamanho) descarregar();
        auto r = std::to_chars(buffer_.get() + usado_, buffer_.get() + kTamanho, x);
        usado_ = static_cast<size_t>(r.ptr - buffer_.get());
    }

    // Numero com casas decimais fixas, ponto como separador, independente de locale.
    void decimal(double x, int casas) {
        if (usado_ + 64 > kTamanho) descarregar();
        auto r = std::to_chars(buffer_.get() + usado_, buffer_.get() + kTamanho, x, std::chars_format::fixed, casas);
        usado_ = static_cast<size_t>(r.ptr - buffer_.get());
    }

    void fechar() {
        descarregar();
        std::FILE* f = arquivo_;
        arquivo_ = nullptr;
        if (std::fclose(f) != 0) falhar();
    }

private:
    static constexpr size_t kTamanho = size_t{1} << 16;

    void descarregar() {
        gravar(buffer_.get(), usado_);
        usado_ = 0;
    }

    void gravar(const char* dados, size_t tamanho) {
        if (tamanho > 0 && std::fwrite(dados, 1, tamanho, arquivo_) != tamanho) falhar();
    }

    [[noreturn]] void falhar() {
        throw ErroGrafo("erro ao escrever '" + caminho_ + "'");
    }

    std::string caminho_;
    std::FILE* arquivo_;
    std::unique_ptr<char[]> buffer_;
    size_t usado_ = 0;
};

}  // namespace

void escrever_arvore(const ArvoreBusca& arvore, const std::string& caminho) {
    Escritor saida(caminho);
    saida.texto(arvore.tipo == TipoBusca::BFS ? "# BFS raiz=" : "# DFS raiz=");
    saida.inteiro(arvore.raiz);
    saida.texto("\n# vertice pai nivel ordem\n");
    const size_t n = arvore.pai.size();
    for (size_t i = 0; i < n; ++i) {
        saida.inteiro(static_cast<long long>(i) + 1);
        saida.caractere(' ');
        saida.inteiro(arvore.pai[i]);
        saida.caractere(' ');
        saida.inteiro(arvore.nivel[i]);
        saida.caractere(' ');
        saida.inteiro(arvore.ordem[i]);
        saida.caractere('\n');
    }
    saida.fechar();
}

void escrever_estatisticas(const Estatisticas& e, const Componentes& c, const std::string& caminho) {
    Escritor saida(caminho);
    saida.texto("# Estatisticas do grafo\nvertices: ");
    saida.inteiro(e.n);
    saida.texto("\narestas: ");
    saida.inteiro(e.m);
    saida.texto("\ngrau_minimo: ");
    saida.inteiro(e.grau_minimo);
    saida.texto("\ngrau_maximo: ");
    saida.inteiro(e.grau_maximo);
    saida.texto("\ngrau_medio: ");
    saida.decimal(e.grau_medio, 4);
    saida.texto("\nmediana_grau: ");
    saida.decimal(e.mediana_grau, 1);
    saida.texto("\n\n# Componentes conexas\nnum_componentes: ");
    saida.inteiro(static_cast<long long>(c.lista.size()));
    saida.caractere('\n');
    for (size_t i = 0; i < c.lista.size(); ++i) {
        const Componente& comp = c.lista[i];
        saida.texto("\ncomponente ");
        saida.inteiro(static_cast<long long>(i) + 1);
        saida.texto(" (tamanho ");
        saida.inteiro(comp.tamanho);
        saida.texto("):\n");
        for (size_t k = 0; k < comp.vertices.size(); ++k) {
            if (k > 0) saida.caractere(' ');
            saida.inteiro(comp.vertices[k]);
        }
        saida.caractere('\n');
    }
    saida.fechar();
}

}  // namespace grafos
