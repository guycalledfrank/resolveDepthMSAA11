#include <iostream>
#include <vector>
#include <map>
using namespace std;

#define Inline __forceinline
#include "float4.h"
#include "float4x4.h"

#include "windows.h"
#undef _range
#undef _xcount
#define _range(x, y)
#define _xcount(x)
#include "d3d11.h"
#include "d3dcompiler.h"
#include "error.h"

namespace engine
{
	HINSTANCE hInstance;
	HWND window;
	const int maxPathSize = 512;
	wchar_t exeDir[maxPathSize];

	IDXGISwapChain* swapChain;
	ID3D11Device* device;
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11DeviceContext* deviceContext;

	ID3D11Resource* backBuffer;
	ID3D11RenderTargetView* backBufferView;

	int screenWidth, screenHeight, backBufferAAlevel, backBufferAAquality;
	float screenAspect;
	DXGI_FORMAT backBufferFormat;
	int vsync;

	map<int,int> inputLayoutHashToIndexPlusOne;
	vector<ID3D11InputLayout*> inputLayouts;

	BYTE KeyDown[256];
	BYTE KeyDownPrev[256];
};
ierror error;

#define CBUFFER_PER_OBJECT		0
#define CBUFFER_PER_MATERIAL	1
#define CBUFFER_PER_CAMERA		2

#define Pi 3.1415926535897932384626433832795f

#define VK_W 0x57
#define VK_A 0x41
#define VK_S 0x53
#define VK_D 0x44

void GetExeDir()
{
	int len = GetModuleFileName(NULL,engine::exeDir, engine::maxPathSize);

	for(int dirsize = len-1; dirsize>1; dirsize--)
	{
		if (engine::exeDir[dirsize] == L'\\')
		{
			engine::exeDir[dirsize] = 0;
			break;
		}
	}
}

FILETIME GetFileDate(const wchar_t* fname)
{
	FILETIME modified;
	modified.dwHighDateTime = 0;
	modified.dwLowDateTime = 0;

	HANDLE f = CreateFile(fname,0,0,NULL,OPEN_EXISTING,0,NULL);
	if (f==INVALID_HANDLE_VALUE)
	{
		error << "Can't open file "<<fname<<".\n";
		return modified;
	}

	if (GetFileTime(f,NULL,NULL,&modified)==0)
	{
		error << "GetFileTime error.\n";
		return modified;
	}

	CloseHandle(f);
	return modified;
}

int CompareFileDates(const wchar_t* fname1, const wchar_t* fname2)
{
	FILETIME f1 = GetFileDate(fname1);
	FILETIME f2 = GetFileDate(fname2);
	return CompareFileTime(&f1,&f2);
}

void RunAndWait(const wchar_t* exe, const wchar_t* params)
{
	PROCESS_INFORMATION proc;
	STARTUPINFO startupInfo = {0};
	startupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	cout << "Running "<<exe<<" with params "<<params<<"...\n";
	if (!CreateProcess(exe,(wchar_t*)params,NULL,NULL,TRUE,NORMAL_PRIORITY_CLASS,
																		NULL,engine::exeDir,
																				&startupInfo,&proc))
	{
		error << "Error creating process "<<exe<<"\n";
	}
	WaitForSingleObject(proc.hProcess, INFINITE);
	CloseHandle(proc.hProcess);
}


int GetFileSize(FILE* f)
{
	fseek(f,0,SEEK_END);
	int fileSize = ftell(f);
	fseek(f,0,SEEK_SET);
	return fileSize;
}

int RSHash(const char* cr, int size)
{
	//int b    = 378551; //Ramdom Ranges I've chosen (can be modified)
	int b    = 378551;
	int a    = 63689;
	int hash = 0;  //Output hash
	int c = cr[0];  //Temp number that scrolls through the string array
	int i = 0;

	for(int i=1;i<size;i++)
	{
		hash = hash * a + c;  //Algorithm that hashs
		a    = a * b;
		c = cr[i];
	}
	return (hash & 0x7FFFFFFF); //Returns the hashed string
}


const char* GetDXError(HRESULT h)
{
	if (h==D3D11_ERROR_FILE_NOT_FOUND){
		return "D3D11_ERROR_FILE_NOT_FOUND";
	}
	else if (h==D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS){
		return "D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS";
	}
	else if (h==D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD){
		return "D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD";
	}
	else if (h==E_FAIL){
		return "E_FAIL";
	}
	else if (h==E_INVALIDARG){
		return "E_INVALIDARG";
	}
	else if (h==E_OUTOFMEMORY){
		return "E_OUTOFMEMORY";
	}
	else if (h==S_FALSE){
		return "S_FALSE";
	}
	else if (h==S_OK){
		return "S_OK";
	}
	else if (h==DXGI_ERROR_DEVICE_HUNG){
		return "DXGI_ERROR_DEVICE_HUNG";
	}
	else if (h==DXGI_ERROR_DEVICE_REMOVED){
		return "DXGI_ERROR_DEVICE_REMOVED";
	}
		else if (h==DXGI_ERROR_DEVICE_RESET){
		return "DXGI_ERROR_DEVICE_RESET";
	}
	else if (h==DXGI_ERROR_DRIVER_INTERNAL_ERROR){
		return "DXGI_ERROR_DRIVER_INTERNAL_ERROR";
	}
	else if (h==DXGI_ERROR_FRAME_STATISTICS_DISJOINT){
		return "DXGI_ERROR_FRAME_STATISTICS_DISJOINT";
	}
	else if (h==DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE){
		return "DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE";
	}
	else if (h==DXGI_ERROR_INVALID_CALL){
		return "DXGI_ERROR_INVALID_CALL";
	}
	else if (h==DXGI_ERROR_MORE_DATA){
		return "DXGI_ERROR_MORE_DATA";
	}
		else if (h==DXGI_ERROR_NONEXCLUSIVE){
		return "DXGI_ERROR_NONEXCLUSIVE";
		}
		else if (h==DXGI_ERROR_NOT_CURRENTLY_AVAILABLE){
			return "DXGI_ERROR_NOT_CURRENTLY_AVAILABLE";
		}
		else if (h==DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED){
			return "DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED";
		}
		else if (h==DXGI_ERROR_REMOTE_OUTOFMEMORY){
			return "DXGI_ERROR_REMOTE_OUTOFMEMORY";
		}
		else if (h==DXGI_ERROR_WAS_STILL_DRAWING){
			return "DXGI_ERROR_WAS_STILL_DRAWING";
		}
		else if (h==DXGI_ERROR_UNSUPPORTED){
			return "DXGI_ERROR_UNSUPPORTED";
		}

	return "Unknown error";
}

struct mesh
{
	ID3D11Buffer* vb;
	ID3D11Buffer* ib;
	int vbOffset, numIndices;

	mesh()
	{
		vb = ib = 0;
		vbOffset = 0;
	}

	~mesh()
	{
		if (vb!=0) vb->Release();
		if (ib!=0) ib->Release();
	}

	void CreateVB(void* vbPtr, int size)
	{
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage            = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth        = size;
		bufferDesc.BindFlags        = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags   = 0;
		bufferDesc.MiscFlags        = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem			= vbPtr;
		initData.SysMemPitch		= 0;
		initData.SysMemSlicePitch	= 0;

		HRESULT h = engine::device->CreateBuffer( &bufferDesc, &initData, &vb );
		if (FAILED(h))
		{
			error << "Can't create vertex buffer: "<<GetDXError(h)<<"\n";
		}
	}

	void CreateIB(void* ibPtr, int size)
	{
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage            = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth        = size;
		bufferDesc.BindFlags        = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags   = 0;
		bufferDesc.MiscFlags        = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem			= ibPtr;
		initData.SysMemPitch		= 0;
		initData.SysMemSlicePitch	= 0;

		HRESULT h = engine::device->CreateBuffer( &bufferDesc, &initData, &ib );
		if (FAILED(h))
		{
			error << "Can't create index buffer: "<<GetDXError(h)<<"\n";
		}

		numIndices = size / sizeof(short);
	}
};

template<class gapiShader, wchar_t shaderType>
struct genericShader
{
private:
	virtual void Create(char* buff, int size, const wchar_t* cPath) = 0;
public:

	gapiShader* shader;

	genericShader()
	{
		shader = 0;
	}

	~genericShader()
	{
		if (shader!=0) shader->Release();
	}

	void Load(const wchar_t* sourcePath)
	{
		wchar_t cPath[engine::maxPathSize];
		int len = wcslen(sourcePath);
		if (len>511)
		{
			error << "sourcePath is too big.\n";
			return;
		}
		memcpy(cPath,sourcePath,sizeof(wchar_t)*(len+1));
		cPath[len] =	L'o';
		cPath[len+1] =	0;

		FILE* sfile = _wfopen(sourcePath,L"rb");
		FILE* cfile = _wfopen(cPath,L"rb");
		if (sfile!=0)
		{
			fclose(sfile);
			bool recompile = false;
			if (cfile==0)
			{
				recompile = true;
			}
			else if (CompareFileDates(sourcePath,cPath)>0)
			{
				cout << "Shader "<<sourcePath<<" is out of date - recompiling...\n";
				fclose(cfile);
				DeleteFile(cPath);
				recompile = true;
			}

			if (recompile)
			{
				wstring commandline =	L" /T ";
				commandline +=			shaderType;
				commandline +=			L"s_5_0 /O3 /Zpc /Fo ";

				commandline +=			L'\"';
				commandline +=			wstring(cPath);
				commandline +=			L'\"';
				commandline +=			L" \"";
				commandline +=			wstring(sourcePath);
				commandline +=			L'\"';

				RunAndWait(L"fxc.exe",commandline.c_str());
				cfile = _wfopen(cPath,L"rb");
			}
		}

		if (cfile==0)
		{
			error << "Error loading shader "<<cPath<<"\n";
			return;
		}

		int fileSize = GetFileSize(cfile);
		char* shaderBuff = new char[fileSize];
		fread(shaderBuff,fileSize,1,cfile);

		Create(shaderBuff, fileSize, cPath);

		delete[] shaderBuff;
	}
};

struct vertexShader : public genericShader<ID3D11VertexShader, L'v'>
{
	int inputLayoutID;
	int stride;

private:
	virtual void Create(char* buff, int size, const wchar_t* cPath)
	{
		HRESULT h = engine::device->CreateVertexShader(buff,size,NULL,&shader);
		if (FAILED(h))
		{
			error << "Can't create vertex shader from "<<cPath<<"\n";
		}

		// Read input layout description from shader info
		ID3D11ShaderReflection* refl;
		if ( FAILED( D3DReflect( buff, size, IID_ID3D11ShaderReflection, (void**)&refl ) ) )
		{
			error << "Can't reflect vertex shader "<<cPath<<"\n";
			return;
		}

		D3D11_SHADER_DESC shaderDesc;
		refl->GetDesc(&shaderDesc);

		int byteOffset = 0;

		vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
		for ( unsigned int i=0; i<shaderDesc.InputParameters; i++ )
		{
			D3D11_SIGNATURE_PARAMETER_DESC paramDesc;      
			refl->GetInputParameterDesc(i, &paramDesc);

			// fill out input element desc
			D3D11_INPUT_ELEMENT_DESC elementDesc;  
			elementDesc.SemanticName =				paramDesc.SemanticName;     
			elementDesc.SemanticIndex =				paramDesc.SemanticIndex;
			elementDesc.InputSlot =					paramDesc.Stream;
			elementDesc.AlignedByteOffset =			0;
			elementDesc.InputSlotClass =			D3D11_INPUT_PER_VERTEX_DATA;
			elementDesc.InstanceDataStepRate =		0;

			// determine DXGI format

			if ( paramDesc.Mask == 1 )
			{
				if		( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32 ) elementDesc.Format = DXGI_FORMAT_R32_UINT;
				else if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32 ) elementDesc.Format = DXGI_FORMAT_R32_SINT;
				else if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
				byteOffset += 4;
			}
			else if ( paramDesc.Mask <= 3 )
			{
				if		( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
				else if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
				else if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
				byteOffset += 8;
			}
			else if ( paramDesc.Mask <= 7 )
			{
				if		( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
				else if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
				else if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
				byteOffset += 12;
			}
			else if ( paramDesc.Mask <= 15 )
			{
				if		( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
				else if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
				else if ( paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32 ) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				byteOffset += 16;
			}

			//save element desc
			inputLayoutDesc.push_back(elementDesc);
		}

		stride = byteOffset;
		int inputLayoutHash = RSHash((const char*)&(inputLayoutDesc[0]), inputLayoutDesc.size()*sizeof(D3D11_INPUT_ELEMENT_DESC));
		if (engine::inputLayoutHashToIndexPlusOne[inputLayoutHash]==0)
		{
			ID3D11InputLayout* layout;
			HRESULT h = engine::device->CreateInputLayout( &inputLayoutDesc[0], inputLayoutDesc.size(), buff, size, &layout);
			if (FAILED(h))
			{
				error << "Can't create input layout from shader "<<cPath<<"\n";
				return;
			}
			engine::inputLayoutHashToIndexPlusOne[inputLayoutHash] = engine::inputLayouts.size()+1;
			engine::inputLayouts.push_back(layout);
		}

		inputLayoutID = engine::inputLayoutHashToIndexPlusOne[inputLayoutHash]-1;
		refl->Release();

		cout << "Loaded vertex shader: "<<cPath<<"\n";
	}
};

struct pixelShader : public genericShader<ID3D11PixelShader, L'p'>
{
private:
	virtual void Create(char* buff, int size, const wchar_t* cPath)
	{
		HRESULT h = engine::device->CreatePixelShader(buff,size,NULL,&shader);
		if (FAILED(h))
		{
			error << "Can't create pixel shader from "<<cPath<<"\n";
		}
		cout << "Loaded pixel shader: "<<cPath<<"\n";
	}
};

struct texture
{
private:
	template<D3D11_BIND_FLAG flag>
	void Create(int width, int height, DXGI_FORMAT format, int mips, int AAlevel, int AAquality)
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc,sizeof(desc));
		desc.Width =				width;
		desc.Height =				height;
		desc.Format =				format;
		desc.MipLevels =			mips;
		desc.ArraySize =			1;
		desc.SampleDesc.Count =		AAlevel;
		desc.SampleDesc.Quality =	AAquality;
		desc.BindFlags =			flag;
	
		HRESULT h = engine::device->CreateTexture2D(&desc,NULL,(ID3D11Texture2D**)&tex);
		if (FAILED(h))
		{
			error << "Can't create texture: "<<GetDXError(h)<<"\n";
			return;
		}
	}
public:
	ID3D11Resource* tex;
	ID3D11DepthStencilView* DSSview;
	ID3D11ShaderResourceView* SRview;

	texture()
	{
		tex = 0;
	}

	~texture()
	{
		// TODO: release views
		tex->Release();
	}

	void CreateDepthTexture(int width, int height, DXGI_FORMAT format, int AAlevel, int AAquality, bool dss)
	{
		DXGI_FORMAT texFormat, shaderFormat;

		switch (format)
		{
			case DXGI_FORMAT_D16_UNORM:
					texFormat = DXGI_FORMAT_R16_TYPELESS;
					shaderFormat = DXGI_FORMAT_R16_FLOAT;
					break;
			case DXGI_FORMAT_D24_UNORM_S8_UINT:
					texFormat = DXGI_FORMAT_R24G8_TYPELESS;
					shaderFormat = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
					break;
			case DXGI_FORMAT_D32_FLOAT:
					texFormat = DXGI_FORMAT_R32_TYPELESS;
					shaderFormat = DXGI_FORMAT_R32_FLOAT;
					break;
			case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
					texFormat = DXGI_FORMAT_R32G8X24_TYPELESS;
					shaderFormat = DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
					break;
		}

		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc,sizeof(desc));
		desc.Width =				width;
		desc.Height =				height;
		desc.Format =				texFormat;
		desc.MipLevels =			1;
		desc.ArraySize =			1;
		desc.SampleDesc.Count =		AAlevel;
		desc.SampleDesc.Quality =	AAquality;
		desc.BindFlags =			D3D11_BIND_SHADER_RESOURCE;
		if (dss) desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
		desc.Usage =				D3D11_USAGE_DEFAULT;
		HRESULT h = engine::device->CreateTexture2D(&desc, NULL, (ID3D11Texture2D**)&tex);
		if (FAILED(h))
		{
			error << "Can't create depth texture: "<<GetDXError(h)<<"\n";
			return;
		}

		if (dss)
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC rtDesc;
			ZeroMemory(&rtDesc,sizeof(rtDesc));
			rtDesc.Format = format;
			rtDesc.ViewDimension = AAlevel<2? D3D11_DSV_DIMENSION_TEXTURE2D : D3D11_DSV_DIMENSION_TEXTURE2DMS;
			if (AAlevel<2)
			{
				rtDesc.Texture2D.MipSlice = 0;
			}

			h = engine::device->CreateDepthStencilView(tex, &rtDesc, &DSSview);
			if (FAILED(h))
			{
				error << "Can't create depth stencil view: "<<GetDXError(h)<<"\n";
				return;
			}
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc,sizeof(srvDesc));
		srvDesc.Format =					shaderFormat;
		srvDesc.ViewDimension =				AAlevel<2? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DMS;
		if (AAlevel < 2) {
			srvDesc.Texture2D.MipLevels =		1;
			srvDesc.Texture2D.MostDetailedMip =	0;
		}
		h = engine::device->CreateShaderResourceView(tex, &srvDesc, &SRview);
		if (FAILED(h))
		{
			error << "Can't create shader resource view: "<<GetDXError(h)<<"\n";
			return;
		}
	}
};

struct constantBuffer
{
	ID3D11Buffer* buff;

	constantBuffer()
	{
		buff = 0;
	}

	~constantBuffer()
	{
		if (buff!=0) buff->Release();
	}

	void Create(void* ptr, int size)
	{
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth =			 size;
		cbDesc.Usage =				 D3D11_USAGE_DEFAULT;//D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags =			 D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags =		 0;//D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags =			 0;
		cbDesc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem =			 ptr;
		initData.SysMemPitch =		 0;
		initData.SysMemSlicePitch =	 0;

		HRESULT h = engine::device->CreateBuffer(&cbDesc, &initData, &buff);
		if (FAILED(h))
		{
			error << "Can't create constant buffer: "<<GetDXError(h)<<"\n";
			return;
		}
	}

	void Create(int size)
	{
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth =			 size;
		cbDesc.Usage =				 D3D11_USAGE_DEFAULT;//D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags =			 D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags =		 0;//D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags =			 0;
		cbDesc.StructureByteStride = 0;

		HRESULT h = engine::device->CreateBuffer(&cbDesc, NULL, &buff);
		if (FAILED(h))
		{
			error << "Can't create constant buffer: "<<GetDXError(h)<<"\n";
			return;
		}
	}
};

struct rasterizerState
{
	D3D11_RASTERIZER_DESC desc;
	ID3D11RasterizerState* state;

	rasterizerState()
	{
		desc.FillMode				= D3D11_FILL_SOLID;
		desc.CullMode				= D3D11_CULL_BACK;
		desc.FrontCounterClockwise = FALSE;
		desc.DepthBias				= 0;
		desc.DepthBiasClamp		= 0.0f;
		desc.SlopeScaledDepthBias	= 0.0f;
		desc.DepthClipEnable		= TRUE;
		desc.ScissorEnable			= FALSE;
		desc.MultisampleEnable		= TRUE;
		desc.AntialiasedLineEnable	= FALSE;
	}

	void Create()
	{
		HRESULT h = engine::device->CreateRasterizerState(&desc, &state);
		if (FAILED(h)) error << "Can't create rasterizer state:"<<GetDXError(h)<<"\n";
	}
};

struct depthStencilState
{
	D3D11_DEPTH_STENCIL_DESC desc;
	ID3D11DepthStencilState* state;

	depthStencilState()
	{
		desc.DepthEnable			= TRUE;
		desc.DepthWriteMask			= D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc				= D3D11_COMPARISON_LESS;
		desc.StencilEnable			= FALSE;
	}

	void Create()
	{
		HRESULT h = engine::device->CreateDepthStencilState(&desc, &state);
		if (FAILED(h)) error << "Can't create depth stencil state:"<<GetDXError(h)<<"\n";
	}
};

struct blendState
{
	D3D11_BLEND_DESC desc;
	ID3D11BlendState* state;

	blendState()
	{
		desc.AlphaToCoverageEnable	= FALSE;
		desc.IndependentBlendEnable = FALSE;
		desc.RenderTarget[0].BlendEnable = FALSE;
		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	}

	void Create()
	{
		HRESULT h = engine::device->CreateBlendState(&desc, &state);
		if (FAILED(h)) error << "Can't create blend state:"<<GetDXError(h)<<"\n";
	}
};

struct samplerState
{
	D3D11_SAMPLER_DESC desc;
	ID3D11SamplerState* state;

	samplerState()
	{
		desc.Filter					= D3D11_FILTER_ANISOTROPIC;
		desc.AddressU				= D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV				= D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW				= D3D11_TEXTURE_ADDRESS_WRAP;
		desc.MipLODBias				= 0;
		desc.MaxAnisotropy			= 8;
		desc.ComparisonFunc			= D3D11_COMPARISON_ALWAYS;
		desc.MinLOD					= 0;
		desc.MaxLOD					= D3D11_FLOAT32_MAX;
	}

	ID3D11SamplerState* Create()
	{
		HRESULT h = engine::device->CreateSamplerState(&desc, &state);
		if (FAILED(h)) error << "Can't create sampler state:"<<GetDXError(h)<<"\n";
		return state;
	}
};

struct viewport
{
	D3D11_VIEWPORT vp;

	viewport()
	{
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		vp.Width =		engine::screenWidth;
		vp.Height =		engine::screenHeight;
		vp.MinDepth = 0;
		vp.MaxDepth = 1;
	}
};

LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
{
	if (message == WM_DESTROY)
	{
			::PostQuitMessage(0);
			return 0;
	}
	
	return ::DefWindowProc(hwnd, message, wparam, lparam);
}

void CreateEngineWindow(HINSTANCE appHandle, int width, int height)
{
	engine::hInstance = (HINSTANCE)appHandle;

	WNDCLASS wc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject( WHITE_BRUSH );
	wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	wc.hIcon          = 0;
	wc.hInstance = engine::hInstance;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = L"IEngine_C";
	wc.lpszMenuName = 0;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	
	if( !RegisterClass(&wc) ) 
	{
		error << "Can't register window class: "<<GetLastError()<<"\n";
		cout << "Can't register window class.\n";
		return;
	}

	RECT wr = {0,0,width,height};
	AdjustWindowRect(&wr,WS_OVERLAPPEDWINDOW, FALSE);

	engine::window = CreateWindow(L"IEngine_C",L"iengine_C",WS_OVERLAPPEDWINDOW,
									0,0, wr.right-wr.left, wr.bottom-wr.top, NULL,NULL,engine::hInstance,NULL);

	ShowWindow(engine::window,SW_SHOW);
	UpdateWindow(engine::window);

	ZeroMemory(engine::KeyDown, sizeof(BYTE)*256);
	ZeroMemory(engine::KeyDownPrev, sizeof(BYTE)*256);
}


void InitDevice(int width, int height, int AA, bool fullScreen, bool vsync, DXGI_FORMAT backBufferFormat)
{
	engine::vsync = vsync? 1 : 0;

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

	//set buffer dimensions and format
	swapChainDesc.BufferCount = 1;
	swapChainDesc.BufferDesc.Width = width;
	swapChainDesc.BufferDesc.Height = height;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferDesc.Format = backBufferFormat;

	//set refresh rate
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;

	// set AA
	engine::backBufferAAlevel =							AA;
	engine::backBufferAAquality =						D3D11_STANDARD_MULTISAMPLE_PATTERN;

	swapChainDesc.SampleDesc.Count =					engine::backBufferAAlevel;
	swapChainDesc.SampleDesc.Quality =					engine::backBufferAAquality;

	//output window handle
	swapChainDesc.OutputWindow = engine::window;
	swapChainDesc.Windowed =	 !fullScreen;

	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	HRESULT h = 
#ifdef _DEBUG
	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_SINGLETHREADED | D3D11_CREATE_DEVICE_DEBUG,
#else
	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_SINGLETHREADED,
#endif
		NULL, // Default feature levels (max = D3D11.0)
		0, D3D11_SDK_VERSION, &swapChainDesc, &engine::swapChain,
		&engine::device, &engine::featureLevel, &engine::deviceContext);

	if (FAILED(h))
	{
		error << "Can't create Direct3D device: "<<GetDXError(h)<<"\n";
		return;
	}

	h = engine::swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&engine::backBuffer);
	if (FAILED(h))
	{
		error << "Can't get swap chain buffer: "<<GetDXError(h)<<"\n";
	}

	D3D11_RENDER_TARGET_VIEW_DESC rtDesc;
	rtDesc.Format = backBufferFormat;
	if (AA<2)
	{
		rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	}
	else
	{
		rtDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
	}
	rtDesc.Texture2D.MipSlice = 0;

	h = engine::device->CreateRenderTargetView(engine::backBuffer, &rtDesc, &engine::backBufferView);
	if (FAILED(h))
	{
		cout << engine::backBuffer << endl;
		error << "Can't create backbuffer RT view: "<<GetDXError(h)<<"\n";
	}

	engine::screenWidth = width;
	engine::screenHeight = height;
	engine::screenAspect = width/float(height);
	engine::backBufferFormat = backBufferFormat;

	cout << "D3D11 initialized." <<endl;
}

void InitEngine(HINSTANCE hInstance, int width, int height, int AA, bool fullScreen, bool vsync, DXGI_FORMAT backBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)
{
	GetExeDir();
	SetCurrentDirectory(engine::exeDir);
	CreateEngineWindow(hInstance, width, height);
	InitDevice(width, height, AA, fullScreen, vsync, backBufferFormat);
}

bool EngineRunning()
{
	MSG msg;

	if(::PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
		if (msg.message == 18)
		{
			return false; // window closed
		}
	}

	memcpy(engine::KeyDownPrev, engine::KeyDown, sizeof(BYTE)*256);
	GetKeyboardState(engine::KeyDown);
	SetCursorPos(engine::screenWidth/2, engine::screenHeight/2);

	return true;
}


void CreateQuad(mesh& quad)
{
	float quadVerts[] =
	{
		-1,	-1,	0,
		1,	-1,	0,
		-1,	1,	0,
		1,	1,	0,
	};
	short quadIndices[] = 
	{
		2,	1,	0,
		3,	1,	2
	};
	quad.CreateVB(quadVerts,	3*sizeof(float) * 4);
	quad.CreateIB(quadIndices,	3*sizeof(short) * 2);
}

void CreateBox(mesh& box, float minX=-1, float minY=-1, float minZ=-1, float maxX=1, float maxY=1, float maxZ=1)
{
	float boxVerts[] =
	{
		minX,	minY,	minZ,		0,0,-1,		0,0,
		maxX,	minY,	minZ,		0,0,-1,		1,0,
		minX,	maxY,	minZ,		0,0,-1,		0,1,
		maxX,	maxY,	minZ,		0,0,-1,		1,1,

		maxX,	minY,	maxZ,		0,0,1,		0,0,
		minX,	minY,	maxZ,		0,0,1,		1,0,
		maxX,	maxY,	maxZ,		0,0,1,		0,1,
		minX,	maxY,	maxZ,		0,0,1,		1,1,

		minX,	minY,	minZ,		-1,0,0,		0,0,
		minX,	maxY,	minZ,		-1,0,0,		1,0,
		minX,	minY,	maxZ,		-1,0,0,		0,1,
		minX,	maxY,	maxZ,		-1,0,0,		1,1,

		maxX,	minY,	maxZ,		1,0,0,		0,0,
		maxX,	maxY,	maxZ,		1,0,0,		1,0,
		maxX,	minY,	minZ,		1,0,0,		0,1,
		maxX,	maxY,	minZ,		1,0,0,		1,1,

		minX,	minY,	minZ,		0,-1,0,		0,0,
		maxX,	minY,	minZ,		0,-1,0,		1,0,
		minX,	minY,	maxZ,		0,-1,0,		0,1,
		maxX,	minY,	maxZ,		0,-1,0,		1,1,

		maxX,	maxY,	minZ,		0,1,0,		0,0,
		minX,	maxY,	minZ,		0,1,0,		1,0,
		maxX,	maxY,	maxZ,		0,1,0,		0,1,
		minX,	maxY,	maxZ,		0,1,0,		1,1
	};
	short boxIndices[] = 
	{
		2,	1,	0,
		3,	1,	2,

		6,	5,	4,
		7,	5,	6,

		10,	9,	8,
		11,	9,	10,

		14,	13,	12,
		15,	13,	14,

		16,	17,	18,
		18,	17,	19,

		20, 21,	22,
		22,	21,	23
	};
	box.CreateVB(boxVerts,		(3*sizeof(float)+3*sizeof(float)+2*sizeof(float)) * 24);
	box.CreateIB(boxIndices,	3*sizeof(short) * 12);
}

BOOL KeyDown(int id)
{
	return engine::KeyDown[id]&0x80;
}

BOOL KeyHit(int id)
{
	return (engine::KeyDown[id]&0x80) && (!(engine::KeyDownPrev[id]&0x80));
}

