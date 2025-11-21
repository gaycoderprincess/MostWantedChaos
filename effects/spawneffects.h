class Effect_SpawnCarRandomized : public ChaosEffect {
public:
	Effect_SpawnCarRandomized() : ChaosEffect() {
		sName = "Spawn Eldritch Monstrosity";
	}

	void InitFunction() override {
		auto car = Attrib::StringHash32("gti");
		auto customization = CreateStockCustomizations(car);
		for (auto& part : customization.InstalledPartIndices) {
			part--;
		}
		if (auto pCar = SpawnCarInWorld(car, &customization)) {
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
	}
} E_SpawnCarRandomized;