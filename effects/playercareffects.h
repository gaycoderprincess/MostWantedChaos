class Effect_StopCar : public ChaosEffect {
public:
	Effect_StopCar() : ChaosEffect() {
		sName = "Stop Car";
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
		sName = "Launch Car Forward";
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
		sName = "Launch Car Backwards";
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayerVehicle()) {
			ply->SetSpeed(TOMPS(-200));
		}
	}
} E_LaunchCarBwd;

class Effect_LaunchCarUp : public ChaosEffect {
public:
	Effect_LaunchCarUp() : ChaosEffect() {
		sName = "Launch Car Up";
	}

	void InitFunction() override {
		if (auto ply = GetLocalPlayerInterface<IRigidBody>()) {
			UMath::Vector3 vel = *ply->GetLinearVelocity();
			vel.y = TOMPS(200);
			ply->SetLinearVelocity(&vel);
		}
	}
} E_LaunchCarUp;

class Effect_SpinCar : public ChaosEffect {
public:
	Effect_SpinCar() : ChaosEffect() {
		sName = "Spin Car Out";
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
		sName = "Spin Car Rapidly";
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
	}

	void TickFunction(double delta) override {
		if (auto ply = GetLocalPlayer()) {
			ply->ChargeGameBreaker(1);
		}
	}
	bool HasTimer() override { return true; }
} E_InfGameBreaker;

class Effect_InfNitro : public ChaosEffect {
public:
	Effect_InfNitro() : ChaosEffect() {
		sName = "Infinite Nitro";
		IncompatibilityGroup = Attrib::StringHash32("nitro");
	}

	void TickFunction(double delta) override {
		if (auto ply = GetLocalPlayerEngine()) {
			ply->ChargeNOS(1);
		}
	}
	bool IsAvailable() override {
		if (auto ply = GetLocalPlayerEngine()) {
			return ply->HasNOS();
		}
		return false;
	}
	bool HasTimer() override { return true; }
	bool IsConditionallyAvailable() override { return true; }
} E_InfNitro;

class Effect_NoGameBreaker : public ChaosEffect {
public:
	Effect_NoGameBreaker() : ChaosEffect() {
		sName = "Disable Speedbreaker";
		IncompatibilityGroup = Attrib::StringHash32("speedbreaker");
	}

	void TickFunction(double delta) override {
		if (auto ply = GetLocalPlayer()) {
			ply->ResetGameBreaker(false);
		}
	}
	bool HasTimer() override { return true; }
} E_NoGameBreaker;

class Effect_NoNitro : public ChaosEffect {
public:
	Effect_NoNitro() : ChaosEffect() {
		sName = "Disable Nitro";
		IncompatibilityGroup = Attrib::StringHash32("nitro");
	}

	void TickFunction(double delta) override {
		if (auto ply = GetLocalPlayerEngine()) {
			ply->ChargeNOS(-1);
		}
	}
	bool IsAvailable() override {
		if (auto ply = GetLocalPlayerEngine()) {
			return ply->HasNOS();
		}
		return false;
	}
	bool HasTimer() override { return true; }
	bool IsConditionallyAvailable() override { return true; }
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

class Effect_BlowEngine : public ChaosEffect {
public:
	Effect_BlowEngine() : ChaosEffect() {
		sName = "Blow Car Engine";
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
		EAXDispatch::Report911(SoundAI::mInstance->mDispatch, Csis::Type_pursuit_type_Reckless);
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
			Physics::Tunings tune = *ply->GetTunings();
			tune.Value[Physics::Tunings::AERODYNAMICS] = 50;
			ply->SetTunings(&tune);
		}
	}
	void DeinitFunction() override {
		if (auto ply = GetLocalPlayerVehicle()) {
			Physics::Tunings tune = *ply->GetTunings();
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
			Physics::Tunings tune = *ply->GetTunings();
			tune.Value[Physics::Tunings::RIDEHEIGHT] = -2;
			ply->SetTunings(&tune);
		}
	}
	void DeinitFunction() override {
		if (auto ply = GetLocalPlayerVehicle()) {
			Physics::Tunings tune = *ply->GetTunings();
			tune.Value[Physics::Tunings::RIDEHEIGHT] = 0;
			ply->SetTunings(&tune);
		}
	}
	bool HasTimer() override { return true; }
} E_PlayerCarTuneHeight;

class Effect_PlayerCarTuneHeight2 : public ChaosEffect {
public:
	Effect_PlayerCarTuneHeight2() : ChaosEffect() {
		sName = "Super Stanced Suspension";
		fTimerLength = 60;
		IncompatibilityGroup = Attrib::StringHash32("suspension");
	}

	void TickFunction(double delta) override {
		if (auto ply = GetLocalPlayerVehicle()) {
			Physics::Tunings tune = *ply->GetTunings();
			tune.Value[Physics::Tunings::RIDEHEIGHT] = -10;
			ply->SetTunings(&tune);
		}
	}
	void DeinitFunction() override {
		if (auto ply = GetLocalPlayerVehicle()) {
			Physics::Tunings tune = *ply->GetTunings();
			tune.Value[Physics::Tunings::RIDEHEIGHT] = 0;
			ply->SetTunings(&tune);
		}
	}
	bool HasTimer() override { return true; }
} E_PlayerCarTuneHeight2;

class Effect_PlayerCarTuneHeight3 : public ChaosEffect {
public:
	Effect_PlayerCarTuneHeight3() : ChaosEffect() {
		sName = "Monster Truck Suspension";
		fTimerLength = 60;
		IncompatibilityGroup = Attrib::StringHash32("suspension");
	}

	void TickFunction(double delta) override {
		if (auto ply = GetLocalPlayerVehicle()) {
			Physics::Tunings tune = *ply->GetTunings();
			tune.Value[Physics::Tunings::RIDEHEIGHT] = 25;
			ply->SetTunings(&tune);
		}
	}
	void DeinitFunction() override {
		if (auto ply = GetLocalPlayerVehicle()) {
			Physics::Tunings tune = *ply->GetTunings();
			tune.Value[Physics::Tunings::RIDEHEIGHT] = 0;
			ply->SetTunings(&tune);
		}
	}
	bool HasTimer() override { return true; }
} E_PlayerCarTuneHeight3;