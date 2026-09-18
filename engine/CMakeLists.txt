include(FetchContent)

FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG        v3.13.2
)
FetchContent_MakeAvailable(entt)

FetchContent_Declare(
    eastl
    GIT_REPOSITORY https://github.com/electronicarts/EASTL.git
    GIT_TAG        master
)
set(EASTL_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(EASTL_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(eastl)

FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)

FetchContent_MakeAvailable(nlohmann_json)

file(GLOB_RECURSE SRC_FILES "*.cpp")

add_library(engine_submodule ${SRC_FILES})

target_include_directories(engine_submodule PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(engine_submodule 
    PUBLIC 
        EnTT::EnTT
        EASTL
        nlohmann_json::nlohmann_json
)