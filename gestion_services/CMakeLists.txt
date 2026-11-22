cmake_minimum_required(VERSION 3.16)
project(GestionServicesRewards LANGUAGES CXX)

# Bibliotheque du service "Rewards Program".
# Elle ne compile que Service.cpp et les sources de ce service : les autres
# services (Prime, Retirement, Training) dependent du module Employes et sont
# volontairement exclus pour que ce module reste compilable seul.
add_library(services_rewards STATIC
    source/Service.cpp
    source/ProgrammeRecompense.cpp
    source/Recompense.cpp
    source/RewardsProgramService.cpp
)

target_include_directories(services_rewards PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/header)
target_compile_features(services_rewards PUBLIC cxx_std_17)
set_target_properties(services_rewards PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

option(REWARDS_BUILD_DEMO "Compiler la demonstration autonome" ON)
option(REWARDS_BUILD_TESTS "Compiler les tests du service" ON)

if(REWARDS_BUILD_DEMO)
    add_executable(rewards_demo demo/main.cpp)
    target_link_libraries(rewards_demo PRIVATE services_rewards)
endif()

if(REWARDS_BUILD_TESTS)
    enable_testing()
    add_executable(test_rewards tests/test_rewards.cpp)
    target_link_libraries(test_rewards PRIVATE services_rewards)
    add_test(NAME test_rewards COMMAND test_rewards)
endif()

if(MSVC)
    foreach(cible services_rewards rewards_demo test_rewards)
        if(TARGET ${cible})
            target_compile_options(${cible} PRIVATE /utf-8)
        endif()
    endforeach()
endif()
