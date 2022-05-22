set(ThirdPartyDir "${EngineSourceDir}/ThirdParty")
set(EngineCoreDir "${EngineSourceDir}/ReEngineCore")
set(EngineEditorDir "${EngineSourceDir}/ReEngineEditor")


add_subdirectory(ThirdParty)
add_subdirectory(ReEngineCore)
add_subdirectory(ReEngineEditor)

if(MSVC)
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ReEngineEditor)
endif()