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