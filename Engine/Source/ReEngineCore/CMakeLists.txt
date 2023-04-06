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

set(TARGET_NAME ReEngineCore)

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

add_dependencies(${TARGET_NAME} ${SHADER_COMPILE_TARGET})
target_include_directories(
  ${TARGET_NAME}
  PUBLIC ${EngineShadersDir}/generated/cpp)


target_link_libraries(ReEngineCore PUBLIC spdlog)
target_link_libraries(ReEngineCore PUBLIC imgui)
target_link_libraries(ReEngineCore PUBLIC glfw)
target_link_libraries(ReEngineCore PUBLIC glm)
target_link_libraries(ReEngineCore PUBLIC stb)
target_link_libraries(ReEngineCore PUBLIC ImGuizmo)
target_link_libraries(ReEngineCore PUBLIC Glad)
target_link_libraries(ReEngineCore PUBLIC yaml-cpp)
target_link_libraries(ReEngineCore PUBLIC headers)
target_link_libraries(ReEngineCore PUBLIC ${vulkan_lib})
target_link_libraries(ReEngineCore PUBLIC assimp)