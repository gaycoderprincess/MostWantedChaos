class ChaosEffectInstance;
class ChaosEffect {
public:
	const char* sName;
	double fTimerLength = 30;
	double fLastTriggerTime = 99999;
	bool bTriggeredThisCycle = false;
	uint32_t IncompatibilityGroup = 0;
	ChaosEffectInstance* EffectInstance;

	static inline std::vector<ChaosEffect*> aEffects;

	//ChaosEffect(const std::string& name, double length) : sName(name), fTimerLength(length) {
	ChaosEffect() {
		aEffects.push_back(this);
	};

	virtual void InitFunction() {};
	virtual void TickFunction(double delta) {};
	virtual void DeinitFunction() {};
	virtual bool HasTimer() { return false; };
	virtual bool IsAvailable() { return true; };
	virtual bool IsConditionallyAvailable() { return false; };
	virtual bool AbortOnConditionFailed() { return false; };
	virtual bool RunInMenus() { return false; }
};

float fEffectX = 0.95;
float fEffectY = 0.4;
float fEffectSize = 0.04;
float fEffectSpacing = 0.045;

class ChaosEffectInstance {
public:
	ChaosEffect* pEffect = nullptr;
	const char* sNameToDisplay = nullptr;
	double fActiveTimer = 0;
	double fTimer = 0;
	double fTimeConditionMet = 3;
	bool bFirstFrame = true;
	bool bAborted = false;

	ChaosEffectInstance(ChaosEffect* effect) : pEffect(effect) {
		fTimer = pEffect->fTimerLength;
	}

	bool IsActive() const {
		return fTimer > 0 && !bAborted;
	}

	bool HasTimer() const {
		return pEffect && pEffect->HasTimer();
	}

	const char* GetName() const {
		if (sNameToDisplay) return sNameToDisplay;
		return pEffect->sName;
	}

	float GetOffscreenPercentage() const {
		if (fActiveTimer < 0.5) return std::lerp(1, 0, easeInOutQuart(fActiveTimer * 2));
		else if (fTimer < 0.5) return std::lerp(1, 0, easeInOutQuart(fTimer * 2));
		return 0;
	}

	void Draw(float y) const {
		if (!IsActive()) return;
		if (bFirstFrame) return;

		auto x = fEffectX;
		x = 1 - x;
		x *= GetAspectRatioInv();
		x = 1 - x;

		std::string str = GetName();
		if (HasTimer()) {
			str += std::format(" ({})", (int)fTimer);
		}

		auto width = GetStringWidth(fEffectSize, str.c_str());

		if (fActiveTimer < 0.5) x = std::lerp(1 + width, x, easeInOutQuart(fActiveTimer * 2));
		else if (fTimer < 0.5) x = std::lerp(1 + width, x, easeInOutQuart(fTimer * 2));
		//else if (HasTimer()) {
		//}

		tNyaStringData data;
		data.x = x;
		data.y = fEffectY + (fEffectSpacing * y);
		data.size = fEffectSize;
		data.XRightAlign = true;
		data.outlinea = 255;
		DrawString(data, str);
	}

	void OnTick(double delta, bool inMenu) {
		if (!pEffect) {
			if (!inMenu) {
				fTimer -= delta;
				fActiveTimer += delta;
			}
			return;
		}

		// conditional effects will show after 3 seconds of the conditions being met, or immediately if the condition was met on trigger
		if (bFirstFrame && pEffect->IsConditionallyAvailable()) {
			if (pEffect->IsAvailable()) {
				fTimeConditionMet += delta;
				if (fTimeConditionMet < 3) {
					return;
				}
			}
			else {
				fTimeConditionMet = 0;
				if (pEffect->AbortOnConditionFailed()) bAborted = true;
				return;
			}
		}

		pEffect->EffectInstance = this;
		if (bFirstFrame) {
			pEffect->InitFunction();
			bFirstFrame = false;
		}

		if (!inMenu) {
			if (fTimer > 0 && fTimer - delta <= 0) {
				pEffect->DeinitFunction();
			}
			fTimer -= delta;
			fActiveTimer += delta;
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
}

bool IsEffectRunning(ChaosEffect* effect) {
	for (auto& running : aRunningEffects) {
		if (!running.IsActive()) continue;
		if (running.pEffect == effect) return true;
	}
	return false;
}

bool IsEffectRunningFromGroup(uint32_t IncompatibilityGroup) {
	if (!IncompatibilityGroup) return false;
	for (auto& running : aRunningEffects) {
		if (!running.IsActive()) continue;
		if (running.pEffect->IncompatibilityGroup == IncompatibilityGroup) return true;
	}
	return false;
}

ChaosEffect* GetRandomEffect() {
	std::vector<ChaosEffect*> availableEffects;
	for (auto& effect : ChaosEffect::aEffects) {
		if (effect->bTriggeredThisCycle) continue;
		if (IsEffectRunning(effect)) continue;
		if (IsEffectRunningFromGroup(effect->IncompatibilityGroup)) continue;
		//if (effect->IsConditionallyAvailable() && !effect->IsAvailable()) continue;
		//if (effect->fLastTriggerTime) // todo
		availableEffects.push_back(effect);
	}
	if (availableEffects.empty()) {
		for (auto& effect : ChaosEffect::aEffects) {
			effect->bTriggeredThisCycle = false;
		}
		return GetRandomEffect();
	}
	return availableEffects[rand()%availableEffects.size()];
}

bool RunningEffectsCleanup() {
	for (auto& effect : aRunningEffects) {
		if (!effect.IsActive()) {
			aRunningEffects.erase(aRunningEffects.begin() + (&effect - &aRunningEffects[0]));
			return true;
		}
	}
	return false;
}

#include "effects/effectbases.h"
#include "effects/basiceffects.h"
#include "effects/playercareffects.h"
#include "effects/opponentcareffects.h"
#include "effects/allcareffects.h"
#include "effects/raceeffects.h"
#include "effects/hudeffects.h"
#include "effects/pursuiteffects.h"
#include "effects/profileeffects.h"
#include "effects/texteffects.h"
#include "effects/worldeffects.h"
#include "effects/carmodeleffects.h"
#include "effects/physicseffects.h"
#include "effects/effect_wreckonflip.h"
#include "effects/effect_safehouse.h"