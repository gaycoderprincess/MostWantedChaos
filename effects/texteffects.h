// mostly the same as zolikas but replaces more things
class Effect_TransRights : public ChaosEffect {
public:
	Effect_TransRights() : ChaosEffect() {
		sName = "Trans Rights";
		fTimerLength = 120;
		IncompatibilityGroups.push_back(Attrib::StringHash32("string_replace"));
	}

	void InitFunction() override {
		SummonChyron("Trans Rights", "Trans Rights", "Trans Rights");
		TextHook::pReplaceText = "Trans Rights";
		NyaHookLib::Patch<uint8_t>(0x5DC86E, 0xEB); // remove duplicate racer name check
	}
	void TickFunction(double delta) override {
		Chyron::mArtist = "Trans Rights";
		Chyron::mTitle = "Trans Rights";
		Chyron::mAlbum = "Trans Rights";
	}
	void DeinitFunction() override {
		TextHook::pReplaceText = nullptr;
		NyaHookLib::Patch<uint8_t>(0x5DC86E, 0x7E);
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
} E_TransRights;

class Effect_ReverseText : public ChaosEffect {
public:
	Effect_ReverseText() : ChaosEffect() {
		sName = "Reverse Text";
		fTimerLength = 120;
	}

	void TickFunction(double delta) override {
		TextHook::bReverseText = true;
	}
	void DeinitFunction() override {
		TextHook::bReverseText = false;
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
} E_ReverseText;

class Effect_RandomText : public ChaosEffect {
public:
	Effect_RandomText() : ChaosEffect() {
		sName = "Random Text";
		fTimerLength = 120;
		IncompatibilityGroups.push_back(Attrib::StringHash32("string_replace"));
	}

	void InitFunction() override {
		TextHook::bRandomText = true;
	}
	void DeinitFunction() override {
		TextHook::bRandomText = false;
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
} E_RandomText;