#define EFFECT_CATEGORY_TEMP "Time"

class Effect_SpeedSlow : public ChaosEffect {
public:
	Effect_SpeedSlow() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Slow Motion";
		fTimerLength = 30;
		AddToIncompatiblityGroup("gamespeed");
		MakeIncompatibleWithFilterGroup("speedbreaker");
		bCanQuickTrigger = false;
	}

	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayer()) {
			ply->ResetGameBreaker(false);
		}

		GameSpeedModifier = 1 - (GetEffectFadeInOut(this, 1, false) * 0.5);
	}
	void DeinitFunction() override {
		GameSpeedModifier = 1.0;
	}
	bool HasTimer() override { return true; }
} E_SpeedSlow;

class Effect_SpeedFast : public ChaosEffect {
public:
	Effect_SpeedFast() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Fast Motion";
		fTimerLength = 30;
		AddToIncompatiblityGroup("gamespeed");
		MakeIncompatibleWithFilterGroup("speedbreaker");
		bCanQuickTrigger = false;
	}

	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayer()) {
			ply->ResetGameBreaker(false);
		}

		GameSpeedModifier = 1 + GetEffectFadeInOut(this, 1, false);
	}
	void DeinitFunction() override {
		GameSpeedModifier = 1.0;
	}
	bool HasTimer() override { return true; }
} E_SpeedFast;

class Effect_SpeedSpeedBased : public ChaosEffect {
public:
	Effect_SpeedSpeedBased() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Reverse Superhot";
		fTimerLength = 30;
		AddToIncompatiblityGroup("gamespeed");
		MakeIncompatibleWithFilterGroup("speedbreaker");
		bCanQuickTrigger = false;
	}

	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayer()) {
			ply->ResetGameBreaker(false);
		}

		auto speed = GetLocalPlayerVehicle()->GetSpeed();
		if (speed < TOMPS(10)) {
			GameSpeedModifier = TOMPS(150) / TOMPS(10);
		}
		else {
			GameSpeedModifier = TOMPS(150) / speed;
		}
		if (GameSpeedModifier > 3) GameSpeedModifier = 3;
	}
	void DeinitFunction() override {
		GameSpeedModifier = 1.0;
	}
	bool HasTimer() override { return true; }
} E_SpeedSpeedBased;

class Effect_RealTimeSpeedbrk : public ChaosEffect {
public:
	Effect_RealTimeSpeedbrk() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Real-Time Speedbreaker";
		fTimerLength = 90;
	}

	void TickFunctionMain(double delta) override {
		IgnoreSpeedbreakerTime = true;
	}
	void DeinitFunction() override {
		IgnoreSpeedbreakerTime = false;
	}
	bool HasTimer() override { return true; }
} E_RealTimeSpeedbrk;

class Effect_SpeedRandom : public ChaosEffect {
public:
	Effect_SpeedRandom() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Random Game Speed";
		fTimerLength = 30;
		AddToIncompatiblityGroup("gamespeed");
		MakeIncompatibleWithFilterGroup("speedbreaker");
		bCanQuickTrigger = false;
	}

	static inline float fStopSpeed = 0.1;
	static inline float fSlowSpeed = 3.0;
	static inline float fFastSpeed = 5.0;

	static inline float fCurrentSpeed = fStopSpeed;
	static inline double fTimeUntilNextSpeed = 3.0;

	void InitFunction() override {
		fCurrentSpeed = fStopSpeed;
		fTimeUntilNextSpeed = 3.0;
	}
	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayer()) {
			ply->ResetGameBreaker(false);
		}

		static auto audioSlow = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/oiiaslow.mp3");
		static auto audioFast = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/oiiafast.mp3");

		// slow lasts about 2 sec
		// fast lasts 1.3 sec

		fTimeUntilNextSpeed -= delta;
		if (fTimeUntilNextSpeed <= 0.0) {
			// always stop
			if (fCurrentSpeed > 1.0) {
				fCurrentSpeed = fStopSpeed;
				fTimeUntilNextSpeed = 2.0;
			}
			// then go
			else {
				if (PercentageChanceCheck(50)) {
					fCurrentSpeed = fSlowSpeed;
					fTimeUntilNextSpeed = 2.0;
					NyaAudio::SetVolume(audioSlow, GetSFXVolume());
					NyaAudio::Play(audioSlow);
				}
				else {
					fCurrentSpeed = fFastSpeed;
					fTimeUntilNextSpeed = 1.4;
					NyaAudio::SetVolume(audioFast, GetSFXVolume());
					NyaAudio::Play(audioFast);
				}
			}
		}
		GameSpeedModifier = fCurrentSpeed;
	}
	void DeinitFunction() override {
		GameSpeedModifier = 1.0;
	}
	bool HasTimer() override { return true; }
	//bool RunInMenus() override { return true; }
	//bool RunWhenBlocked() override { return true; }
} E_SpeedRandom;