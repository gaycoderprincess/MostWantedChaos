#define EFFECT_CATEGORY_TEMP "Unfair"

// thank you zchaos for this horrible, horrible idea
class Effect_DisableSave : public ChaosEffect {
public:
	Effect_DisableSave() : ChaosEffect(EFFECT_CATEGORY_TEMP, true) {
		sName = "Disable Saving Until Next Profile Reload";
	}
	
	void InitFunction() override {
		strcpy_s(FEDatabase->mUserProfile->m_aProfileName, 32, "NUL");
	}
} E_DisableSave;

class Effect_GetBustedInstant : public ChaosEffect {
public:
	Effect_GetBustedInstant() : ChaosEffect(EFFECT_CATEGORY_TEMP, true) {
		sName = "Get Busted";
	}

	void InitFunction() override {
		NISListenerActivity::MessageBusted(nullptr, 0);
	}
} E_GetBustedInstant;