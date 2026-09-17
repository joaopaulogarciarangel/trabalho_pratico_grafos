// Programa `estudos`: estudos de caso de um grafo em uma representacao (secao 10.2).
// Grava <saida>/<grafo>_<repr>.csv (chave,valor) e <saida>/<grafo>_<repr>_estatisticas.txt.

#include <unistd.h>

#include <charconv>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "grafos/grafos.hpp"
#include "grafos/sistema.hpp"

namespace {

const char* const AJUDA = R"(uso:
  estudos --grafo <arquivo> --repr lista|matriz --saida <pasta>
          [--semente 42] [--pausar] [--limite-ifub 1800]

Processa um grafo em uma representacao e grava <pasta>/<grafo>_<repr>.csv.
)";

constexpr double MB = 1024.0 * 1024.0;
constexpr int32_t NUM_BUSCAS = 100;

class ErroUso : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

struct Opcoes {
    std::string grafo;
    std::string repr;
    std::string saida;
    uint64_t semente = 42;
    bool pausar = false;
    double limite_ifub = 1800.0;
};

Opcoes ler_opcoes(int argc, char** argv) {
    Opcoes o;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        auto valor = [&]() {
            if (i + 1 >= argc) throw ErroUso("opcao " + a + " exige um valor");
            return std::string(argv[++i]);
        };
        if (a == "--grafo") {
            o.grafo = valor();
        } else if (a == "--repr") {
            o.repr = valor();
        } else if (a == "--saida") {
            o.saida = valor();
        } else if (a == "--semente") {
            const std::string s = valor();
            auto r = std::from_chars(s.data(), s.data() + s.size(), o.semente);
            if (r.ec != std::errc() || r.ptr != s.data() + s.size()) throw ErroUso("semente invalida: '" + s + "'");
        } else if (a == "--pausar") {
            o.pausar = true;
        } else if (a == "--limite-ifub") {
            const std::string s = valor();
            size_t usados = 0;
            try {
                o.limite_ifub = std::stod(s, &usados);
            } catch (const std::exception&) {
                usados = 0;
            }
            if (usados != s.size() || !(o.limite_ifub >= 0)) throw ErroUso("limite invalido: '" + s + "'");
        } else {
            throw ErroUso("opcao desconhecida: " + a);
        }
    }
    if (o.grafo.empty() || o.saida.empty()) throw ErroUso("--grafo e --saida sao obrigatorios");
    if (o.repr != "lista" && o.repr != "matriz") throw ErroUso("--repr deve ser lista ou matriz");
    return o;
}

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

void secao(const char* titulo) { std::cout << "\n[" << titulo << "]\n"; }

struct MediaDesvio {
    double media;
    double desvio;  // amostral (n - 1)
};

MediaDesvio media_desvio(const std::vector<double>& x) {
    double soma = 0;
    for (double v : x) soma += v;
    const double media = soma / static_cast<double>(x.size());
    double quad = 0;
    for (double v : x) quad += (v - media) * (v - media);
    const double desvio = x.size() > 1 ? std::sqrt(quad / static_cast<double>(x.size() - 1)) : 0.0;
    return {media, desvio};
}

template <class Busca>
std::vector<double> cronometrar(const grafos::Grafo& g, const std::vector<int32_t>& vertices, Busca busca) {
    std::vector<double> tempos;
    tempos.reserve(vertices.size());
    for (int32_t v : vertices) {
        const auto inicio = std::chrono::steady_clock::now();
        grafos::ArvoreBusca arvore = busca(g, v);
        const auto fim = std::chrono::steady_clock::now();
        tempos.push_back(std::chrono::duration<double>(fim - inicio).count());
        (void)arvore;  // destruida fora do intervalo cronometrado
    }
    return tempos;
}

std::string texto_pai(const grafos::ArvoreBusca& a, int32_t v, int32_t n) {
    if (v > n) return "inexistente";
    const int32_t p = a.pai[static_cast<size_t>(v - 1)];
    return p < 0 ? "nao_alcancado" : std::to_string(p);
}

std::string lista_texto(const std::vector<int32_t>& v) {
    std::string s;
    for (size_t i = 0; i < v.size(); ++i) {
        if (i > 0) s += ' ';
        s += std::to_string(v[i]);
    }
    return s;
}

std::string sim_nao(bool b) { return b ? "sim" : "nao"; }

void registrar_diametro(Registro& r, const std::string& prefixo, const grafos::Diametro& d) {
    r.adicionar(prefixo + "_valor", d.valor);
    r.adicionar(prefixo + "_limite_inferior", d.limite_inferior);
    r.adicionar(prefixo + "_limite_superior", d.limite_superior);
    r.adicionar(prefixo + "_u", d.u);
    r.adicionar(prefixo + "_v", d.v);
    r.adicionar(prefixo + "_exato", sim_nao(d.exato));
    r.adicionar(prefixo + "_tempo_esgotado", sim_nao(d.tempo_esgotado));
    r.adicionar(prefixo + "_num_bfs", d.num_bfs);
    r.adicionar(prefixo + "_segundos", grafos::formatar_fixo(d.segundos, 6));
}

int executar(const Opcoes& o) {
    const std::filesystem::path arquivo(o.grafo);
    std::string nome = arquivo.filename().string();
    if (nome.size() > 4 && nome.compare(nome.size() - 4, 4, ".txt") == 0) nome.resize(nome.size() - 4);
    std::filesystem::create_directories(o.saida);
    const std::filesystem::path csv = std::filesystem::path(o.saida) / (nome + "_" + o.repr + ".csv");
    const std::filesystem::path estatisticas_txt = std::filesystem::path(o.saida) / (nome + "_" + o.repr + "_estatisticas.txt");
    const bool eh_lista = o.repr == "lista";
    const grafos::Representacao rep = eh_lista ? grafos::Representacao::Lista : grafos::Representacao::Matriz;

    Registro r;
    secao("execucao");
    r.adicionar("data_hora", grafos::data_hora_atual());
    r.adicionar("grafo", nome);
    r.adicionar("arquivo", o.grafo);
    r.adicionar("representacao", o.repr);
    r.adicionar("semente", std::to_string(o.semente));
    r.adicionar("limite_ifub_s", grafos::formatar_fixo(o.limite_ifub, 1));
    r.adicionar("cpu", grafos::modelo_cpu());
    r.adicionar_mb("memtotal_mb", static_cast<double>(grafos::meminfo_bytes("MemTotal")));
    r.adicionar("compilador", grafos::versao_compilador());

    // Estudo 1: memoria.
    secao("estudo 1: memoria");
    const int64_t rss_base = grafos::memoria_residente_bytes();
    if (eh_lista) {
        // Sem CSV: um n absurdo e erro do arquivo, nao resultado de estudo.
        const int32_t n = grafos::ler_num_vertices(o.grafo);
        const grafos::Viabilidade v = grafos::checar_viabilidade_lista(n);
        if (!v.viavel) {
            throw grafos::ErroGrafo("lista exigiria pelo menos " + grafos::formatar_fixo(static_cast<double>(v.bytes_estimados) / MB, 1) +
                                    " MB (n = " + std::to_string(n) + "); memoria disponivel " +
                                    grafos::formatar_fixo(static_cast<double>(v.bytes_disponiveis) / MB, 1) + " MB");
        }
    } else {
        const int32_t n = grafos::ler_num_vertices(o.grafo);
        const grafos::Viabilidade v = grafos::checar_viabilidade_matriz(n);
        r.adicionar_mb("matriz_estimada_mb", static_cast<double>(v.bytes_estimados));
        r.adicionar_mb("memavailable_mb", static_cast<double>(v.bytes_disponiveis));
        if (!v.viavel) {
            r.adicionar("n", n);
            r.adicionar("memoria_status", "inviavel");
            r.gravar(csv);
            std::cout << "\nmatriz inviavel: nao carregada. CSV: " << csv.string() << "\n";
            return 0;
        }
    }
    grafos::InfoLeitura info;
    const auto inicio_carga = std::chrono::steady_clock::now();
    grafos::Grafo g = grafos::carregar(o.grafo, rep, false, &info);
    const double tempo_carga = std::chrono::duration<double>(std::chrono::steady_clock::now() - inicio_carga).count();
    const int64_t rss_depois = grafos::memoria_residente_bytes();
    const int32_t n = g.n();

    r.adicionar("memoria_status", "ok");
    r.adicionar("n", n);
    r.adicionar("m", g.m());
    r.adicionar("linhas_aresta", info.linhas_aresta);
    r.adicionar("lacos_ignorados", info.lacos_ignorados);
    r.adicionar("repetidas_ignoradas", info.repetidas_ignoradas);
    r.adicionar_mb("rss_base_mb", static_cast<double>(rss_base));
    r.adicionar_mb("rss_depois_mb", static_cast<double>(rss_depois));
    r.adicionar_mb("rss_diferenca_mb", static_cast<double>(rss_depois - rss_base));
    r.adicionar_mb("estrutura_mb", static_cast<double>(g.bytes_estrutura()));
    r.adicionar("estrutura_bytes", static_cast<int64_t>(g.bytes_estrutura()));
    r.adicionar("tempo_carga_s", grafos::formatar_fixo(tempo_carga, 6));

    if (o.pausar) {
        std::cout << "\nPID " << getpid() << ": grafo carregado. Confira a memoria com top/ps e tecle Enter..."
                  << std::flush;
        std::string linha;
        std::getline(std::cin, linha);
    }

    // Estudos 2 e 3: tempo medio de BFS e DFS.
    secao("estudos 2 e 3: tempo de BFS e DFS");
    const std::vector<int32_t> sorteados = grafos::sortear_vertices(n, NUM_BUSCAS, o.semente);
    {
        const grafos::ArvoreBusca aquecimento = grafos::bfs(g, sorteados.front());
        (void)aquecimento;
    }
    const std::vector<double> tempos_bfs = cronometrar(g, sorteados, [](const grafos::Grafo& gr, int32_t v) {
        return grafos::bfs(gr, v);
    });
    const std::vector<double> tempos_dfs = cronometrar(g, sorteados, [](const grafos::Grafo& gr, int32_t v) {
        return grafos::dfs(gr, v);
    });
    const MediaDesvio bfs_md = media_desvio(tempos_bfs);
    const MediaDesvio dfs_md = media_desvio(tempos_dfs);
    r.adicionar("num_buscas", static_cast<int64_t>(sorteados.size()));
    r.adicionar_segundos("bfs_media_s", bfs_md.media);
    r.adicionar_segundos("bfs_desvio_s", bfs_md.desvio);
    r.adicionar_segundos("dfs_media_s", dfs_md.media);
    r.adicionar_segundos("dfs_desvio_s", dfs_md.desvio);
    r.adicionar("vertices_sorteados", lista_texto(sorteados));

    // Estudo 4: pais de 10, 20 e 30 nas arvores a partir de 1, 2 e 3.
    secao("estudo 4: pais");
    for (int32_t raiz : {1, 2, 3}) {
        std::vector<std::pair<std::string, grafos::ArvoreBusca>> arvores;
        if (raiz <= n) {
            arvores.emplace_back("bfs", grafos::bfs(g, raiz));
            arvores.emplace_back("dfs", grafos::dfs(g, raiz));
        }
        for (const char* tipo : {"bfs", "dfs"}) {
            for (int32_t v : {10, 20, 30}) {
                const std::string chave =
                    std::string("pai_") + tipo + "_raiz" + std::to_string(raiz) + "_v" + std::to_string(v);
                if (raiz > n) {
                    r.adicionar(chave, "inexistente");
                    continue;
                }
                const grafos::ArvoreBusca& a = std::string(tipo) == "bfs" ? arvores[0].second : arvores[1].second;
                r.adicionar(chave, texto_pai(a, v, n));
            }
        }
    }

    // Estudo 5: distancias.
    secao("estudo 5: distancias");
    for (auto [a, b] : {std::pair<int32_t, int32_t>{10, 20}, {10, 30}, {20, 30}}) {
        const std::string chave = "dist_" + std::to_string(a) + "_" + std::to_string(b);
        if (a > n || b > n) {
            r.adicionar(chave, "inexistente");
            continue;
        }
        const int32_t d = grafos::distancia(g, a, b);
        r.adicionar(chave, d < 0 ? std::string("infinito") : std::to_string(d));
    }

    // Estudo 6: estatisticas e componentes.
    secao("estudo 6: componentes");
    const grafos::Estatisticas e = grafos::estatisticas(g);
    const grafos::Componentes comps = grafos::componentes_conexas(g);
    r.adicionar("grau_minimo", e.grau_minimo);
    r.adicionar("grau_maximo", e.grau_maximo);
    r.adicionar("grau_medio", grafos::formatar_fixo(e.grau_medio, 4));
    r.adicionar("mediana_grau", grafos::formatar_fixo(e.mediana_grau, 1));
    r.adicionar("num_componentes", static_cast<int64_t>(comps.lista.size()));
    r.adicionar("maior_componente", comps.lista.front().tamanho);
    r.adicionar("menor_componente", comps.lista.back().tamanho);
    grafos::escrever_estatisticas(e, comps, estatisticas_txt.string());
    r.adicionar("arquivo_estatisticas", estatisticas_txt.filename().string());

    // Estudo 7: diametro, so na lista.
    secao("estudo 7: diametro");
    if (eh_lista) {
        registrar_diametro(r, "diam_aprox", grafos::diametro_aproximado(g));
        registrar_diametro(r, "diam_exato", grafos::diametro_exato(g, o.limite_ifub));
    } else {
        r.adicionar("diametro_status", "nao_calculado_na_matriz");
    }

    r.gravar(csv);
    std::cout << "\nCSV: " << csv.string() << "\n";
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "--help" || a == "-h") {
            std::cout << AJUDA;
            return 0;
        }
    }
    try {
        return executar(ler_opcoes(argc, argv));
    } catch (const ErroUso& e) {
        std::cerr << "erro: " << e.what() << "\n" << AJUDA;
        return 2;
    } catch (const std::bad_alloc&) {
        std::cerr << "erro: memoria insuficiente\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "erro: " << e.what() << "\n";
        return 1;
    }
}
