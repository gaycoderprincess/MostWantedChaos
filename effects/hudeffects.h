#define EFFECT_CATEGORY_TEMP "HUD"

class Effect_DragHUD : public ChaosEffect {
public:
	Effect_DragHUD() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Drag HUD";
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayer()) {
			ply->SetHud(PHT_DRAG);
		}
	}
} E_DragHUD;

class Effect_ChallengeCam : public ChaosEffect {
public:
	Effect_ChallengeCam() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Challenge Series Camera";
	}

	void InitFunction() override {
		Camera_SetGenericCamera("Cinematics", "Challenge");
	}
} E_Challenge;

class Effect_NoHUD : public ChaosEffect {
public:
	Effect_NoHUD() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Disable HUD";
		sFriendlyName = "Disable Game HUD";
		fTimerLength = 60;
		IncompatibilityGroups.push_back(Attrib::StringHash32("hud_replace"));
	}

	static uint32_t __thiscall DetermineHudFeaturesHooked(IHud* pThis, IPlayer* a2) {
		return 0;
	}

	void TickFunction(double delta) override {
		NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x58CA50, &DetermineHudFeaturesHooked);
	}
	void DeinitFunction() override {
		NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x58CA50, 0x57CA60);
	}
	bool HasTimer() override { return true; }
} E_NoHUD;

class Effect_PauseMenu : public ChaosEffect {
public:
	Effect_PauseMenu() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Open Pause Menu";
	}

	void InitFunction() override {
		aMainLoopFunctionsOnce.push_back([]() { EPause::Create(0, 0, 0); });
	}
} E_PauseMenu;

class Effect_FO1HUD : public ChaosEffect {
public:
	Effect_FO1HUD() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "FlatOut Style HUD";
		fTimerLength = 120;
		IncompatibilityGroups.push_back(Attrib::StringHash32("hud_replace"));
	}

	static uint32_t __thiscall DetermineHudFeaturesHooked(IHud* pThis, IPlayer* a2) {
		//auto flags = 0x39FF5C12; // freeroam hud features, no race overlays

		auto flags = IHud::DetermineHudFeatures(pThis, a2);
		// remove RaceInformation
		flags &= ~0x4000000;
		// remove LeaderBoard
		flags &= ~8;
		flags &= ~0x10;
		// remove MilestoneBoard
		//flags &= ~4;
		// remove PursuitBoard
		flags &= ~0x100000;
		// remove CostToState
		flags &= ~0x1000;
		// remove Minimap
		//flags &= ~0x10000;
		return flags;
	}

	void InitFunction() override {
		FlatOutHUD::NewGameHud::OnReset();
	}
	void TickFunction(double delta) override {
		NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x58CA50, &DetermineHudFeaturesHooked);
		FlatOutHUD::NewGameHud::OnHUDTick();
		NyaHookLib::Patch<uint8_t>(0x57CFA8, 0xEB); // remove MilestoneBoard
	}
	void DeinitFunction() override {
		NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x58CA50, 0x57CA60);
		NyaHookLib::Patch<uint8_t>(0x57CFA8, 0x74);
	}
	bool HasTimer() override { return true; }
	bool IsAvailable() override {
		return IsInNormalRace() || (GetLocalPlayerInterface<IPerpetrator>() && GetLocalPlayerInterface<IPerpetrator>()->IsBeingPursued());
	}
	bool IsRehideable() override { return true; }
	bool IsConditionallyAvailable() override { return true; }
} E_FO1HUD;