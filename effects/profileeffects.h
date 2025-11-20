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
		return !FEDatabase->mUserProfile->TheOptionsSettings.TheGameplaySettings.AutoSaveOn;
	}
	bool IsConditionallyAvailable() override { return true; }
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