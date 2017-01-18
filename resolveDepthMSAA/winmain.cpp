#include "windows.h"
#include "engine_lvl2.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreviousInstance, LPSTR lpcmdline, int nCmdShow)
{
	// Init D3D
	InitEngine(hInstance, 1280,1024, 4, false, false);

	// Setup box
	vertexShader vs;
	pixelShader ps;
	vs.Load(L"box.vs");
	ps.Load(L"box.ps");
	mesh box;
	entity rotBox;
	CreateBox(box);
	rotBox.m	= &box;

	// Setup resolve quad
	vertexShader vsResolveQuad;
	pixelShader psResolveQuad;
	vsResolveQuad.Load(L"quad.vs");
	psResolveQuad.Load(L"quadResolve.ps");
	mesh quad;
	CreateQuad(quad);

	// Setup resolved draw quad
	pixelShader psDrawDepthQuad;
	psDrawDepthQuad.Load(L"quadDrawDepth.ps");

	// Create MSAA depth buffer
	texture depth;
	depth.CreateDepthTexture(engine::screenWidth, engine::screenHeight, DXGI_FORMAT_D32_FLOAT, 
													engine::backBufferAAlevel, engine::backBufferAAquality, true);
	// Create Non-MSAA depth buffer
	texture resolvedDepth;
	resolvedDepth.CreateDepthTexture(engine::screenWidth, engine::screenHeight, DXGI_FORMAT_D32_FLOAT, 1, 0, true);

	// Setup states and constants
	rasterizerState rstate;
	rstate.desc.MultisampleEnable = TRUE;
	rstate.Create();
	rasterizerState rstateNoMsaa;
	rstateNoMsaa.desc.MultisampleEnable	= FALSE;
	rstateNoMsaa.Create();
	engine::deviceContext->RSSetState(rstate.state);

	depthStencilState dstate;
	dstate.Create();
	depthStencilState dstateAlways;
	dstateAlways.desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	dstateAlways.Create();
	engine::deviceContext->OMSetDepthStencilState(dstate.state,0);

	blendState bstate;
	bstate.Create();
	engine::deviceContext->OMSetBlendState(bstate.state, NULL, 0xFFFFFFFF);

	viewport vp;
	engine::deviceContext->RSSetViewports(1,&vp.vp);

	engine::deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11ShaderResourceView* noSrv[1];
	noSrv[0] = NULL;

	ID3D11RenderTargetView* noRt[1];
	noRt[0] = NULL;

	// Setup constants
	float4 gray = Set(0.2333f, 0.2333f, 0.2333f, 1);

	float4x4 cameraWorld;
	SetIdentity(cameraWorld);
	SetPosition(cameraWorld, Set(0,0,-2,1));

	float4x4 MatView = cameraWorld;
	Inverse(MatView);

	float4x4 MatProj = PerspProjection(engine::screenAspect, Pi/2, 0.01f, 1000);
	float4x4 MatViewProj = MatView * MatProj;

	float4 rot = quatFromAxisAngle(1,0,0, 0.0001f);
	float4 rot2 = quatFromAxisAngle(0,1,0, 0.0001f);
	float4 rot3 = quatFromAxisAngle(0,0,1, 0.0001f);
	rot = mulQuat(rot,rot2);
	rot = mulQuat(rot,rot3);
	SetIdentity(rotBox.cb.MatWorld);
	SetPosition(rotBox.cb.MatWorld, Set(0,0,0,1));

	constantBuffers shaderConstants;
	engine::deviceContext->VSSetConstantBuffers(CBUFFER_PER_OBJECT, 1, &shaderConstants.perObject.buff);
	engine::deviceContext->VSSetConstantBuffers(CBUFFER_PER_CAMERA, 1, &shaderConstants.perCamera.buff);

	// Render loop
	while(EngineRunning())
	{
		if (KeyDown(VK_ESCAPE)) break;

		// Attach backbuffer and MSAA depth buffer
		engine::deviceContext->OMSetRenderTargets(1, &engine::backBufferView, depth.DSSview);
		engine::deviceContext->RSSetState(rstate.state);
		engine::deviceContext->ClearRenderTargetView(engine::backBufferView,(float*)&gray);
		engine::deviceContext->ClearDepthStencilView(depth.DSSview, D3D11_CLEAR_DEPTH, 1, 0);

		// Rotate box
		rotBox.cb.MatWorld			= Rotation(rot) * rotBox.cb.MatWorld;
		rotBox.cb.MatWorldViewProj	= rotBox.cb.MatWorld * MatViewProj;
		engine::deviceContext->UpdateSubresource(shaderConstants.perObject.buff, 0,0, &rotBox.cb, 0,0);

		// Render box
		engine::deviceContext->VSSetShader(vs.shader,NULL,0);
		engine::deviceContext->PSSetShader(ps.shader,NULL,0);
		engine::deviceContext->IASetInputLayout(engine::inputLayouts[vs.inputLayoutID]);
		engine::deviceContext->IASetVertexBuffers(0, 1, &box.vb, (const UINT*)&vs.stride, (const UINT*)&box.vbOffset);
		engine::deviceContext->IASetIndexBuffer(box.ib, DXGI_FORMAT_R16_UINT, 0);
		engine::deviceContext->DrawIndexed(box.numIndices, 0, 0);

		// Won't work
		//engine::deviceContext->ResolveSubresource(resolvedDepth.tex, 0, depth.tex, 0, DXGI_FORMAT_D32_FLOAT);

		// ----- Resolve depth -----
		// Attach Non-MSAA depth buffer with no color RT
		engine::deviceContext->OMSetRenderTargets(1, noRt, resolvedDepth.DSSview);
		// Use rasterizer state with multisample off
		engine::deviceContext->RSSetState(rstateNoMsaa.state);
		// Make depth test always pass
		engine::deviceContext->OMSetDepthStencilState(dstateAlways.state,0);
		// Give MSAA depth buffer to shader
		engine::deviceContext->PSSetShaderResources(0, 1, &depth.SRview);
		// Draw quad which writes to SV_Depth
		engine::deviceContext->VSSetShader(vsResolveQuad.shader,NULL,0);
		engine::deviceContext->PSSetShader(psResolveQuad.shader,NULL,0);
		engine::deviceContext->IASetInputLayout(engine::inputLayouts[vsResolveQuad.inputLayoutID]);
		engine::deviceContext->IASetVertexBuffers(0, 1, &quad.vb, (const UINT*)&vsResolveQuad.stride, (const UINT*)&quad.vbOffset);
		engine::deviceContext->IASetIndexBuffer(quad.ib, DXGI_FORMAT_R16_UINT, 0);
		engine::deviceContext->DrawIndexed(quad.numIndices, 0, 0);

		// ----- Draw quad with resolved depth buffer to test if it's OK -----
		// Attach backbuffer back
		engine::deviceContext->OMSetRenderTargets(1, &engine::backBufferView, NULL);
		// Give resolved depth buffer to shader
		engine::deviceContext->PSSetShaderResources(0, 1, &resolvedDepth.SRview);
		// Draw quad which visualizes depth
		engine::deviceContext->PSSetShader(psDrawDepthQuad.shader,NULL,0);
		engine::deviceContext->DrawIndexed(quad.numIndices, 0, 0);

		engine::swapChain->Present(engine::vsync,0);
	}
}