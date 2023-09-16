set(StbSourceDir ${ThirdPartyDir}/stb_image)
set(cglTFSourceDir ${ThirdPartyDir}/cgltf)

file(GLOB_RECURSE Box2DSrc 
    "${StbSourceDir}/*.h"
    "${StbSourceDir}/*.cpp"
    "${cglTFSourceDir}/*.h"
)

add_library(stb STATIC ${Box2DSrc})
target_include_directories(stb PUBLIC "${StbSourceDir}")