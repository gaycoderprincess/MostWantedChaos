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

class Effect_NoCollision : public ChaosEffect {
public:
	Effect_NoCollision() : ChaosEffect() {
		sName = "Disable World Collision";
		fTimerLength = 90;
	}

	// RigidBody::DoWorldCollisions
	void InitFunction() override {
		NyaHookLib::Patch<uint8_t>(0x6BB796, 0xEB);
	}
	void DeinitFunction() override {
		NyaHookLib::Patch<uint8_t>(0x6BB796, 0x75);
	}
	bool HasTimer() override { return true; }
} E_NoCollision;