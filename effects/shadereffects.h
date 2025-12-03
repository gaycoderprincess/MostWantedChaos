#define EFFECT_CATEGORY_TEMP "Shader"

class EffectBase_ScreenShader : public ChaosEffect {
public:
	// todo add this to the d3d reset handler
	IDirect3DPixelShader9* pShader = nullptr;
	std::string sFileName = "mirror";
	bool bEasedTimer = true;
	bool bInvertTimerAtEnd = true;

	EffectBase_ScreenShader(const char* category) : ChaosEffect(category) {
		IncompatibilityGroups.push_back(Attrib::StringHash32("screenshader"));
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
		if (bEasedTimer) bShaderTimerEase1 = true;
		if (bInvertTimerAtEnd && EffectInstance->fTimer < 1) bShaderTimerInvert = true;
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
	bool RunWhenBlocked() override { return true; }
	bool ShouldAbort() override { return !pShader; }
};

class Effect_ShaderMirror : public EffectBase_ScreenShader {
public:
	Effect_ShaderMirror() : EffectBase_ScreenShader(EFFECT_CATEGORY_TEMP) {
		sName = "British";
		sFriendlyName = "Mirror The Screen";
		fTimerLength = 60;
		sFileName = "mirror";
	}
} E_ShaderMirror;

class Effect_ShaderPixel : public EffectBase_ScreenShader {
public:
	Effect_ShaderPixel() : EffectBase_ScreenShader(EFFECT_CATEGORY_TEMP) {
		sName = "Pixelate The Screen";
		fTimerLength = 30;
		sFileName = "pixelate";
		//bEasedTimer = false;
	}
} E_ShaderPixel;

class Effect_ShaderDisco : public EffectBase_ScreenShader {
public:
	Effect_ShaderDisco() : EffectBase_ScreenShader(EFFECT_CATEGORY_TEMP) {
		sName = "Disco Time";
		sFriendlyName = "Disco Screen Overlay";
		fTimerLength = 60;
		sFileName = "default";
	}

	void TickFunction(double delta) override {
		bShaderDisco = true;
		EffectBase_ScreenShader::TickFunction(delta);
	}
} E_ShaderDisco;

class Effect_ShaderSquared : public EffectBase_ScreenShader {
public:
	Effect_ShaderSquared() : EffectBase_ScreenShader(EFFECT_CATEGORY_TEMP) {
		sName = "High Contrast";
		fTimerLength = 60;
		sFileName = "squared";
	}
} E_ShaderSquared;

class Effect_ShaderSqrt : public EffectBase_ScreenShader {
public:
	Effect_ShaderSqrt() : EffectBase_ScreenShader(EFFECT_CATEGORY_TEMP) {
		sName = "Low Contrast";
		fTimerLength = 60;
		sFileName = "sqrt";
	}
} E_ShaderSqrt;

class Effect_ShaderInverted : public EffectBase_ScreenShader {
public:
	Effect_ShaderInverted() : EffectBase_ScreenShader(EFFECT_CATEGORY_TEMP) {
		sName = "Inverted Colors";
		fTimerLength = 60;
		sFileName = "invert";
	}
} E_ShaderInverted;

class Effect_ShaderBlue : public EffectBase_ScreenShader {
public:
	Effect_ShaderBlue() : EffectBase_ScreenShader(EFFECT_CATEGORY_TEMP) {
		sName = "Da Ba Dee";
		sFriendlyName = "Blue Screen Overlay";
		fTimerLength = 60;
		sFileName = "blue";
	}
} E_ShaderBlue;

class Effect_ShaderWide : public EffectBase_ScreenShader {
public:
	Effect_ShaderWide() : EffectBase_ScreenShader(EFFECT_CATEGORY_TEMP) {
		sName = "4:3 Stretched";
		fTimerLength = 60;
		sFileName = "wide";
	}
} E_ShaderWide;

class Effect_ShaderTikTok : public EffectBase_ScreenShader {
public:
	Effect_ShaderTikTok() : EffectBase_ScreenShader(EFFECT_CATEGORY_TEMP) {
		sName = "Tik Tok Mode";
		sFriendlyName = "Tik Tok Screen Overlay";
		fTimerLength = 60;
		sFileName = "portrait";
	}
} E_ShaderTikTok;

class Effect_ShaderZoomIn : public EffectBase_ScreenShader {
public:
	Effect_ShaderZoomIn() : EffectBase_ScreenShader(EFFECT_CATEGORY_TEMP) {
		sName = "Tunnel Vision";
		fTimerLength = 30;
		sFileName = "zoom";
	}
} E_ShaderZoomIn;

/*class Effect_ShaderRed : public EffectBase_ScreenShader {
public:
	Effect_ShaderRed() : EffectBase_ScreenShader(EFFECT_CATEGORY_TEMP) {
		sName = "Ketchup";
		sFriendlyName = "Ketchup Screen Overlay";
		fTimerLength = 60;
		sFileName = "red";
	}
} E_ShaderRed;*/

class Effect_ShaderUpsideDown : public EffectBase_ScreenShader {
public:
	Effect_ShaderUpsideDown() : EffectBase_ScreenShader(EFFECT_CATEGORY_TEMP) {
		sName = "Meanwhile In Australia";
		sFriendlyName = "Upside Down Screen";
		fTimerLength = 45;
		sFileName = "flip";
	}
} E_ShaderUpsideDown;

class Effect_ShaderGray : public EffectBase_ScreenShader {
public:
	Effect_ShaderGray() : EffectBase_ScreenShader(EFFECT_CATEGORY_TEMP) {
		sName = "Grayscale";
		sFriendlyName = "Grayscale Screen Overlay";
		fTimerLength = 60;
		sFileName = "gray";
	}
} E_ShaderGray;

class Effect_ShaderBW : public EffectBase_ScreenShader {
public:
	Effect_ShaderBW() : EffectBase_ScreenShader(EFFECT_CATEGORY_TEMP) {
		sName = "Black & White";
		sFriendlyName = "Black & White Screen Overlay";
		fTimerLength = 30;
		sFileName = "blackwhite";
	}
} E_ShaderBW;