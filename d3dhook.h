void UpdateD3DProperties() {
	g_pd3dDevice = *(IDirect3DDevice9**)0x982BDC;
	ghWnd = *(HWND*)0x982BF4;

	// todo this can prolly be done better
	RECT rect;
	GetWindowRect(ghWnd, &rect);
	nResX = rect.right - rect.left;
	nResY = rect.bottom - rect.top;
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

void ChaosLoop();
void HookLoop() {
	ChaosLoop();
	CommonMain();
}