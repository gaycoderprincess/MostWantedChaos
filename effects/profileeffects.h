class Effect_EnableMomentCam : public ChaosEffect {
public:
	Effect_EnableMomentCam() : ChaosEffect() {
		sName = "Enable Jump Camera";
	}

	void InitFunction() override {
		FEDatabase->mUserProfile->TheOptionsSettings.TheGameplaySettings.JumpCam = true;
	}
	bool IsAvailable() override {
		return !FEDatabase->mUserProfile->TheOptionsSettings.TheGameplaySettings.JumpCam;
	}
	bool IsConditionallyAvailable() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
	bool HasTimer() override { return true; }
} E_EnableMomentCam;

class Effect_NoAutosave : public ChaosEffect {
public:
	Effect_NoAutosave() : ChaosEffect() {
		sName = "Disable Autosave";
		fTimerLength = 240;
	}

	void TickFunction(double delta) override {
		FEDatabase->mUserProfile->TheOptionsSettings.TheGameplaySettings.AutoSaveOn = false;
	}
	void DeinitFunction() override {
		FEDatabase->mUserProfile->TheOptionsSettings.TheGameplaySettings.AutoSaveOn = true;
	}
	bool IsAvailable() override {
		return FEDatabase->mUserProfile->TheOptionsSettings.TheGameplaySettings.AutoSaveOn;
	}
	bool IsConditionallyAvailable() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
	bool HasTimer() override { return true; }
} E_NoAutosave;

class Effect_Millionaire : public ChaosEffect {
public:
	Effect_Millionaire() : ChaosEffect() {
		sName = "Millionaire";
	}

	void InitFunction() override {
		FEDatabase->mUserProfile->TheCareerSettings.CurrentCash += 1000000;
	}
} E_Millionaire;

class Effect_Millionaire2 : public ChaosEffect {
public:
	Effect_Millionaire2() : ChaosEffect() {
		sName = "Anti-Millionaire";
	}

	void InitFunction() override {
		FEDatabase->mUserProfile->TheCareerSettings.CurrentCash -= 1000000;
	}
} E_Millionaire2;

class Effect_ManualTrans : public ChaosEffect {
public:
	Effect_ManualTrans() : ChaosEffect() {
		sName = "Force Manual Transmission";
		fTimerLength = 60;
		IncompatibilityGroup = Attrib::StringHash32("transmission");
	}

	static bool __thiscall IsAutomaticShiftHooked(IInput* pThis) {
		return false;
	}

	void TickFunction(double delta) override {
		NyaHookLib::Patch(0x8AC6B0, &IsAutomaticShiftHooked); // normal races
		NyaHookLib::Patch(0x8AC748, &IsAutomaticShiftHooked); // drag races
	}
	void DeinitFunction() override {
		NyaHookLib::Patch(0x8AC6B0, 0x68D090);
		NyaHookLib::Patch(0x8AC748, 0x68D090);
	}
	bool IsAvailable() override {
		return GetLocalPlayerInterface<IInput>()->IsAutomaticShift();
	}
	bool IsConditionallyAvailable() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
	bool HasTimer() override { return true; }
} E_ManualTrans;

class Effect_AutoTrans : public ChaosEffect {
public:
	Effect_AutoTrans() : ChaosEffect() {
		sName = "Force Automatic Transmission";
		fTimerLength = 60;
		IncompatibilityGroup = Attrib::StringHash32("transmission");
	}

	static bool __thiscall IsAutomaticShiftHooked(IInput* pThis) {
		return true;
	}

	void TickFunction(double delta) override {
		NyaHookLib::Patch(0x8AC6B0, &IsAutomaticShiftHooked); // normal races
		NyaHookLib::Patch(0x8AC748, &IsAutomaticShiftHooked); // drag races
	}
	void DeinitFunction() override {
		NyaHookLib::Patch(0x8AC6B0, 0x68D090);
		NyaHookLib::Patch(0x8AC748, 0x68D090);
	}
	bool IsAvailable() override {
		return !GetLocalPlayerInterface<IInput>()->IsAutomaticShift();
	}
	bool IsConditionallyAvailable() override { return true; }
	bool HasTimer() override { return true; }
} E_AutoTrans;