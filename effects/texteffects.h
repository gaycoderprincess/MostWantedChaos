#define EFFECT_CATEGORY_TEMP "Text"

// mostly the same as zolikas but replaces more things
class Effect_TransRights : public ChaosEffect {
public:
	Effect_TransRights() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Trans Rights";
		fTimerLength = 120;
		AddToIncompatiblityGroup("string_replace");
	}

	void InitFunction() override {
		TextHook::EnableSubtitles();
		SummonChyron("Trans Rights", "Trans Rights", "Trans Rights");
		TextHook::pReplaceText = "Trans Rights";
		NyaHookLib::Patch<uint8_t>(0x5DC86E, 0xEB); // remove duplicate racer name check
	}
	void TickFunctionMain(double delta) override {
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
	bool RunWhenBlocked() override { return true; }
} E_TransRights;

class Effect_JJ : public ChaosEffect {
public:
	Effect_JJ() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Shoutouts to JJ902";
		fTimerLength = 120;
		AddToIncompatiblityGroup("string_replace");
	}

	double timer = 0;
	NyaAudio::NyaSound sound[3] = {};

	void InitFunction() override {
		TextHook::EnableSubtitles();
		SummonChyron("Let That Sink In", "Let That Sink In", "Let That Sink In");
		TextHook::pReplaceText = "Let That Sink In";
		NyaHookLib::Patch<uint8_t>(0x5DC86E, 0xEB); // remove duplicate racer name check

		for (int i = 0; i < 3; i++) {
			if (!sound[i]) sound[i] = NyaAudio::LoadFile(std::format("CwoeeChaos/data/sound/effect/sink{}.mp3", i+1));
		}
	}
	void TickFunctionMain(double delta) override {
		Chyron::mArtist = "Let That Sink In";
		Chyron::mTitle = "Let That Sink In";
		Chyron::mAlbum = "Let That Sink In";

		timer += delta;
		if (timer > 2) {
			if (PercentageChanceCheck(25)) {
				int r = rand() % 3;
				if (sound[r]) {
					NyaAudio::SetVolume(sound[r], GetSFXVolume() * 1.5);
					NyaAudio::Play(sound[r]);
				}
			}
			timer -= 2;
		}
	}
	void DeinitFunction() override {
		TextHook::pReplaceText = nullptr;
		NyaHookLib::Patch<uint8_t>(0x5DC86E, 0x7E);
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
	bool RunWhenBlocked() override { return true; }
} E_JJ;

class Effect_ReverseText : public ChaosEffect {
public:
	Effect_ReverseText() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Reverse Text";
		fTimerLength = 120;
	}

	void TickFunctionMain(double delta) override {
		TextHook::EnableSubtitles();
		TextHook::bReverseText = true;
	}
	void DeinitFunction() override {
		TextHook::bReverseText = false;
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
	bool RunWhenBlocked() override { return true; }
} E_ReverseText;

class Effect_RandomText : public ChaosEffect {
public:
	Effect_RandomText() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Random Text";
		fTimerLength = 120;
		AddToIncompatiblityGroup("string_replace");
	}

	void InitFunction() override {
		TextHook::EnableSubtitles();
		TextHook::bRandomText = true;
	}
	void DeinitFunction() override {
		TextHook::bRandomText = false;
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
	bool RunWhenBlocked() override { return true; }
} E_RandomText;

class Effect_ShuffleText : public ChaosEffect {
public:
	Effect_ShuffleText() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Shuffled Letters";
		fTimerLength = 120;
	}

	void InitFunction() override {
		TextHook::EnableSubtitles();
		TextHook::bShuffledText = true;
	}
	void DeinitFunction() override {
		TextHook::bShuffledText = false;
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
	bool RunWhenBlocked() override { return true; }
} E_ShuffleText;

class Effect_InterspersedText : public ChaosEffect {
public:
	Effect_InterspersedText() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Cocksbot";
		fTimerLength = 120;
		nFrequency *= 4;
		bCanMultiTrigger = true;
	}

	void InitFunction() override {
		TextHook::EnableSubtitles();
		TextHook::nInterspersedTextRuns++;
		TextHook::pInterspersedText = "cock";
		TextHook::pInterspersedTextUpper = "COCK";
		TextHook::pInterspersedTextProper = "Cock";
	}
	void DeinitFunction() override {
		TextHook::nInterspersedTextRuns--;
		TextHook::pInterspersedText = nullptr;
		TextHook::pInterspersedTextUpper = nullptr;
		TextHook::pInterspersedTextProper = nullptr;
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
	bool RunWhenBlocked() override { return true; }
} E_InterspersedText;