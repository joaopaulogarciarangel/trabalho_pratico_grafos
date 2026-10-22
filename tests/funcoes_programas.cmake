# Funcoes comuns aos testes de ponta a ponta (programas.cmake e programas_dijkstra.cmake).

# Roda o comando e exige o codigo de saida; deixa SAIDA e ERRO no escopo de quem chamou.
function(rodar esperado_codigo)
    execute_process(COMMAND ${ARGN} RESULT_VARIABLE codigo OUTPUT_VARIABLE saida ERROR_VARIABLE erro)
    if(NOT "${codigo}" STREQUAL "${esperado_codigo}")
        message(FATAL_ERROR "codigo ${codigo} (esperado ${esperado_codigo}) em: ${ARGN}\nsaida:\n${saida}\nerro:\n${erro}")
    endif()
    set(SAIDA "${saida}" PARENT_SCOPE)
    set(ERRO "${erro}" PARENT_SCOPE)
endfunction()

function(exigir_texto texto trecho)
    string(FIND "${texto}" "${trecho}" pos)
    if(pos EQUAL -1)
        message(FATAL_ERROR "trecho nao encontrado: '${trecho}'\nem:\n${texto}")
    endif()
endfunction()

function(comparar arquivo esperado)
    file(READ "${arquivo}" conteudo)
    if(NOT conteudo STREQUAL esperado)
        message(FATAL_ERROR "conteudo de ${arquivo} difere.\nobtido:\n${conteudo}\nesperado:\n${esperado}")
    endif()
endfunction()
