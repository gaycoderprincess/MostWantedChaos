class ChaosEffect {
public:
	const char* sName;
	double fTimerLength = 30;
	double fLastTriggerTime = 99999;
	bool bTriggeredThisCycle = false;

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
	virtual bool RunInMenus() { return false; }
};

float fEffectX = 0.95;
float fEffectY = 0.4;
float fEffectSize = 0.04;
float fEffectSpacing = 0.045;

class ChaosEffectInstance {
public:
	ChaosEffect* pEffect = nullptr;
	double fActiveTimer = 0;
	double fTimer = 0;
	bool bFirstFrame = true;

	ChaosEffectInstance(ChaosEffect* effect) : pEffect(effect) {
		fTimer = pEffect->fTimerLength;
	}

	bool IsActive() const {
		return fTimer > 0;
	}

	bool HasTimer() const {
		return pEffect && pEffect->HasTimer();
	}

	float GetOffscreenPercentage() const {
		if (fActiveTimer < 0.5) return std::lerp(1, 0, easeInOutQuart(fActiveTimer * 2));
		else if (fTimer < 0.5) return std::lerp(1, 0, easeInOutQuart(fTimer * 2));
		return 0;
	}

	void Draw(float y) const {
		if (!IsActive()) return;

		auto x = fEffectX;
		x = 1 - x;
		x *= GetAspectRatioInv();
		x = 1 - x;

		std::string str = pEffect->sName;
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
		DrawString(data, str);
	}

	void OnTick(double delta) {
		if (!pEffect) {
			fTimer -= delta;
			fActiveTimer += delta;
			return;
		}

		if (bFirstFrame) {
			pEffect->InitFunction();
			bFirstFrame = false;
		}

		if (fTimer > 0 && fTimer - delta <= 0) {
			pEffect->DeinitFunction();
		}
		fTimer -= delta;
		fActiveTimer += delta;
		if (IsActive()) {
			pEffect->TickFunction(delta);
		}
	}
};
static inline std::vector<ChaosEffectInstance> aRunningEffects;

void AddRunningEffect(ChaosEffect* effect) {
	effect->bTriggeredThisCycle = true;
	effect->fLastTriggerTime = 0;
	aRunningEffects.push_back(ChaosEffectInstance(effect));
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

#include "effects/basiceffects.h"
#include "effects/playercareffects.h"
#include "effects/opponentcareffects.h"
#include "effects/allcareffects.h"
#include "effects/raceeffects.h"
#include "effects/hudeffects.h"
#include "effects/pursuiteffects.h"
#include "effects/profileeffects.h"
#include "effects/effect_wreckonflip.h"
#include "effects/effect_safehouse.h"