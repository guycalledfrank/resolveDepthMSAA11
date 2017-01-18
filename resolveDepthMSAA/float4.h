#pragma once
#include "xmmintrin.h"
#include "smmintrin.h"

typedef __m128 float4;

Inline float4 Set(float x, float y, float z, float w=0)
{
	return _mm_set_ps(w,z,y,x);
}

Inline float4& operator += (float4& v1, const float4 v2)
{
	v1 = _mm_add_ps(v1, v2);
	return v1;
}

Inline float4& operator -= (float4& v1, const float4 v2)
{
	v1 = _mm_sub_ps(v1, v2);
	return v1;
}

Inline float4& operator *= (float4& v1, const float4 v2)
{
	v1 = _mm_mul_ps(v1, v2);
	return v1;
}

Inline float4& operator /= (float4& v1, const float4 v2)
{
	v1 = _mm_div_ps(v1, v2);
	return v1;
}

Inline float4 operator + (const float4 v1, const float4 v2)
{
	return _mm_add_ps(v1,v2);
}

Inline float4 operator - (const float4 v1, const float4 v2)
{
	return _mm_sub_ps(v1,v2);
}

Inline float4 operator * (const float4 v1, const float4 v2)
{
	return _mm_mul_ps(v1,v2);
}

Inline float4 operator * (const float4 v1, float v2)
{
	return _mm_mul_ps(v1,Set(v2,v2,v2,v2));
}

Inline float4 operator / (const float4 v1, const float4 v2)
{
	return _mm_div_ps(v1,v2);
}

Inline bool operator > (const float4 v1, const float4 v2)
{
	return _mm_movemask_ps( _mm_cmpgt_ps(v1,v2) )!=0;
}

Inline bool operator < (const float4 v1, const float4 v2)
{
	return _mm_movemask_ps( _mm_cmplt_ps(v1,v2) )!=0;
}

Inline float4 dotToX(const float4 v1, const float4 v2) // writes dot product to X component
{
	/*float4 vDot = v1 * v2;
	float4 vTemp = _mm_shuffle_ps(vDot,vDot,_MM_SHUFFLE(2,1,2,1));
	vDot = _mm_add_ss(vDot,vTemp);
	vTemp = _mm_shuffle_ps(vTemp,vTemp,_MM_SHUFFLE(1,1,1,1));
	vDot = _mm_add_ss(vDot,vTemp);
	return vDot;*/

	return _mm_dp_ps(v1,v2, 0x71);
}


Inline float4 dot(const float4 v1, const float4 v2) // writes dot product to all components
{
	//float4 vDot = dotToX(v1,v2);
	//return _mm_shuffle_ps(vDot,vDot,_MM_SHUFFLE(0,0,0,0));

	//return _mm_dp_ps(v1,v2, 0x77);	// splat to XYZ
	return _mm_dp_ps(v1,v2, 0x7F);		// splat to XYZW
}

Inline float4 dot4(const float4 v1, const float4 v2) // writes dot product to all components
{
	return _mm_dp_ps(v1,v2, 0xFF);		// splat to XYZW
}

Inline float4 squaredLength(const float4 v)
{
	return dot(v,v);
}

Inline float4 length(const float4 v)
{
	return _mm_sqrt_ps( dot(v,v) );
}

Inline float4 normalize(const float4 v)
{
	//float4 rsq = _mm_rsqrt_ss( dotToX(v,v) ); // x is rsqrt
	//return _mm_shuffle_ps(rsq,rsq,_MM_SHUFFLE(0,0,0,0)) * v;

	float4 rsq = _mm_rsqrt_ps( _mm_dp_ps(v,v, 0x77) );
	return rsq * v;
}

Inline float4 distance3(const float4 v1, const float4 v2)
{
	float4 delta = v2 - v1;
	return length(delta);
}

Inline float getX(const float4 v)
{
	return ((float*)&v)[0];
}

Inline float getY(const float4 v)
{
	return ((float*)&v)[1];
}

Inline float getZ(const float4 v)
{
	return ((float*)&v)[2];
}

Inline float getW(const float4 v)
{
	return ((float*)&v)[3];
}

Inline float4 max3(const float4 v1, const float4 v2)
{
	return _mm_max_ps(v1, v2);
}

Inline float4 min3(const float4 v1, const float4 v2)
{
	return _mm_min_ps(v1, v2);
}

Inline float4 splatX(const float4 v1)
{
	return _mm_shuffle_ps(v1,v1,_MM_SHUFFLE(0,0,0,0));
}

Inline float4 splatY(const float4 v1)
{
	return _mm_shuffle_ps(v1,v1,_MM_SHUFFLE(1,1,1,1));
}

Inline float4 splatZ(const float4 v1)
{
	return _mm_shuffle_ps(v1,v1,_MM_SHUFFLE(2,2,2,2));
}

Inline float4 splatW(const float4 v1)
{
	return _mm_shuffle_ps(v1,v1,_MM_SHUFFLE(3,3,3,3));
}

Inline float4 abs(const float4 V)
{
	float4 vResult = _mm_setzero_ps();
	vResult = _mm_sub_ps(vResult,V);
	vResult = _mm_max_ps(vResult,V);
    return vResult;
}

Inline float4 quatIdentity()
{
	return Set(0,0,0,1);
}



const int quatZero=0;
const int quatFlipSign=0x80000000;
const float4 quat_mask = _mm_setr_ps(*(float*)&quatZero, *(float*)&quatZero, *(float*)&quatZero, *(float*)&quatFlipSign);

Inline float4 mulQuat(const float4& q1, const float4& q2)
{
	float4 swiz1=_mm_shuffle_ps(q2,q2,_MM_SHUFFLE(3,3,3,3));
	float4 swiz2=_mm_shuffle_ps(q1,q1,_MM_SHUFFLE(2,0,1,0));
	float4 swiz3=_mm_shuffle_ps(q2,q2,_MM_SHUFFLE(1,2,0,0));
	float4 swiz4=_mm_shuffle_ps(q1,q1,_MM_SHUFFLE(3,3,3,1));
	float4 swiz5=_mm_shuffle_ps(q2,q2,_MM_SHUFFLE(0,1,2,1));
	float4 swiz6=_mm_shuffle_ps(q1,q1,_MM_SHUFFLE(1,2,0,2));
	float4 swiz7=_mm_shuffle_ps(q2,q2,_MM_SHUFFLE(2,0,1,2));
	float4 mul4=_mm_mul_ps(swiz6,swiz7);
	float4 mul3=_mm_mul_ps(swiz4,swiz5);
	float4 mul2=_mm_mul_ps(swiz2,swiz3);
	float4 mul1=_mm_mul_ps(q1,swiz1);
	float4 flip1=_mm_xor_ps(mul4,quat_mask);
	float4 flip2=_mm_xor_ps(mul3,quat_mask);
	float4 retVal=_mm_sub_ps(mul1,mul2);
	float4 retVal2=_mm_add_ps(flip1,flip2);
	return _mm_add_ps(retVal,retVal2);

    /*static CONST float4 ControlWZYX = Set( 1.0f,-1.0f, 1.0f,-1.0f);
    static CONST float4 ControlZWXY = Set( 1.0f, 1.0f,-1.0f,-1.0f);
    static CONST float4 ControlYXWZ = Set(-1.0f, 1.0f, 1.0f,-1.0f);
    // Copy to SSE registers and use as few as possible for x86
    float4 Q2X = Q2;
    float4 Q2Y = Q2;
    float4 Q2Z = Q2;
    float4 vResult = Q2;
    // Splat with one instruction
    vResult = _mm_shuffle_ps(vResult,vResult,_MM_SHUFFLE(3,3,3,3));
    Q2X = _mm_shuffle_ps(Q2X,Q2X,_MM_SHUFFLE(0,0,0,0));
    Q2Y = _mm_shuffle_ps(Q2Y,Q2Y,_MM_SHUFFLE(1,1,1,1));
    Q2Z = _mm_shuffle_ps(Q2Z,Q2Z,_MM_SHUFFLE(2,2,2,2));
    // Retire Q1 and perform Q1*Q2W
    vResult = _mm_mul_ps(vResult,Q1);
    float4 Q1Shuffle = Q1;
    // Shuffle the copies of Q1
    Q1Shuffle = _mm_shuffle_ps(Q1Shuffle,Q1Shuffle,_MM_SHUFFLE(0,1,2,3));
    // Mul by Q1WZYX
    Q2X = _mm_mul_ps(Q2X,Q1Shuffle);
    Q1Shuffle = _mm_shuffle_ps(Q1Shuffle,Q1Shuffle,_MM_SHUFFLE(2,3,0,1));
    // Flip the signs on y and z
    Q2X = _mm_mul_ps(Q2X,ControlWZYX);
    // Mul by Q1ZWXY
    Q2Y = _mm_mul_ps(Q2Y,Q1Shuffle);
    Q1Shuffle = _mm_shuffle_ps(Q1Shuffle,Q1Shuffle,_MM_SHUFFLE(0,1,2,3));
    // Flip the signs on z and w
    Q2Y = _mm_mul_ps(Q2Y,ControlZWXY);
    // Mul by Q1YXWZ
    Q2Z = _mm_mul_ps(Q2Z,Q1Shuffle);
    vResult = _mm_add_ps(vResult,Q2X);
    // Flip the signs on x and w
    Q2Z = _mm_mul_ps(Q2Z,ControlYXWZ);
    Q2Y = _mm_add_ps(Q2Y,Q2Z);
    vResult = _mm_add_ps(vResult,Q2Y);
    return vResult;*/
}

Inline float4 quatFromAxisAngle(float _x,float _y,float _z,float _w)
{
	float a = sinf(_w/2.0f);
	return _mm_set_ps(cosf(_w/2.0f),_z * a,_y * a,_x * a);
}

Inline float4 quatFromAxisAngle(float4 vec,float _w)
{
	return quatFromAxisAngle(getX(vec), getY(vec), getZ(vec), _w);
}

static const float4 float4_one = Set(1,1,1,1);

static const float4 float4_PPNP = Set(1,1,-1,1);
static const float4 float4_NPPP = Set(-1,1,1,1);
static const float4 float4_PNPP = Set(1,-1,1,1);
static const float4 float4_NNNP = Set(-1,-1,-1,1);
static const float4 float4_PPNN = Set(1,1,-1,-1);
static const float4 float4_NPPN = Set(-1,1,1,-1);
static const float4 float4_PNPN = Set(1,-1,1,-1);
static const float4 float4_PNNP = Set(1,-1,-1,1);
static const float4 float4_NPNP = Set(-1,1,-1,1);
static const float4 float4_NNPP = Set(-1,-1,1,1);

static const float4 g_XMMask3             = Set(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000);
static const float4  Constant1110 = Set(1.0f, 1.0f, 1.0f, 0.0f);
static const float4  Constant0001 = Set(0.0f, 0.0f, 0.0f, 1.0f);



Inline float4 cross(float4& V1,float4& V2)
{
    // y1,z1,x1,w1
    float4 vTemp1 = _mm_shuffle_ps(V1,V1,_MM_SHUFFLE(3,0,2,1));
    // z2,x2,y2,w2
    float4 vTemp2 = _mm_shuffle_ps(V2,V2,_MM_SHUFFLE(3,1,0,2));
    // Perform the left operation
    float4 vResult = _mm_mul_ps(vTemp1,vTemp2);
    // z1,x1,y1,w1
    vTemp1 = _mm_shuffle_ps(vTemp1,vTemp1,_MM_SHUFFLE(3,0,2,1));
    // y2,z2,x2,w2
    vTemp2 = _mm_shuffle_ps(vTemp2,vTemp2,_MM_SHUFFLE(3,1,0,2));
    // Perform the right operation
    vTemp1 = _mm_mul_ps(vTemp1,vTemp2);
    // Subract the right from left, and return answer
    vResult = _mm_sub_ps(vResult,vTemp1);
    // Set w to zero
    return vResult;//_mm_and_ps(vResult,g_XMMask3);
}
