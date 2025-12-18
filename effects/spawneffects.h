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

	std::vector<Render3D::tModel*> models;

	static inline float rX = 90;
	static inline float rY = 0;
	static inline float rZ = 0;
	static inline float offX = 0;
	static inline float offY = -1;
	static inline float offZ = 4;
	static inline float scale = 1.0;
	static inline float colScale = 0.65;

	void InitFunction() override {
		if (models.empty() || models[0]->bInvalidated) {
			models = Render3D::CreateModels("teddie.fbx");
		}

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
			Render3DObjects::aObjects.push_back(Render3DObjects::Object(models, mat, colPos, colScale));
		}
	}
} E_Teddie;

class Effect_173 : public ChaosEffect {
public:
	Effect_173() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Spawn SCP-173";
	}

	std::vector<Render3D::tModel*> models;

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
					if (car == GetLocalPlayerVehicle()) {
						if (sound) {
							NyaAudio::SetVolume(sound, FEDatabase->mUserProfile->TheOptionsSettings.TheAudioSettings.SoundEffectsVol);
							NyaAudio::Play(sound);
						}

						obj->aModels.clear(); // despawn after one kill
						aMainLoopFunctionsOnce.push_back([]() { EQuitToFE::Create(GARAGETYPE_MAIN_FE, "MainMenu.fng"); });
					}
					else if (!IsCarDestroyed(car)) {
						if (sound) {
							NyaAudio::SetVolume(sound, FEDatabase->mUserProfile->TheOptionsSettings.TheAudioSettings.SoundEffectsVol);
							NyaAudio::Play(sound);
						}

						car->mCOMObject->Find<IDamageable>()->Destroy();
					}
				}
			}
		}
	}

	void InitFunction() override {
		if (models.empty() || models[0]->bInvalidated) {
			models = Render3D::CreateModels("scp173.fbx");
		}

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
			Render3DObjects::aObjects.push_back(Render3DObjects::Object(models, mat, mat.p, colScale, PeanutMove));
		}
	}
	bool CanQuickTrigger() override { return false; }
} E_173;