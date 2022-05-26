file(GLOB EditorHeaderFiles CONFIGUE_DEPENDS "*.h" )
file(GLOB EditorSourceFiles CONFIGUE_DEPENDS "*.cpp" )

source_group(TREE ${EngineEditorDir} FILES ${EditorHeaderFiles} ${EditorSourceFiles})
add_executable(ReEngineEditor ${EditorHeaderFiles} ${EditorSourceFiles})

add_compile_definitions("ENGINE_ROOT_DIR=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

target_link_libraries(ReEngineEditor PRIVATE ReEngineCore)

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