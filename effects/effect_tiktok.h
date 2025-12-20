class Effect_TikTok : public EffectBase_ScreenShader {
public:
	Effect_TikTok() : EffectBase_ScreenShader("Uncategorized") {
		sName = "Tik Tok Mode";
		fTimerLength = 30;
		sFileName = "portrait";
		AddToIncompatiblityGroup("gamespeed");
		AddToIncompatiblityGroup("speedbreaker");
	}

	NyaAudio::NyaSound sound = 0;

	static inline std::vector<std::string> aSentences = {
		"Major Skill Issue",
		"Did you know that in Need for Speed: Most Wanted you can be cock?",
		"Sergeant Cross will impound you car even though you can pay your fines.",
	};
	std::vector<std::string> aWords;
	int currentWord = 0;
	double wordTimer = 0;
	double wordAppearTimer = 0;

	static inline float fWordFrequency = 0.25;
	static inline float fWordTimerSpeed = 1.5;

	void InitFunction() override {
		EffectBase_ScreenShader::InitFunction();
		aWords = TextHook::SplitStringIntoWords("Top 10 facts about Need for Speed: Most Wanted");
		currentWord = 0;
		wordTimer = 0;
		wordAppearTimer = 0;

		static bool bOnce = true;
		if (bOnce) {
			for (int i = 0; i < NumStringRecords; i++) {
				if (!RecordTable[i].PackedString) continue;
				if (strlen(RecordTable[i].PackedString) < 48) continue;
				bool valid = true;
				std::string str = RecordTable[i].PackedString;
				for (uint8_t c : str) {
					if (c == '%') valid = false;
					if (c == '\n') valid = false;
					if (c == '^') valid = false;
					if (c == 0xA0) valid = false;
				}
				if (!valid) continue;
				if (str.find("Are you sure") != std::string::npos) continue;
				if (str.find("Do you wish") != std::string::npos) continue;
				if (str.find("unused") != std::string::npos) continue;
				if (str.find("Unused") != std::string::npos) continue;
				aSentences.push_back(str);
			}
			bOnce = false;
		}
	}
	void TickFunctionMain(double delta) override {
		if (!sound) sound = NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/candyland.mp3");
		if (aWords.empty()) {
			aWords = TextHook::SplitStringIntoWords(aSentences[rand()%aSentences.size()]);
		}

		aShadersToDraw.push_back(pShader);

		if (auto ply = GetLocalPlayer()) {
			ply->ResetGameBreaker(false);
		}
		GameSpeedModifier = 2;

		if (sound && NyaAudio::IsFinishedPlaying(sound)) {
			NyaAudio::SetVolume(sound, FEDatabase->mUserProfile->TheOptionsSettings.TheAudioSettings.SoundEffectsVol);
			NyaAudio::Play(sound);
		}

		wordTimer += delta;
		while (wordTimer > fWordFrequency) {
			currentWord++;
			wordAppearTimer = 0;
			if (currentWord >= aWords.size()) {
				aWords = TextHook::SplitStringIntoWords(aSentences[rand()%aSentences.size()]);
				wordTimer = -0.5;
				currentWord = 0;
			}
			else {
				wordTimer -= fWordFrequency;
			}
		}

		if (wordTimer < 0) return;
		wordAppearTimer += delta * fWordTimerSpeed;
		if (wordAppearTimer >= 0) {
			tNyaStringData data;
			data.x = 0.5;
			data.y = 0.8;
			data.size = 0.05;
			if (wordAppearTimer < 0.25) data.size += (0.25 - wordAppearTimer) * 0.05;
			data.outlinea = 255;
			data.outlinedist = 0.025;
			data.XCenterAlign = true;
			DrawString(data, aWords[currentWord]);
		}
	}
	void DeinitFunction() override {
		GameSpeedModifier = 1;
		if (sound && !NyaAudio::IsFinishedPlaying(sound)) {
			NyaAudio::Stop(sound);
		}
	}
	bool RunInMenus() override { return false; }
	bool RunWhenBlocked() override { return false; }
} E_TikTok;