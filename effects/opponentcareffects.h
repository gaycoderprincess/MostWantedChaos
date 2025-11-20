class Effect_BlowEngineOpponents : public EffectBase_OpponentConditional {
public:
	Effect_BlowEngineOpponents() : EffectBase_OpponentConditional() {
		sName = "Blow Opponents' Engines";
	}

	void InitFunction() override {
		auto& list = VEHICLE_LIST::GetList(VEHICLE_AIRACERS);
		for (int i = 0; i < list.size(); i++) {
			auto racer = list[i];
			auto engine = racer->mCOMObject->Find<IEngineDamage>();
			if (!engine) continue;
			engine->Sabotage(1);
		}
	}
} E_BlowEngineOpponents;

class Effect_BlowEngineOpponents1 : public EffectBase_OpponentConditional {
public:
	Effect_BlowEngineOpponents1() : EffectBase_OpponentConditional() {
		sName = "Blow One Opponent's Engine";
	}

	void InitFunction() override {
		auto& list = VEHICLE_LIST::GetList(VEHICLE_AIRACERS);
		int i = rand() % list.size();
		auto racer = list[i];
		auto engine = racer->mCOMObject->Find<IEngineDamage>();
		if (!engine) return;
		engine->Sabotage(1);
	}
} E_BlowEngineOpponents1;

class Effect_LaunchCarFwdOpponents : public EffectBase_OpponentConditional {
public:
	Effect_LaunchCarFwdOpponents() : EffectBase_OpponentConditional() {
		sName = "Launch Opponents Forward";
	}

	void InitFunction() override {
		auto& list = VEHICLE_LIST::GetList(VEHICLE_AIRACERS);
		for (int i = 0; i < list.size(); i++) {
			auto racer = list[i];
			racer->SetSpeed(TOMPS(400));
		}
	}
} E_LaunchCarFwdOpponents;

class Effect_LaunchCarBwdOpponents : public EffectBase_OpponentConditional {
public:
	Effect_LaunchCarBwdOpponents() : EffectBase_OpponentConditional() {
		sName = "Launch Opponents Backwards";
	}

	void InitFunction() override {
		auto& list = VEHICLE_LIST::GetList(VEHICLE_AIRACERS);
		for (int i = 0; i < list.size(); i++) {
			auto racer = list[i];
			racer->SetSpeed(TOMPS(-200));
		}
	}
} E_LaunchCarBwdOpponents;

class Effect_LaunchCarSideOpponents : public EffectBase_OpponentConditional {
public:
	Effect_LaunchCarSideOpponents() : EffectBase_OpponentConditional() {
		sName = "Launch Opponents Sideways";
	}

	void InitFunction() override {
		auto& list = VEHICLE_LIST::GetList(VEHICLE_AIRACERS);
		for (int i = 0; i < list.size(); i++) {
			auto racer = list[i];
			auto ply = racer->mCOMObject->Find<IRigidBody>();
			UMath::Vector3 side;
			ply->GetRightVector(&side);
			side.x *= TOMPS(200);
			side.y *= TOMPS(200);
			side.z *= TOMPS(200);
			ply->SetLinearVelocity(&side);
		}
	}
} E_LaunchCarSideOpponents;

class Effect_LaunchCarUpOpponents : public EffectBase_OpponentConditional {
public:
	Effect_LaunchCarUpOpponents() : EffectBase_OpponentConditional() {
		sName = "Launch Opponents Up";
	}

	void InitFunction() override {
		auto& list = VEHICLE_LIST::GetList(VEHICLE_AIRACERS);
		for (int i = 0; i < list.size(); i++) {
			auto racer = list[i];
			auto ply = racer->mCOMObject->Find<IRigidBody>();
			if (!ply) continue;
			UMath::Vector3 vel = *ply->GetLinearVelocity();
			vel.y = TOMPS(200);
			ply->SetLinearVelocity(&vel);
		}
	}
} E_LaunchCarUpOpponents;

class Effect_SpinningOpponents : public EffectBase_OpponentConditional {
public:
	Effect_SpinningOpponents() : EffectBase_OpponentConditional() {
		sName = "Spinning Opponents";
		fTimerLength = 30;
	}

	void TickFunction(double delta) override {
		auto& list = VEHICLE_LIST::GetList(VEHICLE_AIRACERS);
		for (int i = 0; i < list.size(); i++) {
			auto car = list[i];
			auto rb = car->mCOMObject->Find<IRigidBody>();
			auto vel = *rb->GetAngularVelocity();
			vel.y += 25 * delta;
			rb->SetAngularVelocity(&vel);
		}
	}
	bool HasTimer() override { return true; }
} E_SpinningOpponents;

class Effect_RubberbandOpponents : public EffectBase_OpponentConditional {
public:
	Effect_RubberbandOpponents() : EffectBase_OpponentConditional() {
		sName = "FUCKING RUBBERBAND";
		fTimerLength = 45;
		IncompatibilityGroup = Attrib::StringHash32("rubberband");
	}

	static float __thiscall GetCatchupCheatHooked(ICheater* pThis) {
		return 100;
	}

	void InitFunction() override {
		NyaHookLib::Patch(0x8925C8, &GetCatchupCheatHooked);
	}
	void DeinitFunction() override {
		NyaHookLib::Patch(0x8925C8, 0x409390);
	}
	bool HasTimer() override { return true; }
} E_RubberbandOpponents;

class Effect_NoRubberbandOpponents : public EffectBase_OpponentConditional {
public:
	Effect_NoRubberbandOpponents() : EffectBase_OpponentConditional() {
		sName = "Disable Rubberbanding";
		fTimerLength = 90;
		IncompatibilityGroup = Attrib::StringHash32("rubberband");
	}

	static float __thiscall GetCatchupCheatHooked(ICheater* pThis) {
		return 0;
	}

	void InitFunction() override {
		NyaHookLib::Patch(0x8925C8, &GetCatchupCheatHooked);
	}
	void DeinitFunction() override {
		NyaHookLib::Patch(0x8925C8, 0x409390);
	}
	bool HasTimer() override { return true; }
} E_NoRubberbandOpponents;

class Effect_SpikeAllOpponents : public EffectBase_OpponentConditional {
public:
	Effect_SpikeAllOpponents() : EffectBase_OpponentConditional() {
		sName = "Puncture Opponents' Tires";
	}

	void InitFunction() override {
		auto& list = VEHICLE_LIST::GetList(VEHICLE_AIRACERS);
		for (int i = 0; i < list.size(); i++) {
			auto car = list[i];
			auto ply = car->mCOMObject->Find<ISpikeable>();
			ply->Puncture(0);
			ply->Puncture(1);
			ply->Puncture(2);
			ply->Puncture(3);
		}
	}
} E_SpikeAllOpponents;