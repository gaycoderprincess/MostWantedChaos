class Effect_DisableAll : public ChaosEffect {
public:
	Effect_DisableAll() : ChaosEffect("Uncategorized") {
		sName = "Disable Everything";
		fTimerLength = 10;
	}

	void TickFunctionMain(double delta) override {
		DrawCars = false;
		DrawLightFlares = false;
		ICopMgr::mDisableCops = true;

		// world
		NyaHookLib::Patch(0x723FA0, 0x530008C2);
		NyaHookLib::Patch<uint8_t>(0x6DEF49, 0xEB); // FE

		// traffic & racers
		auto cars = GetActiveVehicles();
		for (auto& car : cars) {
			if (car->GetDriverClass() == DRIVER_TRAFFIC) {
				car->Deactivate();
			}
			else {
				car->mCOMObject->Find<IRigidBody>()->SetLinearVelocity(&UMath::Vector3::kZero);
				car->mCOMObject->Find<IRigidBody>()->SetAngularVelocity(&UMath::Vector3::kZero);
			}
		}

		// input
		if (auto ply = GetLocalPlayerInterface<IInput>()) {
			ply->ClearInput();
		}

		// nitro
		if (auto ply = GetLocalPlayerEngine()) {
			ply->ChargeNOS(-1);
		}

		// speedbreaker
		if (auto ply = GetLocalPlayer()) {
			ply->ResetGameBreaker(false);
		}
	}
	void DeinitFunction() override {
		DrawCars = true;
		DrawLightFlares = true;
		ICopMgr::mDisableCops = false;

		NyaHookLib::Patch(0x723FA0, 0x5314EC83);
		NyaHookLib::Patch<uint8_t>(0x6DEF49, 0x74);
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
} E_DisableAll;