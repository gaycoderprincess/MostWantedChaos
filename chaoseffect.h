class ChaosEffectInstance;
class ChaosEffect {
public:
	const char* sName = nullptr;
	const char* sFriendlyName = nullptr;
	double fTimerLength = 15;
	double fUnhideTime = 3;
	std::vector<uint32_t> FilterGroups;
	std::vector<uint32_t> IncompatibleFilterGroups; // effects that contain these in FilterGroups are incompatible with this
	const char* sListCategory = nullptr;
	const char* sAuthor = "gaycoderprincess"; // in case anyone else contributes or helps meaningfully! :3
	int nFrequency = 10; // 10 is standard, make this higher or lower to make an effect more or less likely to appear

	std::time_t LastTriggerTime = 0;
	uint32_t nTotalTimesActivated = 0;
	bool bTriggeredThisCycle = false;
	bool bTriggeredThisCycleSmart = false;
	ChaosEffectInstance* EffectInstance = nullptr;

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

	void AddToFilterGroup(const std::string& group) { FilterGroups.push_back(Attrib::StringHash32(group.c_str())); }
	void MakeIncompatibleWithFilterGroup(const std::string& group) { IncompatibleFilterGroups.push_back(Attrib::StringHash32(group.c_str())); }
	void AddToIncompatiblityGroup(const std::string& group) {
		FilterGroups.push_back(Attrib::StringHash32(group.c_str()));
		IncompatibleFilterGroups.push_back(Attrib::StringHash32(group.c_str()));
	}

	enum eChaosHook {
		HOOK_GAMETICK,
		HOOK_CAMERA,
		HOOK_3D,
		HOOK_PRE3D,
		HOOK_POST3D,
		HOOK_PREPROPS,
		HOOK_POSTPROPS,
		HOOK_INPUT,
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
	virtual bool ShouldAbort() { return false; }
	virtual bool IgnoreHUDState() { return false; } // display even when disable chaos hud is active
	virtual bool CanQuickTrigger() { return true; } // activate 3 effects and such
	virtual bool CanMultiTrigger() { return false; } // multiple instances at once
	virtual bool InitImmediately() { return false; }
	virtual void OnAnyEffectTriggered() {}
	virtual void OnTimerRefill() {}
};

class ChaosEffectInstance {
public:
	ChaosEffect* pEffect = nullptr;
	std::string sNameToDisplay;
	double fTimer = 0;
	double fTimeConditionMet = 3;
	bool bFirstFrame = true;
	bool bAborted = false;

	ChaosUIPopup Popup;

	ChaosEffectInstance(ChaosEffect* effect) : pEffect(effect) {
		fTimer = pEffect->fTimerLength;
		fTimeConditionMet = pEffect->fUnhideTime;
	}

	bool IsActive() const {
		return fTimer > 0 && !bAborted;
	}

	bool IsNameOnScreen() const {
		return Popup.fTextTimer > 0;
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
		return pEffect && pEffect->HasTimer();
	}

	const char* GetName() const {
		if (!sNameToDisplay.empty()) return sNameToDisplay.c_str();
		return pEffect->sName;
	}

	bool IsHidden() const {
		return pEffect->IsRehideable() && !pEffect->IsAvailable();
	}

	void Draw(float y, bool inMenu) {
		if (!IsActive() && inMenu) return;

		Popup.fTimerLength = pEffect->fTimerLength;
		Popup.fTimer = fTimer;
		Popup.bHasTimer = HasTimer();
		Popup.Draw(GetName(), y, inMenu);
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
			Popup.Update(delta, ShouldNameBeOnScreen());
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

			fTimer -= delta;
		}
		if (IsActive()) {
			pEffect->TickFunctionMain(delta);
		}
		pEffect->EffectInstance = nullptr;
	}
};
static inline std::vector<ChaosEffectInstance> aRunningEffects;

void DoChaosSave();
void DoChaosLoad();

ChaosEffectInstance* GetEffectRunning(ChaosEffect* effect) {
	for (auto& running : aRunningEffects) {
		if (!running.IsActive()) continue;
		if (running.pEffect == effect) return &running;
	}
	return nullptr;
}

int GetNumEffectsRunning(ChaosEffect* except = nullptr) {
	int count = 0;
	for (auto& running : aRunningEffects) {
		if (!running.IsActive()) continue;
		if (running.pEffect == except) continue;
		count++;
	}
	return count;
}

bool IsEffectRunningFromIncompatibleGroup(uint32_t IncompatibilityGroup) {
	if (!IncompatibilityGroup) return false;
	for (auto& running : aRunningEffects) {
		if (!running.IsActive()) continue;
		for (auto& group : running.pEffect->IncompatibleFilterGroups) {
			if (group == IncompatibilityGroup) return true;
		}
	}
	return false;
}

bool IsEffectRunningFromFilterGroup(uint32_t IncompatibilityGroup) {
	if (!IncompatibilityGroup) return false;
	for (auto& running : aRunningEffects) {
		if (!running.IsActive()) continue;
		for (auto& group : running.pEffect->FilterGroups) {
			if (group == IncompatibilityGroup) return true;
		}
	}
	return false;
}

bool CanEffectActivate(ChaosEffect* effect) {
	if (!effect->CanMultiTrigger() && GetEffectRunning(effect)) return false;
	for (auto& group : effect->FilterGroups) {
		if (IsEffectRunningFromIncompatibleGroup(group)) return false;
	}
	for (auto& group : effect->IncompatibleFilterGroups) {
		if (IsEffectRunningFromFilterGroup(group)) return false;
	}
	if (effect->AbortOnConditionFailed()) {
		if (IsChaosBlocked()) return false; // IsAvailable can run in-game code, so always skip abortonconditionfailed effects in menus
		if (!effect->IsAvailable()) return false;
	}
	return true;
}

void AddRunningEffect(ChaosEffect* effect) {
	if (!effect->CanMultiTrigger() && GetEffectRunning(effect)) return;
	if (!CanEffectActivate(effect)) return;

	effect->bTriggeredThisCycle = true;
	effect->LastTriggerTime = std::time(0);
	effect->nTotalTimesActivated++;
	aRunningEffects.push_back(ChaosEffectInstance(effect));
	WriteLog(std::format("Activating {}", effect->sName));

	auto running = &aRunningEffects[aRunningEffects.size()-1];
	if (running->pEffect->InitImmediately()) {
		running->pEffect->InitFunction();
		running->bFirstFrame = false;
	}

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

bool CanEffectBeRandomlyPicked(ChaosEffect* effect) {
	if (effect->bTriggeredThisCycle) return false;

	// wait 30 minutes minimum before repeating an effect
	auto time = std::time(0);
	int timeDiff = time - effect->LastTriggerTime;
	if (timeDiff < 30 * 60) return false;

	if (!CanEffectActivate(effect)) return false;
	return true;
}

bool CanEffectBeSmartlyPicked(ChaosEffect* effect) {
	if (effect->bTriggeredThisCycleSmart) return false;
	if (!CanEffectActivate(effect)) return false;
	return true;
}

int GetNumEffectsAvailableForRandom(bool quickTrigger = false) {
	std::vector<ChaosEffect*> availableEffects;
	for (auto& effect : ChaosEffect::aEffects) {
		if (!CanEffectBeRandomlyPicked(effect)) continue;
		if (quickTrigger && !effect->CanQuickTrigger()) continue;

		availableEffects.push_back(effect);
	}
	return availableEffects.size();
}

ChaosEffect* GetRandomEffect(bool quickTrigger = false) {
	std::vector<ChaosEffect*> availableEffects;
	for (auto& effect : ChaosEffect::aEffects) {
		if (!CanEffectBeRandomlyPicked(effect)) continue;
		if (quickTrigger && !effect->CanQuickTrigger()) continue;

		for (int i = 0; i < effect->nFrequency; i++) {
			availableEffects.push_back(effect);
		}
	}
	if (availableEffects.empty()) {
		bool anyReset = false;
		for (auto& e : ChaosEffect::aEffects) {
			if (e->bTriggeredThisCycle) anyReset = true;
			e->bTriggeredThisCycle = false;
		}
		if (!anyReset) {
			for (auto& e : ChaosEffect::aEffects) {
				e->LastTriggerTime = 0;
			}
		}
		return GetRandomEffect(quickTrigger);
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
#include "effects/fueleffects.h"
#include "effects/votingeffects.h"
#include "effects/effect_safehouse.h"
#include "effects/effect_flashbacks.h"
#include "effects/effect_qte.h"
#include "effects/effect_disableall.h"
#include "effects/effect_tiktok.h"
#include "effects/unfaireffects.h"