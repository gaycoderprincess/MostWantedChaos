class Effect_NoTirePhysics : public ChaosEffect {
public:
	Effect_NoTirePhysics() : ChaosEffect() {
		sName = "Break Tire Physics";
		fTimerLength = 15;
	}

	void InitFunction() override {
		NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x6716D7, 0x789D41);
	}
	void DeinitFunction() override {
		NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x6716D7, 0x789CC0);
	}
	bool HasTimer() override { return true; }
} E_NoTirePhysics;

class Effect_NoWallCollision : public ChaosEffect {
public:
	Effect_NoWallCollision() : ChaosEffect() {
		sName = "Disable Wall Collision";
		fTimerLength = 90;
	}

	// RigidBody::DoWorldCollisions
	void InitFunction() override {
		NoResetCount++;
		NyaHookLib::Patch<uint8_t>(0x6BB796, 0xEB);
	}
	void DeinitFunction() override {
		NoResetCount--;
		NyaHookLib::Patch<uint8_t>(0x6BB796, 0x75);
	}
	bool HasTimer() override { return true; }
} E_NoWallCollision;

class Effect_NoCollision : public ChaosEffect {
public:
	Effect_NoCollision() : ChaosEffect() {
		sName = "Disable World Collision";
		fTimerLength = 30;
	}

	// RigidBody::UpdateCollider
	void InitFunction() override {
		NoResetCount++;
		NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x6BB780, 0x699AF8);
	}
	void DeinitFunction() override {
		NoResetCount--;
		NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x6BB780, 0x699A90);
	}
	bool HasTimer() override { return true; }
} E_NoCollision;

class Effect_NoCarCarCollision : public ChaosEffect {
public:
	Effect_NoCarCarCollision() : ChaosEffect() {
		sName = "Disable Car To Car Collision";
		fTimerLength = 90;
	}

	void TickFunction(double delta) override {
		if (auto ply = GetLocalPlayerInterface<IRBVehicle>()) {
			ply->EnableObjectCollisions(false);
		}
	}
	void DeinitFunction() override {
		if (auto ply = GetLocalPlayerInterface<IRBVehicle>()) {
			ply->EnableObjectCollisions(true);
		}
	}
	bool HasTimer() override { return true; }
} E_NoCarCarCollision;