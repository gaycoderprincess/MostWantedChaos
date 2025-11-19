class Effect_WreckOnFlip : public ChaosEffect {
public:
	Effect_WreckOnFlip() : ChaosEffect() {
		sName = "Cars Wreck When Flipped";
	}

	virtual void TickFunction() {
		auto& list = VEHICLE_LIST::GetList(VEHICLE_RACERS);
		for (int i = 0; i < list.size(); i++) {
			auto racer = list[i];
			auto rb = racer->mCOMObject->Find<IRigidBody>();
			auto damage = racer->mCOMObject->Find<IDamageable>();
			if (!rb || !damage) continue;
			if (damage->IsDestroyed()) continue;
			UMath::Vector3 up;
			rb->GetUpVector(&up);
			if (up.y < 0) {
				damage->Destroy();
			}
		}
	}
} E_WreckOnFlip;