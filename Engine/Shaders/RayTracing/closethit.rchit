#version 460

#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : require


#include "lib/Common.h"
#include "lib/ShadingCommon.h"

layout(location = 0) rayPayloadInEXT RayPayloadInfo rayInfo;

struct Vertex
{
    float vertices[11];
};

struct Material
{
    vec4 albedo;
	vec4 params; // roughness, metallic, occlusion
	ivec4 textureIDs; // albedo, roughness, metallic
};

struct Triangle 
{
    Vertex v0;
    Vertex v1;
    Vertex v2;
};

struct Face
{
    uint indices[3];
};

layout(std430, binding = 0, set = 1) buffer Vertices
{
    Vertex datas[];
} vertices[];

layout(std430, binding = 1, set = 1) buffer Indices
{
    Face datas[];
} indices[];

layout(std430, binding = 2, set = 1) buffer Materials
{
    Material datas[];
} materials;

layout(std430, binding = 3, set = 1) buffer ObjectInstances
{
    ivec4 datas[];
} objects;

layout(binding = 4, set = 1) uniform sampler2D textures[];

hitAttributeEXT vec3 attribs;

vec2 Blerp(vec2 b, vec2 p1, vec2 p2, vec2 p3)
{
    return (1.0 - b.x - b.y) * p1 + b.x * p2 + b.y * p3;
}

vec3 Blerp(vec2 b, vec3 p1, vec3 p2, vec3 p3)
{
    return (1.0 - b.x - b.y) * p1 + b.x * p2 + b.y * p3;
}

Triangle FetchTriangle(uint geometryIndex, uint faceIndex)
{
    ivec4 object = objects.datas[geometryIndex];

    Face face = indices[object.y].datas[faceIndex];

    Triangle result;
    result.v0 = vertices[object.y].datas[face.indices[0]];
    result.v1 = vertices[object.y].datas[face.indices[1]];
    result.v2 = vertices[object.y].datas[face.indices[2]];

    return result;
}

vec3 FetchNormal(Triangle triangle, vec2 b)
{
    vec3 normal0 = vec3(triangle.v0.vertices[5], triangle.v0.vertices[6], triangle.v0.vertices[7]);
    vec3 normal1 = vec3(triangle.v1.vertices[5], triangle.v1.vertices[6], triangle.v1.vertices[7]);
    vec3 normal2 = vec3(triangle.v2.vertices[5], triangle.v2.vertices[6], triangle.v2.vertices[7]);
    vec3 normal = Blerp(b, normal0, normal1, normal2);

    return gl_ObjectToWorldEXT * vec4(normal, 0.0);
}

vec2 FetchUV(Triangle triangle, vec2 b)
{
    vec2 uv0 = vec2(triangle.v0.vertices[3], triangle.v0.vertices[4]);
    vec2 uv1 = vec2(triangle.v1.vertices[3], triangle.v1.vertices[4]);
    vec2 uv2 = vec2(triangle.v2.vertices[3], triangle.v2.vertices[4]);
    return Blerp(b, uv0, uv1, uv2);
}

Material FetchMaterial(uint geometryIndex)
{
    ivec4 object = objects.datas[geometryIndex];

    return materials.datas[object.x];
}

vec3 GammaToLinearSpace(vec3 sRGB)
{
    // Approximate version from http://chilliant.blogspot.com.au/2012/08/srgb-approximations-for-hlsl.html?m=1
    return sRGB * (sRGB * (sRGB * 0.305306011 + 0.682171111) + 0.012522878);
}

void main()
{
    Triangle triangle = FetchTriangle(gl_InstanceID, gl_PrimitiveID);
    Material material = FetchMaterial(gl_InstanceID);
    
    vec3 normal = FetchNormal(triangle, attribs.xy);
    vec2 uv = FetchUV(triangle, attribs.xy);

    vec3 albedo = vec3(0, 0, 0);
    if (material.textureIDs.x >= 0) {
        albedo = GammaToLinearSpace(texture(textures[material.textureIDs.x], uv).xyz);
    }
    else {
        albedo = material.albedo.xyz;
    }

    float roughness = material.params.x;
    if (material.textureIDs.y >= 0) {
        roughness = texture(textures[material.textureIDs.y], uv).x;
    }

    float metallic  = material.params.y;
    if (material.textureIDs.z >= 0) {
        roughness = texture(textures[material.textureIDs.z], uv).x;
    }

    float specular = 0.5;
    rayInfo.hitT = gl_HitTEXT;
    rayInfo.worldPos = gl_WorldRayOriginEXT.xyz + gl_WorldRayDirectionEXT.xyz * gl_HitTEXT;
    rayInfo.worldNormal = normal;
    rayInfo.radiance = vec3(0.0, 0.0, 0.0);
    rayInfo.baseColor = albedo;
    rayInfo.ior = 1.5;
    rayInfo.opacity = 1.0;
    rayInfo.shadingMode = 0.0;

    rayInfo.roughness = roughness;
    rayInfo.metallic = metallic;
    rayInfo.diffuseColor = albedo - albedo * metallic;
    rayInfo.specularColor = ComputeF0(specular, albedo, metallic);
}

// accelerationStructureEXT type -> OpTypeAccelerationStructureEXT instruction

// rayPayloadEXT storage qualifier -> RayPayloadEXT storage class
// rayPayloadInEXT storage qualifier -> IncomingRayPayloadEXT storage class
// hitAttributeEXT storage qualifier -> HitAttributeEXT storage class
// callableDataEXT storage qualifier -> CallableDataEXT storage class
// callableDataInEXT storage qualifier -> IncomingCallableDataEXT storage class

// shaderRecordEXT decorated buffer block -> ShaderRecordBufferEXT storage class

// gl_LaunchIDEXT -> LaunchIdEXT decorated OpVariable
// gl_LaunchSizeEXT -> LaunchSizeEXT decorated OpVariable
// gl_PrimitiveID -> PrimitiveId decorated OpVariable
// gl_InstanceID -> InstanceId decorated OpVariable
// gl_InstanceCustomIndexEXT -> InstanceCustomIndexEXT decorated OpVariable
// gl_WorldRayOriginEXT -> WorldRayOriginEXT decorated OpVariable
// gl_WorldRayDirectionEXT -> WorldRayDirectionEXT decorated OpVariable
// gl_ObjectRayOriginEXT -> ObjectRayOriginEXT decorated OpVariable
// gl_ObjectRayDirectionEXT -> ObjectRayDirectionEXT decorated OpVariable
// gl_RayTminEXT -> RayTminEXT decorated OpVariable
// gl_RayTmaxEXT -> RayTmaxEXT decorated OpVariable
// gl_IncomingRayFlagsEXT -> IncomingRayFlagsEXT decorated OpVariable
// gl_HitTEXT -> HitTEXT decorated OpVariable
// gl_HitKindEXT -> HitKindEXT decorated OpVariable
// gl_ObjectToWorldEXT -> ObjectToWorldEXT decorated OpVariable
// gl_WorldToObjectEXT -> WorldToObjectEXT decorated OpVariable

// gl_RayFlagsNoneEXT -> constant, no semantic needed
// gl_RayFlagsOpaqueEXT -> constant, no semantic needed
// gl_RayFlagsNoOpaqueEXT -> constant, no semantic needed
// gl_RayFlagsTerminateOnFirstHitEXT -> constant, no semantic needed
// gl_RayFlagsSkipClosestHitShaderEXT -> constant, no semantic needed
// gl_RayFlagsCullBackFacingTrianglesEXT -> constant, no semantic needed
// gl_RayFlagsCullFrontFacingTrianglesEXT -> constant, no semantic needed
// gl_RayFlagsCullOpaqueEXT -> constant, no semantic needed
// gl_RayFlagsCullNoOpaqueEXT -> constant, no semantic needed

// traceEXT -> OpTraceEXT instruction
// reportIntersectionEXT -> OpReportIntersectionEXT instruction
// ignoreIntersectionEXT -> OpIgnoreIntersectionEXT instruction
// terminateRayEXT -> OpTerminateRayEXT instruction
// executeCallableEXT -> OpExecuteCallableEXT instruction