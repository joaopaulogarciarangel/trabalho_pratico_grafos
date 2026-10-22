#pragma once

// Pecas comuns aos programas de estudos de caso (`estudos` e `estudos_dijkstra`).

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "grafos/erro.hpp"
#include "grafos/sistema.hpp"

namespace comum {

constexpr double MB = 1024.0 * 1024.0;

// Linhas chave,valor na ordem de insercao; tambem ecoa um resumo legivel no terminal.
class Registro {
public:
    void adicionar(const std::string& chave, const std::string& valor) {
        std::string limpo = valor;
        for (char& c : limpo) {
            if (c == ',' || c == '\n' || c == '\r') c = ' ';
        }
        linhas_.emplace_back(chave, limpo);
        std::cout << "  " << chave << ": " << limpo << "\n";
    }
    void adicionar(const std::string& chave, int64_t valor) { adicionar(chave, std::to_string(valor)); }
    void adicionar_mb(const std::string& chave, double bytes) { adicionar(chave, grafos::formatar_fixo(bytes / MB, 3)); }
    void adicionar_segundos(const std::string& chave, double s) { adicionar(chave, grafos::formatar_cientifico(s)); }

    // Grava em .tmp e renomeia: um processo interrompido nao deixa CSV "completo".
    void gravar(const std::filesystem::path& caminho) const {
        const std::filesystem::path temporario = caminho.string() + ".tmp";
        {
            std::ofstream f(temporario, std::ios::binary | std::ios::trunc);
            if (!f) throw grafos::ErroGrafo("nao foi possivel criar '" + temporario.string() + "'");
            f << "chave,valor\n";
            for (const auto& [chave, valor] : linhas_) f << chave << ',' << valor << '\n';
            f.flush();
            if (!f) throw grafos::ErroGrafo("erro ao escrever '" + temporario.string() + "'");
        }
        std::filesystem::rename(temporario, caminho);
    }

private:
    std::vector<std::pair<std::string, std::string>> linhas_;
};

inline void secao(const char* titulo) { std::cout << "\n[" << titulo << "]\n"; }

struct MediaDesvio {
    double media;
    double desvio;  // amostral (n - 1)
};

inline MediaDesvio media_desvio(const std::vector<double>& x) {
    double soma = 0;
    for (double v : x) soma += v;
    const double media = soma / static_cast<double>(x.size());
    double quad = 0;
    for (double v : x) quad += (v - media) * (v - media);
    const double desvio = x.size() > 1 ? std::sqrt(quad / static_cast<double>(x.size() - 1)) : 0.0;
    return {media, desvio};
}

inline std::string lista_texto(const std::vector<int32_t>& v) {
    std::string s;
    for (size_t i = 0; i < v.size(); ++i) {
        if (i > 0) s += ' ';
        s += std::to_string(v[i]);
    }
    return s;
}

// Registra as informacoes da maquina e do compilador, iguais nos dois programas.
inline void registrar_ambiente(Registro& r) {
    r.adicionar("cpu", grafos::modelo_cpu());
    r.adicionar_mb("memtotal_mb", static_cast<double>(grafos::meminfo_bytes("MemTotal")));
    r.adicionar("compilador", grafos::versao_compilador());
}

}  // namespace comum
