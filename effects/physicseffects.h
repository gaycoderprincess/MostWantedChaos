#define EFFECT_CATEGORY_TEMP "Physics"

class Effect_NoTirePhysics : public ChaosEffect {
public:
	Effect_NoTirePhysics() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
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

class Effect_NoTirePhysics2 : public ChaosEffect {
public:
	Effect_NoTirePhysics2() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Remove All Tires";
		fTimerLength = 10;
	}

	void InitFunction() override {
		NyaHookLib::Patch(0x8ABB60, 0x6B20E1);
		NyaHookLib::Patch<uint64_t>(0x74FCD0, 0xB60F900000084AE9); // render brakes
		NyaHookLib::Patch<uint64_t>(0x74F2ED, 0x046A90000009B7E9); // render tires
	}
	void DeinitFunction() override {
		NyaHookLib::Patch(0x8ABB60, 0x6B1DD0);
		NyaHookLib::Patch<uint64_t>(0x74FCD0, 0xB60F00000849840F);
		NyaHookLib::Patch<uint64_t>(0x74F2ED, 0x046A000009B68F0F);
	}
	bool HasTimer() override { return true; }
} E_NoTirePhysics2;

class Effect_NoWallCollision : public ChaosEffect {
public:
	Effect_NoWallCollision() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
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
	Effect_NoCollision() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
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
	Effect_NoCarCarCollision() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Disable Car To Car Collision";
		fTimerLength = 90;
	}

	void TickFunctionMain(double delta) override {
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