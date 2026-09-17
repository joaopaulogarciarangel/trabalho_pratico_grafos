// Programa de linha de comando `grafos`: estatisticas, BFS, DFS, distancia e diametro.

#include <charconv>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "grafos/grafos.hpp"
#include "grafos/sistema.hpp"

namespace {

const char* const AJUDA = R"(uso:
  grafos estatisticas <arquivo> [--repr lista|matriz] -o <saida>
      grava estatisticas de grau e componentes conexas em <saida>
  grafos bfs <arquivo> --origem <v> [--repr lista|matriz] -o <saida>
  grafos dfs <arquivo> --origem <v> [--repr lista|matriz] -o <saida>
      grava a arvore de busca (vertice pai nivel ordem) em <saida>
  grafos distancia <arquivo> <u> <v> [--repr lista|matriz] [--caminho]
      imprime a distancia entre u e v ("infinito" se nao houver caminho)
  grafos diametro <arquivo> [--repr lista|matriz] [--ingenuo | --aproximado | --exato] [--limite <segundos>]
      imprime o diametro (padrao: --exato com limite de 1800 s)
  grafos --help

Vertices numerados de 1 a n. Representacao padrao: lista.
)";

class ErroUso : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

struct Opcoes {
    std::string comando;
    std::vector<std::string> posicionais;
    std::string repr = "lista";
    std::optional<std::string> saida;
    std::optional<std::string> origem;
    std::optional<std::string> limite;
    std::vector<std::string> algoritmos;
    bool caminho = false;
};

Opcoes ler_opcoes(int argc, char** argv) {
    Opcoes o;
    o.comando = argv[1];
    for (int i = 2; i < argc; ++i) {
        const std::string a = argv[i];
        auto valor = [&](const std::string& nome) {
            if (i + 1 >= argc) throw ErroUso("opcao " + nome + " exige um valor");
            return std::string(argv[++i]);
        };
        if (a == "--repr") {
            o.repr = valor(a);
        } else if (a == "-o") {
            o.saida = valor(a);
        } else if (a == "--origem") {
            o.origem = valor(a);
        } else if (a == "--limite") {
            o.limite = valor(a);
        } else if (a == "--caminho") {
            o.caminho = true;
        } else if (a == "--ingenuo" || a == "--aproximado" || a == "--exato") {
            o.algoritmos.push_back(a);
        } else if (a.size() > 1 && a[0] == '-' && !(a[1] >= '0' && a[1] <= '9')) {
            throw ErroUso("opcao desconhecida: " + a);
        } else {
            o.posicionais.push_back(a);
        }
    }
    return o;
}

int32_t ler_vertice(const std::string& texto, const std::string& nome) {
    int32_t v = 0;
    auto r = std::from_chars(texto.data(), texto.data() + texto.size(), v);
    if (r.ec != std::errc() || r.ptr != texto.data() + texto.size()) {
        throw ErroUso(nome + " invalido: '" + texto + "'");
    }
    return v;
}

double ler_segundos(const std::string& texto) {
    size_t usados = 0;
    double s = -1;
    try {
        s = std::stod(texto, &usados);
    } catch (const std::exception&) {
        usados = 0;
    }
    if (usados != texto.size() || !(s >= 0)) throw ErroUso("limite invalido: '" + texto + "'");
    return s;
}

void exigir_posicionais(const Opcoes& o, size_t quantidade, const char* formato) {
    if (o.posicionais.size() != quantidade) {
        throw ErroUso(std::string("argumentos invalidos para '") + o.comando + "'; uso: grafos " + formato);
    }
}

std::string formatar_gb(double bytes) {
    return grafos::formatar_fixo(bytes / (1024.0 * 1024.0 * 1024.0), 1) + " GB";
}

grafos::Grafo carregar_com_resumo(const std::string& arquivo, const std::string& repr_texto) {
    grafos::Representacao rep;
    if (repr_texto == "lista") {
        rep = grafos::Representacao::Lista;
    } else if (repr_texto == "matriz") {
        rep = grafos::Representacao::Matriz;
    } else {
        throw ErroUso("representacao invalida: '" + repr_texto + "' (use lista ou matriz)");
    }
    // Checa a viabilidade so pelo n, antes de alocar (um OOM no WSL derruba o servico inteiro).
    const int32_t n = grafos::ler_num_vertices(arquivo);
    const bool matriz = rep == grafos::Representacao::Matriz;
    const grafos::Viabilidade v = matriz ? grafos::checar_viabilidade_matriz(n) : grafos::checar_viabilidade_lista(n);
    if (!v.viavel) {
        throw grafos::ErroGrafo(std::string(matriz ? "matriz exigiria " : "lista exigiria pelo menos ") +
                                formatar_gb(static_cast<double>(v.bytes_estimados)) + " (n = " + std::to_string(n) +
                                "); memoria disponivel " + formatar_gb(static_cast<double>(v.bytes_disponiveis)) +
                                " (limite de 80%: " + formatar_gb(static_cast<double>(v.bytes_limite)) + ")." +
                                (matriz ? " Use --repr lista." : ""));
    }
    grafos::InfoLeitura info;
    grafos::Grafo g = grafos::carregar(arquivo, rep, false, &info);
    std::cout << "grafo: " << arquivo << " (" << repr_texto << ")\n"
              << "  n = " << g.n() << ", m = " << g.m() << "\n"
              << "  linhas de aresta = " << info.linhas_aresta << ", lacos ignorados = " << info.lacos_ignorados
              << ", repetidas ignoradas = " << info.repetidas_ignoradas << "\n";
    return g;
}

class Cronometro {
public:
    Cronometro() : inicio_(std::chrono::steady_clock::now()) {}
    double segundos() const {
        return std::chrono::duration<double>(std::chrono::steady_clock::now() - inicio_).count();
    }

private:
    std::chrono::steady_clock::time_point inicio_;
};

void imprimir_tempo(double segundos) {
    std::cout << "tempo de calculo (sem leitura/escrita): " << grafos::formatar_fixo(segundos, 6) << " s\n";
}

int executar(const Opcoes& o) {
    if (o.comando == "estatisticas") {
        exigir_posicionais(o, 1, "estatisticas <arquivo> [--repr lista|matriz] -o <saida>");
        if (!o.saida) throw ErroUso("estatisticas exige -o <saida>");
        grafos::Grafo g = carregar_com_resumo(o.posicionais[0], o.repr);
        Cronometro c;
        const grafos::Estatisticas e = grafos::estatisticas(g);
        const grafos::Componentes comp = grafos::componentes_conexas(g);
        imprimir_tempo(c.segundos());
        grafos::escrever_estatisticas(e, comp, *o.saida);
        std::cout << "componentes: " << comp.lista.size() << "\nsaida: " << *o.saida << "\n";
        return 0;
    }
    if (o.comando == "bfs" || o.comando == "dfs") {
        exigir_posicionais(o, 1, (o.comando + " <arquivo> --origem <v> [--repr lista|matriz] -o <saida>").c_str());
        if (!o.origem) throw ErroUso(o.comando + " exige --origem <v>");
        if (!o.saida) throw ErroUso(o.comando + " exige -o <saida>");
        const int32_t origem = ler_vertice(*o.origem, "vertice de origem");
        grafos::Grafo g = carregar_com_resumo(o.posicionais[0], o.repr);
        Cronometro c;
        const grafos::ArvoreBusca a = o.comando == "bfs" ? grafos::bfs(g, origem) : grafos::dfs(g, origem);
        imprimir_tempo(c.segundos());
        grafos::escrever_arvore(a, *o.saida);
        std::cout << "saida: " << *o.saida << "\n";
        return 0;
    }
    if (o.comando == "distancia") {
        exigir_posicionais(o, 3, "distancia <arquivo> <u> <v> [--repr lista|matriz] [--caminho]");
        const int32_t u = ler_vertice(o.posicionais[1], "vertice u");
        const int32_t v = ler_vertice(o.posicionais[2], "vertice v");
        grafos::Grafo g = carregar_com_resumo(o.posicionais[0], o.repr);
        Cronometro c;
        if (o.caminho) {
            const std::vector<int32_t> caminho = grafos::caminho_minimo(g, u, v);
            imprimir_tempo(c.segundos());
            std::cout << "distancia(" << u << ", " << v << ") = ";
            if (caminho.empty()) {
                std::cout << "infinito\ncaminho: nenhum\n";
            } else {
                std::cout << caminho.size() - 1 << "\ncaminho:";
                for (int32_t x : caminho) std::cout << ' ' << x;
                std::cout << "\n";
            }
        } else {
            const int32_t d = grafos::distancia(g, u, v);
            imprimir_tempo(c.segundos());
            std::cout << "distancia(" << u << ", " << v << ") = ";
            if (d < 0) {
                std::cout << "infinito\n";
            } else {
                std::cout << d << "\n";
            }
        }
        return 0;
    }
    if (o.comando == "diametro") {
        exigir_posicionais(o, 1, "diametro <arquivo> [--repr ...] [--ingenuo | --aproximado | --exato] [--limite <segundos>]");
        if (o.algoritmos.size() > 1) throw ErroUso("escolha apenas um de --ingenuo, --aproximado, --exato");
        const std::string algoritmo = o.algoritmos.empty() ? "--exato" : o.algoritmos[0];
        if (o.limite && algoritmo != "--exato") throw ErroUso("--limite so vale com --exato");
        const double limite = o.limite ? ler_segundos(*o.limite) : 1800.0;
        grafos::Grafo g = carregar_com_resumo(o.posicionais[0], o.repr);
        grafos::Diametro d;
        if (algoritmo == "--ingenuo") {
            d = grafos::diametro_ingenuo(g);
        } else if (algoritmo == "--aproximado") {
            d = grafos::diametro_aproximado(g);
        } else {
            d = grafos::diametro_exato(g, limite);
        }
        imprimir_tempo(d.segundos);
        std::cout << "algoritmo: " << algoritmo.substr(2);
        if (algoritmo == "--exato") std::cout << " (limite " << grafos::formatar_fixo(limite, 1) << " s)";
        std::cout << "\ndiametro: " << d.valor << (d.exato ? "" : " (limite inferior)") << "\n"
                  << "limites: [" << d.limite_inferior << ", " << d.limite_superior << "]\n"
                  << "par: " << d.u << " " << d.v << "\n"
                  << "exato: " << (d.exato ? "sim" : "nao") << "\n"
                  << "tempo_esgotado: " << (d.tempo_esgotado ? "sim" : "nao") << "\n"
                  << "num_bfs: " << d.num_bfs << "\n"
                  << "segundos: " << grafos::formatar_fixo(d.segundos, 6) << "\n";
        return 0;
    }
    throw ErroUso("comando desconhecido: '" + o.comando + "'");
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << AJUDA;
        return 2;
    }
    const std::string primeiro = argv[1];
    if (primeiro == "--help" || primeiro == "-h" || primeiro == "help") {
        std::cout << AJUDA;
        return 0;
    }
    try {
        return executar(ler_opcoes(argc, argv));
    } catch (const ErroUso& e) {
        std::cerr << "erro: " << e.what() << "\n(use grafos --help)\n";
        return 2;
    } catch (const std::bad_alloc&) {
        std::cerr << "erro: memoria insuficiente\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "erro: " << e.what() << "\n";
        return 1;
    }
}
