param(
    [string]$Compiler = 'C:\msys64\ucrt64\bin\g++.exe',
    [switch]$Test
)

$ErrorActionPreference = 'Stop'
$repo = Split-Path -Parent $PSScriptRoot
$saida = Join-Path $repo 'build_windows'
New-Item -ItemType Directory -Force -Path $saida | Out-Null
if (-not (Test-Path -LiteralPath $Compiler)) { throw "Compilador nao encontrado: $Compiler" }
$env:PATH = (Split-Path -Parent $Compiler) + ';' + $env:PATH
$env:TMP = $saida
$env:TEMP = $saida

Push-Location $repo
try {
    $fontes = @(Get-ChildItem -LiteralPath (Join-Path $repo 'src') -Filter '*.cpp' | ForEach-Object FullName)
    & $Compiler -std=c++17 -O2 -DNDEBUG -static -Iinclude @fontes app/grafos_cli.cpp -o (Join-Path $saida 'grafos.exe')
    if ($LASTEXITCODE -ne 0) { throw 'Falha ao compilar grafos.exe' }
    & $Compiler -std=c++17 -O2 -DNDEBUG -static -Iinclude @fontes estudos/estudos_parte3.cpp -o (Join-Path $saida 'estudos_parte3.exe')
    if ($LASTEXITCODE -ne 0) { throw 'Falha ao compilar estudos_parte3.exe' }
    & $Compiler -std=c++17 -O3 -DNDEBUG -static estudos/estudo_compacto.cpp -o (Join-Path $saida 'estudo_compacto.exe')
    if ($LASTEXITCODE -ne 0) { throw 'Falha ao compilar estudo_compacto.exe' }
    if ($Test) {
        $testes = @(Get-ChildItem -LiteralPath (Join-Path $repo 'tests') -Filter '*.cpp' | ForEach-Object FullName)
        & $Compiler -std=c++17 -O2 -DNDEBUG -static -Iinclude -Ithird_party -Itests @fontes @testes -o (Join-Path $saida 'testes.exe')
        if ($LASTEXITCODE -ne 0) { throw 'Falha ao compilar testes.exe' }
        & (Join-Path $saida 'testes.exe') --no-colors
        if ($LASTEXITCODE -ne 0) { throw 'Testes falharam' }
    }
} finally {
    Pop-Location
}
