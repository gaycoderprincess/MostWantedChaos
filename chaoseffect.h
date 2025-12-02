class ChaosEffectInstance;
class ChaosEffect {
public:
	const char* sName;
	const char* sFriendlyName = nullptr;
	double fTimerLength = 15;
	double fUnhideTime = 3;
	std::vector<uint32_t> IncompatibilityGroups;
	std::vector<uint32_t> ActivateIncompatibilityGroups; // checks IncompatibilityGroups but doesn't deactivate for effects that only share ActivateIncompatibilityGroups
	bool DebugNeverPick = false;

	double fLastTriggerTime = 99999;
	bool bTriggeredThisCycle = false;
	ChaosEffectInstance* EffectInstance;

	static inline std::vector<ChaosEffect*> aEffects;

	//ChaosEffect(const std::string& name, double length) : sName(name), fTimerLength(length) {
	ChaosEffect() {
		aEffects.push_back(this);
	};

	const char* GetFriendlyName() const {
		if (sFriendlyName) return sFriendlyName;
		return sName;
	}

	virtual void InitFunction() {}
	virtual void TickFunction(double delta) {}
	virtual void TickFunctionCamera(Camera* pCamera, double delta) {}
	virtual void DeinitFunction() {}
	virtual bool HasTimer() { return false; };
	virtual bool IsAvailable() { return true; };
	virtual bool IsConditionallyAvailable() { return false; };
	virtual bool IsRehideable() { return false; };
	virtual bool AbortOnConditionFailed() { return false; };
	virtual bool RunInMenus() { return false; }
	virtual bool InfiniteTimer() { return false; }
	virtual bool ShouldAbort() { return false; }
	virtual bool IgnoreHUDState() { return false; }
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

class ChaosEffectInstance {
public:
	ChaosEffect* pEffect = nullptr;
	std::string sNameToDisplay;
	double fTimer = 0;
	double fTextTimer = 0;
	double fTimeConditionMet = 3;
	bool bFirstFrame = true;
	bool bAborted = false;

	ChaosEffectInstance(ChaosEffect* effect) : pEffect(effect) {
		fTimer = pEffect->fTimerLength;
		fTimeConditionMet = pEffect->fUnhideTime;
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
		return true;
	}

	bool CanBeDeleted() const {
		return !IsActive() && !IsNameOnScreen();
	}

	bool HasTimer() const {
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
		return pEffect->IsConditionallyAvailable() && pEffect->IsRehideable() && !pEffect->IsAvailable();
	}

	void Draw(float y, bool inMenu) const {
		if (!IsNameOnScreen() && !inMenu) return;

		auto x = fEffectX;
		x = 1 - x;
		x *= GetAspectRatioInv();
		x = 1 - x;
		y = fEffectY + (fEffectSpacing * y);

		std::string str = GetName();

		auto width = GetStringWidth(fEffectSize, str.c_str());
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
			DrawRectangle(barX, 1, y - fEffectTextureYSpacing, y + fEffectTextureYSpacing, {255,255,255,255}, 0, textureBar);
			DrawRectangle(barX - barTipWidth, barX, y - fEffectTextureYSpacing, y + fEffectTextureYSpacing, {255,255,255,255}, 0, textureTip);
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
		//	DrawRectangle(x - width - (fEffectTextureXSpacing * GetAspectRatioInv()), 1, y - fEffectTextureYSpacing, y + fEffectTextureYSpacing, {255,255,255,255}, 0, texture);
		//}

		tNyaStringData data;
		data.x = x;
		data.y = y;
		data.size = fEffectSize;
		data.XRightAlign = true;
		if (!bDarkMode) {
			data.outlinea = 255;
			data.outlinedist = 0.025;
		}
		DrawString(data, str);
	}

	void OnTickCamera(Camera* pCamera, double delta) {
		if (IsHidden()) return;
		if (bFirstFrame) return;
		if (!IsActive()) return;

		pEffect->EffectInstance = this;
		pEffect->TickFunctionCamera(pCamera, delta);
		pEffect->EffectInstance = nullptr;
	}

	void OnTick(double delta, bool inMenu) {
		if (!pEffect) {
			fTimer -= delta;
			return;
		}

		if (!inMenu) {
			if (ShouldNameBeOnScreen()) {
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

		// conditional effects will show after 3 seconds of the conditions being met, or immediately if the condition was met on trigger
		if (bFirstFrame && pEffect->IsConditionallyAvailable()) {
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
			pEffect->TickFunction(delta);
		}
		pEffect->EffectInstance = nullptr;
	}
};
static inline std::vector<ChaosEffectInstance> aRunningEffects;

void AddRunningEffect(ChaosEffect* effect) {
	effect->bTriggeredThisCycle = true;
	effect->fLastTriggerTime = 0;
	aRunningEffects.push_back(ChaosEffectInstance(effect));
	WriteLog(std::format("Activating {}", effect->sName));

	for (auto& running : aRunningEffects) {
		running.pEffect->OnAnyEffectTriggered();
	}
}

bool IsEffectRunning(ChaosEffect* effect) {
	for (auto& running : aRunningEffects) {
		if (!running.IsActive()) continue;
		if (running.pEffect == effect) return true;
	}
	return false;
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

int GetRandomNumber(int min, int max) {
	static std::random_device device;
	static std::mt19937 randGen(device());
	std::uniform_int_distribution<> distr(min, max-1);
	return distr(randGen);
}

ChaosEffect* GetRandomEffect() {
	std::vector<ChaosEffect*> availableEffects;
	for (auto& effect : ChaosEffect::aEffects) {
		if (effect->DebugNeverPick) continue;
		if (effect->bTriggeredThisCycle) continue;
		if (IsEffectRunning(effect)) continue;
		bool incompatible = false;
		for (auto& group : effect->IncompatibilityGroups) {
			if (IsEffectRunningFromGroup(group, true)) incompatible = true;
		}
		for (auto& group : effect->ActivateIncompatibilityGroups) {
			if (IsEffectRunningFromGroup(group, false)) incompatible = true;
		}
		if (incompatible) continue;
		if (effect->IsConditionallyAvailable() && effect->AbortOnConditionFailed() && !effect->IsAvailable()) continue;
		//if (effect->fLastTriggerTime) // todo
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

#include <numbers>

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
#include "effects/effect_safehouse.h"
#include "effects/effect_leaktank.h"
#include "effects/effect_flashbacks.h"