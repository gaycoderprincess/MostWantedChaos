#define EFFECT_CATEGORY_TEMP "Spawn"

// uhh idk this seems a bit lame now
/*class Effect_SpawnCarRandomized : public ChaosEffect {
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
} E_SpawnCarRandomized;*/

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

class Effect_Teddie : public ChaosEffect {
public:
	Effect_Teddie() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "pee.";
		sFriendlyName = "Spawn Teddie From Persona 4";
	}

	static inline std::vector<Render3D::tModel*> models;

	static inline float rX = 90;
	static inline float rY = 0;
	static inline float rZ = 0;
	static inline float offX = 0;
	static inline float offY = -1;
	static inline float offZ = 4;
	static inline float scale = 1.0;
	static inline float colScale = 0.65;

	static inline std::vector<int> aTeddiesInWorld;

	static void SpawnTeddie(UMath::Matrix4 mat, NyaVec3 colPos) {
		if (models.empty() || models[0]->bInvalidated) {
			models = Render3D::CreateModels("teddie.fbx");
		}

		aTeddiesInWorld.push_back(Render3DObjects::aObjects.size());
		Render3DObjects::aObjects.push_back(Render3DObjects::Object(models, mat, colPos, colScale));
	}

	void InitFunction() override {
		if (auto veh = GetLocalPlayerInterface<IRigidBody>()) {
			auto mat = UMath::Matrix4::kIdentity;
			veh->GetMatrix4(&mat);
			mat.p = *veh->GetPosition();
			auto colPos = mat.p;
			colPos += mat.x * offX;
			colPos += mat.z * offZ;
			mat.p += mat.x * offX;
			mat.p += mat.y * offY;
			mat.p += mat.z * offZ;
			mat.x *= scale;
			mat.y *= scale;
			mat.z *= scale;

			UMath::Matrix4 rotation;
			rotation.Rotate(NyaVec3(rX * 0.01745329, rY * 0.01745329, rZ * 0.01745329));
			mat = (UMath::Matrix4)(mat * rotation);
			SpawnTeddie(mat, colPos);
		}
	}
} E_Teddie;

class Effect_173 : public ChaosEffect {
public:
	Effect_173() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Spawn SCP-173";
	}

	static inline std::vector<Render3D::tModel*> models;

	static inline float rX = 90;
	static inline float rY = 0;
	static inline float rZ = 0;
	static inline float offX = 0;
	static inline float offY = 0;
	static inline float offZ = 4;
	static inline float scale = 1.0;
	static inline float colScale = 0.5;

	static inline float peanutSpeed = 30;
	static inline float lastPeanutDistance = 0;
	static inline float lastPeanutDot = 0;

	static inline std::vector<int> aPeanutsInWorld;
	static inline bool bPeanutEverSpawned = false;

	static void PeanutMove(Render3DObjects::Object* obj, double delta) {
		if (IsChaosBlocked()) return;
		auto objDir = (GetLocalPlayerCamera()->CurrentKey.Position - WorldToRenderCoords(obj->vColPosition));
		objDir.Normalize();
		auto dot = GetLocalPlayerCamera()->CurrentKey.Direction.Dot(objDir);
		lastPeanutDot = dot;
		if (dot > 0) {
			auto movePos = (*GetLocalPlayerVehicle()->GetPosition() - obj->vColPosition);
			lastPeanutDistance = movePos.length();
			movePos.Normalize();
			obj->vColPosition += movePos * peanutSpeed * Sim::Internal::mSystem->mSpeed * delta;
			WCollisionMgr::GetWorldHeightAtPointRigorous((UMath::Vector3*)&obj->vColPosition, &obj->vColPosition.y, nullptr);

			obj->mMatrix = NyaMat4x4::LookAt(-movePos);

			UMath::Matrix4 rotation;
			rotation.Rotate(NyaVec3(rX * 0.01745329, rY * 0.01745329, rZ * 0.01745329));
			obj->mMatrix = (UMath::Matrix4)(obj->mMatrix * rotation);

			obj->mMatrix.p = obj->vColPosition;

			auto cars = GetActiveVehicles();
			for (auto& car : cars) {
				auto distFromCar = (*car->GetPosition() - obj->vColPosition).length();
				if (distFromCar < 5) {
					static auto sound = NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/173/NeckSnap3.ogg");
					static auto soundAlt = NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/173/NeckSnap1.ogg");
					if (car == GetLocalPlayerVehicle()) {
						if (sound) {
							NyaAudio::SetVolume(sound, FEDatabase->mUserProfile->TheOptionsSettings.TheAudioSettings.SoundEffectsVol);
							NyaAudio::Play(sound);
						}

						obj->aModels.clear(); // despawn after one kill
						aMainLoopFunctionsOnce.push_back([]() { EQuitToFE::Create(GARAGETYPE_MAIN_FE, "MainMenu.fng"); });

						DoChaosSave();
					}
					else if (!IsCarDestroyed(car)) {
						if (soundAlt) {
							NyaAudio::SetVolume(soundAlt, FEDatabase->mUserProfile->TheOptionsSettings.TheAudioSettings.SoundEffectsVol);
							NyaAudio::Play(soundAlt);
						}

						car->mCOMObject->Find<IDamageable>()->Destroy();
					}
				}
			}
		}
	}

	static void SpawnPeanut(UMath::Matrix4 mat) {
		if (models.empty() || models[0]->bInvalidated) {
			models = Render3D::CreateModels("scp173.fbx");
		}

		aPeanutsInWorld.push_back(Render3DObjects::aObjects.size());
		Render3DObjects::aObjects.push_back(Render3DObjects::Object(models, mat, mat.p, colScale, PeanutMove));

		bPeanutEverSpawned = true;
	}

	void InitFunction() override {
		if (auto veh = GetLocalPlayerInterface<IRigidBody>()) {
			auto mat = UMath::Matrix4::kIdentity;
			veh->GetMatrix4(&mat);
			mat.p = *veh->GetPosition();
			WCollisionMgr::GetWorldHeightAtPointRigorous((UMath::Vector3*)&mat.p, &mat.p.y, nullptr);

			mat.p += mat.x * offX;
			mat.p += mat.y * offY;
			mat.p += mat.z * offZ;
			mat.x *= scale;
			mat.y *= scale;
			mat.z *= scale;

			UMath::Matrix4 rotation;
			rotation.Rotate(NyaVec3(rX * 0.01745329, rY * 0.01745329, rZ * 0.01745329));
			mat = (UMath::Matrix4)(mat * rotation);
			SpawnPeanut(mat);

			veh->SetLinearVelocity(&UMath::Vector3::kZero);
			veh->SetAngularVelocity(&UMath::Vector3::kZero);
		}
	}
	bool CanQuickTrigger() override { return false; }
} E_173;

class Effect_173Somewhere : public ChaosEffect {
public:
	Effect_173Somewhere() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Spawn SCP-173 Somewhere";
	}

	void InitFunction() override {
		auto mat = UMath::Matrix4::kIdentity;
		mat.p.x = GetRandomNumber(-5000, 5000);
		mat.p.z = GetRandomNumber(-5000, 5000);
		Effect_173::SpawnPeanut(mat);
	}
	bool IsAvailable() override { return Effect_173::bPeanutEverSpawned; }
	bool AbortOnConditionFailed() override { return true; }
	bool CanMultiTrigger() override { return true; }
} E_173Somewhere;

class Effect_8Down : public ChaosEffect {
public:
	Effect_8Down() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "8 Down";
		sFriendlyName = "Spawn Vending Machine";
	}

	static inline std::vector<Render3D::tModel*> models;

	static inline float rX = 180;
	static inline float rY = 0;
	static inline float rZ = -90;
	static inline float offX = 0;
	static inline float offY = -0.5;
	static inline float offZ = 4;
	static inline float scale = 1.0;
	static inline float colScale = 0.65;

	static inline std::vector<int> aObjectsInWorld;

	static void SpawnObject(UMath::Matrix4 mat, NyaVec3 colPos) {
		if (models.empty() || models[0]->bInvalidated) {
			models = Render3D::CreateModels("8down.fbx");
		}

		aObjectsInWorld.push_back(Render3DObjects::aObjects.size());
		Render3DObjects::aObjects.push_back(Render3DObjects::Object(models, mat, colPos, colScale));
	}

	void InitFunction() override {
		if (auto veh = GetLocalPlayerInterface<IRigidBody>()) {
			auto mat = UMath::Matrix4::kIdentity;
			veh->GetMatrix4(&mat);
			mat.p = *veh->GetPosition();
			auto colPos = mat.p;
			colPos += mat.x * offX;
			colPos += mat.z * offZ;
			mat.p += mat.x * offX;
			mat.p += mat.y * offY;
			mat.p += mat.z * offZ;
			mat.x *= scale;
			mat.y *= scale;
			mat.z *= scale;

			UMath::Matrix4 rotation;
			rotation.Rotate(NyaVec3(rX * 0.01745329, rY * 0.01745329, rZ * 0.01745329));
			mat = (UMath::Matrix4)(mat * rotation);
			SpawnObject(mat, colPos);
		}
	}
} E_8Down;

class Effect_ReVoltBomb : public ChaosEffect {
public:
	Effect_ReVoltBomb() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Spawn Bomb Behind Player";
	}

	static inline std::vector<Render3D::tModel*> models;

	static inline float rX = 90;
	static inline float rY = 0;
	static inline float rZ = 0;
	static inline float offX = 0;
	static inline float offY = 1.5;
	static inline float offZ = -6;
	static inline float scale = 2;

	static inline float rotSpeedX = 0;
	static inline float rotSpeedY = 0;
	static inline float rotSpeedZ = -1.5;

	static inline std::vector<int> aBombsInWorld;

	static void BombOnTick(Render3DObjects::Object* obj, double delta) {
		if (IsChaosBlocked()) return;

		// using colposition to store the rotation delta
		obj->vColPosition.x += delta;

		auto p = obj->mMatrix.p;
		obj->mMatrix = UMath::Matrix4::kIdentity;
		obj->mMatrix.Rotate(NyaVec3(obj->vColPosition.x * rotSpeedX, obj->vColPosition.x * rotSpeedY, obj->vColPosition.x * rotSpeedZ));

		UMath::Matrix4 rotation;
		rotation.Rotate(NyaVec3(rX * 0.01745329, rY * 0.01745329, rZ * 0.01745329));
		obj->mMatrix = (UMath::Matrix4)(obj->mMatrix * rotation);
		obj->mMatrix.x *= scale;
		obj->mMatrix.y *= scale;
		obj->mMatrix.z *= scale;
		obj->mMatrix.p = p;

		auto cars = GetActiveVehicles();
		for (auto& car : cars) {
			auto distFromCar = (*car->GetPosition() - obj->mMatrix.p).length();
			if (distFromCar < 5) {
				if (!IsCarDestroyed(car)) {
					static auto sound = NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/puttbang.wav");
					if (sound) {
						NyaAudio::SetVolume(sound, FEDatabase->mUserProfile->TheOptionsSettings.TheAudioSettings.SoundEffectsVol);
						NyaAudio::Play(sound);
					}

					if (auto rb = car->mCOMObject->Find<IRigidBody>()) {
						auto vel = *rb->GetLinearVelocity();
						vel.y += 10;
						rb->SetLinearVelocity(&vel);

						auto avel = *rb->GetAngularVelocity();
						UMath::Vector3 right;
						rb->GetForwardVector(&right);
						avel.x += 10 * right.x;
						avel.y += 10 * right.y;
						avel.z += 10 * right.z;
						rb->SetAngularVelocity(&vel);
					}
					car->mCOMObject->Find<IDamageable>()->Destroy();
					obj->aModels.clear();
				}
			}
		}
	}

	static void SpawnBomb(UMath::Matrix4 mat) {
		if (models.empty() || models[0]->bInvalidated) {
			models = Render3D::CreateModels("pickup.fbx");
		}

		aBombsInWorld.push_back(Render3DObjects::aObjects.size());
		Render3DObjects::aObjects.push_back(Render3DObjects::Object(models, mat, {0,0,0}, 0, BombOnTick));
	}

	void InitFunction() override {
		if (auto veh = GetLocalPlayerInterface<IRigidBody>()) {
			auto mat = UMath::Matrix4::kIdentity;
			veh->GetMatrix4(&mat);
			mat.p = *veh->GetPosition();
			WCollisionMgr::GetWorldHeightAtPointRigorous((UMath::Vector3*)&mat.p, &mat.p.y, nullptr);
			mat.p += mat.x * offX;
			mat.p += mat.y * offY;
			mat.p += mat.z * offZ;
			SpawnBomb(mat);
		}
	}
	bool CanQuickTrigger() override { return false; }
} E_ReVoltBomb;