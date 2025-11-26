class EffectBase_ScreenShader : public ChaosEffect {
public:
	// todo add this to the d3d reset handler
	IDirect3DPixelShader9* pShader = nullptr;
	std::string sFileName = "mirror";
	
	EffectBase_ScreenShader() : ChaosEffect() {
		IncompatibilityGroup = Attrib::StringHash32("screenshader");
	};
	
	void InitFunction() override {
		if (!pShader) {
			auto path = std::format("CwoeeChaos/data/shaders/{}.sha", sFileName);
			if (!std::filesystem::exists(path)) {
				MessageBoxA(nullptr, std::format("Failed to find shader {}", path).c_str(), "nya?!~", MB_ICONERROR);
				exit(0);
			}
			std::ifstream file(path);
			auto size = std::filesystem::file_size(path);
			auto shader = new char[size];
			file.read(shader, size);
			
			LPD3DXBUFFER pixel_shader_buffer = nullptr;
			LPD3DXBUFFER error_buffer = nullptr;
			auto hr = D3DXCompileShader(shader, size, nullptr, nullptr, "main", "ps_3_0", 0, &pixel_shader_buffer, &error_buffer, nullptr);
			delete[] shader;
			if (hr != D3D_OK) {
				MessageBoxA(nullptr, (char*)error_buffer->GetBufferPointer(), "nya?!~", MB_ICONERROR);
				exit(0);
				return;
			}
			g_pd3dDevice->CreatePixelShader((DWORD*)pixel_shader_buffer->GetBufferPointer(), &pShader);
			pixel_shader_buffer->Release();
		}
	}
	void TickFunction(double delta) override {
		if (!pShader) return;
		pShaderToDraw = pShader;
	}
	bool HasTimer() override { return true; }
	bool ShouldAbort() override { return !pShader; }
};

class Effect_ShaderMirror : public EffectBase_ScreenShader {
public:
	Effect_ShaderMirror() : EffectBase_ScreenShader() {
		sName = "British";
		sFriendlyName = "Mirror The Screen";
		fTimerLength = 60;
		sFileName = "mirror";
	}

	bool RunInMenus() override { return true; }
} E_ShaderMirror;

class Effect_ShaderPixel : public EffectBase_ScreenShader {
public:
	Effect_ShaderPixel() : EffectBase_ScreenShader() {
		sName = "Pixelate The Screen";
		fTimerLength = 30;
		sFileName = "pixelate";
	}

	bool RunInMenus() override { return true; }
} E_ShaderPixel;

class Effect_ShaderDisco : public EffectBase_ScreenShader {
public:
	Effect_ShaderDisco() : EffectBase_ScreenShader() {
		sName = "Disco Time";
		sFriendlyName = "Disco Screen Overlay";
		fTimerLength = 60;
		sFileName = "disco";
	}

	void TickFunction(double delta) override {
		bShaderDisco = true;
		EffectBase_ScreenShader::TickFunction(delta);
	}
	bool RunInMenus() override { return true; }
} E_ShaderDisco;

class Effect_ShaderSquared : public EffectBase_ScreenShader {
public:
	Effect_ShaderSquared() : EffectBase_ScreenShader() {
		sName = "High Contrast";
		fTimerLength = 60;
		sFileName = "squared";
	}

	bool RunInMenus() override { return true; }
} E_ShaderSquared;