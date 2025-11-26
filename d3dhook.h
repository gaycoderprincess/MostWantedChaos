bool bDrawingGameUI = false;
void UpdateD3DProperties() {
	g_pd3dDevice = *(IDirect3DDevice9**)0x982BDC;
	ghWnd = *(HWND*)0x982BF4;
	GetRacingResolution(&nResX, &nResY);
}

bool bDeviceJustReset = false;
void D3DHookMain() {
	if (!g_pd3dDevice) {
		UpdateD3DProperties();
		InitHookBase();
	}

	if (bDeviceJustReset) {
		ImGui_ImplDX9_CreateDeviceObjects();
		bDeviceJustReset = false;
	}
	HookBaseLoop();
}

void OnD3DReset() {
	if (g_pd3dDevice) {
		UpdateD3DProperties();
		ImGui_ImplDX9_InvalidateDeviceObjects();
		bDeviceJustReset = true;
	}
}

void D3DHookPreHUD() {
	bDrawingGameUI = true;
	D3DHookMain();
}

void ChaosLoop();
void HookLoop() {
	if (bDrawingGameUI) {
		for (auto& func : aDrawingGameUILoopFunctions) {
			func();
		}

		for (auto& func : aDrawingGameUILoopFunctionsOnce) {
			func();
		}
		aDrawingGameUILoopFunctionsOnce.clear();

		bDontRefreshInputsThisLoop = true;
		CommonMain();
		bDrawingGameUI = false;
		return;
	}
	ChaosLoop();
	CommonMain();
}