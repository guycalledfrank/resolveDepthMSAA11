#include "engine.h"

struct cbPerObject
{
	float4x4 MatWorld;
	float4x4 MatWorldViewProj;
};

struct cbPerCamera
{
	float4x4 MatViewProj;
};

struct constantBuffers
{
	constantBuffer perObject;
	//constantBuffer perMaterial;
	constantBuffer perCamera;

	constantBuffers()
	{
		perObject.Create(sizeof(cbPerObject));
		perCamera.Create(sizeof(cbPerCamera));
	}
};

struct entity
{
	mesh* m;
	cbPerObject cb;
};


