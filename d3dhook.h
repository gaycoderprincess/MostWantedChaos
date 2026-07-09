bool bDrawingGameUI = false;
std::vector<void(*)()> aDrawingGameUILoopFunctions;
void UpdateD3DProperties() {
	g_pd3dDevice = GameD3DDevice;
	ghWnd = GameWindow;
	GetRacingResolution(&nResX, &nResY);
}

bool bDeviceJustReset = false;
void D3DHookMain() {
	DLLDirSetter _setdir;

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
	DLLDirSetter _setdir;

	if (g_pd3dDevice) {
		UpdateD3DProperties();
		ImGui_ImplDX9_InvalidateDeviceObjects();
		bDeviceJustReset = true;
	}
}

void D3DHookPreHUD() {
	PerformanceBenchmarker _perf("D3DHookPreHUD");

	bDrawingGameUI = true;
	D3DHookMain();
}

void ChaosLoop();
void HookLoop() {
	if (bDrawingGameUI) {
		PerformanceBenchmarker _perf("HookLoop(bDrawingGameUI)");

		for (auto& func : aDrawingGameUILoopFunctions) {
			func();
		}

		bDontRefreshInputsThisLoop = true;
		CommonMain();
		bDrawingGameUI = false;
		return;
	}

	PerformanceBenchmarker _perf("HookLoop(!bDrawingGameUI)");

	ChaosLoop();
	CommonMain();
}