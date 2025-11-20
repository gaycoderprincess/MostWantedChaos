class EffectBase_OpponentConditional : public ChaosEffect {
public:

	EffectBase_OpponentConditional() : ChaosEffect() {
		sName = "(EFFECT BASE) Opponent Conditional";
	}

	bool IsAvailable() override {
		return !VEHICLE_LIST::GetList(VEHICLE_AIRACERS).empty();
	}
	bool IsConditionallyAvailable() override { return true; }
};

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