cmake_minimum_required(VERSION 3.20)
project(GestionAbsences LANGUAGES CXX)

add_library(gestion_absences_lib STATIC
    src/Absence.cpp
    src/GestionnaireAbsences.cpp
    src/GestionnaireFichiersAbsences.cpp
    src/MenuAbsences.cpp)
add_library(GestionAbsences::module ALIAS gestion_absences_lib)
target_include_directories(gestion_absences_lib PUBLIC include)
target_compile_features(gestion_absences_lib PUBLIC cxx_std_17)
set_target_properties(gestion_absences_lib PROPERTIES
    CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO)

set(gestion_absences_autonome OFF)
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(gestion_absences_autonome ON)
endif()
option(GESTION_ABSENCES_BUILD_DEMO "Compiler la démonstration autonome des absences" ${gestion_absences_autonome})

set(gestion_absences_tests_disponibles ON)
foreach(fichier test_absences.cpp test_fichiers_absences.cpp DateTest.h EmployesTest.h)
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests/${fichier}")
        set(gestion_absences_tests_disponibles OFF)
    endif()
endforeach()
set(gestion_absences_tests_par_defaut OFF)
if(gestion_absences_autonome AND gestion_absences_tests_disponibles)
    set(gestion_absences_tests_par_defaut ON)
endif()
option(GESTION_ABSENCES_BUILD_TESTS "Compiler les tests du module absences" ${gestion_absences_tests_par_defaut})

if(GESTION_ABSENCES_BUILD_DEMO)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data")
    add_executable(gestion_absences src/main.cpp)
    target_link_libraries(gestion_absences PRIVATE GestionAbsences::module)
    target_compile_definitions(gestion_absences PRIVATE
        GESTION_ABSENCES_FICHIER_DEMO="${CMAKE_CURRENT_SOURCE_DIR}/data/absences.txt")
endif()

if(GESTION_ABSENCES_BUILD_TESTS AND gestion_absences_tests_disponibles)
    enable_testing()
    add_executable(test_absences tests/test_absences.cpp)
    target_link_libraries(test_absences PRIVATE GestionAbsences::module)
    add_test(NAME test_absences COMMAND test_absences)

    add_executable(test_fichiers_absences tests/test_fichiers_absences.cpp)
    target_link_libraries(test_fichiers_absences PRIVATE GestionAbsences::module)
    add_test(NAME test_fichiers_absences COMMAND test_fichiers_absences
             tests "${CMAKE_CURRENT_BINARY_DIR}/test_fichiers_absences.donnees")
elseif(GESTION_ABSENCES_BUILD_TESTS)
    message(STATUS "GestionAbsences : tests ignorés, sources locales indisponibles.")
endif()

if(MSVC)
    foreach(cible gestion_absences_lib gestion_absences test_absences test_fichiers_absences)
        if(TARGET ${cible})
            target_compile_options(${cible} PRIVATE /utf-8)
        endif()
    endforeach()
endif()
