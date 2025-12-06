IDirect3DPixelShader9* pShaderToDraw = nullptr;
bool bShaderDisco = false;
float fDiscoSpeed = 255 * 2;
bool bShaderTimerEase1 = false;
bool bShaderTimerInvert = false;

void DoScreenShaders() {
	static IDirect3DTexture9* pRenderTargetCopy = nullptr;
	static IDirect3DPixelShader9* pShader = nullptr;
	static float fTime[4] = {0,0,0,0};
	static float fTimeEased[4] = {0,0,0,0};

	static CNyaTimer gTimer;
	gTimer.Process();
	for (auto& time : fTime) {
		time += gTimer.fDeltaTime * (bShaderTimerInvert ? -1 : 1);
		if (bShaderTimerInvert) {
			if (time < 0) time = 0;
			if (time > 1) time = 1;
		}
	}
	for (auto& eased : fTimeEased) {
		eased = fTime[0];
		if (eased > 1) eased = 1;
		eased = easeInOutQuart(eased);
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
			g_pd3dDevice->SetPixelShaderConstantF(1, bShaderTimerEase1 ? fTimeEased : fTime, 1);
			float temp[] = {(float)nResX, (float)nResY, 0, 0};
			g_pd3dDevice->SetPixelShaderConstantF(2, temp, 1);
		}, false);
		if (bShaderDisco) {
			static double r = 0;
			static double g = 255;
			static double b = 255;
			static int stage = 0;
			switch (stage) {
				case 0:
					b -= fDiscoSpeed * gTimer.fDeltaTime;
					if (b <= 0) {
						b = 0;
						stage++;
					}
					break;
				case 1:
					g += fDiscoSpeed * gTimer.fDeltaTime;
					if (g >= 255) {
						g = 255;
						stage++;
					}
					break;
				case 2:
					r -= fDiscoSpeed * gTimer.fDeltaTime;
					if (r <= 0) {
						r = 0;
						stage++;
					}
					break;
				case 3:
					b += fDiscoSpeed * gTimer.fDeltaTime;
					if (b >= 255) {
						b = 255;
						stage++;
					}
					break;
				case 4:
					g -= fDiscoSpeed * gTimer.fDeltaTime;
					if (g <= 0) {
						g = 0;
						stage++;
					}
					break;
				case 5:
					r += fDiscoSpeed * gTimer.fDeltaTime;
					if (r >= 255) {
						r = 255;
						stage = 0;
					}
					break;
			}
			DrawRectangle(0,1,0,1,{(uint8_t)r,(uint8_t)g,(uint8_t)b,255},0,pRenderTargetCopy);
		}
		else {
			DrawRectangle(0,1,0,1,{255,255,255,255},0,pRenderTargetCopy);
		}
		DrawCallback([](const ImDrawList* parent_list, const ImDrawCmd* cmd) {
			pRenderTargetCopy->Release();
			pRenderTargetCopy = nullptr;
			pShader = nullptr;
		}, false);
		DrawCallback(ImDrawCallback_ResetRenderState, false);
	}
	else {
		memset(fTime, 0, sizeof(fTime));
	}
	pShaderToDraw = nullptr;
	bShaderDisco = false;
	bShaderTimerInvert = false;
}

ChloeHook Hook_ScreenShaders([]() {
	aDrawingGameUILoopFunctions.push_back(DoScreenShaders);
});