#define EFFECT_CATEGORY_TEMP "Spawn"

class Effect_SpawnCarRandomized : public ChaosEffect {
public:
	bool abort = false;

	Effect_SpawnCarRandomized() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Spawn Eldritch Monstrosity";
		sFriendlyName = "Spawn Glitched Golf";
	}

	void InitFunction() override {
		abort = false;
		auto car = Attrib::StringHash32("gti");
		auto customization = CreateStockCustomizations(car);
		for (auto& part : customization.InstalledPartIndices) {
			part -= 4;
		}
		if (auto pCar = SpawnCarInWorld(car, &customization)) {
			pCar->SetDriverClass(DRIVER_RACER);
			if (auto rb = pCar->mCOMObject->Find<IRigidBody>()) {
				auto pos = *rb->GetPosition();
				UMath::Vector3 fwd;
				rb->GetForwardVector(&fwd);
				pos.x += fwd.x * 6;
				pos.y += fwd.y * 6;
				pos.z += fwd.z * 6;
				rb->SetPosition(&pos);
			}
		}
		else {
			abort = true;
		}
	}
	bool ShouldAbort() override {
		return abort;
	}
	bool CanQuickTrigger() override { return false; }
} E_SpawnCarRandomized;

class Effect_SpawnCarTruck : public ChaosEffect {
public:
	bool abort = false;

	Effect_SpawnCarTruck() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Obligatory Truck Effect";
		sFriendlyName = "Spawn Truck In Front Of Player";
	}

	void InitFunction() override {
		abort = false;
		auto car = Attrib::StringHash32("semia");
		if (auto pCar = SpawnCarInWorld(car, nullptr)) {
			if (auto rb = pCar->mCOMObject->Find<IRigidBody>()) {
				auto pos = *rb->GetPosition();
				UMath::Vector3 fwd;
				rb->GetForwardVector(&fwd);
				pos.x += fwd.x * 10;
				pos.y += fwd.y * 10;
				pos.z += fwd.z * 10;
				rb->SetPosition(&pos);
			}
		}
		else {
			abort = true;
		}
	}
	bool ShouldAbort() override {
		return abort;
	}
	bool CanQuickTrigger() override { return false; }
} E_SpawnCarTruck;