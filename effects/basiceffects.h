class Effect_Blind : public ChaosEffect {
public:
	Effect_Blind() : ChaosEffect() {
		sName = "Blind";
		fTimerLength = 10;
	}

	void TickFunction() override {
		DrawRectangle(0, 1, 0, 1, {0,0,0,255});
	}
	bool HasTimer() override { return true; }
} E_Blind;

class Effect_PortraitMode : public ChaosEffect {
public:
	Effect_PortraitMode() : ChaosEffect() {
		sName = "Portrait Mode";
	}

	void TickFunction() override {
		DrawRectangle(0, 0.25, 0, 1, {0,0,0,255});
		DrawRectangle(0.75, 1, 0, 1, {0,0,0,255});
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
} E_PortraitMode;

class Effect_WidescreenMode : public ChaosEffect {
public:
	Effect_WidescreenMode() : ChaosEffect() {
		sName = "Widescreen Borders";
	}

	void TickFunction() override {
		DrawRectangle(0, 1, 0, 0.1, {0,0,0,255});
		DrawRectangle(0, 1, 0.9, 1, {0,0,0,255});
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
} E_WidescreenMode;

class Effect_FakeCrash : public ChaosEffect {
public:
	Effect_FakeCrash() : ChaosEffect() {
		sName = "Fake Crash";
	}

	void InitFunction() override {
		if (rand() % 100 > 50) {
			Sleep(5000);
		}
		else {
			MessageBoxA(nullptr, "Debug Error!\n\nProgram: speed.exe\n\nR6025\n- pure virtual function call\n\n(Press Retry to debug the application)", "Microsoft Visual C++ Debug Library", MB_ICONERROR | MB_ABORTRETRYIGNORE);
			Sleep(2000);
		}
	}
} E_FakeCrash;