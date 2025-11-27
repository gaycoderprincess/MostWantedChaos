namespace Achievements {
	std::string GetAchievementSavePath() {
		std::filesystem::create_directory("CwoeeChaos");
		return "CwoeeChaos/achievements.sav";
	}

	int nTotalProgression = 0;

	class CAchievement {
	public:
		const char* sIdentifier;
		const char* sName;
		const char* sDescription;
		int nProgress = 0;
		float fInternalProgress = 0;
		float fMaxInternalProgress = 0;
		bool bUnlocked = false;
		bool bHidden = false;
		void(*pTickFunction)(CAchievement*, double) = nullptr;

		CAchievement() = delete;
		CAchievement(const char* identifier, const char* name, const char* description, bool hidden = false) {
			sIdentifier = identifier;
			sName = name;
			sDescription = description;
			bHidden = hidden;
		}
	};

	std::vector<CAchievement*> gAchievements = {
		new CAchievement("WIN_RACE", "Starting Point", "Win a race"),
		new CAchievement("WIN_RACE_LEAKTANK", "Fuel Budgeting", "Win a race with Leaking Gas Tank"),
		new CAchievement("WIN_RACE_TRAFPIZZA", "Express Delivery", "Win a race with TRAFPIZZA"),
		new CAchievement("WIN_PINKSLIP", "Take Your Ride", "Win a pinkslip from an effect"),
		new CAchievement("MILLIONAIRE", "Multi-Millionaire", "Earn $2,000,000"),
		new CAchievement("ANTI_MILLIONAIRE", "Multi-Brokeionaire", "'Earn' -$2,000,000"),
		new CAchievement("LAPS_10", "Time Waster", "Up a race to 10 laps"),
		new CAchievement("LAPS_1", "Lap Glitch", "Lower a race to 1 lap"),
		new CAchievement("RESTART_LATE", "Trolled", "Have a race restart at over 90% complete"),
	};

	const float fSpriteBGX = 960;
	const float fSpriteBGY = 960;
	const float fSpriteFGX = 1187;
	const float fSpriteFGY = 953;
	const float fSpriteBGSX = 507;
	const float fSpriteFGSX = 202;
	const float fSpriteBGSY = 83;
	const float fSpriteFGSY = 159;
	const float fSpritePopTimerSpeed = 2;
	const float fSpritePopBalloonSize = 1.2;
	const float fSpriteExpandTimerSpeed = 1;
	const float fSpriteStayTimerSpeed = 1.0 / 6.0;
	const float fTextFGX = 740;
	const float fTextFGY = 946;
	const float fTextFGS = 0.035;
	const float fTextBGX = 740;
	const float fTextBGY = 976;
	const float fTextBGS = 0.025;

	double fSpritePopTimer = 0;
	double fSpriteExpandTimer = 0;
	double fSpriteStayTimer = 1;
	double fSpriteHideTimer = 1;
	std::string sTextTitle;
	std::string sTextDesc;
	bool bHiddenAchievementUnlocked = false;

	bool DrawAchievementSprite(float left, float right, float top, float bottom, NyaDrawing::CNyaRGBA32 rgb, TEXTURE_TYPE* texture) {
		left -= 960;
		right -= 960;
		left /= 1920.0;
		right /= 1920.0;
		top /= 1080.0;
		bottom /= 1080.0;

		// aspect correction
		left *= 16.0 / 9.0;
		right *= 16.0 / 9.0;
		left /= GetAspectRatio();
		right /= GetAspectRatio();

		// recenter
		left += 0.5;
		right += 0.5;
		return DrawRectangle(left, right, top, bottom, rgb, 0, texture);
	}

	void DrawAchievementString(tNyaStringData data, const std::string& text) {
		data.x -= 960;
		data.x /= 1920.0;
		data.y /= 1080.0;

		// aspect correction
		data.x *= 16.0 / 9.0;
		data.x /= GetAspectRatio();

		// recenter
		data.x += 0.5;
		return DrawString(data, text);
	}

	void Save() {
		auto file = std::ofstream(GetAchievementSavePath(), std::ios::out | std::ios::binary);
		if (!file.is_open()) return;

		for (auto& achievement : gAchievements) {
			if (!achievement->bUnlocked && achievement->fInternalProgress == 0.0) continue;

			WriteStringToFile(file, achievement->sIdentifier);
			file.write((char*)&achievement->fInternalProgress, sizeof(achievement->fInternalProgress));
			file.write((char*)&achievement->bUnlocked, sizeof(achievement->bUnlocked));
		}
	}

	void Delete() {
		if (!std::filesystem::exists(GetAchievementSavePath())) return;
		std::filesystem::remove(GetAchievementSavePath());
	}

	void Clear() {
		for (auto& achievement : gAchievements) {
			achievement->nProgress = 0;
			achievement->fInternalProgress = 0;
			achievement->bUnlocked = false;
		}
	}

	std::vector<CAchievement*> aUnlockBuffer;
	auto gFailAchievement = CAchievement("ERROR", "ERROR", "ERROR", 0);

	CAchievement* GetAchievement(const std::string& identifier) {
		for (auto& achievement : gAchievements) {
			if (achievement->sIdentifier == identifier) return achievement;
		}

		static std::string failDesc;
		failDesc = "Failed to find achievement " + identifier;
		gFailAchievement.sDescription = failDesc.c_str();
		aUnlockBuffer.push_back(&gFailAchievement);
		return nullptr;
	}

	void AwardAchievement(CAchievement* achievement) {
		if (!achievement) return;

		if (achievement->bUnlocked) return;
		achievement->bUnlocked = true;

		Save();

		aUnlockBuffer.push_back(achievement);
	}

	void Load() {
		for (auto& achievement : gAchievements) {
			achievement->nProgress = 0;
			achievement->fInternalProgress = 0;
			achievement->bUnlocked = false;
		}

		auto file = std::ifstream(GetAchievementSavePath(), std::ios::in | std::ios::binary);
		if (!file.is_open()) return;

		auto identifier = ReadStringFromFile(file);
		while (!identifier.empty()) {
			if (auto achievement = GetAchievement(identifier)) {
				file.read((char*)&achievement->fInternalProgress, sizeof(achievement->fInternalProgress));
				file.read((char*)&achievement->bUnlocked, sizeof(achievement->bUnlocked));
			}
			else {
				file.read((char*)&gFailAchievement.fInternalProgress, sizeof(gFailAchievement.fInternalProgress));
				file.read((char*)&gFailAchievement.bUnlocked, sizeof(gFailAchievement.bUnlocked));
			}
			identifier = ReadStringFromFile(file);
		}
	}

	// from easing-functions by nicolausYes
	double easeInOutQuart(double t) {
		if (t < 0.5) {
			t *= t;
			return 8 * t * t;
		} else {
			t = (--t) * t;
			return 1 - 8 * t * t;
		}
	}

	int GetNumUnlockedAchievements() {
		int count = 0;
		for (auto& achievement : gAchievements) {
			if (achievement->bHidden) continue;
			if (achievement->bUnlocked) count++;
		}
		return count;
	}

	int GetNumUnlockedHiddenAchievements() {
		int count = 0;
		for (auto& achievement : gAchievements) {
			if (!achievement->bHidden) continue;
			if (achievement->bUnlocked) count++;
		}
		return count;
	}

	int GetNumVisibleAchievements() {
		int count = 0;
		for (auto& achievement : gAchievements) {
			if (achievement->bHidden) continue;
			count++;
		}
		return count;
	}

	int GetNumHiddenAchievements() {
		int count = 0;
		for (auto& achievement : gAchievements) {
			if (!achievement->bHidden) continue;
			count++;
		}
		return count;
	}

	void DrawUnlockUI() {
		if (fSpriteHideTimer >= 1 && fSpritePopTimer <= 0) {
			if (aUnlockBuffer.empty()) return;

			auto achievement = aUnlockBuffer[0];
			fSpritePopTimer = 0;
			fSpriteExpandTimer = 0;
			fSpriteStayTimer = 0;
			fSpriteHideTimer = 0;
			sTextTitle = achievement->sName;
			sTextDesc = achievement->sDescription;
			bHiddenAchievementUnlocked = achievement->bHidden;
			aUnlockBuffer.erase(aUnlockBuffer.begin());

			static auto sound = NyaAudio::LoadFile("CwoeeChaos/data/sound/achievement/unlock.mp3");
			if (sound) {
				NyaAudio::SetVolume(sound, FEDatabase->mUserProfile->TheOptionsSettings.TheAudioSettings.SoundEffectsVol);
				NyaAudio::Play(sound);
			}
		}

		static CNyaTimer timer;
		float delta = 0;
		bool isExpanding = false;
		// stage 4 - remove icon
		if (fSpriteHideTimer >= 1) {
			//if (fSpritePopTimer > 0.5) fSpritePopTimer = 0.5;
			fSpritePopTimer -= timer.Process() * fSpritePopTimerSpeed;
			delta = fSpritePopTimer;
		}
		// stage 3 - hide
		else if (fSpriteStayTimer >= 1) {
			fSpriteHideTimer += timer.Process() * fSpriteExpandTimerSpeed;
			delta = 1.0 - fSpriteHideTimer;
			isExpanding = true;
		}
		// stage 3 - stay for a few seconds
		else if (fSpriteExpandTimer >= 1) {
			fSpriteStayTimer += timer.Process() * fSpriteStayTimerSpeed;
			delta = 1.0;
			isExpanding = true;
		}
		// stage 2 - expand to size
		else if (fSpritePopTimer >= 1) {
			fSpriteExpandTimer += timer.Process() * fSpriteExpandTimerSpeed;
			delta = fSpriteExpandTimer;
			isExpanding = true;
		}
		// stage 1 - pop and deflate
		else {
			fSpritePopTimer += timer.Process() * fSpritePopTimerSpeed;
			delta = fSpritePopTimer;
		}

		static auto bgTex = LoadTexture("CwoeeChaos/data/textures/achievement/bg.png");
		static auto bg2Tex = LoadTexture("CwoeeChaos/data/textures/achievement/flowersponk.png");
		static auto fgTex = LoadTexture("CwoeeChaos/data/textures/achievement/flowers.png");

		if (delta <= 0) delta = 0;
		if (delta >= 1) delta = 1;

		if (isExpanding) {
			float bgLeft = std::lerp(fSpriteBGX + (fSpriteBGSX * 0.5), fSpriteBGX - (fSpriteBGSX * 0.5), easeInOutQuart(delta));
			DrawAchievementSprite(bgLeft, fSpriteBGX + (fSpriteBGSX * 0.5), fSpriteBGY - (fSpriteBGSY * 0.5), fSpriteBGY + (fSpriteBGSY * 0.5), {255,255,255,255}, bgTex);
			float textAlpha = (delta - 0.9) * 10;
			delta = 1;

			// if we've expanded further than the flower's extents, start drawing the background
			if (bgLeft < fSpriteFGX - (fSpriteFGSX * 0.5)) {
				DrawAchievementSprite(fSpriteBGX - (fSpriteBGSX * 0.5), fSpriteBGX + (fSpriteBGSX * 0.5), fSpriteBGY - (fSpriteBGSY * 0.5), fSpriteBGY + (fSpriteBGSY * 0.5), {255,255,255,255}, bg2Tex);
			}

			if (textAlpha > 0) {
				if (fSpriteStayTimer > 2.0 / 3.0 && aUnlockBuffer.empty()) {
					if (bHiddenAchievementUnlocked) {
						sTextTitle = "Hidden achievement progress";
						sTextDesc = std::format("{}/{}", GetNumUnlockedHiddenAchievements(), GetNumHiddenAchievements());
					}
					else {
						sTextTitle = "Achievement progress";
						sTextDesc = std::format("{}/{}", GetNumUnlockedAchievements(), GetNumVisibleAchievements());
					}
				}

				tNyaStringData data;
				data.a = data.outlinea = 255 * textAlpha;
				data.x = fTextFGX;
				data.y = fTextFGY;
				data.size = fTextFGS;
				data.outlinedist = 0.025;
				DrawAchievementString(data, sTextTitle);
				data.x = fTextBGX;
				data.y = fTextBGY;
				data.size = fTextBGS;
				DrawAchievementString(data, sTextDesc);
			}
		}

		float fgSize = 1;
		if (delta <= 0.5) {
			float newDelta = delta * 2;
			fgSize = std::lerp(0, fSpritePopBalloonSize, easeInOutQuart(newDelta));
		}
		else {
			float newDelta = (delta - 0.5) * 2;
			fgSize = std::lerp(fSpritePopBalloonSize, 1, easeInOutQuart(newDelta));
		}
		fgSize *= 0.5;

		DrawAchievementSprite(fSpriteFGX - (fSpriteFGSX * fgSize), fSpriteFGX + (fSpriteFGSX * fgSize), fSpriteFGY - (fSpriteFGSY * fgSize), fSpriteFGY + (fSpriteFGSY * fgSize), {255,255,255,255}, fgTex);
	}

	void OnTick() {
		nTotalProgression = ((double)GetNumUnlockedAchievements() / (double)GetNumVisibleAchievements()) * 100;

		static CNyaTimer gTimer;
		gTimer.Process();

		for (auto achievement: gAchievements) {
			if (achievement->pTickFunction) {
				achievement->pTickFunction(achievement, gTimer.fDeltaTime);
			}

			if (achievement->fMaxInternalProgress > 0) {
				achievement->nProgress = (achievement->fInternalProgress / achievement->fMaxInternalProgress) * 100;
				if (achievement->fInternalProgress >= (achievement->fMaxInternalProgress - 0.001)) {
					AwardAchievement(achievement);
				}
			}
		}

		//if (pLoadingScreen) return;

		DrawUnlockUI();
	}

	ChloeHook Init([]() {
		Load();
		aDrawingLoopFunctionsOnce.push_back([](){ NyaAudio::Init(ghWnd); });
		aDrawingLoopFunctions.push_back(OnTick);
	});
}

Achievements::CAchievement* GetAchievement(const std::string& identifier) {
	return Achievements::GetAchievement(identifier);
}