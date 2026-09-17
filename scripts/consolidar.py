#!/usr/bin/env python3
"""Consolida os CSVs de resultados/ gerados pelo programa `estudos`.

Gera:
  resultados/consolidado.csv  uma linha por grafo, uma coluna por caracteristica (lista + matriz)
  resultados/tabelas.md       tabelas Markdown para o relatorio

Uso: python3 scripts/consolidar.py [pasta_resultados]
Somente biblioteca padrao do Python 3.
"""

import csv
import re
import sys
from pathlib import Path

REPRESENTACOES = ("lista", "matriz")

# Caracteristicas do grafo (iguais nas duas representacoes): uma coluna so, lida da lista
# (ou da matriz, se a lista faltar).
COMUNS = [
    "n", "m", "linhas_aresta", "lacos_ignorados", "repetidas_ignoradas",
    "grau_minimo", "grau_maximo", "grau_medio", "mediana_grau",
    "num_componentes", "maior_componente", "menor_componente",
    "semente", "vertices_sorteados", "cpu", "memtotal_mb", "compilador",
]
IGNORADAS = {"chave", "grafo", "representacao", "arquivo_estatisticas"}


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


def carregar(pasta):
    grafos = {}
    for caminho in sorted(pasta.glob("*.csv")):
        m = re.fullmatch(r"(.+)_(lista|matriz)\.csv", caminho.name)
        if not m:
            continue
        nome, repr_ = m.group(1), m.group(2)
        grafos.setdefault(nome, {})[repr_] = ler_csv(caminho)
    return dict(sorted(grafos.items(), key=lambda item: chave_natural(item[0])))


def valor_comum(execucoes, chave):
    for repr_ in REPRESENTACOES:
        if chave in execucoes.get(repr_, {}):
            return execucoes[repr_][chave]
    return ""


def escrever_consolidado(grafos, caminho):
    especificas = []
    for repr_ in REPRESENTACOES:
        chaves = []
        for execucoes in grafos.values():
            for chave in execucoes.get(repr_, {}):
                if chave not in COMUNS and chave not in IGNORADAS and chave not in chaves:
                    chaves.append(chave)
        especificas += [(repr_, chave) for chave in chaves]

    cabecalho = ["grafo"] + COMUNS + [f"{repr_}_{chave}" for repr_, chave in especificas]
    with open(caminho, "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(cabecalho)
        for nome, execucoes in grafos.items():
            linha = [nome] + [valor_comum(execucoes, c) for c in COMUNS]
            linha += [execucoes.get(repr_, {}).get(chave, "") for repr_, chave in especificas]
            w.writerow(linha)


# ---------- formatacao legivel (pt-BR: milhar com ponto, decimal com virgula) ----------

def numero(texto):
    try:
        return float(texto)
    except (TypeError, ValueError):
        return None


def inteiro_br(texto):
    x = numero(texto)
    if x is None:
        return texto if texto else "—"
    return f"{int(x):,}".replace(",", ".")


def decimal_br(x, casas):
    s = f"{x:,.{casas}f}"
    return s.replace(",", "_").replace(".", ",").replace("_", ".")


def mb_br(texto):
    x = numero(texto)
    if x is None:
        return "—"
    return decimal_br(0.0 if abs(x) < 0.05 else x, 1)  # evita "-0,0" em diferencas minimas de RSS


def tempo_br(texto):
    """Segundos em unidade legivel (µs, ms ou s)."""
    x = numero(texto)
    if x is None:
        return "—"
    if x < 1e-3:
        return decimal_br(x * 1e6, 1) + " µs"
    if x < 1:
        return decimal_br(x * 1e3, 2) + " ms"
    return decimal_br(x, 2) + " s"


def tabela(cabecalho, linhas):
    saida = ["| " + " | ".join(cabecalho) + " |", "|" + "|".join("---" for _ in cabecalho) + "|"]
    saida += ["| " + " | ".join(str(c) for c in linha) + " |" for linha in linhas]
    return "\n".join(saida)


def texto_valor(texto):
    return {"nao_alcancado": "não alcançado", "inexistente": "inexistente", "infinito": "∞", "": "—"}.get(
        texto, inteiro_br(texto) if numero(texto) is not None else texto)


def confere(execucoes, chaves):
    lista, matriz = execucoes.get("lista"), execucoes.get("matriz")
    if not lista or not matriz or matriz.get("memoria_status") != "ok" or lista.get("memoria_status") != "ok":
        return "—"
    return "sim" if all(lista.get(c) == matriz.get(c) for c in chaves) else "**não**"


def secao_memoria(grafos):
    linhas = []
    for nome, ex in grafos.items():
        l, mz = ex.get("lista", {}), ex.get("matriz", {})
        if mz.get("memoria_status") == "inviavel":
            matriz_rss = "inviável"
            matriz_est = f"{mb_br(mz.get('matriz_estimada_mb'))} (estimada)"
        else:
            matriz_rss = mb_br(mz.get("rss_diferenca_mb"))
            matriz_est = mb_br(mz.get("estrutura_mb"))
        linhas.append([nome, inteiro_br(valor_comum(ex, "n")), inteiro_br(valor_comum(ex, "m")),
                       mb_br(l.get("rss_diferenca_mb")), mb_br(l.get("estrutura_mb")), matriz_rss, matriz_est])
    return "## Memória (MB = 1024² bytes)\n\n" + tabela(
        ["grafo", "n", "m", "lista: RSS (MB)", "lista: estimada (MB)", "matriz: RSS (MB)", "matriz: estimada (MB)"],
        linhas) + "\n\nRSS = VmRSS depois da carga − VmRSS antes. Estimada = `bytes_estrutura()`."


def secao_tempo(grafos):
    linhas = []
    for nome, ex in grafos.items():
        linha = [nome]
        for repr_ in REPRESENTACOES:
            r = ex.get(repr_, {})
            for busca in ("bfs", "dfs"):
                media, desvio = r.get(f"{busca}_media_s"), r.get(f"{busca}_desvio_s")
                if media is None:
                    linha.append("inviável" if r.get("memoria_status") == "inviavel" else "—")
                else:
                    linha.append(f"{tempo_br(media)} ± {tempo_br(desvio)}")
        linhas.append(linha)
    return "## Tempo médio de BFS e DFS (100 buscas; média ± desvio padrão)\n\n" + tabela(
        ["grafo", "BFS lista", "DFS lista", "BFS matriz", "DFS matriz"], linhas)


def secao_pais(grafos):
    linhas = []
    for nome, ex in grafos.items():
        for raiz in (1, 2, 3):
            chaves = [f"pai_{b}_raiz{raiz}_v{v}" for b in ("bfs", "dfs") for v in (10, 20, 30)]
            linha = [nome, raiz] + [texto_valor(valor_comum(ex, c)) for c in chaves]
            linhas.append(linha + [confere(ex, chaves)])
    cab = ["grafo", "raiz"] + [f"{b.upper()}: pai de {v}" for b in ("bfs", "dfs") for v in (10, 20, 30)]
    return "## Pais nas árvores de busca\n\n" + tabela(cab + ["lista = matriz"], linhas)


def secao_distancias(grafos):
    chaves = ["dist_10_20", "dist_10_30", "dist_20_30"]
    linhas = [[nome] + [texto_valor(valor_comum(ex, c)) for c in chaves] + [confere(ex, chaves)]
              for nome, ex in grafos.items()]
    return "## Distâncias\n\n" + tabela(["grafo", "d(10, 20)", "d(10, 30)", "d(20, 30)", "lista = matriz"], linhas)


def secao_componentes(grafos):
    chaves = ["num_componentes", "maior_componente", "menor_componente"]
    linhas = [[nome] + [inteiro_br(valor_comum(ex, c)) for c in chaves] + [confere(ex, chaves)]
              for nome, ex in grafos.items()]
    return "## Componentes conexas\n\n" + tabela(
        ["grafo", "componentes", "maior", "menor", "lista = matriz"], linhas)


def secao_diametro(grafos):
    linhas = []
    for nome, ex in grafos.items():
        l = ex.get("lista", {})
        if "diam_exato_valor" not in l:
            linhas.append([nome] + ["—"] * 8)
            continue
        li_a, ls_a = l.get("diam_aprox_limite_inferior"), l.get("diam_aprox_limite_superior")
        aprox = inteiro_br(li_a) if li_a == ls_a else f"[{inteiro_br(li_a)}, {inteiro_br(ls_a)}]"
        li_e, ls_e = l.get("diam_exato_limite_inferior"), l.get("diam_exato_limite_superior")
        exato = inteiro_br(li_e) if l.get("diam_exato_exato") == "sim" else f"[{inteiro_br(li_e)}, {inteiro_br(ls_e)}]"
        linhas.append([
            nome,
            aprox, f"({l.get('diam_aprox_u')}, {l.get('diam_aprox_v')})",
            tempo_br(l.get("diam_aprox_segundos")), inteiro_br(l.get("diam_aprox_num_bfs")),
            exato, f"({l.get('diam_exato_u')}, {l.get('diam_exato_v')})",
            tempo_br(l.get("diam_exato_segundos")) + (" (esgotado)" if l.get("diam_exato_tempo_esgotado") == "sim" else ""),
            inteiro_br(l.get("diam_exato_num_bfs")),
        ])
    return "## Diâmetro (representação lista)\n\n" + tabela(
        ["grafo", "aproximado", "par", "tempo", "BFS", "exato (iFUB)", "par", "tempo", "BFS"], linhas) + (
        "\n\nIntervalo [inferior, superior] quando não exato. O iFUB tem limite de tempo; "
        "\"esgotado\" indica interrupção.")


def main():
    raiz = Path(__file__).resolve().parent.parent
    pasta = Path(sys.argv[1]) if len(sys.argv) > 1 else raiz / "resultados"
    if not pasta.is_dir():
        print(f"erro: pasta '{pasta}' nao existe", file=sys.stderr)
        return 1
    grafos = carregar(pasta)
    if not grafos:
        print(f"erro: nenhum CSV <grafo>_<lista|matriz>.csv em '{pasta}'", file=sys.stderr)
        return 1

    faltando = [f"{nome}_{r}.csv" for nome, ex in grafos.items() for r in REPRESENTACOES if r not in ex]
    escrever_consolidado(grafos, pasta / "consolidado.csv")

    partes = ["# Resultados dos estudos de caso"]
    if faltando:
        partes.append("> CSVs ausentes: " + ", ".join(faltando))
    partes += [secao_memoria(grafos), secao_tempo(grafos), secao_pais(grafos),
               secao_distancias(grafos), secao_componentes(grafos), secao_diametro(grafos)]
    (pasta / "tabelas.md").write_text("\n\n".join(partes) + "\n", encoding="utf-8")

    print(f"{len(grafos)} grafo(s): {', '.join(grafos)}")
    if faltando:
        print("aviso: CSVs ausentes: " + ", ".join(faltando))
    print(f"gerados: {pasta / 'consolidado.csv'} e {pasta / 'tabelas.md'}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
