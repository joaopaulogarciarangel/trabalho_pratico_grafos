"""Perfil e experimentos da Parte 3. Nunca tenta carregar grafo acima do limite de RAM."""

import argparse
import csv
import gzip
import os
import shutil
import struct
import subprocess
import sys
from pathlib import Path


def memoria_disponivel():
    if sys.platform == 'win32':
        import ctypes
        from ctypes import wintypes
        class Estado(ctypes.Structure):
            _fields_ = [('dwLength', wintypes.DWORD), ('dwMemoryLoad', wintypes.DWORD)] + [
                (name, ctypes.c_ulonglong) for name in (
                    'ullTotalPhys', 'ullAvailPhys', 'ullTotalPageFile', 'ullAvailPageFile',
                    'ullTotalVirtual', 'ullAvailVirtual', 'ullAvailExtendedVirtual')]
        estado = Estado()
        estado.dwLength = ctypes.sizeof(estado)
        if ctypes.windll.kernel32.GlobalMemoryStatusEx(ctypes.byref(estado)):
            return estado.ullAvailPhys
    else:
        for linha in Path('/proc/meminfo').read_text().splitlines():
            if linha.startswith('MemAvailable:'):
                return int(linha.split()[1]) * 1024
    return 0


def perfil(path):
    with path.open('rb') as f:
        f.seek(-4, os.SEEK_END)
        bytes_descompactados = struct.unpack('<I', f.read(4))[0]
    with gzip.open(path, 'rt', encoding='ascii') as f:
        n = int(next(f))
        m = negativos = lacos = 0
        menor = float('inf')
        maior = float('-inf')
        for linha in f:
            if not linha.strip():
                continue
            u, v, w = linha.split()[:3]
            w = float(w)
            m += 1
            negativos += w < 0
            lacos += u == v
            menor = min(menor, w)
            maior = max(maior, w)
    # Limite conservador: buffer de leitura, arrays temporarios, lista final e alocador.
    pico = int(1.5 * (bytes_descompactados + 28*m + 80*n))
    return dict(arquivo=path.name, n=n, linhas_arcos=m,
                pesos_negativos=negativos, lacos=lacos, peso_min=menor,
                peso_max=maior, bytes_descompactados=bytes_descompactados,
                pico_estimado_bytes=pico)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('arquivos', nargs='+', type=Path, help='grafo_W_*.txt.gz')
    parser.add_argument('--executavel', type=Path, required=True)
    parser.add_argument('--saida', type=Path, required=True)
    parser.add_argument('--repeticoes', type=int, default=10)
    parser.add_argument('--max-trabalho', type=float, default=1e10)
    args = parser.parse_args()
    args.saida.mkdir(parents=True, exist_ok=True)
    perfis = []
    for path in args.arquivos:
        p = perfil(path)
        disponivel = memoria_disponivel()
        p['memoria_disponivel_bytes'] = disponivel
        if not disponivel or p['pico_estimado_bytes'] > 0.8 * disponivel:
            p['status'] = 'nao_carregado_memoria'
            perfis.append(p)
            continue
        dados = args.saida / (path.name[:-3])
        criado = not dados.exists()
        if criado:
            with gzip.open(path, 'rb') as entrada, dados.open('wb') as saida:
                shutil.copyfileobj(entrada, saida)
        resultado = args.saida / (path.name[:-7] + '.csv')
        temporario = resultado.with_suffix('.csv.tmp')
        try:
            with temporario.open('w', encoding='utf-8') as saida:
                subprocess.run([str(args.executavel.resolve()), str(dados.resolve()),
                                str(args.repeticoes), str(args.max_trabalho)],
                               stdout=saida, check=True)
            temporario.replace(resultado)
            p['status'] = 'executado'
        except subprocess.CalledProcessError as e:
            p['status'] = f'erro_codigo_{e.returncode}'
        finally:
            temporario.unlink(missing_ok=True)
            if criado:
                dados.unlink()
        perfis.append(p)
    with (args.saida / 'perfil.csv').open('w', newline='', encoding='utf-8') as f:
        writer = csv.DictWriter(f, fieldnames=perfis[0].keys())
        writer.writeheader()
        writer.writerows(perfis)
    for p in perfis:
        print(p['arquivo'], p['status'])


if __name__ == '__main__':
    main()
