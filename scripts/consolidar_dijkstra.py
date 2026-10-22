#!/usr/bin/env python3
"""Consolida os CSVs de resultados/parte2/ gerados pelo programa `estudos_dijkstra`.

Gera resultados/parte2/tabelas_dijkstra.md com:
  - carga dos grafos (n, m, memoria);
  - P2-1: distancia e caminho de 10 a 20, 30, 40, 50 e 60;
  - P2-2: tempo medio do Dijkstra com heap e com vetor;
  - P2-3: rede de colaboracao (Edsger W. Dijkstra -> cinco pesquisadores).

Uso: python3 scripts/consolidar_dijkstra.py [pasta_resultados]
Somente biblioteca padrao do Python 3.
"""

import csv
import re
import sys
from pathlib import Path

DESTINOS = (20, 30, 40, 50, 60)
PESQUISADORES = ("turing", "kruskal", "kleinberg", "tardos", "figueiredo")


def chave_natural(texto):
    return [int(p) if p.isdigit() else p for p in re.split(r"(\d+)", texto)]


def ler_csv(caminho):
    dados = {}
    with open(caminho, newline="", encoding="utf-8") as f:
        for linha in csv.reader(f):
            if len(linha) < 2 or linha[0] == "chave":
                continue
            dados[linha[0]] = ",".join(linha[1:])
    return dados


def inteiro(texto):
    return f"{int(texto):,}".replace(",", ".") if texto not in ("", None) else "—"


def segundos(texto):
    """Segundos em notacao legivel: us abaixo de 1 ms, ms abaixo de 1 s, s acima, h acima de 2 h."""
    if texto in ("", None):
        return "—"
    s = float(texto)
    if s < 1e-3:
        return f"{s * 1e6:.1f} µs".replace(".", ",")
    if s < 1:
        return f"{s * 1000:.1f} ms".replace(".", ",")
    if s < 7200:
        return f"{s:.2f} s".replace(".", ",")
    return f"{s / 3600:.1f} h".replace(".", ",")


def decimal(texto, casas=1):
    return f"{float(texto):.{casas}f}".replace(".", ",") if texto not in ("", None) else "—"


def distancia(texto):
    if texto in ("infinito", "inexistente", "", None):
        return texto or "—"
    return f"{float(texto):.2f}".replace(".", ",")


def tabela(cabecalho, linhas):
    saida = ["| " + " | ".join(cabecalho) + " |", "|" + "|".join("---" for _ in cabecalho) + "|"]
    saida += ["| " + " | ".join(str(c) for c in linha) + " |" for linha in linhas]
    return "\n".join(saida)


def secao_carga(grafos):
    linhas = []
    for nome, d in grafos.items():
        linhas.append([
            nome, inteiro(d.get("n")), inteiro(d.get("m")), decimal(d.get("estrutura_mb")),
            decimal(d.get("rss_depois_mb")), decimal(d.get("tempo_carga_s"), 2),
        ])
    return tabela(["grafo", "vértices", "arestas", "estrutura (MB)", "RSS após carga (MB)", "carga (s)"], linhas)


def secao_p2_1(grafos):
    distancias = []
    caminhos = []
    for nome, d in grafos.items():
        distancias.append([nome] + [distancia(d.get(f"dist_10_{x}")) for x in DESTINOS])
        for x in DESTINOS:
            arestas = d.get(f"arestas_10_{x}", "—")
            caminho = d.get(f"caminho_10_{x}", "—")
            caminhos.append([nome, f"10 → {x}", distancia(d.get(f"dist_10_{x}")), arestas,
                             caminho.replace(" ", " → ") if caminho not in ("-", "—") else "—"])
    return (tabela(["grafo"] + [f"10 → {x}" for x in DESTINOS], distancias) + "\n\nCaminhos mínimos:\n\n"
            + tabela(["grafo", "par", "distância", "arestas", "caminho"], caminhos))


def secao_p2_2(grafos):
    linhas = []
    for nome, d in grafos.items():
        k_vetor = int(d.get("vetor_execucoes", "0") or 0)
        if k_vetor > 0:
            vetor = f"{segundos(d.get('vetor_media_s'))} ± {segundos(d.get('vetor_desvio_s'))}"
            heap_k = segundos(d.get("heap_media_mesmas_origens_s"))
            razao = decimal(d.get("razao_vetor_heap"))
        elif "vetor_estimado_s" in d:
            fracao = float(d["vetor_estimado_fracao"]) * 100
            vetor = f"≈ {segundos(d['vetor_estimado_s'])} (estimado: {fracao:g}% dos vértices)".replace(".", ",")
            heap_k = "—"
            razao = "≈ " + decimal(d.get("razao_vetor_heap_estimada"))
        else:
            vetor, heap_k, razao = "—", "—", "—"
        linhas.append([
            nome, inteiro(d.get("n")), d.get("heap_execucoes", "—"),
            f"{segundos(d.get('heap_media_s'))} ± {segundos(d.get('heap_desvio_s'))}",
            k_vetor if k_vetor > 0 else "—", heap_k, vetor, razao,
        ])
    return tabela(["grafo", "vértices", "k heap", "heap: média ± desvio", "k vetor",
                   "heap nas mesmas origens", "vetor: média ± desvio", "vetor / heap"], linhas)


def secao_p2_3(rede):
    linhas = []
    for p in PESQUISADORES:
        linhas.append([
            rede.get(f"rede_{p}_nome", p), rede.get(f"rede_{p}_id", "—"),
            rede.get(f"rede_{p}_distancia", "—"), rede.get(f"rede_{p}_arestas", "—"),
            rede.get(f"rede_{p}_caminho", "—"),
        ])
    cabecalho = (f"Origem: {rede.get('origem_nome', 'Edsger W. Dijkstra')} (id {rede.get('origem_id', '?')}); "
                 f"{inteiro(rede.get('n'))} vértices, {inteiro(rede.get('m'))} arestas.\n\n")
    return cabecalho + tabela(["pesquisador", "id", "distância", "arestas", "caminho"], linhas)


def main():
    pasta = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parent.parent / "resultados" / "parte2"
    grafos = {}
    rede = None
    for caminho in sorted(pasta.glob("*_dijkstra.csv"), key=lambda c: chave_natural(c.name)):
        dados = ler_csv(caminho)
        if "origem_id" in dados:
            rede = dados
        else:
            grafos[caminho.name[: -len("_dijkstra.csv")]] = dados
    if not grafos and rede is None:
        print(f"nenhum *_dijkstra.csv em {pasta}", file=sys.stderr)
        return 1

    partes = ["# Resultados da Parte 2 (Dijkstra)\n"]
    if grafos:
        partes += ["## Grafos carregados (lista de adjacência)\n", secao_carga(grafos), "",
                   "## P2-1: distância e caminho mínimo a partir do vértice 10\n", secao_p2_1(grafos), "",
                   "## P2-2: tempo médio do Dijkstra (heap × vetor)\n", secao_p2_2(grafos), ""]
    if rede is not None:
        partes += ["## P2-3: rede de colaboração\n", secao_p2_3(rede), ""]
    destino = pasta / "tabelas_dijkstra.md"
    destino.write_text("\n".join(partes), encoding="utf-8")
    print(f"gerado: {destino}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
