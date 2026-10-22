// Programa `estudos_dijkstra`: estudos de caso da Parte 2 (secao 5.2 da especificacao).
// Modo grafo: grava <saida>/<grafo>_dijkstra.csv (estudos P2-1 e P2-2).
// Modo rede:  grava <saida>/<rede>_dijkstra.csv (estudo P2-3).

#include <algorithm>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "comum.hpp"
#include "grafos/grafos.hpp"
#include "grafos/sistema.hpp"

namespace {

const char* const AJUDA = R"(uso:
  estudos_dijkstra --grafo <arquivo> --saida <pasta> [--semente 42]
                   [--k-heap 100] [--k-vetor 100] [--estimar-vetor <fracao>]
  estudos_dijkstra --rede <arquivo> --nomes <arquivo> --saida <pasta>

Modo grafo: distancia e caminho de 10 a 20, 30, 40, 50 e 60 (heap) e tempo medio do
Dijkstra com heap (k-heap origens sorteadas) e com vetor (as primeiras k-vetor delas).
--estimar-vetor f (so com --k-vetor 0) estima o tempo do vetor por uma execucao
interrompida apos uma fracao f dos vertices explorados.
Modo rede: distancia e caminho de Edsger W. Dijkstra a cinco pesquisadores.
Sempre na lista de adjacencia. Grava <pasta>/<nome>_dijkstra.csv.
)";

using comum::media_desvio;
using comum::Registro;
using comum::secao;

class ErroUso : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

struct Opcoes {
    std::string grafo;
    std::string rede;
    std::string nomes;
    std::string saida;
    uint64_t semente = 42;
    int32_t k_heap = 100;
    int32_t k_vetor = 100;
    std::optional<double> estimar_vetor;
};

template <class T>
T ler_inteiro(const std::string& texto, const std::string& opcao) {
    T x{};
    auto r = std::from_chars(texto.data(), texto.data() + texto.size(), x);
    if (r.ec != std::errc() || r.ptr != texto.data() + texto.size()) {
        throw ErroUso(opcao + " invalido: '" + texto + "'");
    }
    return x;
}

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
        } else if (a == "--rede") {
            o.rede = valor();
        } else if (a == "--nomes") {
            o.nomes = valor();
        } else if (a == "--saida") {
            o.saida = valor();
        } else if (a == "--semente") {
            o.semente = ler_inteiro<uint64_t>(valor(), a);
        } else if (a == "--k-heap") {
            o.k_heap = ler_inteiro<int32_t>(valor(), a);
        } else if (a == "--k-vetor") {
            o.k_vetor = ler_inteiro<int32_t>(valor(), a);
        } else if (a == "--estimar-vetor") {
            const std::string s = valor();
            double f = -1;
            auto r = std::from_chars(s.data(), s.data() + s.size(), f);
            if (r.ec != std::errc() || r.ptr != s.data() + s.size() || !(f > 0 && f <= 1)) {
                throw ErroUso("fracao invalida: '" + s + "' (use um valor em (0, 1])");
            }
            o.estimar_vetor = f;
        } else {
            throw ErroUso("opcao desconhecida: " + a);
        }
    }
    if (o.saida.empty()) throw ErroUso("--saida e obrigatorio");
    if (o.grafo.empty() == o.rede.empty()) throw ErroUso("use exatamente um de --grafo e --rede");
    if (!o.rede.empty() && o.nomes.empty()) throw ErroUso("--rede exige --nomes");
    if (o.k_heap < 1) throw ErroUso("--k-heap deve ser pelo menos 1");
    if (o.k_vetor < 0 || o.k_vetor > o.k_heap) throw ErroUso("--k-vetor deve estar entre 0 e --k-heap");
    if (o.estimar_vetor && o.k_vetor != 0) throw ErroUso("--estimar-vetor so vale com --k-vetor 0");
    return o;
}

std::string nome_base(const std::string& caminho) {
    std::string nome = std::filesystem::path(caminho).filename().string();
    if (nome.size() > 4 && nome.compare(nome.size() - 4, 4, ".txt") == 0) nome.resize(nome.size() - 4);
    return nome;
}

double segundos_desde(std::chrono::steady_clock::time_point inicio) {
    return std::chrono::duration<double>(std::chrono::steady_clock::now() - inicio).count();
}

std::string sim_nao(bool b) { return b ? "sim" : "nao"; }

std::string texto_distancia(const grafos::ResultadoDijkstra& r, int32_t v) {
    return r.pai[static_cast<size_t>(v - 1)] < 0 ? std::string("infinito")
                                                  : grafos::formatar_curto(r.dist[static_cast<size_t>(v - 1)]);
}

// Carrega na lista (checando a viabilidade so pelo n antes de alocar) e registra a carga.
grafos::Grafo carregar_lista(const std::string& arquivo, Registro& r) {
    const int32_t n = grafos::ler_num_vertices(arquivo);
    const grafos::Viabilidade v = grafos::checar_viabilidade_lista(n);
    if (!v.viavel) {
        throw grafos::ErroGrafo("lista exigiria pelo menos " +
                                grafos::formatar_fixo(static_cast<double>(v.bytes_estimados) / comum::MB, 1) +
                                " MB (n = " + std::to_string(n) + "); memoria disponivel " +
                                grafos::formatar_fixo(static_cast<double>(v.bytes_disponiveis) / comum::MB, 1) + " MB");
    }
    const int64_t rss_base = grafos::memoria_residente_bytes();
    grafos::InfoLeitura info;
    const auto inicio = std::chrono::steady_clock::now();
    grafos::Grafo g = grafos::carregar(arquivo, grafos::Representacao::Lista, false, &info);
    const double tempo_carga = segundos_desde(inicio);
    const int64_t rss_depois = grafos::memoria_residente_bytes();
    r.adicionar("n", g.n());
    r.adicionar("m", g.m());
    r.adicionar("linhas_aresta", info.linhas_aresta);
    r.adicionar("lacos_ignorados", info.lacos_ignorados);
    r.adicionar("repetidas_ignoradas", info.repetidas_ignoradas);
    r.adicionar("ponderado", sim_nao(g.ponderado()));
    r.adicionar("pesos_negativos", sim_nao(g.tem_peso_negativo()));
    r.adicionar_mb("rss_base_mb", static_cast<double>(rss_base));
    r.adicionar_mb("rss_depois_mb", static_cast<double>(rss_depois));
    r.adicionar_mb("estrutura_mb", static_cast<double>(g.bytes_estrutura()));
    r.adicionar("tempo_carga_s", grafos::formatar_fixo(tempo_carga, 6));
    return g;
}

bool resultados_iguais(const grafos::ResultadoDijkstra& a, const grafos::ResultadoDijkstra& b) {
    return a.dist == b.dist && a.pai == b.pai && a.ordem == b.ordem && a.explorados == b.explorados;
}

int estudo_grafo(const Opcoes& o) {
    const std::string nome = nome_base(o.grafo);
    std::filesystem::create_directories(o.saida);
    const std::filesystem::path csv = std::filesystem::path(o.saida) / (nome + "_dijkstra.csv");

    Registro r;
    secao("execucao");
    r.adicionar("data_hora", grafos::data_hora_atual());
    r.adicionar("grafo", nome);
    r.adicionar("arquivo", o.grafo);
    r.adicionar("representacao", "lista");
    r.adicionar("semente", std::to_string(o.semente));
    r.adicionar("k_heap", o.k_heap);
    r.adicionar("k_vetor", o.k_vetor);
    comum::registrar_ambiente(r);

    secao("carga");
    const grafos::Grafo g = carregar_lista(o.grafo, r);
    const int32_t n = g.n();
    if (g.tem_peso_negativo()) {
        r.adicionar("status", "pesos_negativos_dijkstra_nao_aplicavel");
        r.gravar(csv);
        std::cout << "\npesos negativos: Dijkstra nao aplicado. CSV: " << csv.string() << "\n";
        return 0;
    }

    // Estudo P2-1: distancia e caminho de 10 a 20, 30, 40, 50 e 60.
    secao("estudo P2-1: distancias e caminhos a partir de 10");
    std::optional<grafos::ResultadoDijkstra> de10;
    if (n >= 10) de10 = grafos::dijkstra(g, 10, grafos::FilaDijkstra::Heap);
    for (int32_t destino : {20, 30, 40, 50, 60}) {
        const std::string sufixo = "_10_" + std::to_string(destino);
        if (!de10 || destino > n) {
            r.adicionar("dist" + sufixo, "inexistente");
            continue;
        }
        const std::vector<int32_t> c = grafos::caminho(*de10, destino);
        r.adicionar("dist" + sufixo, texto_distancia(*de10, destino));
        r.adicionar("arestas" + sufixo, c.empty() ? std::string("-") : std::to_string(c.size() - 1));
        r.adicionar("caminho" + sufixo, c.empty() ? std::string("-") : comum::lista_texto(c));
    }
    de10.reset();

    // Estudo P2-2: tempo medio com heap e com vetor, nas mesmas origens sorteadas.
    secao("estudo P2-2: tempo do Dijkstra (heap x vetor)");
    const std::vector<int32_t> origens = grafos::sortear_vertices(n, o.k_heap, o.semente);
    const size_t k_vetor = std::min(static_cast<size_t>(o.k_vetor), origens.size());
    {
        // Aquecimento (nao contado). O do vetor para cedo: uma execucao completa custaria
        // tanto quanto uma medicao nos grafos grandes.
        const grafos::ResultadoDijkstra h = grafos::dijkstra(g, origens.front(), grafos::FilaDijkstra::Heap);
        if (k_vetor > 0 || o.estimar_vetor) {
            const grafos::ResultadoDijkstra v = g.visitar([&](const auto& c) {
                return grafos::detalhe::dijkstra<grafos::detalhe::FronteiraVetor>(c, origens.front() - 1, 1000);
            });
        }
    }
    std::vector<double> tempos_heap;
    std::vector<double> tempos_vetor;
    int64_t explorados_primeira = 0;
    for (size_t i = 0; i < origens.size(); ++i) {
        const int32_t s = origens[i];
        auto inicio = std::chrono::steady_clock::now();
        const grafos::ResultadoDijkstra h = grafos::dijkstra(g, s, grafos::FilaDijkstra::Heap);
        tempos_heap.push_back(segundos_desde(inicio));
        if (i == 0) explorados_primeira = h.explorados;
        if (i < k_vetor) {
            inicio = std::chrono::steady_clock::now();
            const grafos::ResultadoDijkstra v = grafos::dijkstra(g, s, grafos::FilaDijkstra::Vetor);
            tempos_vetor.push_back(segundos_desde(inicio));
            if (!resultados_iguais(h, v)) {
                throw grafos::ErroGrafo("vetor e heap divergem na origem " + std::to_string(s));
            }
            std::cout << "  vetor " << i + 1 << "/" << k_vetor << " (origem " << s << "): "
                      << grafos::formatar_fixo(tempos_vetor.back(), 3) << " s\n";
        }
    }
    const comum::MediaDesvio heap = media_desvio(tempos_heap);
    r.adicionar("heap_execucoes", static_cast<int64_t>(tempos_heap.size()));
    r.adicionar_segundos("heap_media_s", heap.media);
    r.adicionar_segundos("heap_desvio_s", heap.desvio);
    r.adicionar("vetor_execucoes", static_cast<int64_t>(k_vetor));
    if (k_vetor > 0) {
        const std::vector<double> heap_mesmas(tempos_heap.begin(), tempos_heap.begin() + static_cast<long>(k_vetor));
        const comum::MediaDesvio heap_k = media_desvio(heap_mesmas);
        const comum::MediaDesvio vetor = media_desvio(tempos_vetor);
        r.adicionar_segundos("heap_media_mesmas_origens_s", heap_k.media);
        r.adicionar_segundos("heap_desvio_mesmas_origens_s", heap_k.desvio);
        r.adicionar_segundos("vetor_media_s", vetor.media);
        r.adicionar_segundos("vetor_desvio_s", vetor.desvio);
        r.adicionar("razao_vetor_heap", grafos::formatar_fixo(vetor.media / heap_k.media, 1));
        r.adicionar("conferencia_vetor_heap", "ok");
    }

    if (o.estimar_vetor) {
        // Cada extracao do vetor varre os n vertices: o tempo e quase linear no numero de
        // explorados, entao a execucao parcial e extrapolada para o total R (do heap).
        const int32_t s = origens.front();
        const auto limite = static_cast<int64_t>(std::ceil(*o.estimar_vetor * static_cast<double>(explorados_primeira)));
        const auto inicio = std::chrono::steady_clock::now();
        const grafos::ResultadoDijkstra parcial = g.visitar([&](const auto& c) {
            return grafos::detalhe::dijkstra<grafos::detalhe::FronteiraVetor>(c, s - 1, limite);
        });
        const double tempo_parcial = segundos_desde(inicio);
        const double estimado =
            tempo_parcial * static_cast<double>(explorados_primeira) / static_cast<double>(parcial.explorados);
        r.adicionar("vetor_estimado_origem", s);
        r.adicionar("vetor_estimado_fracao", grafos::formatar_curto(*o.estimar_vetor));
        r.adicionar("vetor_estimado_explorados", parcial.explorados);
        r.adicionar("vetor_estimado_explorados_total", explorados_primeira);
        r.adicionar_segundos("vetor_estimado_tempo_parcial_s", tempo_parcial);
        r.adicionar_segundos("vetor_estimado_s", estimado);
        r.adicionar("razao_vetor_heap_estimada", grafos::formatar_fixo(estimado / heap.media, 1));
    }
    r.adicionar("origens", comum::lista_texto(origens));

    r.gravar(csv);
    std::cout << "\nCSV: " << csv.string() << "\n";
    return 0;
}

int estudo_rede(const Opcoes& o) {
    const std::string nome = nome_base(o.rede);
    std::filesystem::create_directories(o.saida);
    const std::filesystem::path csv = std::filesystem::path(o.saida) / (nome + "_dijkstra.csv");

    Registro r;
    secao("execucao");
    r.adicionar("data_hora", grafos::data_hora_atual());
    r.adicionar("grafo", nome);
    r.adicionar("arquivo", o.rede);
    r.adicionar("arquivo_nomes", o.nomes);
    r.adicionar("representacao", "lista");
    comum::registrar_ambiente(r);

    secao("carga");
    const grafos::Grafo g = carregar_lista(o.rede, r);
    const grafos::NomesVertices nomes = grafos::NomesVertices::carregar(o.nomes);
    r.adicionar("nomes_lidos", nomes.tamanho());
    if (nomes.tamanho() != g.n()) r.adicionar("aviso", "numero de nomes diferente de n");

    secao("estudo P2-3: rede de colaboracao");
    const int32_t origem = nomes.id("Edsger W. Dijkstra");
    r.adicionar("origem_id", origem);
    r.adicionar("origem_nome", nomes.nome(origem));
    const auto inicio = std::chrono::steady_clock::now();
    const grafos::ResultadoDijkstra res = grafos::dijkstra(g, origem, grafos::FilaDijkstra::Heap);
    r.adicionar("dijkstra_heap_s", grafos::formatar_fixo(segundos_desde(inicio), 6));
    r.adicionar("explorados", res.explorados);

    const std::pair<const char*, const char*> alvos[] = {
        {"turing", "Alan M. Turing"},
        {"kruskal", "J. B. Kruskal"},
        {"kleinberg", "Jon M. Kleinberg"},
        {"tardos", "\xC3\x89va Tardos"},  // "Éva Tardos" em UTF-8
        {"figueiredo", "Daniel R. Figueiredo"},
    };
    for (const auto& [chave, alvo] : alvos) {
        const std::string prefixo = std::string("rede_") + chave;
        const int32_t id = nomes.id(alvo);
        const std::vector<int32_t> c = grafos::caminho(res, id);
        r.adicionar(prefixo + "_nome", alvo);
        r.adicionar(prefixo + "_id", id);
        r.adicionar(prefixo + "_distancia", texto_distancia(res, id));
        r.adicionar(prefixo + "_arestas", c.empty() ? std::string("-") : std::to_string(c.size() - 1));
        std::string por_nome;
        for (size_t i = 0; i < c.size(); ++i) por_nome += (i > 0 ? " -> " : "") + nomes.nome(c[i]);
        r.adicionar(prefixo + "_caminho", c.empty() ? std::string("-") : por_nome);
        r.adicionar(prefixo + "_caminho_ids", c.empty() ? std::string("-") : comum::lista_texto(c));
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
        const Opcoes o = ler_opcoes(argc, argv);
        return o.rede.empty() ? estudo_grafo(o) : estudo_rede(o);
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
