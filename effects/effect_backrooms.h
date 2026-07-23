class Effect_Backrooms : public ChaosEffect {
public:
	Effect_Backrooms() : ChaosEffect("Uncategorized") {
		sName = "Teleport To Backrooms";
		bCanQuickTrigger = false;
		bRigProportionalChances = true;
	}

	void InitFunction() override {
		auto map = &MapSpawner::aMaps[MapSpawner::MAP_BACKROOMS];
		if (!map->IsSpawned() || map->IsInvalidated()) {
			map->Despawn();
			map->Spawn();
		}

		if (auto veh = GetLocalPlayerInterface<IRigidBody>()) {
			UMath::Vector3 playerPos = map->vMapPos + map->vPlayerOffset;
			TeleportPlayer(playerPos, {-0.96,0.0,-0.3});
		}
	}
} E_Backrooms;