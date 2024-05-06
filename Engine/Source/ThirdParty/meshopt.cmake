set(MeshOptSourceDir ${ThirdPartyDir}/meshoptimizer)

file(GLOB_RECURSE MeshOptSrc 
    "${MeshOptSourceDir}/*.h"
    "${MeshOptSourceDir}/*.cpp"
)

add_library(MeshOpt STATIC ${MeshOptSrc})
target_include_directories(MeshOpt PUBLIC "${MeshOptSourceDir}")
target_link_libraries(MeshOpt PUBLIC imgui)