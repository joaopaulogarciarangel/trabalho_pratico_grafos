// Programa `gerar_grafo`: grafo aleatorio uniforme no formato da disciplina, para testes de
// desempenho. Cada aresta liga dois vertices distintos sorteados uniformemente em 1..n; como
// no modelo de arestas independentes, repeticoes sao possiveis (a leitura as ignora e conta).

#include <charconv>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>

namespace {

const char* const AJUDA = R"(uso:
  gerar_grafo --n <vertices> --m <arestas> [--semente 1] -o <saida>

Gera m arestas (u, v), u != v, com u e v uniformes em 1..n.
)";

class ErroUso : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

template <class T>
T ler_inteiro(const std::string& texto, const std::string& nome) {
    T valor{};
    auto r = std::from_chars(texto.data(), texto.data() + texto.size(), valor);
    if (r.ec != std::errc() || r.ptr != texto.data() + texto.size()) throw ErroUso(nome + " invalido: '" + texto + "'");
    return valor;
}

// Inteiro uniforme em 0..limite-1 por rejeicao (reprodutivel entre compiladores).
uint64_t uniforme(std::mt19937_64& gerador, uint64_t limite) {
    const uint64_t descarte = (0 - limite) % limite;
    for (;;) {
        const uint64_t x = gerador();
        if (x >= descarte) return x % limite;
    }
}

}  // namespace

int main(int argc, char** argv) {
    try {
        int64_t n = -1;
        int64_t m = -1;
        uint64_t semente = 1;
        std::string saida;
        for (int i = 1; i < argc; ++i) {
            const std::string a = argv[i];
            if (a == "--help" || a == "-h") {
                std::cout << AJUDA;
                return 0;
            }
            if (i + 1 >= argc) throw ErroUso("opcao " + a + " exige um valor");
            const std::string valor = argv[++i];
            if (a == "--n") {
                n = ler_inteiro<int64_t>(valor, "n");
            } else if (a == "--m") {
                m = ler_inteiro<int64_t>(valor, "m");
            } else if (a == "--semente") {
                semente = ler_inteiro<uint64_t>(valor, "semente");
            } else if (a == "-o") {
                saida = valor;
            } else {
                throw ErroUso("opcao desconhecida: " + a);
            }
        }
        if (n < 2 || n > INT32_MAX) throw ErroUso("--n deve estar em 2..2147483647");
        if (m < 0) throw ErroUso("--m e obrigatorio e nao negativo");
        if (saida.empty()) throw ErroUso("-o <saida> e obrigatorio");

        std::unique_ptr<std::FILE, int (*)(std::FILE*)> arquivo(std::fopen(saida.c_str(), "wb"), &std::fclose);
        if (!arquivo) throw std::runtime_error("nao foi possivel criar '" + saida + "'");
        const size_t tamanho_buffer = size_t{1} << 20;
        std::unique_ptr<char[]> buffer(new char[tamanho_buffer]);
        size_t usado = 0;
        auto descarregar = [&]() {
            if (usado > 0 && std::fwrite(buffer.get(), 1, usado, arquivo.get()) != usado) {
                throw std::runtime_error("erro ao escrever '" + saida + "'");
            }
            usado = 0;
        };
        auto escrever = [&](int64_t a, int64_t b, bool linha_de_n) {
            if (usado + 32 > tamanho_buffer) descarregar();
            auto r = std::to_chars(buffer.get() + usado, buffer.get() + tamanho_buffer, a);
            if (!linha_de_n) {
                *r.ptr++ = ' ';
                r = std::to_chars(r.ptr, buffer.get() + tamanho_buffer, b);
            }
            *r.ptr++ = '\n';
            usado = static_cast<size_t>(r.ptr - buffer.get());
        };

        std::mt19937_64 gerador(semente);
        const uint64_t nn = static_cast<uint64_t>(n);
        escrever(n, 0, true);
        for (int64_t k = 0; k < m; ++k) {
            const uint64_t u = uniforme(gerador, nn);
            uint64_t v = uniforme(gerador, nn - 1);  // v != u sem nova rejeicao
            if (v >= u) ++v;
            escrever(static_cast<int64_t>(u) + 1, static_cast<int64_t>(v) + 1, false);
        }
        descarregar();
        if (std::fclose(arquivo.release()) != 0) throw std::runtime_error("erro ao fechar '" + saida + "'");
        std::cout << "gerado: " << saida << " (n = " << n << ", m = " << m << ", semente = " << semente << ")\n";
        return 0;
    } catch (const ErroUso& e) {
        std::cerr << "erro: " << e.what() << "\n" << AJUDA;
        return 2;
    } catch (const std::exception& e) {
        std::cerr << "erro: " << e.what() << "\n";
        return 1;
    }
}
