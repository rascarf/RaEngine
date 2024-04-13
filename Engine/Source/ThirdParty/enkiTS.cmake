set(enkiSourceDir ${ThirdPartyDir}/enkiTS)

file(GLOB enkiTSSrc 
    "${enkiSourceDir}/*.h"
    "${enkiSourceDir}/*.cpp"
)

add_library(enkiTS STATIC ${enkiTSSrc})
target_include_directories(enkiTS PUBLIC "${enkiSourceDir}/")