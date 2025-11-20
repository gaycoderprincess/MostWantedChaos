class EffectBase_PursuitConditional : public ChaosEffect {
public:

	EffectBase_PursuitConditional() : ChaosEffect() {
		sName = "(EFFECT BASE) Pursuit Conditional";
	}

	bool IsAvailable() override {
		if (auto ply = GetLocalPlayerInterface<IPerpetrator>()) {
			return ply->IsBeingPursued();
		}
		return false;
	}
	bool IsConditionallyAvailable() override { return true; }
};

/*class Effect_SpawnCop : public ChaosEffect {
public:
	Effect_SpawnCop() : ChaosEffect() {
		sName = "Spawn Cops";
	}

	void InitFunction() override {
		ICopMgr::mDisableCops = false;
		auto ply = GetLocalPlayerInterface<IRigidBody>();
		UMath::Vector3 fwd;
		ply->GetForwardVector(&fwd);
		ICopMgr::mInstance->SpawnCop(ply->GetPosition(), &fwd, "copghost", false, false);
		ICopMgr::mInstance->SpawnCop(ply->GetPosition(), &fwd, "copmidsize", true, false);
		ICopMgr::mInstance->SpawnCop(ply->GetPosition(), &fwd, "copgto", true, true);
		ICopMgr::mInstance->SpawnCop(ply->GetPosition(), &fwd, "copsporthench", false, true);
	}
} E_SpawnCop;*/

class Effect_Pursuit : public ChaosEffect {
public:
	Effect_Pursuit() : ChaosEffect() {
		sName = "Trigger Police Pursuit";
	}

	void TickFunction(double delta) override {
		ICopMgr::mDisableCops = false;
		ICopMgr::mInstance->PursueAtHeatLevel(GetLocalPlayerInterface<IPerpetrator>()->GetHeat());
	}
	bool IsAvailable() override {
		if (auto ply = GetLocalPlayerInterface<IPerpetrator>()) {
			return !ply->IsBeingPursued();
		}
		return true;
	}
	bool IsConditionallyAvailable() override { return true; }
} E_Pursuit;

/*class Effect_ClearCostToState : public EffectBase_PursuitConditional {
public:
	Effect_ClearCostToState() : EffectBase_PursuitConditional() {
		sName = "Reset Cost To State";
	}

	void TickFunction(double delta) override {
		GetLocalPlayerInterface<IPerpetrator>()->SetCostToState(0);
	}
} E_ClearCostToState;

class Effect_AddCostToState : public EffectBase_PursuitConditional {
public:
	Effect_AddCostToState() : EffectBase_PursuitConditional() {
		sName = "Add 100K Cost To State";
	}

	void TickFunction(double delta) override {
		GetLocalPlayerInterface<IPerpetrator>()->SetCostToState(100000);
	}
} E_AddCostToState;*/