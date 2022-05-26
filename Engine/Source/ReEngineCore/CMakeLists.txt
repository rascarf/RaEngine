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


target_compile_definitions(
    ReEngineCore 
    PRIVATE DEBUG
    PRIVATE GLFW_INCLUDE_NONE
    PRIVATE IMGUI_IMPL_OPENGL_LOADER_CUSTOM
    "ENGINE_ROOT_DIR=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
    )

target_link_libraries(ReEngineCore PUBLIC spdlog)
target_link_libraries(ReEngineCore PUBLIC imgui)
target_link_libraries(ReEngineCore PUBLIC glfw)
target_link_libraries(ReEngineCore PUBLIC glm)
target_link_libraries(ReEngineCore PUBLIC stb)
target_link_libraries(ReEngineCore PUBLIC ImGuizmo)
target_link_libraries(ReEngineCore PUBLIC Glad)
target_link_libraries(ReEngineCore PUBLIC magic_enum::magic_enum)

