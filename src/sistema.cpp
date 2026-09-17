#include "grafos/sistema.hpp"

#include <cerrno>
#include <charconv>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <memory>
#include <random>
#include <string>

#include "grafos/erro.hpp"
#include "grafos/grafo_lista.hpp"
#include "grafos/grafo_matriz.hpp"
#include "grafos/leitura.hpp"

namespace grafos {
namespace {

// Procura "campo:" no inicio de uma linha e devolve o resto da linha, sem espacos iniciais.
bool valor_de_campo(const std::string& arquivo, const std::string& campo, std::string& valor) {
    std::ifstream f(arquivo);
    std::string linha;
    while (std::getline(f, linha)) {
        if (linha.compare(0, campo.size(), campo) != 0) continue;
        size_t pos = campo.size();
        while (pos < linha.size() && (linha[pos] == ' ' || linha[pos] == '\t')) ++pos;
        if (pos >= linha.size() || linha[pos] != ':') continue;
        ++pos;
        while (pos < linha.size() && (linha[pos] == ' ' || linha[pos] == '\t')) ++pos;
        valor = linha.substr(pos);
        return true;
    }
    return false;
}

int64_t kb_para_bytes(const std::string& valor) {
    int64_t kb = -1;
    auto r = std::from_chars(valor.data(), valor.data() + valor.size(), kb);
    if (r.ec != std::errc()) return -1;
    return kb * 1024;
}

}  // namespace

int64_t memoria_residente_bytes() {
    std::string valor;
    if (!valor_de_campo("/proc/self/status", "VmRSS", valor)) return -1;
    return kb_para_bytes(valor);
}

int64_t meminfo_bytes(const std::string& campo) {
    std::string valor;
    if (!valor_de_campo("/proc/meminfo", campo, valor)) return -1;
    return kb_para_bytes(valor);
}

std::string modelo_cpu() {
    std::string valor;
    if (!valor_de_campo("/proc/cpuinfo", "model name", valor)) return "desconhecido";
    return valor;
}

std::string versao_compilador() {
#if defined(__clang__)
    return std::string("clang ") + __VERSION__;
#elif defined(__GNUC__)
    return std::string("g++ ") + __VERSION__;
#else
    return "desconhecido";
#endif
}

std::string data_hora_atual() {
    std::time_t agora = std::time(nullptr);
    std::tm local{};
    localtime_r(&agora, &local);
    char buffer[32];
    std::strftime(buffer, sizeof buffer, "%Y-%m-%d %H:%M:%S", &local);
    return buffer;
}

int32_t ler_num_vertices(const std::string& caminho) {
    std::unique_ptr<std::FILE, int (*)(std::FILE*)> arquivo(std::fopen(caminho.c_str(), "rb"), &std::fclose);
    if (!arquivo) {
        throw ErroGrafo("nao foi possivel abrir '" + caminho + "': " + std::strerror(errno));
    }
    // Le blocos ate ter a primeira linha nao vazia completa (ou o fim do arquivo).
    std::string lido;
    char bloco[4096];
    bool linha_nao_vazia = false;
    bool pronto = false;
    while (!pronto) {
        const size_t k = std::fread(bloco, 1, sizeof bloco, arquivo.get());
        if (k == 0) break;
        const size_t inicio = lido.size();
        lido.append(bloco, k);
        for (size_t i = inicio; i < lido.size(); ++i) {
            const char c = lido[i];
            if (c == '\n') {
                if (linha_nao_vazia) {
                    lido.resize(i + 1);
                    pronto = true;
                    break;
                }
            } else if (c != ' ' && c != '\t' && c != '\r' && c != '\v' && c != '\f') {
                linha_nao_vazia = true;
            }
        }
    }
    return parsear_arestas(lido.data(), lido.size()).n;
}

namespace {

Viabilidade checar_viabilidade(size_t bytes_estimados) {
    Viabilidade v{};
    v.bytes_estimados = bytes_estimados;
    v.bytes_disponiveis = meminfo_bytes("MemAvailable");
    v.bytes_limite = v.bytes_disponiveis < 0 ? -1 : static_cast<int64_t>(0.8 * static_cast<double>(v.bytes_disponiveis));
    v.viavel = v.bytes_limite >= 0 && v.bytes_estimados <= static_cast<size_t>(v.bytes_limite);
    return v;
}

}  // namespace

Viabilidade checar_viabilidade_matriz(int32_t n) {
    return checar_viabilidade(estimar_bytes_matriz(n));
}

Viabilidade checar_viabilidade_lista(int32_t n) {
    return checar_viabilidade(estimar_bytes_lista_minimo(n));
}

std::vector<int32_t> sortear_vertices(int32_t n, int32_t k, uint64_t semente) {
    const int32_t total = k < n ? k : n;
    std::vector<int32_t> sorteados;
    sorteados.reserve(static_cast<size_t>(total > 0 ? total : 0));
    if (total <= 0) return sorteados;

    std::mt19937_64 gerador(semente);
    const uint64_t nn = static_cast<uint64_t>(n);
    // 2^64 mod n sem overflow: rejeitar x < descarte deixa 2^64 - descarte valores, multiplo de n.
    const uint64_t descarte = (0 - nn) % nn;
    std::vector<bool> escolhido(static_cast<size_t>(n) + 1, false);
    while (static_cast<int32_t>(sorteados.size()) < total) {
        uint64_t x = gerador();
        if (x < descarte) continue;
        const int32_t v = static_cast<int32_t>(x % nn) + 1;
        if (escolhido[static_cast<size_t>(v)]) continue;
        escolhido[static_cast<size_t>(v)] = true;
        sorteados.push_back(v);
    }
    return sorteados;
}

std::string formatar_cientifico(double x) {
    char buffer[64];
    auto r = std::to_chars(buffer, buffer + sizeof buffer, x, std::chars_format::scientific, 9);
    return std::string(buffer, r.ptr);
}

std::string formatar_fixo(double x, int casas) {
    char buffer[128];
    auto r = std::to_chars(buffer, buffer + sizeof buffer, x, std::chars_format::fixed, casas);
    return std::string(buffer, r.ptr);
}

}  // namespace grafos
