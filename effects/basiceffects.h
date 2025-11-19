class Effect_Blind : public ChaosEffect {
public:
	Effect_Blind() : ChaosEffect() {
		sName = "Blind";
		fTimerLength = 10;
	}

	virtual void TickFunction() {
		DrawRectangle(0, 1, 0, 1, {0,0,0,255});
	}
} E_Blind;

class Effect_PortraitMode : public ChaosEffect {
public:
	Effect_PortraitMode() : ChaosEffect() {
		sName = "Portrait Mode";
	}

	virtual void TickFunction() {
		DrawRectangle(0, 0.25, 0, 1, {0,0,0,255});
		DrawRectangle(0.75, 1, 0, 1, {0,0,0,255});
	}
	virtual bool RunInMenus() { return true; }
} E_PortraitMode;

class Effect_WidescreenMode : public ChaosEffect {
public:
	Effect_WidescreenMode() : ChaosEffect() {
		sName = "Widescreen Borders";
	}

	virtual void TickFunction() {
		DrawRectangle(0, 1, 0, 0.1, {0,0,0,255});
		DrawRectangle(0, 1, 0.9, 1, {0,0,0,255});
	}
	virtual bool RunInMenus() { return true; }
} E_WidescreenMode;