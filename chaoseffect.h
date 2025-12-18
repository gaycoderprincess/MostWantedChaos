class ChaosEffectInstance;
class ChaosEffect {
public:
	const char* sName = nullptr;
	const char* sFriendlyName = nullptr;
	double fTimerLength = 15;
	double fUnhideTime = 3;
	std::vector<uint32_t> IncompatibilityGroups;
	std::vector<uint32_t> ActivateIncompatibilityGroups; // checks IncompatibilityGroups but doesn't deactivate for effects that only share ActivateIncompatibilityGroups
	const char* sListCategory = nullptr;
	const char* sAuthor = "gaycoderprincess"; // in case anyone else contributes or helps meaningfully! :3

	std::time_t LastTriggerTime = 0;
	uint32_t nTotalTimesActivated = 0;
	bool bTriggeredThisCycle = false;
	ChaosEffectInstance* EffectInstance;

	static inline std::vector<ChaosEffect*> aEffects;

	//ChaosEffect(const std::string& name, double length) : sName(name), fTimerLength(length) {
	ChaosEffect(const char* category, bool neverPick = false) {
		sListCategory = category;
		if (!neverPick) {
			aEffects.push_back(this);
		}
	};

	const char* GetFriendlyName() const {
		if (sFriendlyName) return sFriendlyName;
		return sName;
	}

	enum eChaosHook {
		HOOK_GAMETICK,
		HOOK_CAMERA,
		HOOK_3D,
		HOOK_PRE3D,
		HOOK_POST3D,
		HOOK_PREPROPS,
		HOOK_POSTPROPS,
	};

	virtual void InitFunction() {}
	virtual void TickFunction(eChaosHook hook, double delta) {}
	virtual void TickFunctionMain(double delta) {}
	virtual void DeinitFunction() {}
	virtual bool HasTimer() { return false; }
	virtual bool IsAvailable() { return true; }
	virtual bool IsRehideable() { return false; }
	virtual bool HideFromPlayer() { return false; }
	virtual bool AbortOnConditionFailed() { return false; }
	virtual bool RunInMenus() { return false; } // frontend specifically
	virtual bool RunWhenBlocked() { return false; } // pause menu, race end screen, etc.
	virtual bool InfiniteTimer() { return false; }
	virtual bool ShouldAbort() { return false; }
	virtual bool IgnoreHUDState() { return false; }
	virtual bool CanQuickTrigger() { return true; } // activate 3 effects and such
	virtual bool CanMultiTrigger() { return false; } // multiple instances at once
	virtual void OnAnyEffectTriggered() {}
};

bool bDarkMode = false;

float fEffectX = 0.98;
float fEffectY = 0.4;
float fEffectSize = 0.04;
float fEffectSpacing = 0.045;

float fEffectTextureXSpacing = 0.03;
float fEffectTextureXSpacingNoTimer = 0.0;
float fEffectTextureYSpacing = 0.021;
float fEffectTextureTipX = 0.021;
float fEffectArcX = -0.003;
float fEffectArcSize = 0.008;
float fEffectArcThickness = 0.017;
float fEffectArcRotation = -1.5;
float fEffectTimerTextSize = 0.03;

float fEffectVotingSize = 0.75;

class ChaosEffectInstance {
public:
	ChaosEffect* pEffect = nullptr;
	std::string sNameToDisplay;
	double fTimer = 0;
	double fTextTimer = 0;
	double fTimeConditionMet = 3;
	bool bFirstFrame = true;
	bool bAborted = false;

	bool bIsVotingDummy = false;
	int nVotingDummyVoteCount = 0;
	int nVotingDummyVoteCountStreamer = 0;
	float fVotingDummyVotePercentage = 0;

	ChaosEffectInstance(ChaosEffect* effect) : pEffect(effect) {
		fTimer = pEffect->fTimerLength;
		fTimeConditionMet = pEffect->fUnhideTime;
	}

	int GetVotingDummyVoteCount() {
		return nVotingDummyVoteCount + (nVotingDummyVoteCountStreamer * 9999);
	}

	bool IsActive() const {
		return fTimer > 0 && !bAborted;
	}

	bool IsNameOnScreen() const {
		return fTextTimer > 0;
	}

	bool ShouldNameBeOnScreen() const {
		if (!IsActive()) return false;
		if (bFirstFrame) return false;
		if (IsHidden()) return false;
		if (pEffect->HideFromPlayer()) return false;
		return true;
	}

	bool CanBeDeleted() const {
		return !IsActive() && !IsNameOnScreen();
	}

	bool HasTimer() const {
		if (bIsVotingDummy) return false;
		return pEffect && pEffect->HasTimer();
	}

	const char* GetName() const {
		if (!sNameToDisplay.empty()) return sNameToDisplay.c_str();
		return pEffect->sName;
	}

	float GetOffscreenPercentage() const {
		if (fTextTimer < 1) return std::lerp(1, 0, easeInOutQuart(fTextTimer));
		return 0;
	}

	bool IsHidden() const {
		return pEffect->IsRehideable() && !pEffect->IsAvailable();
	}

	void UpdatePopup(double delta, bool shouldBeOnScreen) {
		if (shouldBeOnScreen) {
			if (fTextTimer < 1) {
				fTextTimer += delta * 2;
			}
			else {
				fTextTimer = 1;
			}
		}
		else {
			if (fTextTimer > 0) {
				fTextTimer -= delta * 2;
			}
			else {
				fTextTimer = 0;
			}
		}
	}

	void Draw(float y, bool inMenu) const {
		if (!IsNameOnScreen() && !inMenu) return;
		if (!IsActive() && inMenu) return;

		float effectSize = fEffectSize;
		float effectSpacing = fEffectSpacing;
		float effectTextureYSpacing = fEffectTextureYSpacing;
		if (bIsVotingDummy) {
			effectSize *= fEffectVotingSize;
			effectSpacing *= fEffectVotingSize;
			effectTextureYSpacing *= fEffectVotingSize;
		}

		auto x = fEffectX;
		x = 1 - x;
		x *= GetAspectRatioInv();
		x = 1 - x;
		y = fEffectY + (effectSpacing * y);

		std::string str = GetName();
		if (bIsVotingDummy && fVotingDummyVotePercentage >= 0) str += std::format(" ({}%)", (int)fVotingDummyVotePercentage);

		auto width = GetStringWidth(effectSize, str.c_str());
		float barWidth = ((HasTimer() && !inMenu ? fEffectTextureXSpacing : fEffectTextureXSpacingNoTimer) * GetAspectRatioInv());
		float barTipWidth = (fEffectTextureTipX * GetAspectRatioInv());

		if (fTextTimer < 1 && !inMenu) x = std::lerp(1 + width + barWidth + barTipWidth, x, easeInOutQuart(fTextTimer));

		static auto textureBarL = LoadTexture("CwoeeChaos/data/textures/effectbg_bar.png");
		static auto textureTipL = LoadTexture("CwoeeChaos/data/textures/effectbg_end.png");
		static auto textureBarD = LoadTexture("CwoeeChaos/data/textures/effectbg_dark_bar.png");
		static auto textureTipD = LoadTexture("CwoeeChaos/data/textures/effectbg_dark_end.png");
		auto textureBar = bDarkMode ? textureBarD : textureBarL;
		auto textureTip = bDarkMode ? textureTipD : textureTipL;
		if (textureBar && textureTip) {
			float barX = x - width - barWidth;
			if (bIsVotingDummy) {
				DrawRectangle(1 - barX, 0, y - effectTextureYSpacing, y + effectTextureYSpacing, {255,255,255,255}, 0, textureBar);
				DrawRectangle(1 - (barX - barTipWidth), 1 - barX, y - effectTextureYSpacing, y + effectTextureYSpacing, {255,255,255,255}, 0, textureTip);
			}
			else {
				DrawRectangle(barX, 1, y - effectTextureYSpacing, y + effectTextureYSpacing, {255,255,255,255}, 0, textureBar);
				DrawRectangle(barX - barTipWidth, barX, y - effectTextureYSpacing, y + effectTextureYSpacing, {255,255,255,255}, 0, textureTip);
			}
			if (!inMenu && HasTimer() && fTimer > 0.1) {
				float arcX = barX - (fEffectArcX * GetAspectRatioInv());
				DrawArc(arcX, y, fEffectArcSize, fEffectArcThickness, fEffectArcRotation, fEffectArcRotation - ((fTimer / pEffect->fTimerLength) * std::numbers::pi * 2), {255,255,255,255});
				if (fTimer < 5 && pEffect->fTimerLength > 5) {
					tNyaStringData data;
					data.x = arcX;
					data.y = y;
					data.size = fEffectTimerTextSize;
					data.XCenterAlign = true;
					data.outlinea = 255;
					data.outlinedist = 0.025;
					int timer = ((int)fTimer) + 1;
					if (timer < 1) timer = 1;
					if (timer > 5) timer = 5;
					DrawString(data, std::to_string(timer));
				}
			}
		}
		//static auto texture = LoadTexture("CwoeeChaos/data/textures/effectbg.png");
		//if (texture) {
		//	DrawRectangle(x - width - (fEffectTextureXSpacing * GetAspectRatioInv()), 1, y - effectTextureYSpacing, y + effectTextureYSpacing, {255,255,255,255}, 0, texture);
		//}

		tNyaStringData data;
		data.x = x;
		data.y = y;
		data.size = effectSize;
		data.XRightAlign = true;
		if (bIsVotingDummy) {
			data.x = 1 - x;
			data.XRightAlign = false;
		}
		if (!bDarkMode) {
			data.outlinea = 255;
			data.outlinedist = 0.025;
		}
		DrawString(data, str);
	}

	void OnTick(ChaosEffect::eChaosHook hook, double delta) {
		if (IsHidden()) return;
		if (bFirstFrame) return;
		if (!IsActive()) return;

		pEffect->EffectInstance = this;
		pEffect->TickFunction(hook, delta);
		pEffect->EffectInstance = nullptr;
	}

	void OnTickMain(double delta, bool inMenu) {
		if (!pEffect) {
			fTimer -= delta;
			return;
		}

		if (!inMenu) {
			UpdatePopup(delta, ShouldNameBeOnScreen());
		}

		// conditional effects will show after 3 seconds of the conditions being met, or immediately if the condition was met on trigger
		if (bFirstFrame) {
			if (pEffect->IsAvailable()) {
				fTimeConditionMet += delta;
				if (fTimeConditionMet < pEffect->fUnhideTime) {
					return;
				}
			}
			else {
				fTimeConditionMet = 0;
				if (pEffect->AbortOnConditionFailed()) bAborted = true;
				return;
			}
		}

		if (IsHidden()) return;

		pEffect->EffectInstance = this;
		if (bFirstFrame) {
			pEffect->InitFunction();
			bFirstFrame = false;
		}

		if (!inMenu) {
			if (pEffect->ShouldAbort()) {
				if (!bAborted) {
					pEffect->DeinitFunction();
				}
				bAborted = true;
			}
			else if (fTimer > 0 && fTimer - delta <= 0) {
				pEffect->DeinitFunction();
			}

			if (!pEffect->InfiniteTimer()) {
				fTimer -= delta;
			}
		}
		if (IsActive()) {
			pEffect->TickFunctionMain(delta);
		}
		pEffect->EffectInstance = nullptr;
	}
};
static inline std::vector<ChaosEffectInstance> aRunningEffects;

void DoChaosSave() {
	std::ofstream file("CwoeeChaos/effects.sav", std::iostream::out | std::iostream::binary);
	if (!file.is_open()) return;

	int num = ChaosEffect::aEffects.size();
	file.write((char*)&num, sizeof(num));

	for (auto& effect : ChaosEffect::aEffects) {
		file.write((char*)&effect->bTriggeredThisCycle, sizeof(effect->bTriggeredThisCycle));
		file.write((char*)&effect->LastTriggerTime, sizeof(effect->LastTriggerTime));
		file.write((char*)&effect->nTotalTimesActivated, sizeof(effect->nTotalTimesActivated));
	}
}

void DoChaosLoad() {
	std::ifstream file("CwoeeChaos/effects.sav", std::iostream::in | std::iostream::binary);
	if (!file.is_open()) return;

	int num = 0;
	file.read((char*)&num, sizeof(num));
	if (num != ChaosEffect::aEffects.size()) return;

	for (auto& effect : ChaosEffect::aEffects) {
		file.read((char*)&effect->bTriggeredThisCycle, sizeof(effect->bTriggeredThisCycle));
		file.read((char*)&effect->LastTriggerTime, sizeof(effect->LastTriggerTime));
		file.read((char*)&effect->nTotalTimesActivated, sizeof(effect->nTotalTimesActivated));
	}
}

ChaosEffectInstance* GetEffectRunning(ChaosEffect* effect) {
	for (auto& running : aRunningEffects) {
		if (!running.IsActive()) continue;
		if (running.pEffect == effect) return &running;
	}
	return nullptr;
}

bool IsEffectRunningFromGroup(uint32_t IncompatibilityGroup, bool includeActivate) {
	if (!IncompatibilityGroup) return false;
	for (auto& running : aRunningEffects) {
		if (!running.IsActive()) continue;
		for (auto& group : running.pEffect->IncompatibilityGroups) {
			if (group == IncompatibilityGroup) return true;
		}
		if (includeActivate) {
			for (auto& group : running.pEffect->ActivateIncompatibilityGroups) {
				if (group == IncompatibilityGroup) return true;
			}
		}
	}
	return false;
}

void AddRunningEffect(ChaosEffect* effect) {
	if (!effect->CanMultiTrigger() && GetEffectRunning(effect)) return;

	effect->bTriggeredThisCycle = true;
	effect->LastTriggerTime = std::time(0);
	effect->nTotalTimesActivated++;
	aRunningEffects.push_back(ChaosEffectInstance(effect));
	WriteLog(std::format("Activating {}", effect->sName));

	DoChaosSave();

	for (auto& running : aRunningEffects) {
		running.pEffect->OnAnyEffectTriggered();
	}
}

int GetRandomNumber(int min, int max) {
	static std::random_device device;
	static std::mt19937 randGen(device());
	std::uniform_int_distribution<> distr(min, max-1);
	return distr(randGen);
}

bool CanEffectActivate(ChaosEffect* effect) {
	if (!effect->CanMultiTrigger() && GetEffectRunning(effect)) return false;
	for (auto& group : effect->IncompatibilityGroups) {
		if (IsEffectRunningFromGroup(group, true)) return false;
	}
	for (auto& group : effect->ActivateIncompatibilityGroups) {
		if (IsEffectRunningFromGroup(group, false)) return false;
	}
	if (effect->AbortOnConditionFailed()) {
		if (IsChaosBlocked()) return false; // IsAvailable can run in-game code, so always skip abortonconditionfailed effects in menus
		if (!effect->IsAvailable()) return false;
	}
	return true;
}

ChaosEffect* GetRandomEffect(bool quickTrigger = false) {
	std::vector<ChaosEffect*> availableEffects;
	for (auto& effect : ChaosEffect::aEffects) {
		if (effect->bTriggeredThisCycle) continue;
		//if (effect->fLastTriggerTime) // todo
		if (!CanEffectActivate(effect)) continue;
		if (quickTrigger && !effect->CanQuickTrigger()) continue;
		availableEffects.push_back(effect);
	}
	if (availableEffects.empty()) {
		for (auto& effect : ChaosEffect::aEffects) {
			effect->bTriggeredThisCycle = false;
		}
		return GetRandomEffect();
	}
	return availableEffects[GetRandomNumber(0, availableEffects.size())];
}

bool RunningEffectsCleanup() {
	for (auto& effect : aRunningEffects) {
		if (effect.CanBeDeleted()) {
			aRunningEffects.erase(aRunningEffects.begin() + (&effect - &aRunningEffects[0]));
			return true;
		}
	}
	return false;
}

#include "chaosvoting.h"

#include "effects/effectbases.h"
#include "effects/basiceffects.h"
#include "effects/shadereffects.h"
#include "effects/playercareffects.h"
#include "effects/opponentcareffects.h"
#include "effects/allcareffects.h"
#include "effects/trafficeffects.h"
#include "effects/raceeffects.h"
#include "effects/hudeffects.h"
#include "effects/pursuiteffects.h"
#include "effects/profileeffects.h"
#include "effects/texteffects.h"
#include "effects/worldeffects.h"
#include "effects/carmodeleffects.h"
#include "effects/physicseffects.h"
#include "effects/spawneffects.h"
#include "effects/cameraeffects.h"
#include "effects/rendereffects.h"
#include "effects/timeeffects.h"
#include "effects/votingeffects.h"
#include "effects/effect_safehouse.h"
#include "effects/effect_leaktank.h"
#include "effects/effect_flashbacks.h"
#include "effects/effect_qte.h"
#include "effects/effect_disableall.h"
#include "effects/effect_tiktok.h"