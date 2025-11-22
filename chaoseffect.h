class ChaosEffectInstance;
class ChaosEffect {
public:
	const char* sName;
	const char* sFriendlyName = nullptr;
	double fTimerLength = 15;
	double fUnhideTime = 3;
	uint32_t IncompatibilityGroup = 0;
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

	virtual void InitFunction() {};
	virtual void TickFunction(double delta) {};
	virtual void DeinitFunction() {};
	virtual bool HasTimer() { return false; };
	virtual bool IsAvailable() { return true; };
	virtual bool IsConditionallyAvailable() { return false; };
	virtual bool IsRehideable() { return false; };
	virtual bool AbortOnConditionFailed() { return false; };
	virtual bool RunInMenus() { return false; }
	virtual bool InfiniteTimer() { return false; }
	virtual bool ShouldAbort() { return false; }
};

float fEffectX = 0.95;
float fEffectY = 0.4;
float fEffectSize = 0.04;
float fEffectSpacing = 0.045;

class ChaosEffectInstance {
public:
	ChaosEffect* pEffect = nullptr;
	const char* sNameToDisplay = nullptr;
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
		if (sNameToDisplay) return sNameToDisplay;
		return pEffect->sName;
	}

	float GetOffscreenPercentage() const {
		if (fTextTimer < 1) return std::lerp(1, 0, easeInOutQuart(fTextTimer));
		return 0;
	}

	bool IsHidden() const {
		return pEffect->IsConditionallyAvailable() && pEffect->IsRehideable() && !pEffect->IsAvailable();
	}

	void Draw(float y) const {
		if (!IsNameOnScreen()) return;

		auto x = fEffectX;
		x = 1 - x;
		x *= GetAspectRatioInv();
		x = 1 - x;

		std::string str;
		if (HasTimer()) {
			// todo does this look fine on the left? should there be a bar instead or something? graphics designers???? woof??????
			str = std::format("({}) {}", (int)fTimer, GetName());
		}
		else {
			str = GetName();
		}

		auto width = GetStringWidth(fEffectSize, str.c_str());

		if (fTextTimer < 1) x = std::lerp(1 + width, x, easeInOutQuart(fTextTimer));

		tNyaStringData data;
		data.x = x;
		data.y = fEffectY + (fEffectSpacing * y);
		data.size = fEffectSize;
		data.XRightAlign = true;
		data.outlinea = 255;
		data.outlinedist = 0.025;
		DrawString(data, str);
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
		if (effect->DebugNeverPick) continue;
		if (effect->bTriggeredThisCycle) continue;
		if (IsEffectRunning(effect)) continue;
		if (IsEffectRunningFromGroup(effect->IncompatibilityGroup)) continue;
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
	return availableEffects[rand()%availableEffects.size()];
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
#include "effects/spawneffects.h"
#include "effects/effect_safehouse.h"
#include "effects/effect_leaktank.h"