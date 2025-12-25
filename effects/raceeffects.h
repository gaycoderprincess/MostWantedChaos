#define EFFECT_CATEGORY_TEMP "Race"

void SetRaceNumLaps(int lapCount) {
	auto race = GRaceStatus::fObj;
	if (!race) return;
	if (race->mPlayMode == GRaceStatus::kPlayMode_Roaming) return;
	if (!GRaceParameters::GetIsLoopingRace(race->mRaceParms)) return;
	if (GRaceParameters::GetIsPursuitRace(race->mRaceParms)) return;

	if (auto index = race->mRaceParms->mIndex) {
		index->mNumLaps = lapCount;
	}

	auto pLaps = (uint32_t*)Attrib::Instance::GetAttributePointer(race->mRaceParms->mRaceRecord, Attrib::StringHash32("NumLaps"), 0);
	if (pLaps) *pLaps = lapCount;
	if (auto parent = race->mRaceParms->mRaceRecord->mCollection->mParent) {
		pLaps = (uint32_t*)Attrib::Collection::GetData(parent, Attrib::StringHash32("NumLaps"), 0);
		if (pLaps) *pLaps = lapCount;
	}
}

int GetLocalPlayerCurrentLap() {
	return GRaceStatus::fObj->mRacerInfo[0].mLapsCompleted;
}

/*class Effect_RemoveLapProgress : public ChaosEffect {
public:
	Effect_RemoveLapProgress() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Revert Progress By A Lap";
	}

	void InitFunction() override {
		GRaceStatus::fObj->mRacerInfo[0].mLapsCompleted--;
	}
	bool IsAvailable() override {
		auto laps = GetRaceNumLaps();
		if (!laps || *laps < 2) return false;
		return GRaceStatus::fObj->mRacerInfo[0].mLapsCompleted > 0;
	}
	bool IsConditionallyAvailable() override { return true; }
} E_RemoveLapProgress;*/

class Effect_RemoveLap : public ChaosEffect {
public:
	Effect_RemoveLap() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Remove A Lap";
	}

	void InitFunction() override {
		auto laps = *GetRaceNumLaps();
		laps--;
		if (laps <= 1) {
			laps = 1;
			Achievements::AwardAchievement(GetAchievement("LAPS_1"));
		}
		SetRaceNumLaps(laps);
		aMainLoopFunctionsOnce.push_back([]() { ERestartRace::Create(); });
	}
	bool IsAvailable() override {
		auto laps = GetRaceNumLaps();
		if (!laps || *laps < 2) return false;
		if (!ChaosVoting::IsEnabled() || ChaosVoting::bSelectingEffectsForVote) {
			return GetLocalPlayerCurrentLap() <= 0;
		}
		return true;
	}
	bool AbortOnConditionFailed() override { return true; }
} E_RemoveLap;

class Effect_AddLap : public ChaosEffect {
public:
	Effect_AddLap() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Add A Lap";
	}

	void InitFunction() override {
		auto laps = *GetRaceNumLaps();
		laps++;
		if (laps >= 10) {
			laps = 10;
			Achievements::AwardAchievement(GetAchievement("LAPS_10"));
		}
		SetRaceNumLaps(laps);
		aMainLoopFunctionsOnce.push_back([]() { ERestartRace::Create(); });
	}
	bool IsAvailable() override {
		auto laps = GetRaceNumLaps();
		if (!laps || *laps >= 10) return false;
		if (!ChaosVoting::IsEnabled() || ChaosVoting::bSelectingEffectsForVote) {
			return GetLocalPlayerCurrentLap() <= 0;
		}
		return true;
	}
	bool AbortOnConditionFailed() override { return true; }
} E_AddLap;

class Effect_Add3Laps : public ChaosEffect {
public:
	Effect_Add3Laps() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Add 3 Laps";
	}

	void InitFunction() override {
		auto laps = *GetRaceNumLaps();
		laps += 3;
		if (laps >= 10) {
			laps = 10;
			Achievements::AwardAchievement(GetAchievement("LAPS_10"));
		}
		SetRaceNumLaps(laps);
		aMainLoopFunctionsOnce.push_back([]() { ERestartRace::Create(); });
	}
	bool IsAvailable() override {
		auto laps = GetRaceNumLaps();
		if (!laps || *laps >= 10) return false;
		if (!ChaosVoting::IsEnabled() || ChaosVoting::bSelectingEffectsForVote) {
			return GetLocalPlayerCurrentLap() <= 0;
		}
		return true;
	}
	bool AbortOnConditionFailed() override { return true; }
} E_Add3Laps;

class Effect_RestartRace : public EffectBase_InAnyRaceConditional {
public:
	Effect_RestartRace() : EffectBase_InAnyRaceConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Restart Event";
	}

	void InitFunction() override {
		if (GRaceStatus::fObj->mRacerInfo[0].mPctRaceComplete >= 90) {
			Achievements::AwardAchievement(GetAchievement("RESTART_LATE"));
		}
		aMainLoopFunctionsOnce.push_back([]() { ERestartRace::Create(); });
	}
	bool AbortOnConditionFailed() override { return true; }
} E_RestartRace;

class Effect_DisableBarriers : public EffectBase_InRaceConditional {
public:
	Effect_DisableBarriers() : EffectBase_InRaceConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Disable Race Barriers";
		fTimerLength = 120;
	}

	void TickFunctionMain(double delta) override {
		GRaceStatus::DisableBarriers();
	}
	void DeinitFunction() override {
		if (IsInNormalRace()) {
			GRaceStatus::EnableBarriers(GRaceStatus::fObj);
		}
	}
	bool HasTimer() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
} E_DisableBarriers;

class Effect_FalseStarts : public ChaosEffect {
public:
	Effect_FalseStarts() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "False Starts";
		fTimerLength = 120;
	}

	void TickFunctionMain(double delta) override {
		auto cars = GetActiveVehicles();
		for (auto& car : cars) {
			if (car->IsStaging()) car->SetStaging(false);
		}
	}
	bool HasTimer() override { return true; }
	bool IsAvailable() override {
		if (!IsInNormalRace()) return false;
		//if (EffectInstance && !EffectInstance->bFirstFrame && GRaceStatus::fObj->mRacerInfo[0].mPctRaceComplete > 5.0) {
		//	return false;
		//}
		return true;
	}
	bool IsRehideable() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
} E_FalseStarts;

class Effect_RestartRaceOn99 : public ChaosEffect {
public:
	bool active = false;

	Effect_RestartRaceOn99() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Exit To Menu At 99% Completion";
	}

	void InitFunction() override {
		active = false;
	}
	void TickFunctionMain(double delta) override {
		if (!active) {
			EffectInstance->fTimer = fTimerLength;
			if ((IsInNormalRace() && GRaceStatus::fObj->mRacerInfo[0].mPctRaceComplete >= 99) || (cFrontendDatabase::IsFinalEpicChase(FEDatabase) && IsPlayerApproachingOldBridge())) {
				aMainLoopFunctionsOnce.push_back([]() { EQuitToFE::Create(GARAGETYPE_MAIN_FE, "MainMenu.fng"); });
				//aMainLoopFunctionsOnce.push_back([]() { ERestartRace::Create(); });
				active = true;
			}
		}
	}
	bool HideFromPlayer() override {
		return !active;
	}
	bool IsAvailable() override { return IsInNormalRace() || cFrontendDatabase::IsFinalEpicChase(FEDatabase); }
	bool AbortOnConditionFailed() override { return true; }
	bool RunInMenus() override { return active; }
	bool CanQuickTrigger() override { return false; }
} E_RestartRaceOn99;

class Effect_SuddenDeath : public ChaosEffect {
public:
	Effect_SuddenDeath() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Sudden Death";
		sFriendlyName = "Blow Engine On Position Loss";
		fTimerLength = 60;
		MakeIncompatibleWithFilterGroup("player_godmode");
	}

	int ranking = 0;
	bool abort = false;
	double fLeewayTimer = 0;

	void InitFunction() override {
		ranking = GRaceStatus::fObj->mRacerInfo[0].mRanking;
		abort = false;
		fLeewayTimer = 0;
	}
	void TickFunctionMain(double delta) override {
		if (GetLocalPlayerVehicle()->IsStaging()) {
			ranking = GRaceStatus::fObj->mRacerCount;
		}

		tNyaStringData data;
		data.x = 0.5;
		data.y = 0.85;
		data.size = 0.04;
		data.XCenterAlign = true;
		data.outlinea = 255;
		data.outlinedist = 0.025;
		DrawString(data, std::format("Stay in #{}{}!", ranking, ranking == 1 ? "" : " or better"));

		auto currRanking = GRaceStatus::fObj->mRacerInfo[0].mRanking;
		if (currRanking < ranking) {
			ranking = currRanking;
		}
		if (currRanking > ranking) {
			data.y += data.size;
			data.SetColor(200,0,0,255);
			DrawString(data, std::format("Time Remaining: {:.1f}", 1.0 - fLeewayTimer));

			fLeewayTimer += delta;
			if (fLeewayTimer > 1.0) {
				GetLocalPlayerInterface<IDamageable>()->Destroy();
				abort = true;
			}
		}
		else {
			fLeewayTimer = 0;
		}
	}
	bool HasTimer() override { return true; }
	bool IsAvailable() override {
		if (!IsInNormalRace() || GetActiveVehicles(DRIVER_RACER).empty()) {
			ranking = 999;
			return false;
		}
		return true;
	}
	bool IsRehideable() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
	bool ShouldAbort() override { return abort; }
	bool CanQuickTrigger() override { return false; }
} E_SuddenDeath;

class Effect_MidnightClub : public EffectBase_InRaceConditional {
public:
	Effect_MidnightClub() : EffectBase_InRaceConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Midnight Club Mode";
		fTimerLength = 240;
	}

	const static inline float fCylinderRotX = 90;
	const static inline float fCylinderRotY = 0;
	const static inline float fCylinderRotZ = 0;
	const static inline float fCylinderSizeX = 3;
	const static inline float fCylinderSizeY = 50;
	const static inline float fCylinderSizeZ = 3;

	const static inline float fConeRotX = 90;
	const static inline float fConeRotY = 0;
	const static inline float fConeRotZ = 0;
	const static inline float fConeSize = 4;
	const static inline float fConeMoveY = 1;

	const static inline float fPlayerArrowRotX = 90;
	const static inline float fPlayerArrowRotY = 0;
	const static inline float fPlayerArrowRotZ = 90;
	const static inline float fPlayerArrowSize = 0.6;
	const static inline float fPlayerArrowMoveFwd = 3.7;
	const static inline float fPlayerArrowMoveY = 1;

	const static inline float fFinishRotX = 90;
	const static inline float fFinishRotY = 0;
	const static inline float fFinishRotZ = 0;
	const static inline float fFinishSize = 2.5;
	const static inline float fFinishMoveY = 2;

	void RenderPlayerArrow(NyaVec3 player, NyaVec3 fwd, NyaVec3 next) {
		Render3D::nVertexColorValue = 0xFF808080;

		static auto models = Render3D::CreateModels("mc/arrow.fbx");
		if (models.empty() || models[0]->bInvalidated) {
			models = Render3D::CreateModels("mc/arrow.fbx");
		}

		auto dir = player - next;
		dir.y = 0;
		dir.Normalize();

		for (auto& mdl : models) {
			auto mat = NyaMat4x4::LookAt(dir);
			mat.x *= fPlayerArrowSize;
			mat.y *= fPlayerArrowSize;
			mat.z *= fPlayerArrowSize;
			mat.p = player;
			mat.p += fwd * fPlayerArrowMoveFwd;
			mat.p.y += fPlayerArrowMoveY;

			UMath::Matrix4 rotation;
			rotation.Rotate(NyaVec3(fPlayerArrowRotX * 0.01745329, fPlayerArrowRotY * 0.01745329, fPlayerArrowRotZ * 0.01745329));
			mat = (UMath::Matrix4)(mat * rotation);

			mat = (UMath::Matrix4)WorldToRenderMatrix(mat);
			mdl->RenderAt(mat, false);
		}

		Render3D::nVertexColorValue = Render3D::nDefaultVertexColor;
	}

	void RenderCheckpointCone(NyaVec3 cp, NyaVec3 next) {
		Render3D::nVertexColorValue = 0xFF404040;

		static auto models = Render3D::CreateModels("mc/cone_fwd_new.fbx");
		if (models.empty() || models[0]->bInvalidated) {
			models = Render3D::CreateModels("mc/cone_fwd_new.fbx");
		}

		auto dir = cp - next;
		dir.Normalize();

		for (auto& mdl : models) {
			auto mat = NyaMat4x4::LookAt(dir);
			mat.x *= fConeSize;
			mat.y *= fConeSize;
			mat.z *= fConeSize;
			mat.p = cp;
			WCollisionMgr::GetWorldHeightAtPointRigorous((UMath::Vector3*)&mat.p, &mat.p.y, nullptr);
			mat.p.y += fConeMoveY;

			UMath::Matrix4 rotation;
			rotation.Rotate(NyaVec3(fConeRotX * 0.01745329, fConeRotY * 0.01745329, fConeRotZ * 0.01745329));
			mat = (UMath::Matrix4)(mat * rotation);

			mat = (UMath::Matrix4)WorldToRenderMatrix(mat);
			mdl->RenderAt(mat, false);
		}

		Render3D::nVertexColorValue = Render3D::nDefaultVertexColor;
	}

	void RenderCheckpointFinish(NyaVec3 fwd, NyaVec3 cp) {
		Render3D::nVertexColorValue = 0xFF404040;

		static auto models = Render3D::CreateModels("mc/finish.fbx");
		if (models.empty() || models[0]->bInvalidated) {
			models = Render3D::CreateModels("mc/finish.fbx");
		}

		for (auto& mdl : models) {
			auto mat = NyaMat4x4::LookAt(fwd);
			mat.x *= fFinishSize;
			mat.y *= fFinishSize;
			mat.z *= fFinishSize;
			mat.p = cp;
			WCollisionMgr::GetWorldHeightAtPointRigorous((UMath::Vector3*)&mat.p, &mat.p.y, nullptr);
			mat.p.y += fFinishMoveY;

			UMath::Matrix4 rotation;
			rotation.Rotate(NyaVec3(fFinishRotX * 0.01745329, fFinishRotY * 0.01745329, fFinishRotZ * 0.01745329));
			mat = (UMath::Matrix4)(mat * rotation);

			mat = (UMath::Matrix4)WorldToRenderMatrix(mat);
			mdl->RenderAt(mat, false);
		}

		Render3D::nVertexColorValue = Render3D::nDefaultVertexColor;
	}

	void RenderCheckpointCylinder(NyaVec3 cp) {
		Render3D::nVertexColorValue = 0xD0808080;

		static auto models = Render3D::CreateModels("mc/cylinder.fbx");
		if (models.empty() || models[0]->bInvalidated) {
			models = Render3D::CreateModels("mc/cylinder.fbx");
		}

		for (auto& mdl : models) {
			auto mat = UMath::Matrix4::kIdentity;
			mat.x *= fCylinderSizeX;
			mat.y *= fCylinderSizeY;
			mat.z *= fCylinderSizeZ;
			mat.p = cp;
			WCollisionMgr::GetWorldHeightAtPointRigorous((UMath::Vector3*)&mat.p, &mat.p.y, nullptr);
			mat.p.y -= 1;

			UMath::Matrix4 rotation;
			rotation.Rotate(NyaVec3(fCylinderRotX * 0.01745329, fCylinderRotY * 0.01745329, fCylinderRotZ * 0.01745329));
			mat = (UMath::Matrix4)(mat * rotation);

			mat = (UMath::Matrix4)WorldToRenderMatrix(mat);
			mdl->RenderAt(mat, true, EEFFECT_WORLD, false);
		}

		Render3D::nVertexColorValue = Render3D::nDefaultVertexColor;
	}

	void TickFunctionMain(double delta) override {
		GRaceStatus::DisableBarriers();
	}
	void TickFunction(eChaosHook hook, double delta) override {
		if (hook != HOOK_3D) return;

		auto race = GRaceStatus::fObj;
		if (!race) return;
		if (!race->mNextCheckpoint) return;

		auto cp = race->mNextCheckpoint;
		int checkpointId = 0;
		for (int i = 0; i < race->mCheckpoints.size(); i++) {
			if (cp == race->mCheckpoints[i]) checkpointId = i;
		}

		Render3D::sTextureSubdir = "mc/";

		auto cam = GetLocalPlayerCamera()->CurrentKey.Matrix.Invert();
		RenderCheckpointCylinder(cp->mWorldTrigger.fPosRadius);
		if (checkpointId < race->mCheckpoints.size() - 1) {
			RenderCheckpointCone(cp->mWorldTrigger.fPosRadius, race->mCheckpoints[checkpointId+1]->mWorldTrigger.fPosRadius);
		}
		else {
			RenderCheckpointFinish(RenderToWorldCoords(cam.z), cp->mWorldTrigger.fPosRadius);
		}

		if (GetLocalPlayer()->GetHud()->IsHudVisible()) {
			RenderPlayerArrow(RenderToWorldCoords(cam.p), RenderToWorldCoords(cam.z), cp->mWorldTrigger.fPosRadius);
		}

		Render3D::sTextureSubdir = "";
	}
	void DeinitFunction() override {
		if (IsInNormalRace()) {
			GRaceStatus::EnableBarriers(GRaceStatus::fObj);
		}
	}
	bool HasTimer() override { return true; }
	bool IsRehideable() override { return true; }
	bool CanQuickTrigger() override { return false; }
} E_MidnightClub;