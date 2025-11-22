class Effect_StopCar : public ChaosEffect {
public:
	Effect_StopCar() : ChaosEffect() {
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
	Effect_LaunchCarFwd() : ChaosEffect() {
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
	Effect_LaunchCarBwd() : ChaosEffect() {
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
	Effect_LaunchCarSide() : ChaosEffect() {
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
	Effect_LaunchCarUp() : ChaosEffect() {
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
	Effect_LaunchCarDown() : ChaosEffect() {
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
	Effect_SpinCar() : ChaosEffect() {
		sName = "Spin Player Out";
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayerInterface<IRigidBody>()) {
			auto vel = *ply->GetAngularVelocity();
			vel.y = 4;
			ply->SetAngularVelocity(&vel);
		}
	}
} E_SpinCar;

class Effect_SpinCar2 : public ChaosEffect {
public:
	Effect_SpinCar2() : ChaosEffect() {
		sName = "Spin Player Rapidly";
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayerInterface<IRigidBody>()) {
			auto vel = *ply->GetAngularVelocity();
			vel.y = TOMPS(200);
			ply->SetAngularVelocity(&vel);
		}
	}
} E_SpinCar2;

class Effect_InfGameBreaker : public ChaosEffect {
public:
	Effect_InfGameBreaker() : ChaosEffect() {
		sName = "Infinite Speedbreaker";
		IncompatibilityGroup = Attrib::StringHash32("speedbreaker");
		fTimerLength = 60;
	}

	void TickFunction(double delta) override {
		if (auto ply = GetLocalPlayer()) {
			ply->ChargeGameBreaker(1);
		}
	}
	bool HasTimer() override { return true; }
} E_InfGameBreaker;

class Effect_InfNitro : public EffectBase_PlayerCarHasNitro {
public:
	Effect_InfNitro() : EffectBase_PlayerCarHasNitro() {
		sName = "Infinite Nitro";
		IncompatibilityGroup = Attrib::StringHash32("nitro");
		fTimerLength = 60;
	}

	void TickFunction(double delta) override {
		if (auto ply = GetLocalPlayerEngine()) {
			ply->ChargeNOS(1);
		}
	}
	bool HasTimer() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
} E_InfNitro;

class Effect_NoGameBreaker : public ChaosEffect {
public:
	Effect_NoGameBreaker() : ChaosEffect() {
		sName = "Disable Speedbreaker";
		fTimerLength = 90;
		IncompatibilityGroup = Attrib::StringHash32("speedbreaker");
	}

	void TickFunction(double delta) override {
		if (auto ply = GetLocalPlayer()) {
			ply->ResetGameBreaker(false);
		}
	}
	bool HasTimer() override { return true; }
} E_NoGameBreaker;

class Effect_NoNitro : public EffectBase_PlayerCarHasNitro {
public:
	Effect_NoNitro() : EffectBase_PlayerCarHasNitro() {
		sName = "Disable Nitro";
		fTimerLength = 60;
		IncompatibilityGroup = Attrib::StringHash32("nitro");
	}

	void TickFunction(double delta) override {
		if (auto ply = GetLocalPlayerEngine()) {
			ply->ChargeNOS(-1);
		}
	}
	bool HasTimer() override { return true; }
	bool AbortOnConditionFailed() override { return true; }
} E_NoNitro;

class Effect_SetHeat1 : public ChaosEffect {
public:
	Effect_SetHeat1() : ChaosEffect() {
		sName = "Reset Heat Level";
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayerInterface<IPerpetrator>()) {
			ply->SetHeat(1);
		}
	}
} E_SetHeat1;

class Effect_SetHeat5 : public ChaosEffect {
public:
	Effect_SetHeat5() : ChaosEffect() {
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
} E_SetHeat5;

class Effect_SetHeat6 : public ChaosEffect {
public:
	Effect_SetHeat6() : ChaosEffect() {
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
} E_SetHeat6;

class Effect_SetHeatDown : public ChaosEffect {
public:
	Effect_SetHeatDown() : ChaosEffect() {
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
	Effect_SetHeatUp() : ChaosEffect() {
		sName = "Increase Heat Level";
	}

	void InitFunction() override {
		if (auto max = GetMaxHeat()) {
			if (*max < 6) *max = 6;
		}
		if (auto ply = GetLocalPlayerInterface<IPerpetrator>()) {
			ply->SetHeat(ply->GetHeat() + 1);
		}
	}
} E_SetHeatUp;

class Effect_BlowEngine : public ChaosEffect {
public:
	Effect_BlowEngine() : ChaosEffect() {
		sName = "Blow Player's Engine";
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayerInterface<IEngineDamage>()) {
			ply->Sabotage(3);
		}
	}
} E_BlowEngine;

class Effect_AutoDrive : public ChaosEffect {
public:
	Effect_AutoDrive() : ChaosEffect() {
		sName = "Auto-Drive";
		IncompatibilityGroup = Attrib::StringHash32("autodrive");
		fTimerLength = 30;
	}

	void TickFunction(double delta) override {
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

class Effect_AutoDrive2 : public ChaosEffect {
public:
	Effect_AutoDrive2() : ChaosEffect() {
		sName = "Auto-Drive (Traffic)";
		fTimerLength = 15;
		IncompatibilityGroup = Attrib::StringHash32("autodrive");
	}

	void TickFunction(double delta) override {
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
} E_AutoDrive2;

/*class Effect_911 : public ChaosEffect {
public:
	Effect_911() : ChaosEffect() {
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

// todo this one could use audio
class Effect_PlayerCarSpin : public ChaosEffect {
public:
	Effect_PlayerCarSpin() : ChaosEffect() {
		sName = "Spain but the A is silent";
		sFriendlyName = "Make Player Spin";
		fTimerLength = 10;
	}

	void TickFunction(double delta) override {
		if (auto ply = GetLocalPlayerInterface<IRigidBody>()) {
			auto vel = *ply->GetAngularVelocity();
			vel.y = 2;
			ply->SetAngularVelocity(&vel);
		}
	}
	bool HasTimer() override { return true; }
} E_PlayerCarSpin;

class Effect_PlayerCarTuneAero : public ChaosEffect {
public:
	Effect_PlayerCarTuneAero() : ChaosEffect() {
		sName = "Aerodynamics +100";
		fTimerLength = 15;
		IncompatibilityGroup = Attrib::StringHash32("aerodynamics");
	}

	void TickFunction(double delta) override {
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

class Effect_PlayerCarTuneHeight : public ChaosEffect {
public:
	Effect_PlayerCarTuneHeight() : ChaosEffect() {
		sName = "Stanced Suspension";
		fTimerLength = 60;
		IncompatibilityGroup = Attrib::StringHash32("suspension");
	}

	void TickFunction(double delta) override {
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
} E_PlayerCarTuneHeight;

/*class Effect_PlayerCarTuneHeight2 : public ChaosEffect {
public:
	Effect_PlayerCarTuneHeight2() : ChaosEffect() {
		sName = "Super Stanced Suspension";
		fTimerLength = 60;
		IncompatibilityGroup = Attrib::StringHash32("suspension");
	}

	void TickFunction(double delta) override {
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

class Effect_PlayerCarTuneHeight3 : public ChaosEffect {
public:
	Effect_PlayerCarTuneHeight3() : ChaosEffect() {
		sName = "Monster Truck Suspension";
		fTimerLength = 60;
		IncompatibilityGroup = Attrib::StringHash32("suspension");
	}

	void TickFunction(double delta) override {
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

class Effect_PlayerCarTuneHeight4 : public ChaosEffect {
public:
	double state = 0;
	bool goBack = false;

	Effect_PlayerCarTuneHeight4() : ChaosEffect() {
		sName = "Bouncy Suspension";
		fTimerLength = 30;
		IncompatibilityGroup = Attrib::StringHash32("suspension");
	}

	void InitFunction() override {
		state = 0;
	}
	void TickFunction(double delta) override {
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
	Effect_PlayerCarTuneNitro() : EffectBase_PlayerCarHasNitro() {
		sName = "Weak Nitro";
		fTimerLength = 60;
		IncompatibilityGroup = Attrib::StringHash32("nitro_tune");
	}

	void TickFunction(double delta) override {
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
	bool AbortOnConditionFailed() override { return true; }
} E_PlayerCarTuneNitro;

/*class Effect_PlayerCarTuneNitro2 : public EffectBase_PlayerCarHasNitro {
public:
	Effect_PlayerCarTuneNitro2() : EffectBase_PlayerCarHasNitro() {
		sName = "Strong Nitro";
		fTimerLength = 60;
		IncompatibilityGroup = Attrib::StringHash32("nitro_tune");
	}

	void TickFunction(double delta) override {
		if (auto ply = GetLocalPlayerVehicle()) {
			Physics::Tunings tune = ply->GetTunings() ? *ply->GetTunings() : Physics::Tunings();
			tune.Value[Physics::Tunings::NOS] = 3;
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
	bool AbortOnConditionFailed() override { return true; }
} E_PlayerCarTuneNitro2;*/

class Effect_PlayerCarReset : public ChaosEffect {
public:
	Effect_PlayerCarReset() : ChaosEffect() {
		sName = "Reset Player";
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayerInterface<IResetable>()) {
			ply->ResetVehicle(true);
		}
	}
} E_PlayerCarReset;

class Effect_PlayerCarSpike1 : public ChaosEffect {
public:
	Effect_PlayerCarSpike1() : ChaosEffect() {
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
	Effect_PlayerCarSpikeAll() : ChaosEffect() {
		sName = "Puncture Player's Tires";
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayerInterface<ISpikeable>()) {
			ply->Puncture(0);
			ply->Puncture(1);
			ply->Puncture(2);
			ply->Puncture(3);
		}
	}
} E_PlayerCarSpikeAll;

class Effect_FixPlayerCar : public ChaosEffect {
public:
	Effect_FixPlayerCar() : ChaosEffect() {
		sName = "Fix Player's Tires";
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayerInterface<IDamageable>()) {
			ply->ResetDamage();
		}
	}
} E_FixPlayerCar;

class Effect_NoReset : public ChaosEffect {
public:
	Effect_NoReset() : ChaosEffect() {
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

class Effect_ResetImmune : public ChaosEffect {
public:
	Effect_ResetImmune() : ChaosEffect() {
		sName = "Reset Immunity From Cops";
		fTimerLength = 120;
	}

	void InitFunction() override {
		ManualResetImmunity = true;
	}
	void DeinitFunction() override {
		ManualResetImmunity = false;
	}
	bool HasTimer() override { return true; }
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

		auto v = (NyaVec3*)source->GetPosition();
		auto c = (NyaVec3*)otherCar->GetPosition();
		if ((*v - *c).length() < 15) {
			auto dir = (*c - *v);
			dir.Normalize();
			dir *= 15;
			auto newPos = *v;
			newPos += dir;
			otherCar->SetPosition((UMath::Vector3*)&newPos);
		}
	}
}

class Effect_CarMagnet : public ChaosEffect {
public:
	Effect_CarMagnet() : ChaosEffect() {
		sName = "Player Car Magnet";
		fTimerLength = 30;
	}

	void TickFunction(double delta) override {
		DoCarMagnet(GetLocalPlayerVehicle(), delta);
	}
	bool HasTimer() override { return true; }
} E_CarMagnet;

class Effect_CarForcefield : public ChaosEffect {
public:
	Effect_CarForcefield() : ChaosEffect() {
		sName = "Player Car Forcefield";
		fTimerLength = 60;
	}

	void TickFunction(double delta) override {
		DoCarForcefield(GetLocalPlayerVehicle());
	}
	bool HasTimer() override { return true; }
} E_CarForcefield;

class Effect_NoInput : public ChaosEffect {
public:
	Effect_NoInput() : ChaosEffect() {
		sName = "Disable Player Input";
		fTimerLength = 15;
	}

	void TickFunction(double delta) override {
		if (auto ply = GetLocalPlayerInterface<IInput>()) {
			ply->ClearInput();
		}
	}
	bool HasTimer() override { return true; }
} E_NoInput;

class Effect_PlayerCarGear1 : public ChaosEffect {
public:
	Effect_PlayerCarGear1() : ChaosEffect() {
		sName = "Force Player To Gear 1";
		fTimerLength = 15;
		IncompatibilityGroup = Attrib::StringHash32("transmission");
	}

	void TickFunction(double delta) override {
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
	Effect_PlayerCarGearTop() : ChaosEffect() {
		sName = "Top Gear";
		sFriendlyName = "Force Player To Top Gear";
		fTimerLength = 30;
		IncompatibilityGroup = Attrib::StringHash32("transmission");
	}

	void TickFunction(double delta) override {
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
	Effect_PlayerCarGearR() : ChaosEffect() {
		sName = "Force Player To Reverse Gear";
		fTimerLength = 15;
		IncompatibilityGroup = Attrib::StringHash32("transmission");
	}

	void TickFunction(double delta) override {
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

class Effect_SetCarTRAFPIZZA : public ChaosEffect {
public:
	Effect_SetCarTRAFPIZZA() : ChaosEffect() {
		sName = "Change Car To TRAFPIZZA";
	}

	void InitFunction() override {
		// copy tunings from the current car if applicable, all junkman if not
		//auto customization = CreateStockCustomizations(Attrib::StringHash32("cs_clio_trafpizza"));
		//if (auto car = GetCurrentCareerCar()) {
		//	auto srcCustomization = FEPlayerCarDB::GetCustomizationRecordByHandle(&FEDatabase->mUserProfile->PlayersCarStable, car->CareerHandle);
		//	if (!srcCustomization) return;
		//	customization.InstalledPhysics = srcCustomization->InstalledPhysics;
		//}
		//else {
		//	for (int i = 0; i < Physics::Upgrades::Package::PUT_MAX; i++) {
		//		customization.InstalledPhysics.Part[i] = 4;
		//	}
		//	customization.InstalledPhysics.Junkman = 0xFFFFFFFF;
		//}
		ChangePlayerCarInWorld(Attrib::StringHash32("cs_clio_trafpizza"), nullptr);
	}
} E_SetCarTRAFPIZZA;

class Effect_SetCarRazor : public EffectBase_SafelyChangePlayerCar {
public:
	Effect_SetCarRazor() : EffectBase_SafelyChangePlayerCar() {
		sName = "Change Car To Razor's Mustang";
	}

	void InitFunction() override {
		auto car = CreatePinkSlipPreset("RAZORMUSTANG");
		ChangePlayerCarInWorld(Attrib::StringHash32("mustanggt"), FEPlayerCarDB::GetCustomizationRecordByHandle(&FEDatabase->mUserProfile->PlayersCarStable, car->Customization));
	}
} E_SetCarRazor;

class Effect_SetCarRandom : public EffectBase_SafelyChangePlayerCar {
public:
	Effect_SetCarRandom() : EffectBase_SafelyChangePlayerCar() {
		sName = "Change Car To Random Model";
	}

	void InitFunction() override {
		std::vector<FECarRecord*> validCars;
		auto cars = &FEDatabase->mUserProfile->PlayersCarStable;
		for (auto& car : cars->CarTable) {
			if (car.Handle == 0xFFFFFFFF) continue;
			validCars.push_back(&car);
		}
		auto car = validCars[rand()%validCars.size()];
		auto pCustomization = FEPlayerCarDB::GetCustomizationRecordByHandle(cars, car->Customization);
		auto customization = pCustomization ? *pCustomization : CreateRandomCustomizations(car->VehicleKey);
		ChangePlayerCarInWorld(car->VehicleKey, &customization);
	}
} E_SetCarRandom;

class Effect_SetCarMassInf : public ChaosEffect {
public:
	Effect_SetCarMassInf() : ChaosEffect() {
		sName = "Infinite Player Mass";
		fTimerLength = 90;
	}

	void TickFunction(double delta) override {
		if (auto ply = GetLocalPlayerInterface<IRBVehicle>()) {
			ply->SetCollisionMass(10000);
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
	Effect_SetCarMass0() : ChaosEffect() {
		sName = "Zero Player Mass";
		fTimerLength = 60;
	}

	void TickFunction(double delta) override {
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

class Effect_Piggyback : public EffectBase_ActiveCarsConditional {
public:
	Effect_Piggyback() : EffectBase_ActiveCarsConditional() {
		sName = "Piggyback Ride";
		sFriendlyName = "Place Player Onto Closest Car";
		fTimerLength = 15;
	}

	void TickFunction(double delta) override {
		if (auto target = GetClosestActiveVehicle(GetLocalPlayerVehicle())) {
			auto playerPos = *GetLocalPlayerVehicle()->GetPosition();
			auto targetPos = *target->GetPosition();
			playerPos.x = targetPos.x;
			playerPos.z = targetPos.z;
			if (playerPos.y < targetPos.y) playerPos.y = targetPos.y + 3;
			GetLocalPlayerInterface<IRigidBody>()->SetPosition(&playerPos);
		}
	}
	bool HasTimer() override { return true; }
	bool IsRehideable() override { return true; }
} E_Piggyback;

class Effect_LockPlayer : public ChaosEffect {
public:
	UMath::Vector3 position;

	Effect_LockPlayer() : ChaosEffect() {
		sName = "Lock Player In Place";
		fTimerLength = 10;
	}

	void InitFunction() override {
		position = *GetLocalPlayerSimable()->GetPosition();
	}
	void TickFunction(double delta) override {
		GetLocalPlayerInterface<IRigidBody>()->SetPosition(&position);
	}
	bool HasTimer() override { return true; }
} E_LockPlayer;

class Effect_LockPlayerOrient : public ChaosEffect {
public:
	UMath::Vector4 orient;

	Effect_LockPlayerOrient() : ChaosEffect() {
		sName = "Lock Player Rotation";
		fTimerLength = 10;
	}

	void InitFunction() override {
		orient = *GetLocalPlayerInterface<IRigidBody>()->GetOrientation();
	}
	void TickFunction(double delta) override {
		GetLocalPlayerInterface<IRigidBody>()->SetOrientation(&orient);
	}
	bool HasTimer() override { return true; }
} E_LockPlayerOrient;

class Effect_SwapPlayerWithOpponent : public EffectBase_OpponentInRaceOrRoamingConditional {
public:

	Effect_SwapPlayerWithOpponent() : EffectBase_OpponentInRaceOrRoamingConditional() {
		sName = "Swap Places With Random Opponent";
	}

	void InitFunction() override {
		auto playerRB =  GetLocalPlayerInterface<IRigidBody>();
		auto playerPos = *playerRB->GetPosition();
		auto playerOrient = *playerRB->GetOrientation();
		auto playerVel = *playerRB->GetLinearVelocity();
		auto playerAVel = *playerRB->GetAngularVelocity();

		auto cars = GetActiveVehicles(DRIVER_RACER);
		auto targetRB = cars[rand()%cars.size()]->mCOMObject->Find<IRigidBody>();
		auto targetPos = *targetRB->GetPosition();
		auto targetOrient = *targetRB->GetOrientation();
		auto targetVel = *targetRB->GetLinearVelocity();
		auto targetAVel = *targetRB->GetAngularVelocity();

		playerRB->SetPosition(&targetPos);
		playerRB->SetOrientation(&targetOrient);
		playerRB->SetLinearVelocity(&targetVel);
		playerRB->SetAngularVelocity(&targetAVel);
		targetRB->SetPosition(&playerPos);
		targetRB->SetOrientation(&playerOrient);
		targetRB->SetLinearVelocity(&playerVel);
		targetRB->SetAngularVelocity(&playerAVel);
	}
	bool AbortOnConditionFailed() override { return true; }
} E_SwapPlayerWithOpponent;

class Effect_PlayerResetTransform : public ChaosEffect {
public:
	Effect_PlayerResetTransform() : ChaosEffect() {
		sName = "My Planet Needs Me";
		sFriendlyName = "Disable Player Physics";
		fTimerLength = 15;
		fUnhideTime = 1;
	}

	void TickFunction(double delta) override {
		if (auto ply = GetLocalPlayerVehicle()) {
			auto playerRB = GetLocalPlayerInterface<IRigidBody>();
			auto playerPos = *playerRB->GetPosition();
			auto playerOrient = *playerRB->GetOrientation();
			auto playerVel = *playerRB->GetLinearVelocity();
			auto playerAVel = *playerRB->GetAngularVelocity();
			if ((*(NyaVec3*)&playerVel).length() < TOMPS(20)) return;

			auto pos = *ply->GetPosition();
			auto fwd = *GetLocalPlayerInterface<ICollisionBody>()->GetForwardVector();
			ply->SetVehicleOnGround(&pos, &fwd);

			//playerRB->SetPosition(&playerPos);
			playerRB->SetOrientation(&playerOrient);
			playerRB->SetLinearVelocity(&playerVel);
			playerRB->SetAngularVelocity(&playerAVel);
		}
	}
	bool HasTimer() override { return true; }
	bool IsAvailable() override {
		if (auto ply = GetLocalPlayerVehicle()) {
			auto playerRB = GetLocalPlayerInterface<IRigidBody>();
			auto playerVel = *playerRB->GetLinearVelocity();
			if ((*(NyaVec3*)&playerVel).length() < TOMPS(60)) return false;
			return true;
		}
		return false;
	}
	bool IsConditionallyAvailable() override { return true; }
} E_PlayerResetTransform;

class Effect_PlayerNoBrakes : public ChaosEffect {
public:
	Effect_PlayerNoBrakes() : ChaosEffect() {
		sName = "They Just Slow Us Down";
		sFriendlyName = "Disable Player Brakes";
		fTimerLength = 60;
	}

	void TickFunction(double delta) override {
		if (auto ply = GetLocalPlayerInterface<IInput>()) {
			ply->SetControlBrake(0);
		}
	}
	bool HasTimer() override { return true; }
} E_PlayerNoBrakes;