#include "float4.h"

struct float4x4
{
	float4 line1;
	float4 line2;
	float4 line3;
	float4 line4;
};


Inline void SetIdentity(float4x4& matrix)
{
	matrix.line1 = Set(1,0,0,0);
	matrix.line2 = Set(0,1,0,0);
	matrix.line3 = Set(0,0,1,0);
	matrix.line4 = Set(0,0,0,1);
}

Inline void SetPosition(float4x4& matrix, const float4 pos)
{
	matrix.line4 = pos;
}

Inline void SetScale(float4x4& matrix, const float4 scale)
{
	matrix.line1 *= splatX(scale);
	matrix.line2 *= splatY(scale);
	matrix.line3 *= splatZ(scale);
}

Inline void Transpose(float4x4& result)
{
	// x.x,x.y,y.x,y.y
	__m128 vTemp1 = _mm_shuffle_ps(result.line1,result.line2,_MM_SHUFFLE(1,0,1,0));
	// x.z,x.w,y.z,y.w
	__m128 vTemp3 = _mm_shuffle_ps(result.line1,result.line2,_MM_SHUFFLE(3,2,3,2));
	// z.x,z.y,w.x,w.y
	__m128 vTemp2 = _mm_shuffle_ps(result.line3,result.line4,_MM_SHUFFLE(1,0,1,0));
	// z.z,z.w,w.z,w.w
	__m128 vTemp4 = _mm_shuffle_ps(result.line3,result.line4,_MM_SHUFFLE(3,2,3,2));

	// x.x,y.x,z.x,w.x
	result.line1 = _mm_shuffle_ps(vTemp1, vTemp2,_MM_SHUFFLE(2,0,2,0));
	// x.y,y.y,z.y,w.y
	result.line2 = _mm_shuffle_ps(vTemp1, vTemp2,_MM_SHUFFLE(3,1,3,1));
	// x.z,y.z,z.z,w.z
	result.line3 = _mm_shuffle_ps(vTemp3, vTemp4,_MM_SHUFFLE(2,0,2,0));
	// x.w,y.w,z.w,w.w
	result.line4 = _mm_shuffle_ps(vTemp3, vTemp4,_MM_SHUFFLE(3,1,3,1));
}

Inline void Inverse(float4x4& result)
{
	Transpose(result);
	__m128 V00 = _mm_shuffle_ps(result.line3, result.line3,_MM_SHUFFLE(1,1,0,0));
	__m128 V10 = _mm_shuffle_ps(result.line4, result.line4,_MM_SHUFFLE(3,2,3,2));
	__m128 V01 = _mm_shuffle_ps(result.line1, result.line1,_MM_SHUFFLE(1,1,0,0));
	__m128 V11 = _mm_shuffle_ps(result.line2, result.line2,_MM_SHUFFLE(3,2,3,2));
	__m128 V02 = _mm_shuffle_ps(result.line3, result.line1,_MM_SHUFFLE(2,0,2,0));
	__m128 V12 = _mm_shuffle_ps(result.line4, result.line2,_MM_SHUFFLE(3,1,3,1));


	__m128 D0 = _mm_mul_ps(V00,V10);
	__m128 D1 = _mm_mul_ps(V01,V11);
	__m128 D2 = _mm_mul_ps(V02,V12);

	V00 = _mm_shuffle_ps(result.line3,result.line3,_MM_SHUFFLE(3,2,3,2));
	V10 = _mm_shuffle_ps(result.line4,result.line4,_MM_SHUFFLE(1,1,0,0));
	V01 = _mm_shuffle_ps(result.line1,result.line1,_MM_SHUFFLE(3,2,3,2));
	V11 = _mm_shuffle_ps(result.line2,result.line2,_MM_SHUFFLE(1,1,0,0));
	V02 = _mm_shuffle_ps(result.line3,result.line1,_MM_SHUFFLE(3,1,3,1));
	V12 = _mm_shuffle_ps(result.line4,result.line2,_MM_SHUFFLE(2,0,2,0));

	V00 = _mm_mul_ps(V00,V10);
	V01 = _mm_mul_ps(V01,V11);
	V02 = _mm_mul_ps(V02,V12);
	D0 = _mm_sub_ps(D0,V00);
	D1 = _mm_sub_ps(D1,V01);
	D2 = _mm_sub_ps(D2,V02);
	// V11 = D0Y,D0W,D2Y,D2Y
	V11 = _mm_shuffle_ps(D0,D2,_MM_SHUFFLE(1,1,3,1));
	V00 = _mm_shuffle_ps(result.line2, result.line2,_MM_SHUFFLE(1,0,2,1));
	V10 = _mm_shuffle_ps(V11,D0,_MM_SHUFFLE(0,3,0,2));
	V01 = _mm_shuffle_ps(result.line1, result.line1,_MM_SHUFFLE(0,1,0,2));
	V11 = _mm_shuffle_ps(V11,D0,_MM_SHUFFLE(2,1,2,1));
	// V13 = D1Y,D1W,D2W,D2W
	__m128 V13 = _mm_shuffle_ps(D1,D2,_MM_SHUFFLE(3,3,3,1));
	V02 = _mm_shuffle_ps(result.line4, result.line4,_MM_SHUFFLE(1,0,2,1));
	V12 = _mm_shuffle_ps(V13,D1,_MM_SHUFFLE(0,3,0,2));
	__m128 V03 = _mm_shuffle_ps(result.line3, result.line3,_MM_SHUFFLE(0,1,0,2));
	V13 = _mm_shuffle_ps(V13,D1,_MM_SHUFFLE(2,1,2,1));

	__m128 C0 = _mm_mul_ps(V00,V10);
	__m128 C2 = _mm_mul_ps(V01,V11);
	__m128 C4 = _mm_mul_ps(V02,V12);
	__m128 C6 = _mm_mul_ps(V03,V13);

	// V11 = D0X,D0Y,D2X,D2X
	V11 = _mm_shuffle_ps(D0,D2,_MM_SHUFFLE(0,0,1,0));
	V00 = _mm_shuffle_ps(result.line2, result.line2,_MM_SHUFFLE(2,1,3,2));
	V10 = _mm_shuffle_ps(D0,V11,_MM_SHUFFLE(2,1,0,3));
	V01 = _mm_shuffle_ps(result.line1, result.line1,_MM_SHUFFLE(1,3,2,3));
	V11 = _mm_shuffle_ps(D0,V11,_MM_SHUFFLE(0,2,1,2));
	// V13 = D1X,D1Y,D2Z,D2Z
	V13 = _mm_shuffle_ps(D1,D2,_MM_SHUFFLE(2,2,1,0));
	V02 = _mm_shuffle_ps(result.line4, result.line4,_MM_SHUFFLE(2,1,3,2));
	V12 = _mm_shuffle_ps(D1,V13,_MM_SHUFFLE(2,1,0,3));
	V03 = _mm_shuffle_ps(result.line3, result.line3,_MM_SHUFFLE(1,3,2,3));
	V13 = _mm_shuffle_ps(D1,V13,_MM_SHUFFLE(0,2,1,2));

	V00 = _mm_mul_ps(V00,V10);
	V01 = _mm_mul_ps(V01,V11);
	V02 = _mm_mul_ps(V02,V12);
	V03 = _mm_mul_ps(V03,V13);
	C0 = _mm_sub_ps(C0,V00);
	C2 = _mm_sub_ps(C2,V01);
	C4 = _mm_sub_ps(C4,V02);
	C6 = _mm_sub_ps(C6,V03);

	V00 = _mm_shuffle_ps(result.line2,result.line2,_MM_SHUFFLE(0,3,0,3));
	// V10 = D0Z,D0Z,D2X,D2Y
	V10 = _mm_shuffle_ps(D0,D2,_MM_SHUFFLE(1,0,2,2));
	V10 = _mm_shuffle_ps(V10,V10,_MM_SHUFFLE(0,2,3,0));
	V01 = _mm_shuffle_ps(result.line1,result.line1,_MM_SHUFFLE(2,0,3,1));
	// V11 = D0X,D0W,D2X,D2Y
	V11 = _mm_shuffle_ps(D0,D2,_MM_SHUFFLE(1,0,3,0));
	V11 = _mm_shuffle_ps(V11,V11,_MM_SHUFFLE(2,1,0,3));
	V02 = _mm_shuffle_ps(result.line4,result.line4,_MM_SHUFFLE(0,3,0,3));
	// V12 = D1Z,D1Z,D2Z,D2W
	V12 = _mm_shuffle_ps(D1,D2,_MM_SHUFFLE(3,2,2,2));
	V12 = _mm_shuffle_ps(V12,V12,_MM_SHUFFLE(0,2,3,0));
	V03 = _mm_shuffle_ps(result.line3,result.line3,_MM_SHUFFLE(2,0,3,1));
	// V13 = D1X,D1W,D2Z,D2W
	V13 = _mm_shuffle_ps(D1,D2,_MM_SHUFFLE(3,2,3,0));
	V13 = _mm_shuffle_ps(V13,V13,_MM_SHUFFLE(2,1,0,3));

	V00 = _mm_mul_ps(V00,V10);
	V01 = _mm_mul_ps(V01,V11);
	V02 = _mm_mul_ps(V02,V12);
	V03 = _mm_mul_ps(V03,V13);
	__m128 C1 = _mm_sub_ps(C0,V00);
	C0 = _mm_add_ps(C0,V00);
	__m128 C3 = _mm_add_ps(C2,V01);
	C2 = _mm_sub_ps(C2,V01);
	__m128 C5 = _mm_sub_ps(C4,V02);
	C4 = _mm_add_ps(C4,V02);
	__m128 C7 = _mm_add_ps(C6,V03);
	C6 = _mm_sub_ps(C6,V03);

	C0 = _mm_shuffle_ps(C0,C1,_MM_SHUFFLE(3,1,2,0));
	C2 = _mm_shuffle_ps(C2,C3,_MM_SHUFFLE(3,1,2,0));
	C4 = _mm_shuffle_ps(C4,C5,_MM_SHUFFLE(3,1,2,0));
	C6 = _mm_shuffle_ps(C6,C7,_MM_SHUFFLE(3,1,2,0));
	C0 = _mm_shuffle_ps(C0,C0,_MM_SHUFFLE(3,1,2,0));
	C2 = _mm_shuffle_ps(C2,C2,_MM_SHUFFLE(3,1,2,0));
	C4 = _mm_shuffle_ps(C4,C4,_MM_SHUFFLE(3,1,2,0));
	C6 = _mm_shuffle_ps(C6,C6,_MM_SHUFFLE(3,1,2,0));

	// Get the determinate
	__m128 vTemp = dot4(C0,result.line1);
	//*pDeterminant = vTemp;
	vTemp = _mm_div_ps(float4_one,vTemp);
	result.line1 = _mm_mul_ps(C0,vTemp);
	result.line2 = _mm_mul_ps(C2,vTemp);
	result.line3 = _mm_mul_ps(C4,vTemp);
	result.line4 = _mm_mul_ps(C6,vTemp);
}

Inline float4x4 operator * (const float4x4& M1, const float4x4& M2)
{
    float4x4 mResult;

    // Use vW to hold the original row
    float4 vW = M1.line1;
    // Splat the component X,Y,Z then W
    float4 vX = _mm_shuffle_ps(vW,vW,_MM_SHUFFLE(0,0,0,0));
    float4 vY = _mm_shuffle_ps(vW,vW,_MM_SHUFFLE(1,1,1,1));
    float4 vZ = _mm_shuffle_ps(vW,vW,_MM_SHUFFLE(2,2,2,2));
    vW =		_mm_shuffle_ps(vW,vW,_MM_SHUFFLE(3,3,3,3));
    // Perform the opertion on the first row
    vX = _mm_mul_ps(vX,M2.line1);
    vY = _mm_mul_ps(vY,M2.line2);
    vZ = _mm_mul_ps(vZ,M2.line3);
    vW = _mm_mul_ps(vW,M2.line4);
    // Perform a binary add to reduce cumulative errors
    vX = _mm_add_ps(vX,vZ);
    vY = _mm_add_ps(vY,vW);
    vX = _mm_add_ps(vX,vY);
    mResult.line1 = vX;

    // Repeat for the other 3 rows
    vW = M1.line2;
    vX = _mm_shuffle_ps(vW,vW,_MM_SHUFFLE(0,0,0,0));
    vY = _mm_shuffle_ps(vW,vW,_MM_SHUFFLE(1,1,1,1));
    vZ = _mm_shuffle_ps(vW,vW,_MM_SHUFFLE(2,2,2,2));
    vW = _mm_shuffle_ps(vW,vW,_MM_SHUFFLE(3,3,3,3));
    vX = _mm_mul_ps(vX,M2.line1);
    vY = _mm_mul_ps(vY,M2.line2);
    vZ = _mm_mul_ps(vZ,M2.line3);
    vW = _mm_mul_ps(vW,M2.line4);
    vX = _mm_add_ps(vX,vZ);
    vY = _mm_add_ps(vY,vW);
    vX = _mm_add_ps(vX,vY);
    mResult.line2 = vX;

    vW = M1.line3;
    vX = _mm_shuffle_ps(vW,vW,_MM_SHUFFLE(0,0,0,0));
    vY = _mm_shuffle_ps(vW,vW,_MM_SHUFFLE(1,1,1,1));
    vZ = _mm_shuffle_ps(vW,vW,_MM_SHUFFLE(2,2,2,2));
    vW = _mm_shuffle_ps(vW,vW,_MM_SHUFFLE(3,3,3,3));
    vX = _mm_mul_ps(vX,M2.line1);
    vY = _mm_mul_ps(vY,M2.line2);
    vZ = _mm_mul_ps(vZ,M2.line3);
    vW = _mm_mul_ps(vW,M2.line4);
    vX = _mm_add_ps(vX,vZ);
    vY = _mm_add_ps(vY,vW);
    vX = _mm_add_ps(vX,vY);
    mResult.line3 = vX;

    vW = M1.line4;
    vX = _mm_shuffle_ps(vW,vW,_MM_SHUFFLE(0,0,0,0));
    vY = _mm_shuffle_ps(vW,vW,_MM_SHUFFLE(1,1,1,1));
    vZ = _mm_shuffle_ps(vW,vW,_MM_SHUFFLE(2,2,2,2));
    vW = _mm_shuffle_ps(vW,vW,_MM_SHUFFLE(3,3,3,3));
    vX = _mm_mul_ps(vX,M2.line1);
    vY = _mm_mul_ps(vY,M2.line2);
    vZ = _mm_mul_ps(vZ,M2.line3);
    vW = _mm_mul_ps(vW,M2.line4);
    vX = _mm_add_ps(vX,vZ);
    vY = _mm_add_ps(vY,vW);
    vX = _mm_add_ps(vX,vY);
    mResult.line4 = vX;

    return mResult;
}

Inline float4x4 PerspProjection(float aspect, float fov, float fnear, float ffar)
{
	float4x4 m;
	m.line1 = Set(1.0f/(aspect*tan(fov/2.0f)),0,0,0);
	m.line2 = Set(0,1.0f/tan(fov/2.0f),0,0);
	m.line3 = Set(0,0,ffar/(ffar-fnear),1.0f);
	m.line4 = Set(0,0,(ffar*fnear)/(fnear-ffar),0);
	return m;
}

Inline float4x4 Rotation(float4& quat)
{
	// this one is a bit slower than D3D's but nicer
	float4x4 m1, m2;
	m1.line1 = m2.line1 = _mm_shuffle_ps(quat,quat,_MM_SHUFFLE(0,1,2,3)); //wzyx // WORKS ON Y ROTATION! Only this one line
	m1.line2 = m2.line2 = _mm_shuffle_ps(quat,quat,_MM_SHUFFLE(1,0,3,2)); //zwxy
	m1.line3 = m2.line3 = _mm_shuffle_ps(quat,quat,_MM_SHUFFLE(2,3,0,1)); //yxwz
	m1.line4 = m2.line4 = quat; //xyzw

	m1.line1 *= float4_PPNP;
	m1.line2 *= float4_NPPP;
	m1.line3 *= float4_PNPP;
	m1.line4 *= float4_NNNP;

	m2.line1 *= float4_PPNN;
	m2.line2 *= float4_NPPN;
	m2.line3 *= float4_PNPN;

    return m1 * m2;

	/*float4x4 M;
    float4               Q0, Q1;
    float4               V0, V1, V2;
    float4               R0, R1, R2;

    Q0 = _mm_add_ps(Quaternion,Quaternion);
    Q1 = _mm_mul_ps(Quaternion,Q0);

    V0 = _mm_shuffle_ps(Q1,Q1,_MM_SHUFFLE(3,0,0,1));
    V0 = _mm_and_ps(V0,g_XMMask3);
//    V0 = XMVectorPermute(Q1, Constant1110,Permute0Y0X0X1W);
    V1 = _mm_shuffle_ps(Q1,Q1,_MM_SHUFFLE(3,1,2,2));
    V1 = _mm_and_ps(V1,g_XMMask3);
//    V1 = XMVectorPermute(Q1, Constant1110,Permute0Z0Z0Y1W);
    R0 = _mm_sub_ps(Constant1110,V0);
    R0 = _mm_sub_ps(R0, V1);

    V0 = _mm_shuffle_ps(Quaternion,Quaternion,_MM_SHUFFLE(3,1,0,0));
//    V0 = XMVectorPermute(Quaternion, Quaternion,SwizzleXXYW);
    V1 = _mm_shuffle_ps(Q0,Q0,_MM_SHUFFLE(3,2,1,2));
//    V1 = XMVectorPermute(Q0, Q0,SwizzleZYZW);
    V0 = _mm_mul_ps(V0, V1);

    V1 = _mm_shuffle_ps(Quaternion,Quaternion,_MM_SHUFFLE(3,3,3,3));
//    V1 = XMVectorSplatW(Quaternion);
    V2 = _mm_shuffle_ps(Q0,Q0,_MM_SHUFFLE(3,0,2,1));
//    V2 = XMVectorPermute(Q0, Q0,SwizzleYZXW);
    V1 = _mm_mul_ps(V1, V2);

    R1 = _mm_add_ps(V0, V1);
    R2 = _mm_sub_ps(V0, V1);

    V0 = _mm_shuffle_ps(R1,R2,_MM_SHUFFLE(1,0,2,1));
    V0 = _mm_shuffle_ps(V0,V0,_MM_SHUFFLE(1,3,2,0));
//    V0 = XMVectorPermute(R1, R2,Permute0Y1X1Y0Z);
    V1 = _mm_shuffle_ps(R1,R2,_MM_SHUFFLE(2,2,0,0));
    V1 = _mm_shuffle_ps(V1,V1,_MM_SHUFFLE(2,0,2,0));
//    V1 = XMVectorPermute(R1, R2,Permute0X1Z0X1Z);

    Q1 = _mm_shuffle_ps(R0,V0,_MM_SHUFFLE(1,0,3,0));
    Q1 = _mm_shuffle_ps(Q1,Q1,_MM_SHUFFLE(1,3,2,0));

    M.line1 = Q1;
//    M.r[0] = XMVectorPermute(R0, V0,Permute0X1X1Y0W);
    Q1 = _mm_shuffle_ps(R0,V0,_MM_SHUFFLE(3,2,3,1));
    Q1 = _mm_shuffle_ps(Q1,Q1,_MM_SHUFFLE(1,3,0,2));
    M.line2 = Q1;
//    M.r[1] = XMVectorPermute(R0, V0,Permute1Z0Y1W0W);
    Q1 = _mm_shuffle_ps(V1,R0,_MM_SHUFFLE(3,2,1,0));
    M.line3 = Q1;
//    M.r[2] = XMVectorPermute(R0, V1,Permute1X1Y0Z0W);
    M.line4 = Constant0001;
    return M;*/
}

void DumpMatrix(float4x4& m)
{
	cout << getX(m.line1)<<"	"<<getY(m.line1)<<"	"<<getZ(m.line1)<<"	"<<getW(m.line1)<<"\n";
	cout << getX(m.line2)<<"	"<<getY(m.line2)<<"	"<<getZ(m.line2)<<"	"<<getW(m.line2)<<"\n";
	cout << getX(m.line3)<<"	"<<getY(m.line3)<<"	"<<getZ(m.line3)<<"	"<<getW(m.line3)<<"\n";
	cout << getX(m.line4)<<"	"<<getY(m.line4)<<"	"<<getZ(m.line4)<<"	"<<getW(m.line4)<<"\n";
	cout << "\n";
}
