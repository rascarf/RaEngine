#pragma once
#include "Core/Core.h"
#include "Core/MathDefine.h"

namespace  ReEngine
{
    class Math
    {
    public:

    		static CONSTEXPR FORCE_INLINE float TruncToFloat(float f)
	{
		return (float)TruncToInt(f);
	}

	static CONSTEXPR FORCE_INLINE int32 TruncToInt(float f)
	{
		return (int32)(f);
	}

	static FORCE_INLINE int32 FloorToInt(float f)
	{
		return TruncToInt(floorf(f));
	}

	static FORCE_INLINE float FloorToFloat(float f)
	{
		return floorf(f);
	}

	static FORCE_INLINE double FloorToDouble(double f)
	{
		return floor(f);
	}

	static FORCE_INLINE int32 RoundToInt(float f)
	{
		return FloorToInt(f + 0.5f);
	}

	static FORCE_INLINE float RoundToFloat(float f)
	{
		return FloorToFloat(f + 0.5f);
	}

	static FORCE_INLINE double RoundToDouble(double f)
	{
		return FloorToDouble(f + 0.5);
	}

	static FORCE_INLINE int32 CeilToInt(float f)
	{
		return TruncToInt(ceilf(f));
	}

	static FORCE_INLINE float CeilToFloat(float f)
	{
		return ceilf(f);
	}

	static FORCE_INLINE double CeilToDouble(double f)
	{
		return ceil(f);
	}

	static FORCE_INLINE float Fractional(float value)
	{
		return value - TruncToFloat(value);
	}

	static FORCE_INLINE float Frac(float value)
	{
		return value - FloorToFloat(value);
	}

	static FORCE_INLINE float Modf(const float value, float* outIntPart)
	{
		return modff(value, outIntPart);
	}

	static FORCE_INLINE double Modf(const double value, double* outIntPart)
	{
		return modf(value, outIntPart);
	}

	static FORCE_INLINE float Exp(float value)
	{ 
		return expf(value);
	}

	static FORCE_INLINE float Exp2(float value)
	{ 
		return powf(2.f, value);
	}

	static FORCE_INLINE float Loge(float value)
	{ 
		return logf(value);
	}

	static FORCE_INLINE float LogX(float base, float value)
	{ 
		return Loge(value) / Loge(base);
	}

	static FORCE_INLINE float Log2(float value)
	{ 
		return Loge(value) * 1.4426950f;
	}

	static FORCE_INLINE float Fmod(float x, float y)
	{
		if (fabsf(y) <= 1.e-8f) {
			return 0.f;
		}

		const float quotient = TruncToFloat(x / y);
		float intPortion = y * quotient;

		if (fabsf(intPortion) > fabsf(x)) {
			intPortion = x;
		}

		const float result = x - intPortion;
		return result;
	}
	
	static FORCE_INLINE float Sin(float value) 
	{ 
		return sinf(value);
	}

	static FORCE_INLINE float Asin(float value)
	{ 
		return asinf((value < -1.f) ? -1.f : ((value < 1.f) ? value : 1.f));
	}

	static FORCE_INLINE float Sinh(float value)
	{ 
		return sinhf(value);
	}

	static FORCE_INLINE float Cos(float value)
	{ 
		return cosf(value);
	}

	static FORCE_INLINE float Acos(float value)
	{ 
		return acosf((value < -1.f) ? -1.f : ((value < 1.f) ? value : 1.f));
	}

	static FORCE_INLINE float Tan(float value)
	{ 
		return tanf(value);
	}

	static FORCE_INLINE float Atan(float value)
	{ 
		return atanf(value);
	}

	static FORCE_INLINE float Sqrt(float value)
	{ 
		return sqrtf(value);
	}

	static FORCE_INLINE float Pow(float a, float b) 
	{ 
		return powf(a, b);
	}

	static FORCE_INLINE float InvSqrt(float f)
	{
		return 1.0f / sqrtf(f);
	}

	static FORCE_INLINE float InvSqrtEst(float f)
	{
		return InvSqrt(f);
	}

	static FORCE_INLINE bool IsNaN(float f)
	{
		return ((*(uint32*)&f) & 0x7FFFFFFF) > 0x7F800000;
	}

	static FORCE_INLINE bool IsFinite(float f)
	{
		return ((*(uint32*)&f) & 0x7F800000) != 0x7F800000;
	}

	static FORCE_INLINE bool IsNegativeFloat(const float& f)
	{
		return ((*(uint32*)&f) >= (uint32)0x80000000);
	}

	static FORCE_INLINE bool IsNegativeDouble(const double& f)
	{
		return ((*(uint64*)&f) >= (uint64)0x8000000000000000);
	}

	static FORCE_INLINE int32 Rand() 
	{ 
		return rand();
	}

	static FORCE_INLINE void RandInit(int32 seed) 
	{ 
		srand(seed);
	}

	static FORCE_INLINE float FRand() 
	{ 
		return Rand() / (float)RAND_MAX;
	}

	static FORCE_INLINE uint32 FloorLog2(uint32 value)
	{
		uint32 pos = 0;

		if (value >= 1 << 16) 
		{ 
			value >>= 16; 
			pos += 16;
		}

		if (value >= 1 << 8) 
		{ 
			value >>= 8; 
			pos += 8;
		}

		if (value >= 1 << 4) 
		{ 
			value >>= 4; 
			pos += 4;
		}

		if (value >= 1 << 2) 
		{ 
			value >>= 2; 
			pos += 2;
		}

		if (value >= 1 << 1) 
		{ 
			pos += 1;
		}

		return (value == 0) ? 0 : pos;
	}

	static FORCE_INLINE uint64 FloorLog2_64(uint64 value)
	{
		uint64 pos = 0;
		if (value >= 1ull << 32) 
		{ 
			value >>= 32; 
			pos += 32;
		}

		if (value >= 1ull << 16) 
		{ 
			value >>= 16; 
			pos += 16;
		}

		if (value >= 1ull << 8) 
		{ 
			value >>= 8; 
			pos += 8;
		}

		if (value >= 1ull << 4) 
		{ 
			value >>= 4;
			pos += 4;
		}

		if (value >= 1ull << 2) 
		{ 
			value >>= 2; 
			pos += 2;
		}

		if (value >= 1ull << 1) 
		{ 
			pos += 1;
		}

		return (value == 0) ? 0 : pos;
	}

	static FORCE_INLINE uint32 CountLeadingZeros(uint32 value)
	{
		if (value == 0) {
			return 32;
		}
		return 31 - FloorLog2(value);
	}

	static FORCE_INLINE uint64 CountLeadingZeros64(uint64 value)
	{
		if (value == 0) {
			return 64;
		}
		return 63 - FloorLog2_64(value);
	}

	static FORCE_INLINE uint32 CountTrailingZeros(uint32 value)
	{
		if (value == 0) {
			return 32;
		}

		uint32 result = 0;

		while ((value & 1) == 0) 
		{
			value >>= 1;
			++result;
		}

		return result;
	}

	static FORCE_INLINE uint64 CountTrailingZeros64(uint64 value)
	{
		if (value == 0) {
			return 64;
		}

		uint64 result = 0;
		while ((value & 1) == 0) 
		{
			value >>= 1;
			++result;
		}

		return result;
	}

	static FORCE_INLINE uint32 CeilLogTwo(uint32 value)
	{
		int32 bitmask = ((int32)(CountLeadingZeros(value) << 26)) >> 31;
		return (32 - CountLeadingZeros(value - 1)) & (~bitmask);
	}

	static FORCE_INLINE uint64 CeilLogTwo64(uint64 value)
	{
		int64 bitmask = ((int64)(CountLeadingZeros64(value) << 57)) >> 63;
		return (64 - CountLeadingZeros64(value - 1)) & (~bitmask);
	}

	static FORCE_INLINE uint32 RoundUpToPowerOfTwo(uint32 value)
	{
		return 1 << CeilLogTwo(value);
	}

	static FORCE_INLINE uint64 RoundUpToPowerOfTwo64(uint64 value)
	{
		return uint64(1) << CeilLogTwo64(value);
	}

	static FORCE_INLINE uint32 MortonCode2(uint32 x)
	{
		x &= 0x0000ffff;
		x = (x ^ (x << 8)) & 0x00ff00ff;
		x = (x ^ (x << 4)) & 0x0f0f0f0f;
		x = (x ^ (x << 2)) & 0x33333333;
		x = (x ^ (x << 1)) & 0x55555555;
		return x;
	}

	static FORCE_INLINE uint32 ReverseMortonCode2(uint32 x)
	{
		x &= 0x55555555;
		x = (x ^ (x >> 1)) & 0x33333333;
		x = (x ^ (x >> 2)) & 0x0f0f0f0f;
		x = (x ^ (x >> 4)) & 0x00ff00ff;
		x = (x ^ (x >> 8)) & 0x0000ffff;
		return x;
	}

	static FORCE_INLINE uint32 MortonCode3(uint32 x)
	{
		x &= 0x000003ff;
		x = (x ^ (x << 16)) & 0xff0000ff;
		x = (x ^ (x << 8)) & 0x0300f00f;
		x = (x ^ (x << 4)) & 0x030c30c3;
		x = (x ^ (x << 2)) & 0x09249249;
		return x;
	}

	static FORCE_INLINE uint32 ReverseMortonCode3(uint32 x)
	{
		x &= 0x09249249;
		x = (x ^ (x >> 2)) & 0x030c30c3;
		x = (x ^ (x >> 4)) & 0x0300f00f;
		x = (x ^ (x >> 8)) & 0xff0000ff;
		x = (x ^ (x >> 16)) & 0x000003ff;
		return x;
	}

	static CONSTEXPR FORCE_INLINE float FloatSelect(float comparand, float valueGEZero, float valueLTZero)
	{
		return comparand >= 0.f ? valueGEZero : valueLTZero;
	}

	static CONSTEXPR FORCE_INLINE double FloatSelect(double comparand, double valueGEZero, double valueLTZero)
	{
		return comparand >= 0.f ? valueGEZero : valueLTZero;
	}

	template< class T >
	static CONSTEXPR FORCE_INLINE T Abs(const T a)
	{
		return (a >= (T)0) ? a : -a;
	}

	template< class T >
	static CONSTEXPR FORCE_INLINE T Sign(const T a)
	{
		return (a > (T)0) ? (T)1 : ((a < (T)0) ? (T)-1 : (T)0);
	}

	template< class T >
	static CONSTEXPR FORCE_INLINE T Max(const T a, const T b)
	{
		return (a >= b) ? a : b;
	}

	template< class T >
	static CONSTEXPR FORCE_INLINE T Min(const T a, const T b)
	{
		return (a <= b) ? a : b;
	}

	static FORCE_INLINE int32 CountBits(uint64 bits)
	{
		bits -= (bits >> 1) & 0x5555555555555555ull;
		bits = (bits & 0x3333333333333333ull) + ((bits >> 2) & 0x3333333333333333ull);
		bits = (bits + (bits >> 4)) & 0x0f0f0f0f0f0f0f0full;
		return (bits * 0x0101010101010101) >> 56;
	}

	template< class T >
	static FORCE_INLINE T Min(const std::vector<T>& values, int32* minIndex = NULL)
	{
		if (values.size() == 0)
		{
			if (minIndex) {
				*minIndex = -1;
			}
			return T();
		}

		T curMin = values[0];
		int32 curMinIndex = 0;
		for (int32 v = 1; v < values.size(); ++v)
		{
			const T value = values[v];
			if (value < curMin) 
			{
				curMin = value;
				curMinIndex = v;
			}
		}

		if (minIndex) {
			*minIndex = curMinIndex;
		}

		return curMin;
	}

	template< class T >
	static FORCE_INLINE T Max(const std::vector<T>& values, int32* maxIndex = NULL)
	{
		if (values.size() == 0)
		{
			if (maxIndex) {
				*maxIndex = -1;
			}
			return T();
		}

		T curMax = values[0];
		int32 curMaxIndex = 0;
		for (int32 v = 1; v < values.size(); ++v)
		{
			const T value = values[v];
			if (curMax < value) 
			{
				curMax = value;
				curMaxIndex = v;
			}
		}

		if (maxIndex) {
			*maxIndex = curMaxIndex;
		}

		return curMax;
	}

	static void SRandInit(int32 seed);

	static int32 GetRandSeed();

	static float SRand();

	static float Atan2(float y, float x);

    	
public:
	static const uint32 BitFlag[32];

public:
	static FORCE_INLINE int32 RandHelper(int32 value)
	{
		return value > 0 ? Min(TruncToInt(FRand() * value), value - 1) : 0;
	}

	static FORCE_INLINE int32 RandRange(int32 inMin, int32 inMax)
	{
		const int32 range = (inMax - inMin) + 1;
		return inMin + RandHelper(range);
	}

	static FORCE_INLINE float RandRange(float inMin, float inMax)
	{
		return FRandRange(inMin, inMax);
	}

	static FORCE_INLINE float FRandRange(float inMin, float inMax)
	{
		return inMin + (inMax - inMin) * FRand();
	}

	static FORCE_INLINE bool RandBool()
	{
		return (RandRange(0, 1) == 1) ? true : false;
	}

	static FORCE_INLINE void Inverse4x4(double* dst, const float* src)
	{
		const double s0 = (double)(src[0]);
		const double s1 = (double)(src[1]);
		const double s2 = (double)(src[2]);
		const double s3 = (double)(src[3]);
		const double s4 = (double)(src[4]);
		const double s5 = (double)(src[5]);
		const double s6 = (double)(src[6]);
		const double s7 = (double)(src[7]);
		const double s8 = (double)(src[8]);
		const double s9 = (double)(src[9]);
		const double s10 = (double)(src[10]);
		const double s11 = (double)(src[11]);
		const double s12 = (double)(src[12]);
		const double s13 = (double)(src[13]);
		const double s14 = (double)(src[14]);
		const double s15 = (double)(src[15]);

		double inv[16];
		inv[0] = s5 * s10 * s15 - s5 * s11 * s14 - s9 * s6 * s15 + s9 * s7 * s14 + s13 * s6 * s11 - s13 * s7 * s10;
		inv[1] = -s1 * s10 * s15 + s1 * s11 * s14 + s9 * s2 * s15 - s9 * s3 * s14 - s13 * s2 * s11 + s13 * s3 * s10;
		inv[2] = s1 * s6  * s15 - s1 * s7  * s14 - s5 * s2 * s15 + s5 * s3 * s14 + s13 * s2 * s7 - s13 * s3 * s6;
		inv[3] = -s1 * s6  * s11 + s1 * s7  * s10 + s5 * s2 * s11 - s5 * s3 * s10 - s9 * s2 * s7 + s9 * s3 * s6;
		inv[4] = -s4 * s10 * s15 + s4 * s11 * s14 + s8 * s6 * s15 - s8 * s7 * s14 - s12 * s6 * s11 + s12 * s7 * s10;
		inv[5] = s0 * s10 * s15 - s0 * s11 * s14 - s8 * s2 * s15 + s8 * s3 * s14 + s12 * s2 * s11 - s12 * s3 * s10;
		inv[6] = -s0 * s6  * s15 + s0 * s7  * s14 + s4 * s2 * s15 - s4 * s3 * s14 - s12 * s2 * s7 + s12 * s3 * s6;
		inv[7] = s0 * s6  * s11 - s0 * s7  * s10 - s4 * s2 * s11 + s4 * s3 * s10 + s8 * s2 * s7 - s8 * s3 * s6;
		inv[8] = s4 * s9  * s15 - s4 * s11 * s13 - s8 * s5 * s15 + s8 * s7 * s13 + s12 * s5 * s11 - s12 * s7 * s9;
		inv[9] = -s0 * s9  * s15 + s0 * s11 * s13 + s8 * s1 * s15 - s8 * s3 * s13 - s12 * s1 * s11 + s12 * s3 * s9;
		inv[10] = s0 * s5  * s15 - s0 * s7  * s13 - s4 * s1 * s15 + s4 * s3 * s13 + s12 * s1 * s7 - s12 * s3 * s5;
		inv[11] = -s0 * s5  * s11 + s0 * s7  * s9 + s4 * s1 * s11 - s4 * s3 * s9 - s8 * s1 * s7 + s8 * s3 * s5;
		inv[12] = -s4 * s9  * s14 + s4 * s10 * s13 + s8 * s5 * s14 - s8 * s6 * s13 - s12 * s5 * s10 + s12 * s6 * s9;
		inv[13] = s0 * s9  * s14 - s0 * s10 * s13 - s8 * s1 * s14 + s8 * s2 * s13 + s12 * s1 * s10 - s12 * s2 * s9;
		inv[14] = -s0 * s5  * s14 + s0 * s6  * s13 + s4 * s1 * s14 - s4 * s2 * s13 - s12 * s1 * s6 + s12 * s2 * s5;
		inv[15] = s0 * s5  * s10 - s0 * s6  * s9 - s4 * s1 * s10 + s4 * s2 * s9 + s8 * s1 * s6 - s8 * s2 * s5;

		double det = s0 * inv[0] + s1 * inv[4] + s2 * inv[8] + s3 * inv[12];
		if (det != 0.0) {
			det = 1.0 / det;
		}

		for (int32 i = 0; i < 16; i++) {
			dst[i] = inv[i] * det;
		}
	}

	template< class U >
	static FORCE_INLINE bool IsWithin(const U& testValue, const U& minValue, const U& maxValue)
	{
		return ((testValue >= minValue) && (testValue < maxValue));
	}

	template< class U >
	static FORCE_INLINE bool IsWithinInclusive(const U& testValue, const U& minValue, const U& maxValue)
	{
		return ((testValue >= minValue) && (testValue <= maxValue));
	}

	static FORCE_INLINE bool IsNearlyEqual(float a, float b, float errorTolerance = SMALL_NUMBER)
	{
		return Abs<float>(a - b) <= errorTolerance;
	}

	static FORCE_INLINE bool IsNearlyEqual(double a, double b, double errorTolerance = SMALL_NUMBER)
	{
		return Abs<double>(a - b) <= errorTolerance;
	}

	static FORCE_INLINE bool IsNearlyZero(float value, float errorTolerance = SMALL_NUMBER)
	{
		return Abs<float>(value) <= errorTolerance;
	}

	static FORCE_INLINE bool IsNearlyZero(double value, double errorTolerance = SMALL_NUMBER)
	{
		return Abs<double>(value) <= errorTolerance;
	}

	template <typename T>
	static FORCE_INLINE bool IsPowerOfTwo(T value)
	{
		return ((value & (value - 1)) == (T)0);
	}

	template< class T >
	static FORCE_INLINE T Max3(const T a, const T b, const T c)
	{
		return Max(Max(a, b), c);
	}

	template< class T >
	static FORCE_INLINE T Min3(const T a, const T b, const T c)
	{
		return Min(Min(a, b), c);
	}

	template< class T >
	static FORCE_INLINE T Square(const T a)
	{
		return a * a;
	}

	template< class T >
	static FORCE_INLINE T Clamp(const T x, const T inMin, const T inMax)
	{
		return x < inMin ? inMin : x < inMax ? x : inMax;
	}

	static FORCE_INLINE float GridSnap(float location, float grid)
	{
		if (grid == 0.0f) {
			return location;
		}
		else {
			return FloorToFloat((location + 0.5f * grid) / grid) * grid;
		}
	}

	static FORCE_INLINE double GridSnap(double location, double grid)
	{
		if (grid == 0.0) {
			return location;
		}
		else {
			return FloorToDouble((location + 0.5 * grid) / grid) * grid;
		}
	}

	template <class T>
	static FORCE_INLINE T DivideAndRoundUp(T dividend, T divisor)
	{
		return (dividend + divisor - 1) / divisor;
	}

	template <class T>
	static FORCE_INLINE T DivideAndRoundDown(T dividend, T divisor)
	{
		return dividend / divisor;
	}

	template <class T>
	static FORCE_INLINE T DivideAndRoundNearest(T dividend, T divisor)
	{
		return (dividend >= 0)
			? (dividend + divisor / 2) / divisor
			: (dividend - divisor / 2 + 1) / divisor;
	}
    	

	static FORCE_INLINE void SinCos(float* scalarSin, float* scalarCos, float value)
	{
		float quotient = (INV_PI * 0.5f) * value;
		if (value >= 0.0f) {
			quotient = (float)((int32)(quotient + 0.5f));
		}
		else {
			quotient = (float)((int32)(quotient - 0.5f));
		}

		float y = value - (2.0f * PI) * quotient;

		float sign;
		if (y > HALF_PI) {
			y = PI - y;
			sign = -1.0f;
		}
		else if (y < -HALF_PI) {
			y = -PI - y;
			sign = -1.0f;
		}
		else {
			sign = +1.0f;
		}

		float y2 = y * y;

		*scalarSin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;

		float p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
		*scalarCos = sign * p;
	}

#define FASTASIN_HALF_PI (1.5707963050f)

	static FORCE_INLINE float FastAsin(float value)
	{
		bool nonnegative = (value >= 0.0f);
		float x = Abs(value);
		float omx = 1.0f - x;
		if (omx < 0.0f) {
			omx = 0.0f;
		}
		float root = Sqrt(omx);
		float result = ((((((-0.0012624911f * x + 0.0066700901f) * x - 0.0170881256f) * x + 0.0308918810f) * x - 0.0501743046f) * x + 0.0889789874f) * x - 0.2145988016f) * x + FASTASIN_HALF_PI;
		result *= root;
		return (nonnegative ? FASTASIN_HALF_PI - result : result - FASTASIN_HALF_PI);
	}
#undef FASTASIN_HALF_PI

	template<class T>
	static FORCE_INLINE auto RadiansToDegrees(T const& radVal) -> decltype(radVal * (180.f / PI))
	{
		return radVal * (180.f / PI);
	}

	template<class T>
	static FORCE_INLINE auto DegreesToRadians(T const& degVal) -> decltype(degVal * (PI / 180.f))
	{
		return degVal * (PI / 180.f);
	}
    
    static FORCE_INLINE void VectorMatrixMultiply(void* result, const void* matrix1, const void* matrix2)
    {
        typedef float Float4x4[4][4];
        const Float4x4& a = *((const Float4x4*) matrix1);
        const Float4x4& b = *((const Float4x4*) matrix2);
        
        Float4x4 temp;
        temp[0][0] = a[0][0] * b[0][0] + a[0][1] * b[1][0] + a[0][2] * b[2][0] + a[0][3] * b[3][0];
        temp[0][1] = a[0][0] * b[0][1] + a[0][1] * b[1][1] + a[0][2] * b[2][1] + a[0][3] * b[3][1];
        temp[0][2] = a[0][0] * b[0][2] + a[0][1] * b[1][2] + a[0][2] * b[2][2] + a[0][3] * b[3][2];
        temp[0][3] = a[0][0] * b[0][3] + a[0][1] * b[1][3] + a[0][2] * b[2][3] + a[0][3] * b[3][3];
        
        temp[1][0] = a[1][0] * b[0][0] + a[1][1] * b[1][0] + a[1][2] * b[2][0] + a[1][3] * b[3][0];
        temp[1][1] = a[1][0] * b[0][1] + a[1][1] * b[1][1] + a[1][2] * b[2][1] + a[1][3] * b[3][1];
        temp[1][2] = a[1][0] * b[0][2] + a[1][1] * b[1][2] + a[1][2] * b[2][2] + a[1][3] * b[3][2];
        temp[1][3] = a[1][0] * b[0][3] + a[1][1] * b[1][3] + a[1][2] * b[2][3] + a[1][3] * b[3][3];
        
        temp[2][0] = a[2][0] * b[0][0] + a[2][1] * b[1][0] + a[2][2] * b[2][0] + a[2][3] * b[3][0];
        temp[2][1] = a[2][0] * b[0][1] + a[2][1] * b[1][1] + a[2][2] * b[2][1] + a[2][3] * b[3][1];
        temp[2][2] = a[2][0] * b[0][2] + a[2][1] * b[1][2] + a[2][2] * b[2][2] + a[2][3] * b[3][2];
        temp[2][3] = a[2][0] * b[0][3] + a[2][1] * b[1][3] + a[2][2] * b[2][3] + a[2][3] * b[3][3];
        
        temp[3][0] = a[3][0] * b[0][0] + a[3][1] * b[1][0] + a[3][2] * b[2][0] + a[3][3] * b[3][0];
        temp[3][1] = a[3][0] * b[0][1] + a[3][1] * b[1][1] + a[3][2] * b[2][1] + a[3][3] * b[3][1];
        temp[3][2] = a[3][0] * b[0][2] + a[3][1] * b[1][2] + a[3][2] * b[2][2] + a[3][3] * b[3][2];
        temp[3][3] = a[3][0] * b[0][3] + a[3][1] * b[1][3] + a[3][2] * b[2][3] + a[3][3] * b[3][3];
        
        memcpy(result, &temp, 16 * sizeof(float));
    }
    
    static FORCE_INLINE void VectorMatrixInverse(void* dstMatrix, const void* srcMatrix)
    {
        typedef float Float4x4[4][4];
        const Float4x4& m = *((const Float4x4*)srcMatrix);
        Float4x4 result;
        float det[4];
        Float4x4 tmp;
        
        tmp[0][0] = m[2][2] * m[3][3] - m[2][3] * m[3][2];
        tmp[0][1] = m[1][2] * m[3][3] - m[1][3] * m[3][2];
        tmp[0][2] = m[1][2] * m[2][3] - m[1][3] * m[2][2];
        
        tmp[1][0] = m[2][2] * m[3][3] - m[2][3] * m[3][2];
        tmp[1][1] = m[0][2] * m[3][3] - m[0][3] * m[3][2];
        tmp[1][2] = m[0][2] * m[2][3] - m[0][3] * m[2][2];
        
        tmp[2][0] = m[1][2] * m[3][3] - m[1][3] * m[3][2];
        tmp[2][1] = m[0][2] * m[3][3] - m[0][3] * m[3][2];
        tmp[2][2] = m[0][2] * m[1][3] - m[0][3] * m[1][2];
        
        tmp[3][0] = m[1][2] * m[2][3] - m[1][3] * m[2][2];
        tmp[3][1] = m[0][2] * m[2][3] - m[0][3] * m[2][2];
        tmp[3][2] = m[0][2] * m[1][3] - m[0][3] * m[1][2];
        
        det[0] = m[1][1] * tmp[0][0] - m[2][1] * tmp[0][1] + m[3][1] * tmp[0][2];
        det[1] = m[0][1] * tmp[1][0] - m[2][1] * tmp[1][1] + m[3][1] * tmp[1][2];
        det[2] = m[0][1] * tmp[2][0] - m[1][1] * tmp[2][1] + m[3][1] * tmp[2][2];
        det[3] = m[0][1] * tmp[3][0] - m[1][1] * tmp[3][1] + m[2][1] * tmp[3][2];
        
        float determinant = m[0][0] * det[0] - m[1][0] * det[1] + m[2][0] * det[2] - m[3][0] * det[3];
        const float rDet = 1.0f / determinant;
        
        result[0][0] =  rDet * det[0];
        result[0][1] = -rDet * det[1];
        result[0][2] =  rDet * det[2];
        result[0][3] = -rDet * det[3];
        result[1][0] = -rDet * (m[1][0]*tmp[0][0] - m[2][0]*tmp[0][1] + m[3][0]*tmp[0][2]);
        result[1][1] =  rDet * (m[0][0]*tmp[1][0] - m[2][0]*tmp[1][1] + m[3][0]*tmp[1][2]);
        result[1][2] = -rDet * (m[0][0]*tmp[2][0] - m[1][0]*tmp[2][1] + m[3][0]*tmp[2][2]);
        result[1][3] =  rDet * (m[0][0]*tmp[3][0] - m[1][0]*tmp[3][1] + m[2][0]*tmp[3][2]);
        result[2][0] =  rDet * (
                                m[1][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) -
                                m[2][0] * (m[1][1] * m[3][3] - m[1][3] * m[3][1]) +
                                m[3][0] * (m[1][1] * m[2][3] - m[1][3] * m[2][1])
                                );
        result[2][1] = -rDet * (
                                m[0][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) -
                                m[2][0] * (m[0][1] * m[3][3] - m[0][3] * m[3][1]) +
                                m[3][0] * (m[0][1] * m[2][3] - m[0][3] * m[2][1])
                                );
        result[2][2] =  rDet * (
                                m[0][0] * (m[1][1] * m[3][3] - m[1][3] * m[3][1]) -
                                m[1][0] * (m[0][1] * m[3][3] - m[0][3] * m[3][1]) +
                                m[3][0] * (m[0][1] * m[1][3] - m[0][3] * m[1][1])
                                );
        result[2][3] = -rDet * (
                                m[0][0] * (m[1][1] * m[2][3] - m[1][3] * m[2][1]) -
                                m[1][0] * (m[0][1] * m[2][3] - m[0][3] * m[2][1]) +
                                m[2][0] * (m[0][1] * m[1][3] - m[0][3] * m[1][1])
                                );
        result[3][0] = -rDet * (
                                m[1][0] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]) -
                                m[2][0] * (m[1][1] * m[3][2] - m[1][2] * m[3][1]) +
                                m[3][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
                                );
        result[3][1] =  rDet * (
                                m[0][0] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]) -
                                m[2][0] * (m[0][1] * m[3][2] - m[0][2] * m[3][1]) +
                                m[3][0] * (m[0][1] * m[2][2] - m[0][2] * m[2][1])
                                );
        result[3][2] = -rDet * (
                                m[0][0] * (m[1][1] * m[3][2] - m[1][2] * m[3][1]) -
                                m[1][0] * (m[0][1] * m[3][2] - m[0][2] * m[3][1]) +
                                m[3][0] * (m[0][1] * m[1][2] - m[0][2] * m[1][1])
                                );
        result[3][3] =  rDet * (
                                m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1]) -
                                m[1][0] * (m[0][1] * m[2][2] - m[0][2] * m[2][1]) +
                                m[2][0] * (m[0][1] * m[1][2] - m[0][2] * m[1][1])
                                );
        
        memcpy(dstMatrix, &result, 16 * sizeof(float));
    }
    
    static FORCE_INLINE void VectorTransformVector(void* result, const void* vec,  const void* matrix)
    {
        typedef float Float4[4];
        typedef float Float4x4[4][4];
        
        const Float4& vec4  = *((const Float4*)vec);
        const Float4x4& m44 = *((const Float4x4*)matrix);
        Float4& rVec4 = *((Float4*)result);
        
        rVec4[0] = vec4[0] * m44[0][0] + vec4[1] * m44[1][0] + vec4[2] * m44[2][0] + vec4[3] * m44[3][0];
        rVec4[1] = vec4[0] * m44[0][1] + vec4[1] * m44[1][1] + vec4[2] * m44[2][1] + vec4[3] * m44[3][1];
        rVec4[2] = vec4[0] * m44[0][2] + vec4[1] * m44[1][2] + vec4[2] * m44[2][2] + vec4[3] * m44[3][2];
        rVec4[3] = vec4[0] * m44[0][3] + vec4[1] * m44[1][3] + vec4[2] * m44[2][3] + vec4[3] * m44[3][3];
    }
    
	static FORCE_INLINE void VectorQuaternionMultiply(void* result, const void* quat1, const void* quat2)
	{
		typedef float Float4[4];
		const Float4& a = *((const Float4*)quat1);
		const Float4& b = *((const Float4*)quat2);
		Float4& r = *((Float4*)result);

		const float t0 = (a[2] - a[1]) * (b[1] - b[2]);
		const float t1 = (a[3] + a[0]) * (b[3] + b[0]);
		const float t2 = (a[3] - a[0]) * (b[1] + b[2]);
		const float t3 = (a[1] + a[2]) * (b[3] - b[0]);
		const float t4 = (a[2] - a[0]) * (b[0] - b[1]);
		const float t5 = (a[2] + a[0]) * (b[0] + b[1]);
		const float t6 = (a[3] + a[1]) * (b[3] - b[2]);
		const float t7 = (a[3] - a[1]) * (b[3] + b[2]);
		const float t8 = t5 + t6 + t7;
		const float t9 = 0.5f * (t4 + t8);

		r[0] = t1 + t9 - t8;
		r[1] = t2 + t9 - t7;
		r[2] = t3 + t9 - t6;
		r[3] = t0 + t9 - t5;
	}

	static FORCE_INLINE float FindDeltaAngleDegrees(float a1, float a2)
	{
		float delta = a2 - a1;

		if (delta > 180.0f) {
			delta = delta - 360.0f;
		}
		else if (delta < -180.0f) {
			delta = delta + 360.0f;
		}

		return delta;
	}

	static FORCE_INLINE float FindDeltaAngleRadians(float a1, float a2)
	{
		float delta = a2 - a1;

		if (delta > PI) {
			delta = delta - (PI * 2.0f);
		}
		else if (delta < -PI) {
			delta = delta + (PI * 2.0f);
		}

		return delta;
	}

	static FORCE_INLINE float UnwindRadians(float value)
	{
		while (value > PI) {
			value -= ((float)PI * 2.0f);
		}

		while (value < -PI) {
			value += ((float)PI * 2.0f);
		}

		return value;
	}

	static FORCE_INLINE float UnwindDegrees(float value)
	{
		while (value > 180.f) {
			value -= 360.f;
		}

		while (value < -180.f) {
			value += 360.f;
		}

		return value;
	}

	static FORCE_INLINE void CartesianToPolar(const float x, const float y, float& outRad, float& outAng)
	{
		outRad = Sqrt(Square(x) + Square(y));
		outAng = Atan2(y, x);
	}
	
	static FORCE_INLINE void PolarToCartesian(const float rad, const float ang, float& outX, float& outY)
	{
		outX = rad * Cos(ang);
		outY = rad * Sin(ang);
	}

	static FORCE_INLINE float GetRangePct(float minValue, float maxValue, float value)
	{
		const float divisor = maxValue - minValue;
		if (IsNearlyZero(divisor)) {
			return (value >= maxValue) ? 1.f : 0.f;
		}

		return (value - minValue) / divisor;
	}

	template< class T, class U >
	static FORCE_INLINE T Lerp(const T& a, const T& b, const U& alpha)
	{
		return (T)(a + alpha * (b - a));
	}

	template< class T >
	static FORCE_INLINE T LerpStable(const T& a, const T& b, double alpha)
	{
		return (T)((a * (1.0 - alpha)) + (b * alpha));
	}

	template< class T >
	static FORCE_INLINE T LerpStable(const T& a, const T& b, float alpha)
	{
		return (T)((a * (1.0f - alpha)) + (b * alpha));
	}

	template< class T, class U >
	static FORCE_INLINE T BiLerp(const T& p00, const T& p10, const T& p01, const T& p11, const U& fracX, const U& fracY)
	{
		return Lerp(
			Lerp(p00, p10, fracX),
			Lerp(p01, p11, fracX),
			fracY
		);
	}

	template< class T, class U >
	static FORCE_INLINE T CubicInterp(const T& p0, const T& t0, const T& p1, const T& t1, const U& a)
	{
		const float a2 = a * a;
		const float a3 = a2 * a;

		return (T)(((2 * a3) - (3 * a2) + 1) * p0) + ((a3 - (2 * a2) + a) * t0) + ((a3 - a2) * t1) + (((-2 * a3) + (3 * a2)) * p1);
	}

	template< class T, class U >
	static FORCE_INLINE T CubicInterpDerivative(const T& p0, const T& t0, const T& p1, const T& t1, const U& a)
	{
		T x =  6.f * p0 + 3.f * t0 + 3.f * t1 - 6.f * p1;
		T y = -6.f * p0 - 4.f * t0 - 2.f * t1 + 6.f * p1;
		T z = t0;
		const float a2 = a * a;
		return (x * a2) + (y * a) + z;
	}

	template< class T, class U >
	static FORCE_INLINE T CubicInterpSecondDerivative(const T& p0, const T& t0, const T& p1, const T& t1, const U& a)
	{
		T x = 12.f * p0 + 6.f * t0 + 6.f * t1 - 12.f * p1;
		T y = -6.f * p0 - 4.f * t0 - 2.f * t1 + 6.f  * p1;

		return (x * a) + y;
	}

	template< class T >
	static FORCE_INLINE T InterpEaseIn(const T& a, const T& b, float alpha, float exp)
	{
		float const modifiedAlpha = Pow(alpha, exp);
		return Lerp<T>(a, b, modifiedAlpha);
	}

	template< class T >
	static FORCE_INLINE T InterpEaseOut(const T& a, const T& b, float alpha, float exp)
	{
		float const modifiedAlpha = 1.f - Pow(1.f - alpha, exp);
		return Lerp<T>(a, b, modifiedAlpha);
	}

	template< class T >
	static FORCE_INLINE T InterpEaseInOut(const T& a, const T& b, float alpha, float exp)
	{
		return Lerp<T>(a, b, (alpha < 0.5f) ?
			InterpEaseIn(0.f, 1.f, alpha * 2.f, exp) * 0.5f :
			InterpEaseOut(0.f, 1.f, alpha * 2.f - 1.f, exp) * 0.5f + 0.5f);
	}

	template< class T >
	static FORCE_INLINE T InterpStep(const T& a, const T& b, float alpha, int32 steps)
	{
		if (steps <= 1 || alpha <= 0) {
			return a;
		}
		else if (alpha >= 1) {
			return b;
		}

		const float stepsAsFloat = static_cast<float>(steps);
		const float numIntervals = stepsAsFloat - 1.f;
		float const modifiedAlpha = FloorToFloat(alpha * stepsAsFloat) / numIntervals;

		return Lerp<T>(a, b, modifiedAlpha);
	}

	template< class T >
	static FORCE_INLINE T InterpSinIn(const T& a, const T& b, float alpha)
	{
		float const modifiedAlpha = -1.f * Cos(alpha * HALF_PI) + 1.f;
		return Lerp<T>(a, b, modifiedAlpha);
	}

	template< class T >
	static FORCE_INLINE T InterpSinOut(const T& a, const T& b, float alpha)
	{
		float const modifiedAlpha = Sin(alpha * HALF_PI);
		return Lerp<T>(a, b, modifiedAlpha);
	}

	template< class T >
	static FORCE_INLINE T InterpSinInOut(const T& a, const T& b, float alpha)
	{
		return Lerp<T>(a, b, (alpha < 0.5f) ?
			InterpSinIn(0.f, 1.f, alpha * 2.f) * 0.5f :
			InterpSinOut(0.f, 1.f, alpha * 2.f - 1.f) * 0.5f + 0.5f);
	}

	template< class T >
	static FORCE_INLINE T InterpExpoIn(const T& a, const T& b, float alpha)
	{
		float const modifiedAlpha = (alpha == 0.f) ? 0.f : Pow(2.f, 10.f * (alpha - 1.f));
		return Lerp<T>(a, b, modifiedAlpha);
	}

	template< class T >
	static FORCE_INLINE T InterpExpoOut(const T& a, const T& b, float alpha)
	{
		float const modifiedAlpha = (alpha == 1.f) ? 1.f : -Pow(2.f, -10.f * alpha) + 1.f;
		return Lerp<T>(a, b, modifiedAlpha);
	}

	template< class T >
	static FORCE_INLINE T InterpExpoInOut(const T& a, const T& b, float alpha)
	{
		return Lerp<T>(a, b, (alpha < 0.5f) ?
			InterpExpoIn(0.f, 1.f, alpha * 2.f) * 0.5f :
			InterpExpoOut(0.f, 1.f, alpha * 2.f - 1.f) * 0.5f + 0.5f);
	}

	template< class T >
	static FORCE_INLINE T InterpCircularIn(const T& a, const T& b, float alpha)
	{
		float const modifiedAlpha = -1.f * (Sqrt(1.f - alpha * alpha) - 1.f);
		return Lerp<T>(a, b, modifiedAlpha);
	}

	template< class T >
	static FORCE_INLINE T InterpCircularOut(const T& a, const T& b, float alpha)
	{
		alpha -= 1.f;
		float const modifiedAlpha = Sqrt(1.f - alpha * alpha);
		return Lerp<T>(a, b, modifiedAlpha);
	}

	template< class T >
	static FORCE_INLINE T InterpCircularInOut(const T& a, const T& b, float alpha)
	{
		return Lerp<T>(a, b, (alpha < 0.5f) ?
			InterpCircularIn(0.f, 1.f, alpha * 2.f) * 0.5f :
			InterpCircularOut(0.f, 1.f, alpha * 2.f - 1.f) * 0.5f + 0.5f);
	}

	template<class U>
	static FORCE_INLINE U CubicCRSplineInterp(const U& p0, const U& p1, const U& p2, const U& p3, const float t0, const float t1, const float t2, const float t3, const float t)
	{
		float invT1MinusT0 = 1.0f / (t1 - t0);
		U l01 = (p0 * ((t1 - t) * invT1MinusT0)) + (p1 * ((t - t0) * invT1MinusT0));
		float invT2MinusT1 = 1.0f / (t2 - t1);
		U l12 = (p1 * ((t2 - t) * invT2MinusT1)) + (p2 * ((t - t1) * invT2MinusT1));
		float invT3MinusT2 = 1.0f / (t3 - t2);
		U l23 = (p2 * ((t3 - t) * invT3MinusT2)) + (p3 * ((t - t2) * invT3MinusT2));

		float invT2MinusT0 = 1.0f / (t2 - t0);
		U l012 = (l01 * ((t2 - t) * invT2MinusT0)) + (l12 * ((t - t0) * invT2MinusT0));
		float invT3MinusT1 = 1.0f / (t3 - t1);
		U l123 = (l12 * ((t3 - t) * invT3MinusT1)) + (l23 * ((t - t1) * invT3MinusT1));

		return  ((l012 * ((t2 - t) * invT2MinusT1)) + (l123 * ((t - t1) * invT2MinusT1)));
	}

	template< class U >
	static FORCE_INLINE U CubicCRSplineInterpSafe(const U& p0, const U& p1, const U& p2, const U& p3, const float t0, const float t1, const float t2, const float t3, const float t)
	{
		float t1MinusT0 = (t1 - t0);
		float t2MinusT1 = (t2 - t1);
		float t3MinusT2 = (t3 - t2);
		float t2MinusT0 = (t2 - t0);
		float t3MinusT1 = (t3 - t1);
		if (IsNearlyZero(t1MinusT0) || IsNearlyZero(t2MinusT1) || IsNearlyZero(t3MinusT2) || IsNearlyZero(t2MinusT0) || IsNearlyZero(t3MinusT1))
		{
			return p1;
		}

		float invT1MinusT0 = 1.0f / t1MinusT0;
		U l01 = (p0 * ((t1 - t) * invT1MinusT0)) + (p1 * ((t - t0) * invT1MinusT0));
		float invT2MinusT1 = 1.0f / t2MinusT1;
		U l12 = (p1 * ((t2 - t) * invT2MinusT1)) + (p2 * ((t - t1) * invT2MinusT1));
		float invT3MinusT2 = 1.0f / t3MinusT2;
		U l23 = (p2 * ((t3 - t) * invT3MinusT2)) + (p3 * ((t - t2) * invT3MinusT2));

		float invT2MinusT0 = 1.0f / t2MinusT0;
		U l012 = (l01 * ((t2 - t) * invT2MinusT0)) + (l12 * ((t - t0) * invT2MinusT0));
		float invT3MinusT1 = 1.0f / t3MinusT1;
		U l123 = (l12 * ((t3 - t) * invT3MinusT1)) + (l23 * ((t - t1) * invT3MinusT1));

		return  ((l012 * ((t2 - t) * invT2MinusT1)) + (l123 * ((t - t1) * invT2MinusT1)));
	}

	static FORCE_INLINE float MakePulsatingValue(const double inCurrentTime, const float inPulsesPerSecond, const float inPhase = 0.0f)
	{
		return (float)(0.5f + 0.5f * Sin(((0.25f + inPhase) * PI * 2.0f) + ((float)inCurrentTime * PI * 2.0f) * inPulsesPerSecond));
	}

	static FORCE_INLINE float RoundFromZero(float f)
	{
		return (f < 0.0f) ? FloorToFloat(f) : CeilToFloat(f);
	}

	static FORCE_INLINE double RoundFromZero(double f)
	{
		return (f < 0.0) ? FloorToDouble(f) : CeilToDouble(f);
	}

	static FORCE_INLINE float RoundToZero(float f)
	{
		return (f < 0.0f) ? CeilToFloat(f) : FloorToFloat(f);
	}

	static FORCE_INLINE double RoundToZero(double f)
	{
		return (f < 0.0) ? CeilToDouble(f) : FloorToDouble(f);
	}

	static FORCE_INLINE float RoundToNegativeInfinity(float f)
	{
		return FloorToFloat(f);
	}

	static FORCE_INLINE double RoundToNegativeInfinity(double f)
	{
		return FloorToDouble(f);
	}

	static FORCE_INLINE float RoundToPositiveInfinity(float f)
	{
		return CeilToFloat(f);
	}

	static FORCE_INLINE double RoundToPositiveInfinity(double f)
	{
		return CeilToDouble(f);
	}

	static FORCE_INLINE float SmoothStep(float a, float b, float X)
	{
		if (X < a) {
			return 0.0f;
		}
		else if (X >= b) {
			return 1.0f;
		}
		const float InterpFraction = (X - a) / (b - a);
		return InterpFraction * InterpFraction * (3.0f - 2.0f * InterpFraction);
	}

	static FORCE_INLINE bool ExtractBoolFromBitfield(uint8* ptr, uint32 index)
	{
		uint8* bytePtr = ptr + index / 8;
		uint8 mast = 1 << (index & 0x7);

		return (*bytePtr & mast) != 0;
	}

	static FORCE_INLINE void SetBoolInBitField(uint8* ptr, uint32 index, bool bset)
	{
		uint8* bytePtr = ptr + index / 8;
		uint8 mast = 1 << (index & 0x7);

		if (bset) {
			*bytePtr |= mast;
		}
		else {
			*bytePtr &= ~mast;
		}
	}

	static FORCE_INLINE uint8 Quantize8UnsignedByte(float x)
	{
		int32 Ret = (int32)(x * 255.999f);
		return Ret;
	}

	static FORCE_INLINE uint8 Quantize8SignedByte(float x)
	{
		float y = x * 0.5f + 0.5f;
		return Quantize8UnsignedByte(y);
	}

	static FORCE_INLINE int32 GreatestCommonDivisor(int32 a, int32 b)
	{
		while (b != 0) {
			int32 t = b;
			b = a % b;
			a = t;
		}
		return a;
	}

	static FORCE_INLINE int32 LeastCommonMultiplier(int32 a, int32 b)
	{
		int32 currentGcd = GreatestCommonDivisor(a, b);
		return currentGcd == 0 ? 0 : (a / currentGcd) * b;
	}
    	

	static float InterpConstantTo(float current, float target, float deltaTime, float interpSpeed);

	static float InterpTo(float current, float target, float deltaTime, float interpSpeed);

	static float TruncateToHalfIfClose(float f);

	static double TruncateToHalfIfClose(double f);

	static void WindRelativeAnglesDegrees(float inAngle0, float& inOutAngle1);

	static float RoundHalfToEven(float f);

	static double RoundHalfToEven(double f);

	static float RoundHalfFromZero(float f);

	static double RoundHalfFromZero(double f);

	static float RoundHalfToZero(float f);

	static double RoundHalfToZero(double f);

	static float PerlinNoise1D(const float value);
    };

}
