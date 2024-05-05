#define PI 3.1415926
const float RCP_PI = 1 / PI; // rcp：求逆
const float TwoPI = 2 * PI;
//==============直角坐标系下的3阶球谐函数============//

//l = 0,m = 0
float GetY00(vec3 xyz){
    return 0.5 * sqrt(RCP_PI);
}

//l = 1,m = 0
float GetY10(vec3 p){
    return 0.5 * sqrt(3 * RCP_PI) * p.z;
}

//l = 1,m = 1
float GetY1p1(vec3 p){
    return 0.5 * sqrt(3 * RCP_PI) * p.x;
}

//l = 1,m = -1
float GetY1n1(vec3 p){
    return 0.5 * sqrt(3 * RCP_PI) * p.y;
}

//l = 2, m = 0
float GetY20(vec3 p){
    return 0.25 * sqrt(5 * RCP_PI) * (2*p.z*p.z - p.x*p.x -p.y*p.y);
}

//l = 2, m = 1
float GetY2p1(vec3 p){
    return 0.5 * sqrt(15 * RCP_PI) * p.z * p.x;
}

//l = 2, m = -1
float GetY2n1(vec3 p){
    return 0.5 * sqrt(15 * RCP_PI) * p.z * p.y;
}

//l = 2, m = 2
float GetY2p2(vec3 p){
    return 0.25 * sqrt(15 * RCP_PI) * (p.x * p.x - p.y * p.y);
}

//l = 2, m = -2
float GetY2n2(vec3 p){
    return 0.5 * sqrt(15 * RCP_PI) * p.x * p.y;
}

//==============极坐标系下的3阶球谐函数============//

//l = 0,m = 0
float GetY00(float theta,float phi){
    return 0.5 * sqrt(RCP_PI);
}

//l = 1,m = 0
float GetY10(float theta,float phi){
    return 0.5 * sqrt(3 * RCP_PI) * cos(theta);
}

//l = 1,m = 1
float GetY1p1(float theta,float phi){
    return 0.5 * sqrt(3 * RCP_PI) * sin(theta) * cos(phi);
}

//l = 1,m = -1
float GetY1n1(float theta,float phi){
    return 0.5 * sqrt(3 * RCP_PI) * sin(theta) * sin(phi);
}

//l = 2, m = 0
float GetY20(float theta,float phi){
    float c = cos(theta);
    return 0.25 * sqrt(5 * RCP_PI) * (3 * c * c - 1);
}

//l = 2, m = 1
float GetY2p1(float theta,float phi){
    return 0.5 * sqrt(15 * RCP_PI) * sin(theta) * cos(theta) * cos(phi);
}

//l = 2, m = -1
float GetY2n1(float theta,float phi){
    return 0.5 * sqrt(15 * RCP_PI) * sin(theta) * cos(theta) * sin(phi);
}

//l = 2, m = 2
float GetY2p2(float theta,float phi){
    float s = sin(theta);
    return 0.25 * sqrt(15 * RCP_PI) * s * s * cos(2 * phi);
}

//l = 2, m = -2
float GetY2n2(float theta,float phi){
    float s = sin(theta);
    return 0.25 * sqrt(15 * RCP_PI) * s * s * sin(2 * phi);
}

float radicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

// 不重复随机序列
vec2 sampleHammersley(uint i,float InvTotalNumSamples)
{
	return vec2(i * InvTotalNumSamples, radicalInverse_VdC(i));
}

// 均匀采样球面
vec3 samplesphere(float u1, float u2)
{
	float theta = acos(1 - u1 * 2.0 );
    float phi = 2 * PI * u2;

    return vec3(sin(theta) * cos(phi),sin(theta) * sin(phi),cos(theta));
}