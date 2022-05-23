file(GLOB_RECURSE RuntimeHeaderFiles CONFIGUE_DEPENDS "*.h")
file(GLOB_RECURSE RuntimeSourceFiles CONFIGUE_DEPENDS "*.cpp")

source_group(
TREE "${EngineCoreDir}" 
FILES ${RuntimeHeaderFiles} ${RuntimeSourceFiles}
)

add_library(
    ReEngineCore 
    STATIC ${RuntimeHeaderFiles} ${RuntimeSourceFiles}
)

target_include_directories(
    ReEngineCore 
    PRIVATE "${EngineSourceDir}"
    PRIVATE "${EngineCoreDir}"
)

#target_compile_definitions(ReEngineCorePRIVATE BUILD_DLLPRIVATE DEBUG"ENGINE_ROOT_DIR=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

target_link_libraries(
    ReEngineCore PUBLIC 
    PUBLIC spdlog
)

target_link_libraries(
    ReEngineCore PUBLIC 
    PUBLIC imgui
)

target_link_libraries(ReEngineCore PUBLIC glfw)
target_link_libraries(ReEngineCore PUBLIC glm)
target_link_libraries(ReEngineCore PUBLIC stb)
target_link_libraries(ReEngineCore PUBLIC ImGuizmo)
