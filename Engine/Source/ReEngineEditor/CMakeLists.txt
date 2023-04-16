file(GLOB_RECURSE EditorHeaderFiles CONFIGUE_DEPENDS "*.h" )
file(GLOB_RECURSE EditorSourceFiles CONFIGUE_DEPENDS "*.cpp" )

source_group(TREE ${EngineEditorDir} FILES ${EditorHeaderFiles} ${EditorSourceFiles})
add_executable(ReEngineEditor ${EditorHeaderFiles} ${EditorSourceFiles})

add_compile_definitions("ENGINE_ROOT_DIR=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

target_link_libraries(ReEngineEditor PRIVATE ReEngineCore)
target_link_libraries(ReEngineEditor PUBLIC ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Lib/vulkan-1.lib)

target_link_libraries(ReEngineCore PUBLIC spirv-cross-glsl)
target_link_libraries(ReEngineCore PUBLIC spirv-cross-hlsl)
target_link_libraries(ReEngineCore PUBLIC spirv-cross-cpp)
target_link_libraries(ReEngineCore PUBLIC spirv-cross-reflect)
target_link_libraries(ReEngineCore PUBLIC spirv-cross-msl)
target_link_libraries(ReEngineCore PUBLIC spirv-cross-util)
target_link_libraries(ReEngineCore PUBLIC spirv-cross-core)

target_link_libraries(ReEngineEditor PUBLIC assimp)
target_link_libraries(ReEngineEditor PUBLIC headers)

target_include_directories(ReEngineEditor PRIVATE
	"${EngineSourceDir}"
	"${EngineCoreDir}"
)

target_compile_definitions(ReEngineEditor PRIVATE
	PLATFORM_WINDOWS
	DEBUG
)

if(MSVC)
	set_target_properties(
        ReEngineEditor PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY "${EngineRootDir}")
endif()