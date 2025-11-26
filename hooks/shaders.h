IDirect3DPixelShader9* pShaderToDraw = nullptr;

void DoScreenShaders() {
	static IDirect3DTexture9* pRenderTargetCopy = nullptr;
	static IDirect3DPixelShader9* pShader = nullptr;
	static float fTime[4] = {0,0,0,0};
	
	static CNyaTimer gTimer;
	gTimer.Process();
	for (auto& time : fTime) {
		time += gTimer.fDeltaTime;
	}
	
	if (auto shader = pShaderToDraw) {
		static IDirect3DSurface9* pRenderTarget = nullptr;
		g_pd3dDevice->GetRenderTarget(0, &pRenderTarget);
		if (!pRenderTarget) return;
		
		D3DSURFACE_DESC desc;
		pRenderTarget->GetDesc(&desc);
		g_pd3dDevice->CreateTexture(desc.Width, desc.Height, 1, D3DUSAGE_RENDERTARGET, desc.Format, D3DPOOL_DEFAULT, &pRenderTargetCopy, nullptr);
		if (!pRenderTargetCopy) return;
		
		pShader = shader;
		DrawCallback([](const ImDrawList* parent_list, const ImDrawCmd* cmd) {
			IDirect3DSurface9* pSurface = nullptr;
			pRenderTargetCopy->GetSurfaceLevel(0, &pSurface);
			g_pd3dDevice->StretchRect(pRenderTarget, nullptr, pSurface, nullptr, D3DTEXF_LINEAR);
			pRenderTargetCopy->Release(); // not deleting here - this is releasing the texture after GetSurfaceLevel
			pRenderTarget->Release();
			pRenderTarget = nullptr;
			
			g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE); // the render target may have alpha baked in!
			g_pd3dDevice->SetPixelShader(pShader);
			g_pd3dDevice->SetPixelShaderConstantF(1, fTime, 1);
		}, false);
		DrawRectangle(0,1,0,1,{255,255,255,255},0,pRenderTargetCopy);
		DrawCallback([](const ImDrawList* parent_list, const ImDrawCmd* cmd) {
			pRenderTargetCopy->Release();
			pRenderTargetCopy = nullptr;
			pShader = nullptr;
		}, false);
		DrawCallback(ImDrawCallback_ResetRenderState, false);
	}
	pShaderToDraw = nullptr;
}

// todo move this to draw under the hud
ChloeHook Hook_ScreenShaders([]() {
	aDrawingLoopFunctions.push_back(DoScreenShaders);
});