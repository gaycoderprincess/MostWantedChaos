class CHUD_DamageMeter : public CIngameHUDElement {
public:
	int nPlayerId = 0;

	float fHealthBarAlpha[32];
	float fHealthBarGlow[32];
	double fHealthBarGlowTimer[32];
	float fHealthBarGlowSpeed = 0.2;
	bool bHealthBarMatches[32];

	float GetScreenXOffset() { return 0; }
	float GetScreenYOffset() { return 0; }

	static constexpr float fHealthTextX = 0.027;
	static constexpr float fHealthTextY = 0.91;
	static constexpr float fHealthTextSize = 0.029;
	static constexpr float fHealthBarX = 0.0205;
	static constexpr float fHealthBarY = 0.941;
	static constexpr float fHealthBarSize = 0.0389;

	CNyaTimer gTimer;
	void ProcessHealthBarAlpha() {
		auto localPlayer = GetLocalPlayerVehicle();

		int closestPlayerId = -1;
		float closestPlayerDist = 9999;
		bool playerJustHit = false;
		auto playerPos = *localPlayer->GetPosition();
		for (int i = 0; i < GRaceStatus::fObj->mRacerCount; i++) {
			if (i == nPlayerId) continue;

			auto plyInfo = &GRaceStatus::fObj->mRacerInfo[i];
			auto ply = GetRacerFromHandle(plyInfo->mhSimable);
			if (!ply) continue;

			float damage = 1 - ply->mCOMObject->Find<IDamageable>()->GetHealth();
			if (plyInfo->mTotalled || plyInfo->mEngineBlown) damage = 1.0;
			if (bHealthBarMatches[i] = fHealthBarGlow[i] == damage) {
				fHealthBarGlowTimer[i] = 0;
			}
			else {
				if (fHealthBarGlowTimer[i] < 0.666) {
					fHealthBarGlowTimer[i] += gTimer.fDeltaTime;
				}
				else {
					if (fHealthBarGlow[i] < damage) {
						fHealthBarGlow[i] += fHealthBarGlowSpeed * gTimer.fDeltaTime;
						if (fHealthBarGlow[i] > damage) fHealthBarGlow[i] = damage;
					}
					if (fHealthBarGlow[i] > damage) {
						fHealthBarGlow[i] -= fHealthBarGlowSpeed * gTimer.fDeltaTime;
						if (fHealthBarGlow[i] < damage) fHealthBarGlow[i] = damage;
					}
				}
			}
			if (plyInfo->mTotalled || plyInfo->mEngineBlown) continue;

			auto dist = (*ply->GetPosition() - playerPos).length();
			if (dist < closestPlayerDist) {
				closestPlayerDist = dist;
				closestPlayerId = i;
			}
		}

		int targetAlpha = -1;
		if (closestPlayerDist < 15) targetAlpha = closestPlayerId;

		for (int i = 0; i < GRaceStatus::fObj->mRacerCount; i++) {
			// keep glowing health bars for a while longer
			if (targetAlpha == -1 && fHealthBarAlpha[i] == 1 && !bHealthBarMatches[i]) {

			}
			else if (targetAlpha == i) {
				fHealthBarAlpha[i] += gTimer.fDeltaTime;
				if (playerJustHit) fHealthBarAlpha[i] = 1;
			}
			else {
				fHealthBarAlpha[i] -= gTimer.fDeltaTime;
				if (playerJustHit) fHealthBarAlpha[i] = 0;
			}

			if (fHealthBarAlpha[i] < 0) fHealthBarAlpha[i] = 0;
			if (fHealthBarAlpha[i] > 1) fHealthBarAlpha[i] = 1;
		}
	}

	void Reset() override {
		memset(fHealthBarAlpha,0,sizeof(fHealthBarAlpha));
		memset(fHealthBarGlow,0,sizeof(fHealthBarGlow));
		memset(fHealthBarGlowTimer,0,sizeof(fHealthBarGlowTimer));
	}
	void Process() override {
		if (!IsRaceHUDUp()) return;
		if (!IsInRace()) return;

		float fXOffset = GetScreenXOffset();
		float fYOffset = GetScreenYOffset();
		int nXOffset = fXOffset * 1920;
		int nYOffset = fYOffset * 1080;

		static auto ai_damage_meter = LoadTexture("CwoeeChaos/data/global/overlay/ai_damage_meter.tga");
		static auto ai_damage_meter_glow = LoadTexture("CwoeeChaos/data/global/overlay/ai_damage_meter_glow.tga");
		static auto ai_damage_meter_bg = LoadTexture("CwoeeChaos/data/global/overlay/ai_damage_meter_bg.tga");

		ProcessHealthBarAlpha();

		for (int i = 0; i < GRaceStatus::fObj->mRacerCount; i++) {
			if (fHealthBarAlpha[i] <= 0.0) continue;

			int alpha = 255*fHealthBarAlpha[i];
			if (alpha < 0) alpha = 0;
			if (alpha > 255) alpha = 255;

			Draw1080pSprite(JUSTIFY_LEFT, 0+nXOffset, 1920+nXOffset, 0+nYOffset, 1080+nYOffset, {255,255,255,(uint8_t)alpha}, ai_damage_meter_bg);

			auto plyInfo = &GRaceStatus::fObj->mRacerInfo[i];
			auto ply = GetRacerFromHandle(plyInfo->mhSimable);
			if (!ply) continue;

			tNyaStringData data;
			data.x = fHealthTextX * GetAspectRatioInv() + fXOffset;
			data.y = fHealthTextY + fYOffset;
			data.size = fHealthTextSize;
			data.SetColor(GetPaletteColor(COLOR_INGAME_YELLOW));
			data.a = alpha;
			DrawStringFO2_Regular18(data, plyInfo->mName);

			float aspect = 402.0 / 42.0;

			auto damage = 1 - ply->mCOMObject->Find<IDamageable>()->GetHealth();
			float uv = 1 - damage;
			float uvGlow = 1 - fHealthBarGlow[i];
			float x1 = fHealthBarX;
			float y1 = fHealthBarY;
			float x2 = fHealthBarX + std::lerp(0, fHealthBarSize * aspect, 1 - damage);
			float x2Glow = fHealthBarX + std::lerp(0, fHealthBarSize * aspect, 1 - fHealthBarGlow[i]);
			float y2 = fHealthBarY + fHealthBarSize;
			if (x2 != x2Glow) DrawRectangle(x1 * GetAspectRatioInv() + fXOffset, x2Glow * GetAspectRatioInv() + fXOffset, y1+fYOffset, y2+fYOffset, {255,255,255,(uint8_t)alpha}, 0, ai_damage_meter_glow, 0, {0,0}, {uvGlow,1});
			DrawRectangle(x1 * GetAspectRatioInv() + fXOffset, x2 * GetAspectRatioInv() + fXOffset, y1+fYOffset, y2+fYOffset, {255,255,255,(uint8_t)alpha}, 0, ai_damage_meter, 0, {0,0}, {uv,1});
		}

		gTimer.Process();
	}
} HUD_DamageMeter;