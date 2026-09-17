#include "grafos/leitura.hpp"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <limits>
#include <memory>

#include "grafos/erro.hpp"

#ifdef __GLIBC__
#include <malloc.h>
#endif

namespace grafos {
namespace {

struct Token {
    const char* ini;
    const char* fim;
};

bool eh_espaco(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\v' || c == '\f';
}

// Le ate dois tokens da linha que comeca em p e avanca p para o inicio da proxima linha.
int ler_linha(const char*& p, const char* fim, Token tokens[2]) {
    int ntok = 0;
    while (p < fim && *p != '\n') {
        if (eh_espaco(*p)) {
            ++p;
            continue;
        }
        if (ntok == 2) {  // colunas extras sao ignoradas (peso, na parte 2)
            p = static_cast<const char*>(std::memchr(p, '\n', static_cast<size_t>(fim - p)));
            if (p == nullptr) p = fim;
            break;
        }
        const char* ini = p;
        while (p < fim && *p != '\n' && !eh_espaco(*p)) ++p;
        tokens[ntok++] = Token{ini, p};
    }
    if (p < fim) ++p;
    return ntok;
}

// Converte um token de digitos (com '-' opcional). Valores enormes saturam,
// continuando fora de qualquer intervalo valido.
bool converter(const Token& t, int64_t& valor) {
    const char* c = t.ini;
    bool negativo = false;
    if (*c == '-') {
        negativo = true;
        ++c;
    }
    if (c == t.fim) return false;
    int64_t x = 0;
    for (; c < t.fim; ++c) {
        unsigned d = static_cast<unsigned>(static_cast<unsigned char>(*c)) - static_cast<unsigned>('0');
        if (d > 9) return false;
        if (x < (int64_t{1} << 40)) x = x * 10 + d;
    }
    valor = negativo ? -x : x;
    return true;
}

std::string texto(const Token& t) {
    std::string s(t.ini, t.fim);
    if (s.size() > 40) {
        s.resize(40);
        s += "...";
    }
    return s;
}

[[noreturn]] void erro_linha(int64_t linha, const std::string& mensagem) {
    throw ErroGrafo("linha " + std::to_string(linha) + ": " + mensagem);
}

int64_t ler_vertice(const Token& t, int64_t linha, int32_t n) {
    int64_t v = 0;
    if (!converter(t, v)) erro_linha(linha, "texto invalido '" + texto(t) + "'");
    if (v < 1 || v > n) {
        erro_linha(linha, "vertice " + texto(t) + " fora do intervalo 1.." + std::to_string(n));
    }
    return v;
}

}  // namespace

ArestasLidas parsear_arestas(const char* dados, size_t tamanho) {
    ArestasLidas r;
    const char* p = dados;
    const char* const fim = dados + tamanho;
    int64_t linha = 0;
    Token tok[2];

    int ntok = 0;
    while (p < fim) {
        ++linha;
        ntok = ler_linha(p, fim, tok);
        if (ntok > 0) break;
    }
    if (ntok == 0) erro_linha(linha == 0 ? 1 : linha, "n ausente");

    int64_t n = 0;
    if (!converter(tok[0], n)) erro_linha(linha, "texto invalido '" + texto(tok[0]) + "' (esperado n)");
    if (n <= 0) erro_linha(linha, "n deve ser positivo (lido " + texto(tok[0]) + ")");
    if (n > std::numeric_limits<int32_t>::max()) {
        erro_linha(linha, "n " + texto(tok[0]) + " maior que o limite de int32_t");
    }
    r.n = static_cast<int32_t>(n);

    // Reserva exata (limite superior de linhas) para evitar realocacoes durante o parse.
    size_t linhas_restantes = 1;
    for (const char* q = p; q < fim; ++q) {
        q = static_cast<const char*>(std::memchr(q, '\n', static_cast<size_t>(fim - q)));
        if (q == nullptr) break;
        ++linhas_restantes;
    }
    r.origem.reserve(linhas_restantes);
    r.destino.reserve(linhas_restantes);

    while (p < fim) {
        ++linha;
        ntok = ler_linha(p, fim, tok);
        if (ntok == 0) continue;
        int64_t u = ler_vertice(tok[0], linha, r.n);
        if (ntok == 1) erro_linha(linha, "esperados dois vertices, encontrado apenas um");
        int64_t v = ler_vertice(tok[1], linha, r.n);
        ++r.linhas_aresta;
        if (u == v) {
            ++r.lacos_ignorados;
            continue;
        }
        r.origem.push_back(static_cast<int32_t>(u - 1));
        r.destino.push_back(static_cast<int32_t>(v - 1));
    }
    return r;
}

ArestasLidas ler_arestas(const std::string& caminho) {
    std::unique_ptr<std::FILE, int (*)(std::FILE*)> arquivo(std::fopen(caminho.c_str(), "rb"), &std::fclose);
    if (!arquivo) {
        throw ErroGrafo("nao foi possivel abrir '" + caminho + "': " + std::strerror(errno));
    }
    const char* erro_leitura = "erro ao ler '";
    if (std::fseek(arquivo.get(), 0, SEEK_END) != 0) throw ErroGrafo(erro_leitura + caminho + "'");
    long tamanho_long = std::ftell(arquivo.get());
    if (tamanho_long < 0 || std::fseek(arquivo.get(), 0, SEEK_SET) != 0) {
        throw ErroGrafo(erro_leitura + caminho + "'");
    }
    size_t tamanho = static_cast<size_t>(tamanho_long);

    std::unique_ptr<char[]> buffer(new char[tamanho > 0 ? tamanho : 1]);
    size_t lidos = 0;
    while (lidos < tamanho) {
        size_t k = std::fread(buffer.get() + lidos, 1, tamanho - lidos, arquivo.get());
        if (k == 0) break;
        lidos += k;
    }
    if (std::ferror(arquivo.get())) throw ErroGrafo(erro_leitura + caminho + "'");
    arquivo.reset();

    ArestasLidas arestas = parsear_arestas(buffer.get(), lidos);
    buffer.reset();
    return arestas;
}

void devolver_memoria_ao_sistema() {
#ifdef __GLIBC__
    malloc_trim(0);
#endif
}

}  // namespace grafos
