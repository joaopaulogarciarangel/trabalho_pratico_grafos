#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace grafos {

// VmRSS do proprio processo (/proc/self/status), em bytes.
int64_t memoria_residente_bytes();

// Campo de /proc/meminfo (ex.: "MemAvailable", "MemTotal"), em bytes; -1 se ausente.
int64_t meminfo_bytes(const std::string& campo);

// "model name" de /proc/cpuinfo; "desconhecido" se ausente.
std::string modelo_cpu();

std::string versao_compilador();

// Data e hora local no formato AAAA-MM-DD HH:MM:SS.
std::string data_hora_atual();

// Le so o n do arquivo (primeira linha nao vazia, primeiro numero), com as mesmas regras e
// mensagens de erro da leitura completa.
int32_t ler_num_vertices(const std::string& caminho);

struct Viabilidade {
    bool viavel;
    size_t bytes_estimados;     // estimativa da estrutura
    int64_t bytes_disponiveis;  // MemAvailable
    int64_t bytes_limite;       // 80% de MemAvailable
};

// Compara a estimativa com 80% de MemAvailable, antes de alocar.
Viabilidade checar_viabilidade_matriz(int32_t n);  // estimar_bytes_matriz(n)
Viabilidade checar_viabilidade_lista(int32_t n);   // estimar_bytes_lista_minimo(n)

// Sorteia min(k, n) vertices distintos de 1..n com mt19937_64 e rejeicao (sem
// uniform_int_distribution, para ser reprodutivel entre compiladores). Ordem do sorteio.
std::vector<int32_t> sortear_vertices(int32_t n, int32_t k, uint64_t semente);

// Numero em notacao cientifica com 9 casas ("1.234567890e-03"), independente de locale.
std::string formatar_cientifico(double x);

// Numero com casas fixas, independente de locale.
std::string formatar_fixo(double x, int casas);

}  // namespace grafos
