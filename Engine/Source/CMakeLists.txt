set(ThirdPartyDir "${EngineSourceDir}/ThirdParty")
set(EngineCoreDir "${EngineSourceDir}/ReEngineCore")
set(EngineEditorDir "${EngineSourceDir}/ReEngineEditor")

add_subdirectory(ThirdParty)
add_subdirectory(ReEngineCore)
add_subdirectory(ReEngineEditor)

set(vulkan_include ${ThirdPartyDir}/Vulkan/include)
# set(vulkan_lib ${ThirdPartyDir}/Vulkan/lib/vulkan-1.lib)
# target_link_libraries(ReEngineCore PRIVATE volk_headers)


if(MSVC)
    set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ReEngineEditor)
endif()