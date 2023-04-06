#pragma once

#undef  PI
#define PI 					(3.1415926535897932f)
#define SMALL_NUMBER		(1.e-8f)
#define KINDA_SMALL_NUMBER	(1.e-4f)
#define BIG_NUMBER			(3.4e+38f)
#define EULERS_NUMBER       (2.71828182845904523536f)

#define MAX_FLT				3.402823466e+38F
#define INV_PI				(0.31830988618f)
#define HALF_PI				(1.57079632679f)
#define DELTA				(0.00001f)

#define MIN_uint8		((uint8)	0x00)
#define	MIN_uint16		((uint16)	0x0000)
#define	MIN_uint32		((uint32)	0x00000000)
#define MIN_uint64		((uint64)	0x0000000000000000)
#define MIN_int8		((int8)		-128)
#define MIN_int16		((int16)	-32768)
#define MIN_int32		((int32)	0x80000000)
#define MIN_int64		((int64)	0x8000000000000000)

#define MAX_uint8		((uint8)	0xff)
#define MAX_uint16		((uint16)	0xffff)
#define MAX_uint32		((uint32)	0xffffffff)
#define MAX_uint64		((uint64)	0xffffffffffffffff)
#define MAX_int8		((int8)		0x7f)
#define MAX_int16		((int16)	0x7fff)
#define MAX_int32		((int32)	0x7fffffff)
#define MAX_int64		((int64)	0x7fffffffffffffff)

#define MIN_flt			(1.175494351e-38F)			
#define MAX_flt			(3.402823466e+38F)
#define MIN_dbl			(2.2250738585072014e-308)	
#define MAX_dbl			(1.7976931348623158e+308)	

#define THRESH_POINT_ON_PLANE			(0.10f)		
#define THRESH_POINT_ON_SIDE			(0.20f)		
#define THRESH_POINTS_ARE_SAME			(0.00002f)	
#define THRESH_POINTS_ARE_NEAR			(0.015f)	
#define THRESH_NORMALS_ARE_SAME			(0.00002f)	
#define THRESH_UVS_ARE_SAME			    (0.0009765625f)
#define THRESH_VECTORS_ARE_NEAR			(0.0004f)	
													
#define THRESH_SPLIT_POLY_WITH_PLANE	(0.25f)		
#define THRESH_SPLIT_POLY_PRECISELY		(0.01f)		
#define THRESH_ZERO_NORM_SQUARED		(0.0001f)	
#define THRESH_NORMALS_ARE_PARALLEL		(0.999845f)	
#define THRESH_NORMALS_ARE_ORTHOGONAL	(0.017455f)	

#define THRESH_VECTOR_NORMALIZED		(0.01f)		
#define THRESH_QUAT_NORMALIZED			(0.01f)