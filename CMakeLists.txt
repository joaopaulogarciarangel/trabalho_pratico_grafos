cmake_minimum_required(VERSION 3.16)
project(grafos LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Tipo de build" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# O padrao do CMake para Release e -O3; a especificacao pede -O2.
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(GRAFOS_SANITIZERS_PADRAO ON)
else()
    set(GRAFOS_SANITIZERS_PADRAO OFF)
endif()
option(GRAFOS_SANITIZERS "Compila com -fsanitize=address,undefined" ${GRAFOS_SANITIZERS_PADRAO})

add_library(grafos_opcoes INTERFACE)
target_compile_options(grafos_opcoes INTERFACE -Wall -Wextra -Wpedantic)
if(GRAFOS_SANITIZERS)
    set(GRAFOS_FLAGS_SAN -fsanitize=address,undefined -fno-omit-frame-pointer -fno-sanitize-recover=all)
    target_compile_options(grafos_opcoes INTERFACE ${GRAFOS_FLAGS_SAN})
    target_link_options(grafos_opcoes INTERFACE ${GRAFOS_FLAGS_SAN})
endif()

add_library(grafos_lib STATIC
    src/leitura.cpp
    src/grafo_lista.cpp
    src/grafo_matriz.cpp
    src/grafo.cpp
    src/busca.cpp
    src/escrita.cpp
    src/analise.cpp
    src/diametro.cpp
    src/sistema.cpp
)
set_target_properties(grafos_lib PROPERTIES OUTPUT_NAME grafos)
target_include_directories(grafos_lib PUBLIC include)
target_link_libraries(grafos_lib PUBLIC grafos_opcoes)

add_executable(grafos app/grafos_cli.cpp)
target_link_libraries(grafos PRIVATE grafos_lib)

add_executable(estudos estudos/estudos.cpp)
target_link_libraries(estudos PRIVATE grafos_lib)

add_executable(gerar_grafo app/gerar_grafo.cpp)
target_link_libraries(gerar_grafo PRIVATE grafos_opcoes)

enable_testing()
add_executable(testes
    tests/main.cpp
    tests/teste_leitura.cpp
    tests/teste_representacoes.cpp
    tests/teste_busca.cpp
    tests/teste_estatisticas.cpp
    tests/teste_distancia.cpp
    tests/teste_diametro.cpp
    tests/teste_sistema.cpp
)
target_include_directories(testes SYSTEM PRIVATE third_party)
target_include_directories(testes PRIVATE tests)
target_compile_definitions(testes PRIVATE GRAFOS_DIR_TESTES_DADOS="${CMAKE_CURRENT_SOURCE_DIR}/tests/dados")
target_link_libraries(testes PRIVATE grafos_lib)
add_test(NAME testes COMMAND testes)
add_test(NAME programas
    COMMAND ${CMAKE_COMMAND}
        -DGRAFOS=$<TARGET_FILE:grafos>
        -DESTUDOS=$<TARGET_FILE:estudos>
        -DGERAR=$<TARGET_FILE:gerar_grafo>
        -DDADOS=${CMAKE_CURRENT_SOURCE_DIR}/tests/dados
        -DTMP=${CMAKE_CURRENT_BINARY_DIR}/teste_programas
        -DSANITIZERS=${GRAFOS_SANITIZERS}
        -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/programas.cmake)
