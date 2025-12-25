#define EFFECT_CATEGORY_TEMP "Player"

class Effect_StopCar : public ChaosEffect {
public:
	Effect_StopCar() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Stop Player Car";
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayerVehicle()) {
			ply->SetSpeed(0);
		}
	}
} E_StopCar;

class Effect_LaunchCarFwd : public ChaosEffect {
public:
	Effect_LaunchCarFwd() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Launch Player Forward";
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayerVehicle()) {
			ply->SetSpeed(TOMPS(400));
		}
	}
} E_LaunchCarFwd;

class Effect_LaunchCarBwd : public ChaosEffect {
public:
	Effect_LaunchCarBwd() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Launch Player Backwards";
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayerVehicle()) {
			ply->SetSpeed(TOMPS(-200));
		}
	}
} E_LaunchCarBwd;

class Effect_LaunchCarSide : public ChaosEffect {
public:
	Effect_LaunchCarSide() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Launch Player Sideways";
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayerInterface<IRigidBody>()) {
			UMath::Vector3 side;
			ply->GetRightVector(&side);
			side.x *= TOMPS(200);
			side.y *= TOMPS(200);
			side.z *= TOMPS(200);
			ply->SetLinearVelocity(&side);
		}
	}
} E_LaunchCarSide;

class Effect_LaunchCarUp : public ChaosEffect {
public:
	Effect_LaunchCarUp() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Launch Player Up";
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayerInterface<IRigidBody>()) {
			UMath::Vector3 vel = *ply->GetLinearVelocity();
			vel.y = TOMPS(200);
			ply->SetLinearVelocity(&vel);
		}
	}
} E_LaunchCarUp;

class Effect_LaunchCarDown : public ChaosEffect {
public:
	Effect_LaunchCarDown() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Launch Player Down";
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayerInterface<IRigidBody>()) {
			UMath::Vector3 pos = *ply->GetPosition();
			pos.y += 15;
			ply->SetPosition(&pos);
			UMath::Vector3 vel = *ply->GetLinearVelocity();
			vel.y = -TOMPS(200);
			ply->SetLinearVelocity(&vel);
		}
	}
} E_LaunchCarDown;

class Effect_SpinCar : public ChaosEffect {
public:
	Effect_SpinCar() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Spin Player Out";
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayerInterface<IRigidBody>()) {
			auto vel = *ply->GetAngularVelocity();
			vel.y = rand() % 2 ? 4 : -4;
			ply->SetAngularVelocity(&vel);
		}
	}
} E_SpinCar;

class Effect_SpinCar2 : public ChaosEffect {
public:
	Effect_SpinCar2() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Spin Player Rapidly";
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayerInterface<IRigidBody>()) {
			auto vel = *ply->GetAngularVelocity();
			vel.y = TOMPS(200) * (rand() % 2 ? 1 : -1);
			ply->SetAngularVelocity(&vel);
		}
	}
} E_SpinCar2;

class Effect_InfGameBreaker : public ChaosEffect {
public:
	Effect_InfGameBreaker() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Infinite Speedbreaker";
		AddToIncompatiblityGroup("speedbreaker");
		fTimerLength = 60;
	}

	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayer()) {
			ply->ChargeGameBreaker(1);
		}
	}
	bool HasTimer() override { return true; }
} E_InfGameBreaker;

class Effect_InfNitro : public EffectBase_PlayerCarHasNitro {
public:
	Effect_InfNitro() : EffectBase_PlayerCarHasNitro(EFFECT_CATEGORY_TEMP) {
		sName = "Infinite Player Nitro";
		AddToIncompatiblityGroup("nitro");
		fTimerLength = 60;
	}

	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayerEngine()) {
			ply->ChargeNOS(1);
		}
	}
	bool HasTimer() override { return true; }
} E_InfNitro;

class Effect_NoGameBreaker : public ChaosEffect {
public:
	Effect_NoGameBreaker() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Disable Speedbreaker";
		fTimerLength = 90;
		AddToIncompatiblityGroup("speedbreaker");
	}

	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayer()) {
			ply->ResetGameBreaker(false);
		}
	}
	bool HasTimer() override { return true; }
} E_NoGameBreaker;

class Effect_NoNitro : public EffectBase_PlayerCarHasNitro {
public:
	Effect_NoNitro() : EffectBase_PlayerCarHasNitro(EFFECT_CATEGORY_TEMP) {
		sName = "Disable Player Nitro";
		fTimerLength = 60;
		AddToIncompatiblityGroup("nitro");
	}

	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayerEngine()) {
			ply->ChargeNOS(-1);
		}
	}
	bool HasTimer() override { return true; }
} E_NoNitro;

class Effect_SetHeat1 : public ChaosEffect {
public:
	Effect_SetHeat1() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Set Heat Level to 1";
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayerInterface<IPerpetrator>()) {
			ply->SetHeat(1);
		}
	}
} E_SetHeat1;

/*class Effect_SetHeat5 : public EffectBase_NotInPursuitConditional {
public:
	Effect_SetHeat5() : EffectBase_NotInPursuitConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Set Heat Level to 5";
	}

	void InitFunction() override {
		if (auto max = GetMaxHeat()) {
			if (*max < 5) *max = 5;
		}
		if (auto ply = GetLocalPlayerInterface<IPerpetrator>()) {
			ply->SetHeat(5);
		}
	}
	bool AbortOnConditionFailed() override { return true; }
} E_SetHeat5;*/

// max heat level doesn't seem to apply when in the middle of an active pursuit
class Effect_SetHeat6 : public EffectBase_NotInPursuitConditional {
public:
	Effect_SetHeat6() : EffectBase_NotInPursuitConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Set Heat Level to 6";
	}

	void InitFunction() override {
		if (auto max = GetMaxHeat()) {
			if (*max < 6) *max = 6;
		}
		if (auto ply = GetLocalPlayerInterface<IPerpetrator>()) {
			ply->SetHeat(6);
		}
	}
	bool AbortOnConditionFailed() override { return true; }
} E_SetHeat6;

class Effect_SetHeatDown : public ChaosEffect {
public:
	Effect_SetHeatDown() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Decrease Heat Level";
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayerInterface<IPerpetrator>()) {
			ply->SetHeat(ply->GetHeat() - 1);
		}
	}
} E_SetHeatDown;

class Effect_SetHeatUp : public ChaosEffect {
public:
	Effect_SetHeatUp() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Increase Heat Level";
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayerInterface<IPerpetrator>()) {
			if (auto max = GetMaxHeat()) {
				if (*max < ply->GetHeat() + 1) *max = ply->GetHeat() + 1;
			}
			ply->SetHeat(ply->GetHeat() + 1);
		}
	}
} E_SetHeatUp;

class Effect_BlowEngine : public ChaosEffect {
public:
	Effect_BlowEngine() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Blow Player's Engine";
		MakeIncompatibleWithFilterGroup("player_godmode");
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayerInterface<IEngineDamage>()) {
			ply->Sabotage(3);
		}
	}
	bool CanQuickTrigger() override { return false; }
} E_BlowEngine;

class Effect_AutoDrive : public ChaosEffect {
public:
	Effect_AutoDrive() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Auto-Drive";
		AddToIncompatiblityGroup("autodrive");
		fTimerLength = 30;
	}

	void TickFunction(eChaosHook hook, double delta) override {
		if (hook != HOOK_GAMETICK) return;
		
		if (auto ply = GetLocalPlayerInterface<IHumanAI>()) {
			if (!ply->GetAiControl()) ply->SetAiControl(true);
		}
	}
	void DeinitFunction() override {
		if (auto ply = GetLocalPlayerInterface<IHumanAI>()) {
			ply->SetAiControl(false);
		}
	}
	bool HasTimer() override { return true; }
} E_AutoDrive;

/*class Effect_AutoDrive2 : public ChaosEffect {
public:
	Effect_AutoDrive2() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Auto-Drive (Traffic)";
		fTimerLength = 15;
		AddToIncompatiblityGroup("autodrive");
	}

	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayerInterface<IHumanAI>()) {
			if (!ply->GetAiControl()) {
				NyaHookLib::Patch(0x42920D + 1, "AIGoalTraffic");
				ply->SetAiControl(true);
				NyaHookLib::Patch(0x42920D + 1, "AIGoalRacer");
			}
		}
	}
	void DeinitFunction() override {
		if (auto ply = GetLocalPlayerInterface<IHumanAI>()) {
			ply->SetAiControl(false);
		}
	}
	bool HasTimer() override { return true; }
} E_AutoDrive2;*/

/*class Effect_911 : public ChaosEffect {
public:
	Effect_911() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Call 911";
	}

	void InitFunction() override {
		aMainLoopFunctions.push_back([]() { E911Call::Create(); });
		//EAXDispatch::Report911(SoundAI::mInstance->mDispatch, Csis::Type_pursuit_type_Reckless);
	}
	bool IsAvailable() override {
		if (auto ply = GetLocalPlayerInterface<IPerpetrator>()) {
			return !ply->IsBeingPursued();
		}
		return true;
	}
	bool IsConditionallyAvailable() override { return true; }
} E_911;*/

class Effect_PlayerCarSpin : public ChaosEffect {
public:
	Effect_PlayerCarSpin() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Spain but the A is silent";
		sFriendlyName = "Make Player Spin";
		fTimerLength = 10;
	}

	NyaAudio::NyaSound sound = 0;

	void InitFunction() override {
		if (!sound) sound = NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/funky.mp3");
		if (sound) {
			NyaAudio::SetVolume(sound, FEDatabase->mUserProfile->TheOptionsSettings.TheAudioSettings.SoundEffectsVol);
			NyaAudio::Play(sound);
		}
	}
	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayerInterface<IRigidBody>()) {
			auto vel = *ply->GetAngularVelocity();
			vel.y = 2;
			ply->SetAngularVelocity(&vel);
		}
	}
	bool HasTimer() override { return true; }
} E_PlayerCarSpin;

class Effect_PlayerCarTuneAero : public EffectBase_HasTuningConditional {
public:
	Effect_PlayerCarTuneAero() : EffectBase_HasTuningConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Aerodynamics +100";
		fTimerLength = 15;
		AddToIncompatiblityGroup("aerodynamics");
	}

	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayerVehicle()) {
			Physics::Tunings tune = ply->GetTunings() ? *ply->GetTunings() : Physics::Tunings();
			tune.Value[Physics::Tunings::AERODYNAMICS] = 50;
			ply->SetTunings(&tune);
		}
	}
	void DeinitFunction() override {
		if (auto ply = GetLocalPlayerVehicle()) {
			Physics::Tunings tune = ply->GetTunings() ? *ply->GetTunings() : Physics::Tunings();
			tune.Value[Physics::Tunings::AERODYNAMICS] = 0;
			ply->SetTunings(&tune);
		}
	}
	bool HasTimer() override { return true; }
} E_PlayerCarTuneAero;

class Effect_PlayerCarTuneHandling : public EffectBase_HasTuningConditional {
public:
	Effect_PlayerCarTuneHandling() : EffectBase_HasTuningConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Handling +10";
		fTimerLength = 60;
		AddToIncompatiblityGroup("handling_tune");
	}

	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayerVehicle()) {
			Physics::Tunings tune = ply->GetTunings() ? *ply->GetTunings() : Physics::Tunings();
			tune.Value[Physics::Tunings::HANDLING] = 2;
			ply->SetTunings(&tune);
		}
	}
	void DeinitFunction() override {
		if (auto ply = GetLocalPlayerVehicle()) {
			Physics::Tunings tune = ply->GetTunings() ? *ply->GetTunings() : Physics::Tunings();
			tune.Value[Physics::Tunings::HANDLING] = 0;
			ply->SetTunings(&tune);
		}
	}
	bool HasTimer() override { return true; }
} E_PlayerCarTuneHandling;

class Effect_PlayerCarTuneHandling2 : public EffectBase_HasTuningConditional {
public:
	Effect_PlayerCarTuneHandling2() : EffectBase_HasTuningConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Handling -10";
		fTimerLength = 60;
		AddToIncompatiblityGroup("handling_tune");
	}

	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayerVehicle()) {
			Physics::Tunings tune = ply->GetTunings() ? *ply->GetTunings() : Physics::Tunings();
			tune.Value[Physics::Tunings::HANDLING] = -2;
			ply->SetTunings(&tune);
		}
	}
	void DeinitFunction() override {
		if (auto ply = GetLocalPlayerVehicle()) {
			Physics::Tunings tune = ply->GetTunings() ? *ply->GetTunings() : Physics::Tunings();
			tune.Value[Physics::Tunings::HANDLING] = 0;
			ply->SetTunings(&tune);
		}
	}
	bool HasTimer() override { return true; }
} E_PlayerCarTuneHandling2;

/*class Effect_PlayerCarTuneHeight : public EffectBase_HasTuningConditional {
public:
	Effect_PlayerCarTuneHeight() : EffectBase_HasTuningConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Stanced Suspension";
		fTimerLength = 60;
		IncompatibilityGroups.push_back(Attrib::StringHash32("suspension");
	}

	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayerVehicle()) {
			Physics::Tunings tune = ply->GetTunings() ? *ply->GetTunings() : Physics::Tunings();
			tune.Value[Physics::Tunings::RIDEHEIGHT] = -2;
			ply->SetTunings(&tune);
		}
	}
	void DeinitFunction() override {
		if (auto ply = GetLocalPlayerVehicle()) {
			Physics::Tunings tune = ply->GetTunings() ? *ply->GetTunings() : Physics::Tunings();
			tune.Value[Physics::Tunings::RIDEHEIGHT] = 0;
			ply->SetTunings(&tune);
		}
	}
	bool HasTimer() override { return true; }
} E_PlayerCarTuneHeight;*/

/*class Effect_PlayerCarTuneHeight2 : public EffectBase_HasTuningConditional {
public:
	Effect_PlayerCarTuneHeight2() : EffectBase_HasTuningConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Super Stanced Suspension";
		fTimerLength = 60;
		IncompatibilityGroups.push_back(Attrib::StringHash32("suspension");
	}

	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayerVehicle()) {
			Physics::Tunings tune = ply->GetTunings() ? *ply->GetTunings() : Physics::Tunings();
			tune.Value[Physics::Tunings::RIDEHEIGHT] = -10;
			ply->SetTunings(&tune);
		}
	}
	void DeinitFunction() override {
		if (auto ply = GetLocalPlayerVehicle()) {
			Physics::Tunings tune = ply->GetTunings() ? *ply->GetTunings() : Physics::Tunings();
			tune.Value[Physics::Tunings::RIDEHEIGHT] = 0;
			ply->SetTunings(&tune);
		}
	}
	bool HasTimer() override { return true; }
} E_PlayerCarTuneHeight2;*/

class Effect_PlayerCarTuneHeight3 : public EffectBase_HasTuningConditional {
public:
	Effect_PlayerCarTuneHeight3() : EffectBase_HasTuningConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Monster Truck Suspension";
		fTimerLength = 60;
		AddToIncompatiblityGroup("suspension");
	}

	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayerVehicle()) {
			Physics::Tunings tune = ply->GetTunings() ? *ply->GetTunings() : Physics::Tunings();
			tune.Value[Physics::Tunings::RIDEHEIGHT] = 25;
			ply->SetTunings(&tune);
		}
	}
	void DeinitFunction() override {
		if (auto ply = GetLocalPlayerVehicle()) {
			Physics::Tunings tune = ply->GetTunings() ? *ply->GetTunings() : Physics::Tunings();
			tune.Value[Physics::Tunings::RIDEHEIGHT] = 0;
			ply->SetTunings(&tune);
		}
	}
	bool HasTimer() override { return true; }
} E_PlayerCarTuneHeight3;

class Effect_PlayerCarTuneHeight4 : public EffectBase_HasTuningConditional {
public:
	double state = 0;
	bool goBack = false;

	Effect_PlayerCarTuneHeight4() : EffectBase_HasTuningConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Bouncy Suspension";
		fTimerLength = 30;
		AddToIncompatiblityGroup("suspension");
	}

	void InitFunction() override {
		state = 0;
	}
	void TickFunctionMain(double delta) override {
		state += delta * (goBack ? -2 : 2);
		if (state > 1) goBack = true;
		if (state < 0) goBack = false;

		if (auto ply = GetLocalPlayerVehicle()) {
			Physics::Tunings tune = ply->GetTunings() ? *ply->GetTunings() : Physics::Tunings();
			tune.Value[Physics::Tunings::RIDEHEIGHT] = std::lerp(0,20, easeInOutQuart(state));
			ply->SetTunings(&tune);
		}
	}
	void DeinitFunction() override {
		if (auto ply = GetLocalPlayerVehicle()) {
			Physics::Tunings tune = ply->GetTunings() ? *ply->GetTunings() : Physics::Tunings();
			tune.Value[Physics::Tunings::RIDEHEIGHT] = 0;
			ply->SetTunings(&tune);
		}
	}
	bool HasTimer() override { return true; }
} E_PlayerCarTuneHeight4;

class Effect_PlayerCarTuneNitro : public EffectBase_PlayerCarHasNitro {
public:
	Effect_PlayerCarTuneNitro() : EffectBase_PlayerCarHasNitro(EFFECT_CATEGORY_TEMP) {
		sName = "Weak Nitro";
		fTimerLength = 60;
		AddToIncompatiblityGroup("nitro_tune");
	}

	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayerVehicle()) {
			Physics::Tunings tune = ply->GetTunings() ? *ply->GetTunings() : Physics::Tunings();
			tune.Value[Physics::Tunings::NOS] = -5;
			ply->SetTunings(&tune);
		}
	}
	void DeinitFunction() override {
		if (auto ply = GetLocalPlayerVehicle()) {
			Physics::Tunings tune = ply->GetTunings() ? *ply->GetTunings() : Physics::Tunings();
			tune.Value[Physics::Tunings::NOS] = 0;
			ply->SetTunings(&tune);
		}
	}
	bool HasTimer() override { return true; }
} E_PlayerCarTuneNitro;

class Effect_PlayerCarTuneNitro2 : public EffectBase_PlayerCarHasNitro {
public:
	Effect_PlayerCarTuneNitro2() : EffectBase_PlayerCarHasNitro(EFFECT_CATEGORY_TEMP) {
		sName = "Strong Nitro";
		fTimerLength = 60;
		AddToIncompatiblityGroup("nitro_tune");
	}

	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayerVehicle()) {
			Physics::Tunings tune = ply->GetTunings() ? *ply->GetTunings() : Physics::Tunings();
			tune.Value[Physics::Tunings::NOS] = 3;
			ply->SetTunings(&tune);
			if (auto engine = GetLocalPlayerEngine()) {
				engine->ChargeNOS(1);
			}
		}
	}
	void DeinitFunction() override {
		if (auto ply = GetLocalPlayerVehicle()) {
			Physics::Tunings tune = ply->GetTunings() ? *ply->GetTunings() : Physics::Tunings();
			tune.Value[Physics::Tunings::NOS] = 0;
			ply->SetTunings(&tune);
		}
	}
	bool HasTimer() override { return true; }
} E_PlayerCarTuneNitro2;

class Effect_PlayerCarReset : public ChaosEffect {
public:
	Effect_PlayerCarReset() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Reset Player";
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayerInterface<IResetable>()) {
			ply->ResetVehicle(true);
		}
	}
	bool IsAvailable() override { return !NoResetCount; }
	bool AbortOnConditionFailed() override { return true; }
} E_PlayerCarReset;

class Effect_PlayerCarSpike1 : public ChaosEffect {
public:
	Effect_PlayerCarSpike1() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Puncture One Player Tire";
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayerInterface<ISpikeable>()) {
			ply->Puncture(rand() % 4);
		}
	}
} E_PlayerCarSpike1;

class Effect_PlayerCarSpikeAll : public ChaosEffect {
public:
	Effect_PlayerCarSpikeAll() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Puncture Player's Tires";
		fTimerLength = 10;
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayerInterface<ISpikeable>()) {
			for (int i = 0; i < 4; i++) {
				ply->Puncture(i);
			}
		}
	}

	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayerInterface<ISpikeable>()) {
			for (int i = 0; i < 4; i++) {
				if (ply->GetTireDamage(i) < TIRE_DAMAGE_PUNCTURED) ply->Puncture(i);
			}
		}
	}

	void DeinitFunction() override {
		if (IsCarDestroyed(GetLocalPlayerVehicle())) return;
		if (auto ply = GetLocalPlayerInterface<IDamageable>()) {
			ply->ResetDamage();
		}
	}
	bool HasTimer() override { return true; }
} E_PlayerCarSpikeAll;

class Effect_FixPlayerCar : public ChaosEffect {
public:
	Effect_FixPlayerCar() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Fix Player's Tires";
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayerInterface<IDamageable>()) {
			ply->ResetDamage();
		}
	}
	bool IsAvailable() override {
		if (auto ply = GetLocalPlayerInterface<ISpikeable>()) {
			for (int i = 0; i < 4; i++) {
				if (ply->GetTireDamage(i) != TIRE_DAMAGE_NONE) return true;
			}
		}
		return false;
	}
	bool AbortOnConditionFailed() override { return true; }
} E_FixPlayerCar;

class Effect_NoReset : public ChaosEffect {
public:
	Effect_NoReset() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Disable Resetting";
		fTimerLength = 120;
	}

	void InitFunction() override {
		NoResetCount++;
	}
	void DeinitFunction() override {
		NoResetCount--;
	}
	bool HasTimer() override { return true; }
} E_NoReset;

class Effect_ResetImmune : public EffectBase_PursuitConditional {
public:
	Effect_ResetImmune() : EffectBase_PursuitConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Disable Instant Bust On Reset";
		fTimerLength = 120;
	}

	void InitFunction() override {
		ManualResetImmunity = true;
	}
	void DeinitFunction() override {
		ManualResetImmunity = false;
	}
	bool HasTimer() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
} E_ResetImmune;

float CarMagnetForce = 3;
void DoCarMagnet(IVehicle* source, double delta) {
	auto cars = GetActiveVehicles();
	for (auto& car : cars) {
		if (car == source) continue;
		auto otherCar = car->mCOMObject->Find<IRigidBody>();
		if (!otherCar) continue;

		auto v = source->GetPosition();
		auto c = otherCar->GetPosition();
		auto vel = *otherCar->GetLinearVelocity();
		vel.x += (v->x - c->x) * CarMagnetForce * delta;
		vel.y += (v->y - c->y) * CarMagnetForce * delta;
		vel.z += (v->z - c->z) * CarMagnetForce * delta;
		otherCar->SetLinearVelocity(&vel);
	}
}

void DoCarForcefield(IVehicle* source) {
	auto cars = GetActiveVehicles();
	for (auto& car : cars) {
		if (car == source) continue;
		auto otherCar = car->mCOMObject->Find<IRigidBody>();
		if (!otherCar) continue;

		auto v = source->GetPosition();
		auto c = otherCar->GetPosition();
		if ((*v - *c).length() < 15) {
			auto dir = (*c - *v);
			dir.Normalize();
			dir *= 15;
			auto newPos = *v;
			newPos += dir;
			otherCar->SetPosition(&newPos);
		}
	}
}

class Effect_CarMagnet : public ChaosEffect {
public:
	Effect_CarMagnet() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Player Car Magnet";
		fTimerLength = 30;
	}

	void TickFunctionMain(double delta) override {
		DoCarMagnet(GetLocalPlayerVehicle(), delta);
	}
	bool HasTimer() override { return true; }
} E_CarMagnet;

class Effect_CarForcefield : public ChaosEffect {
public:
	Effect_CarForcefield() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Player Car Forcefield";
		fTimerLength = 60;
	}

	void TickFunctionMain(double delta) override {
		DoCarForcefield(GetLocalPlayerVehicle());
	}
	bool HasTimer() override { return true; }
} E_CarForcefield;

class Effect_NoInput : public ChaosEffect {
public:
	Effect_NoInput() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Disable Player Input";
		fTimerLength = 15;
	}

	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayerInterface<IInput>()) {
			ply->ClearInput();
		}
	}
	bool HasTimer() override { return true; }
} E_NoInput;

/*class Effect_NoInputMash : public ChaosEffect {
public:
	Effect_NoInputMash() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Mash To Drive";
		fTimerLength = 15;
	}

	void TickFunctionMain(double delta) override {
		static double timer = 0;
		timer += delta;
		while (timer > 0.1) {
			if (auto ply = GetLocalPlayerInterface<IInputPlayer>()) {
				ply->ClearInput();
			}
			timer -= 0.1;
		}
	}
	bool HasTimer() override { return true; }
} E_NoInputMash;*/

class Effect_PlayerCarGear1 : public ChaosEffect {
public:
	Effect_PlayerCarGear1() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Force Player To Gear 1";
		fTimerLength = 15;
		AddToIncompatiblityGroup("transmission");
	}

	void TickFunctionMain(double delta) override {
		ForceManualGearbox = true;
		if (auto ply = GetLocalPlayerInterface<ITransmission>()) {
			if (ply->GetGear() != G_FIRST) ply->Shift(G_FIRST);
		}
	}
	void DeinitFunction() override {
		ForceManualGearbox = false;
	}
	bool HasTimer() override { return true; }
} E_PlayerCarGear1;

class Effect_PlayerCarGearTop : public ChaosEffect {
public:
	Effect_PlayerCarGearTop() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Top Gear";
		sFriendlyName = "Force Player To Top Gear";
		fTimerLength = 30;
		AddToIncompatiblityGroup("transmission");
	}

	void TickFunctionMain(double delta) override {
		ForceManualGearbox = true;
		if (auto ply = GetLocalPlayerInterface<ITransmission>()) {
			if (ply->GetGear() != ply->GetTopGear()) ply->Shift(ply->GetTopGear());
		}
	}
	void DeinitFunction() override {
		ForceManualGearbox = false;
	}
	bool HasTimer() override { return true; }
} E_PlayerCarGearTop;

class Effect_PlayerCarGearR : public ChaosEffect {
public:
	Effect_PlayerCarGearR() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Force Player To Reverse Gear";
		fTimerLength = 15;
		AddToIncompatiblityGroup("transmission");
	}

	void TickFunctionMain(double delta) override {
		ForceManualGearbox = true;
		if (auto ply = GetLocalPlayerInterface<ITransmission>()) {
			if (ply->GetGear() != G_REVERSE) ply->Shift(G_REVERSE);
		}
	}
	void DeinitFunction() override {
		ForceManualGearbox = false;
	}
	bool HasTimer() override { return true; }
} E_PlayerCarGearR;

class Effect_SetCarMassInf : public ChaosEffect {
public:
	Effect_SetCarMassInf() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Infinite Player Mass";
		fTimerLength = 90;
		AddToIncompatiblityGroup("playermass");
	}

	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayerInterface<IRBVehicle>()) {
			ply->SetCollisionMass(100000);
		}
	}
	void DeinitFunction() override {
		if (auto ply = GetLocalPlayerInterface<IRBVehicle>()) {
			ply->SetCollisionMass(GetLocalPlayerInterface<IRigidBody>()->GetMass());
		}
	}
	bool HasTimer() override { return true; }
} E_SetCarMassInf;

class Effect_SetCarMass0 : public ChaosEffect {
public:
	Effect_SetCarMass0() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Zero Player Mass";
		fTimerLength = 60;
		AddToIncompatiblityGroup("playermass");
	}

	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayerInterface<IRBVehicle>()) {
			ply->SetCollisionMass(0.01);
		}
	}
	void DeinitFunction() override {
		if (auto ply = GetLocalPlayerInterface<IRBVehicle>()) {
			ply->SetCollisionMass(GetLocalPlayerInterface<IRigidBody>()->GetMass());
		}
	}
	bool HasTimer() override { return true; }
} E_SetCarMass0;

void DoPiggyback(IVehicle* bottom, IVehicle* top) {
	auto playerPos = *top->GetPosition();
	auto targetPos = *bottom->GetPosition();
	playerPos.x = targetPos.x;
	playerPos.z = targetPos.z;
	if (playerPos.y < targetPos.y) playerPos.y = targetPos.y + 3;
	top->mCOMObject->Find<IRigidBody>()->SetPosition(&playerPos);
	top->mCOMObject->Find<IRBVehicle>()->EnableObjectCollisions(true);
}

class Effect_Piggyback : public EffectBase_ActiveCarsConditional {
public:
	Effect_Piggyback() : EffectBase_ActiveCarsConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Piggyback Ride";
		sFriendlyName = "Place Player Onto Closest Car";
		fTimerLength = 15;
	}

	void InitFunction() override {
		NoResetCount++;
	}
	void TickFunctionMain(double delta) override {
		if (auto target = GetClosestActiveVehicle(GetLocalPlayerVehicle())) {
			DoPiggyback(target, GetLocalPlayerVehicle());
		}
	}
	void DeinitFunction() override {
		NoResetCount--;
	}
	bool HasTimer() override { return true; }
	bool IsRehideable() override { return true; }
} E_Piggyback;

class Effect_LockPlayer : public ChaosEffect {
public:
	UMath::Vector3 position;

	Effect_LockPlayer() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Lock Player In Place";
		fTimerLength = 10;
	}

	void InitFunction() override {
		position = *GetLocalPlayerSimable()->GetPosition();
	}
	void TickFunctionMain(double delta) override {
		GetLocalPlayerInterface<IRigidBody>()->SetPosition(&position);
	}
	bool HasTimer() override { return true; }
} E_LockPlayer;

class Effect_LockPlayerOrient : public ChaosEffect {
public:
	UMath::Vector4 orient;

	Effect_LockPlayerOrient() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Lock Player Rotation";
		fTimerLength = 10;
	}

	void InitFunction() override {
		orient = *GetLocalPlayerInterface<IRigidBody>()->GetOrientation();
	}
	void TickFunctionMain(double delta) override {
		GetLocalPlayerInterface<IRigidBody>()->SetOrientation(&orient);
	}
	bool HasTimer() override { return true; }
} E_LockPlayerOrient;

class Effect_SwapPlayerWithOpponent : public EffectBase_OpponentInRaceOrRoamingConditional {
public:

	Effect_SwapPlayerWithOpponent() : EffectBase_OpponentInRaceOrRoamingConditional(EFFECT_CATEGORY_TEMP) {
		sName = "Swap Places With Random Opponent";
	}

	void InitFunction() override {
		auto cars = GetActiveVehicles(DRIVER_RACER);
		auto playerCar = GetLocalPlayerVehicle();
		auto targetCar = cars[rand()%cars.size()];

		CwoeeCarPhysicalState player(playerCar);
		CwoeeCarPhysicalState target(targetCar);

		player.Apply(targetCar);
		target.Apply(playerCar);
	}
	bool AbortOnConditionFailed() override { return true; }
} E_SwapPlayerWithOpponent;

class Effect_PlayerResetTransform : public ChaosEffect {
public:
	Effect_PlayerResetTransform() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Disable Player Physics";
		fTimerLength = 15;
		fUnhideTime = 1;
	}

	void InitFunction() override {
		NoResetCount++;
	}
	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayerVehicle()) {
			auto playerRB = GetLocalPlayerInterface<IRigidBody>();
			auto playerOrient = *playerRB->GetOrientation();
			auto playerVel = *playerRB->GetLinearVelocity();
			auto playerAVel = *playerRB->GetAngularVelocity();
			//if (playerVel.length() < TOMPS(20)) return;

			auto pos = *ply->GetPosition();
			auto fwd = *GetLocalPlayerInterface<ICollisionBody>()->GetForwardVector();
			ply->SetVehicleOnGround(&pos, &fwd);

			playerRB->SetOrientation(&playerOrient);
			playerRB->SetLinearVelocity(&playerVel);
			playerRB->SetAngularVelocity(&playerAVel);
		}
	}
	void DeinitFunction() override {
		NoResetCount--;
	}
	bool HasTimer() override { return true; }
	bool IsAvailable() override {
		if (auto playerRB = GetLocalPlayerInterface<IRigidBody>()) {
			auto playerVel = *playerRB->GetLinearVelocity();
			if (playerVel.length() < TOMPS(100)) return false;
			return true;
		}
		return false;
	}
} E_PlayerResetTransform;

// doesn't do enough at all to be worth an effect slot
/*class Effect_PlayerNoBrakes : public ChaosEffect {
public:
	Effect_PlayerNoBrakes() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "They Just Slow Us Down";
		sFriendlyName = "Disable Player Brakes";
		fTimerLength = 60;
	}

	void TickFunction(eChaosHook hook, double delta) override {
		if (hook != HOOK_INPUT) return;

		if (auto ply = GetLocalPlayerInterface<IInput>()) {
			auto controls = ply->GetControls();
			controls->fBrake = 0;
			controls->fHandBrake = 0;
		}
	}
	bool HasTimer() override { return true; }
} E_PlayerNoBrakes;*/

class Effect_PlayerNoSteering : public ChaosEffect {
public:
	Effect_PlayerNoSteering() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Broken Steering Wheel";
		sFriendlyName = "Disable Player Steering";
		fTimerLength = 15;
	}

	void TickFunction(eChaosHook hook, double delta) override {
		if (hook != HOOK_INPUT) return;

		if (auto ply = GetLocalPlayerInterface<IInput>()) {
			auto controls = ply->GetControls();
			controls->fSteering = 0;
		}
	}
	bool HasTimer() override { return true; }
} E_PlayerNoSteering;

class Effect_PlayerHalfSteering : public ChaosEffect {
public:
	Effect_PlayerHalfSteering() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Half-Broken Steering Wheel";
		sFriendlyName = "Halve Player Steering";
		fTimerLength = 30;
	}

	void TickFunction(eChaosHook hook, double delta) override {
		if (hook != HOOK_INPUT) return;

		if (auto ply = GetLocalPlayerInterface<IInput>()) {
			auto controls = ply->GetControls();
			if (controls->fSteering < -0.5) controls->fSteering = -0.5;
			if (controls->fSteering > 0.5) controls->fSteering = 0.5;
		}
	}
	bool HasTimer() override { return true; }
} E_PlayerHalfSteering;

class Effect_PlayerHalfGas : public ChaosEffect {
public:
	Effect_PlayerHalfGas() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Half-Broken Throttle";
		sFriendlyName = "Halve Player Throttle";
		fTimerLength = 30;
	}

	void TickFunction(eChaosHook hook, double delta) override {
		if (hook != HOOK_INPUT) return;

		if (auto ply = GetLocalPlayerInterface<IInput>()) {
			auto controls = ply->GetControls();
			if (controls->fGas > 0.5) controls->fGas = 0.5;
		}
	}
	bool HasTimer() override { return true; }
} E_PlayerHalfGas;

class Effect_PlayerLag : public ChaosEffect {
public:
	CwoeeCarPhysicalState lastState;
	double timer = 0;

	Effect_PlayerLag() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Connection Problem";
		sFriendlyName = "Laggy Player";
		fTimerLength = 30;
	}

	void InitFunction() override {
		timer = 0;
		lastState.Capture(GetLocalPlayerVehicle());
	}
	void TickFunctionMain(double delta) override {
		timer += delta;
		if (timer > 1) {
			if (PercentageChanceCheck(25)) {
				lastState.Apply(GetLocalPlayerVehicle());
			}
			else if (PercentageChanceCheck(50)) {
				lastState.Capture(GetLocalPlayerVehicle());
			}
			timer -= 1;
		}
	}
	bool HasTimer() override { return true; }
} E_PlayerLag;

class Effect_PlayerTPHidingSpot : public ChaosEffect {
public:
	Effect_PlayerTPHidingSpot() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Teleport To Perfect Hiding Spot";
	}

	void InitFunction() override {
		TeleportPlayer({-2200.7, 144.6, 1458.7}, {-0.14, 0.0, 0.99});
	}
	bool CanQuickTrigger() override { return false; }
} E_PlayerTPHidingSpot;

class Effect_PlayerTPBridge : public ChaosEffect {
public:
	Effect_PlayerTPBridge() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Teleport Behind The Old Bridge";
	}

	void InitFunction() override {
		TeleportPlayer({-2485.08, 207.38, -1348.54}, {-0.89, 0.0, -0.47});
	}
	bool CanQuickTrigger() override { return false; }
} E_PlayerTPBridge;

// this technically affects all cars but it's here in playercareffects since it mostly only affects the player
class Effect_NOSBoost : public EffectBase_PlayerCarHasNitro {
public:
	Effect_NOSBoost() : EffectBase_PlayerCarHasNitro(EFFECT_CATEGORY_TEMP) {
		sName = "Nitro Boosting";
		fTimerLength = 45;
	}

	void InitFunction() override {
		CwoeeHints::AddHint("Press and hold the NOS button to launch forward!");
	}
	void TickFunctionMain(double delta) override {
		auto cars = GetActiveVehicles();
		for (auto& car : cars) {
			auto engine = car->mCOMObject->Find<IEngine>();
			if (!engine) continue;
			if (!engine->IsNOSEngaged()) continue;
			car->SetSpeed(TOMPS(300));
		}
	}
	bool HasTimer() override { return true; }
} E_NOSBoost;

class Effect_CruiseControl : public ChaosEffect {
public:
	Effect_CruiseControl() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Cruise Control";
		fTimerLength = 15;
	}

	void TickFunctionMain(double delta) override {
		// this results in around 60km/h with my cobalt
		if (auto ply = GetLocalPlayerInterface<ISuspension>()) {
			ply->SetWheelAngularVelocity(0, 50.0);
			ply->SetWheelAngularVelocity(1, 50.0);
			ply->SetWheelAngularVelocity(2, 50.0);
			ply->SetWheelAngularVelocity(3, 50.0);
		}
	}
	bool HasTimer() override { return true; }
} E_CruiseControl;

class Effect_Skyfall : public ChaosEffect {
public:
	Effect_Skyfall() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Skyfall";
		fTimerLength = 20;
	}

	void InitFunction() override {
		NoResetCount++;
		if (auto ply = GetLocalPlayerInterface<IRigidBody>()) {
			auto pos = *ply->GetPosition();
			pos.y += 2500;
			ply->SetPosition(&pos);
			ply->SetLinearVelocity(&UMath::Vector3::kZero);
		}
	}
	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayerInterface<IRigidBody>()) {
			auto vel = *ply->GetLinearVelocity();
			if (vel.y < -2) {
				vel.x = 0;
				vel.z = 0;
				ply->SetLinearVelocity(&vel);
			}
		}
	}
	void DeinitFunction() override {
		NoResetCount--;
	}
	bool HasTimer() override { return true; }
} E_Skyfall;

class Effect_HeatSteer : public ChaosEffect {
public:
	Effect_HeatSteer() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Heated Steering";
		sFriendlyName = "Steer To Change Heat Level";
		fTimerLength = 60;
	}

	static inline float DeltaMult = 0.002;

	void InitFunction() override {
		CwoeeHints::AddHint("Turn left to lower your heat level,");
		CwoeeHints::AddHint("Turn right to raise your heat level.");
	}
	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayerInterface<IInput>()) {
			auto heat = GetLocalPlayerInterface<IPerpetrator>()->GetHeat();
			heat += (ply->GetControls()->fSteering * DeltaMult);
			if (heat < 1.0) heat = 1.0;
			if (heat > 10.0) heat = 10.0;
			GetLocalPlayerInterface<IPerpetrator>()->SetHeat(heat);
		}
	}
	bool HasTimer() override { return true; }
} E_HeatSteer;

class Effect_MinSpeed : public ChaosEffect {
public:
	Effect_MinSpeed() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Need For Speed";
		fTimerLength = 60;
		AddToIncompatiblityGroup("top_bar");
		MakeIncompatibleWithFilterGroup("player_godmode");
	}

	static inline float fMinSpeedSlow = TOMPS(100);
	static inline float fMinSpeedMid = TOMPS(150);
	static inline float fMinSpeedFast = TOMPS(200);
	static inline float fMaxSpeed = TOMPS(100);
	static inline float fTextY = 0.05;

	bool bActive = false;
	double fInactiveTimer = 0;
	double fLeewayTimer = 0;
	float fMinSpeed = fMinSpeedSlow;

	void InitFunction() override {
		CwoeeHints::AddHint("Stay above the target speed!");

		bActive = false;
		fInactiveTimer = 0;
		fLeewayTimer = 0;
		fMinSpeed = fMinSpeedMid;
		if (IsInCareerMode()) {
			if (FEDatabase->mUserProfile->TheCareerSettings.CurrentBin >= BIN_BARON) {
				fMinSpeed = fMinSpeedSlow;
			}
			else if (FEDatabase->mUserProfile->TheCareerSettings.CurrentBin <= BIN_BULL) {
				fMinSpeed = fMinSpeedFast;
			}
		}
	}
	void TickFunctionMain(double delta) override {
		if (GetLocalPlayerVehicle()->IsStaging()) return;

		auto speed = GetLocalPlayerInterface<IRigidBody>()->GetSpeed();

		tNyaStringData data;
		data.x = 0.5;
		data.y = fTextY;
		data.size = 0.04;
		data.XCenterAlign = true;
		data.outlinea = 255;
		data.outlinedist = 0.025;
		DrawString(data, std::format("Current Speed: {:.0f}KM/H", TOKMH(speed)));
		data.y += data.size;
		DrawString(data, std::format("Minimum Speed: {:.0f}KM/H", TOKMH(fMinSpeed)));

		if (!bActive) {
			data.y += data.size;
			data.SetColor(200,0,0,255);
			DrawString(data, std::format("Time Remaining: {:.1f}", 5.0 - fInactiveTimer));

			fInactiveTimer += delta;
			if (fInactiveTimer > 5) {
				bActive = true;
				return;
			}
			else {
				EffectInstance->fTimer = fTimerLength;
				if (speed >= fMinSpeed * 1.1) bActive = true;
			}
			return;
		}

		NyaDrawing::CNyaRGBA32 rgb = {190,240,64,255};
		auto speedAmount = (speed - fMinSpeed) / fMaxSpeed;
		if (speedAmount < 0.33) rgb = {200,0,0,255};
		DrawTopBar(speedAmount, rgb);

		if (speedAmount <= 0) {
			data.y += data.size;
			data.SetColor(200,0,0,255);
			DrawString(data, std::format("Time Remaining: {:.1f}", 0.5 - fLeewayTimer));

			fLeewayTimer += delta;
			if (fLeewayTimer > 0.5) {
				GetLocalPlayerInterface<IDamageable>()->Destroy();
			}
		}
		else {
			fLeewayTimer = 0;
		}
	}
	bool HasTimer() override { return true; }
	bool ShouldAbort() override {
		return IsCarDestroyed(GetLocalPlayerVehicle());
	}
	bool CanQuickTrigger() override { return false; }
} E_MinSpeed;

class Effect_PlayerRandomInput : public ChaosEffect {
public:
	Effect_PlayerRandomInput() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "ChatGPT Take The Wheel";
		sFriendlyName = "Randomize Player Input";
		fTimerLength = 20;
	}

	float gas = 0;
	float brake = 0;
	float steer = 0;
	double timer = 3;
	double runTime = 0;

	void InitFunction() override {
		timer = 3;
		runTime = 0;
	}
	void TickFunctionMain(double delta) override {
		runTime += delta;

		tNyaStringData data;
		data.x = 0.5;
		data.y = 0.85;
		data.size = 0.04;
		data.XCenterAlign = true;
		data.outlinea = 255;
		data.outlinedist = 0.025;
		DrawString(data, std::format("Electricity Cost: ${}", (int)(runTime * 1000)));
		data.y += data.size;
		DrawString(data, std::format("Water Used: {:.1f}L", runTime * 500));
	}
	void TickFunction(eChaosHook hook, double delta) override {
		if (hook != HOOK_INPUT) return;

		timer += delta;
		if (timer > 3) {
			gas = PercentageChanceCheck(25) ? 0 : 1;
			brake = 1 - gas;
			steer = (((rand() % 100) / 100.0) - 0.5) * 2;
			timer -= 3;
		}

		if (auto ply = GetLocalPlayerInterface<IInput>()) {
			auto controls = ply->GetControls();
			controls->fSteering = steer;
			controls->fGas = gas;
			controls->fBrake = brake;
			controls->fHandBrake = 0;
			controls->fActionButton = false;
			controls->fNOS = false;
		}
	}
	bool HasTimer() override { return true; }
	bool CanQuickTrigger() override { return false; }
} E_PlayerRandomInput;

class Effect_PlayerForceSpeedbrk : public ChaosEffect {
public:
	Effect_PlayerForceSpeedbrk() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Forced Speedbreaker";
		fTimerLength = 30;
		fUnhideTime = 0;
		AddToIncompatiblityGroup("speedbreaker");
	}

	void TickFunctionMain(double delta) override {
		auto ply = GetLocalPlayer();
		ply->ChargeGameBreaker(1);

		if (!ply->InGameBreaker()) {
			ply->ToggleGameBreaker();
		}
	}
	void DeinitFunction() override {
		auto ply = GetLocalPlayer();
		if (ply->InGameBreaker()) {
			ply->ToggleGameBreaker();
		}
	}
	bool IsAvailable() override { return GetLocalPlayerVehicle()->GetSpeed() > TOMPS(60); }
	bool IsRehideable() override { return true; }
	bool HasTimer() override { return true; }
	bool CanQuickTrigger() override { return false; }
} E_PlayerForceSpeedbrk;