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
		auto& rotDelta = *(float*)&obj->CustomData;
		rotDelta += delta;

		auto p = obj->mMatrix.p;
		obj->mMatrix = UMath::Matrix4::kIdentity;
		obj->mMatrix.Rotate(NyaVec3(rotDelta * rotSpeedX, rotDelta * rotSpeedY, rotDelta * rotSpeedZ));

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
		Render3D::nVertexColorValue = 0xFF808080;

		if (models.empty() || models[0]->bInvalidated) {
			models = Render3D::CreateModels("pickup.fbx");
		}

		aBombsInWorld.push_back(Render3DObjects::aObjects.size());
		Render3DObjects::aObjects.push_back(Render3DObjects::Object(models, mat, {0,0,0}, 0, BombOnTick));

		Render3D::nVertexColorValue = Render3D::nDefaultVertexColor;
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

class Effect_ReVoltFirework : public ChaosEffect {
public:
	Effect_ReVoltFirework() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Give Player Firework Rockets";
		fTimerLength = 60;
		MakeIncompatibleWithFilterGroup("speedbreaker");
	}

	static inline std::vector<Render3D::tModel*> models;

	static inline float rX = 0;
	static inline float rY = 0;
	static inline float rZ = 0;
	static inline float rotOffX = 10;
	static inline float rotOffXNoTarget = 15;
	static inline float rotOffY = 0;
	static inline float rotOffZ = 0;
	static inline float offX = 0;
	static inline float offY = 1;
	static inline float offZ = 6;
	static inline float scale = 2;
	static inline float moveSpeed = 55;
	static inline float rotSpeed = 2.5;
	static inline float inFrontThreshold = 0.33;
	static inline float crosshairSize = 0.02;

	static inline NyaAudio::NyaSound FireSound = 0;
	static inline NyaAudio::NyaSound ExplodeSound = 0;

	struct tFireworkData {
		IVehicle* target;
		UMath::Vector3 currentDir;
		float speed;
		float timeLeft;
	};

	static void BombOnTick(Render3DObjects::Object* obj, double delta) {
		if (IsChaosBlocked()) return;

		auto data = (tFireworkData*)obj->CustomData;
		auto target = data->target;
		if (!IsVehicleValidAndActive(target)) target = nullptr;

		auto targetDir = (NyaVec3)data->currentDir;
		if (target) {
			targetDir = (*target->GetPosition() - obj->mMatrix.p);
			targetDir.Normalize();
		}
		else {
			targetDir.y = 0;
			targetDir.Normalize();
		}
		auto diff = targetDir - data->currentDir;
		data->currentDir += diff * rotSpeed * delta;
		data->currentDir.Normalize();

		obj->mMatrix.p += data->currentDir * data->speed * delta;

		auto p = obj->mMatrix.p;
		obj->mMatrix = NyaMat4x4::LookAt(data->currentDir);

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
			if (distFromCar < 4) {
				data->timeLeft = 0;
				if (!NyaAudio::IsFinishedPlaying(FireSound)) {
					NyaAudio::Stop(FireSound);
				}
			}
		}

		float groundY = -9999;
		WCollisionMgr::GetWorldHeightAtPointRigorous((UMath::Vector3*)&obj->mMatrix.p, &groundY, nullptr);
		if (obj->mMatrix.p.y < groundY) {
			data->timeLeft = 0;
			if (!NyaAudio::IsFinishedPlaying(FireSound)) {
				NyaAudio::Stop(FireSound);
			}
		}

		data->timeLeft -= delta;
		if (data->timeLeft <= 0.0) {
			if (ExplodeSound) {
				NyaAudio::Stop(ExplodeSound);
				NyaAudio::SkipTo(ExplodeSound, 0, false);
				NyaAudio::SetVolume(ExplodeSound, FEDatabase->mUserProfile->TheOptionsSettings.TheAudioSettings.SoundEffectsVol);
				NyaAudio::Play(ExplodeSound);
			}

			float fExplosionPower = 15;
			float fExplosionAngVelocityMult = 0.25;
			float fExplosionMaxDistance = 10;

			for (auto& car : cars) {
				auto dist = (*car->GetPosition() - obj->mMatrix.p);
				if (dist.length() < fExplosionMaxDistance) {
					auto rb = car->mCOMObject->Find<IRigidBody>();

					auto impulse = dist * (fExplosionPower * rb->GetMass() / 1000.0 * std::min((fExplosionMaxDistance - dist.length()) * 2.0 / fExplosionMaxDistance, 1.0) / std::max(dist.length(), 0.01));

					auto vel = *rb->GetLinearVelocity();
					auto avel = *rb->GetAngularVelocity();
					vel += impulse;
					avel += impulse * fExplosionAngVelocityMult;
					rb->SetLinearVelocity(&vel);
					rb->SetAngularVelocity(&avel);

					if (!IsCarDestroyed(car) && car->GetDriverClass() == DRIVER_COP) {
						car->mCOMObject->Find<IDamageable>()->Destroy();
					}
				}
			}

			obj->aModels.clear();
		}
	}

	static void SpawnBomb(UMath::Matrix4 mat, IVehicle* target) {
		if (models.empty() || models[0]->bInvalidated) {
			models = Render3D::CreateModels("firework.fbx");
		}

		int id = Render3DObjects::aObjects.size();
		Render3DObjects::aObjects.push_back(Render3DObjects::Object(models, mat, {0,0,0}, 0, BombOnTick));

		auto data = new tFireworkData;
		data->target = target;
		data->currentDir = (UMath::Vector3)mat.z;
		//if (target) {
		//	data->currentDir = (UMath::Vector3)(*target->GetPosition() - mat.p);
		//	data->currentDir.y = mat.z.y;
		//	data->currentDir.Normalize();
		//}
		data->speed = moveSpeed + GetLocalPlayerVehicle()->GetSpeed();
		data->timeLeft = 2;
		Render3DObjects::aObjects[id].CustomData = data;

		if (FireSound) {
			NyaAudio::Stop(FireSound);
			NyaAudio::SkipTo(FireSound, 0, false);
			NyaAudio::SetVolume(FireSound, FEDatabase->mUserProfile->TheOptionsSettings.TheAudioSettings.SoundEffectsVol);
			NyaAudio::Play(FireSound);
		}
	}

	void InitFunction() override {
		if (!FireSound) FireSound = NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/firefire.wav");
		if (!ExplodeSound) ExplodeSound = NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/firebang.wav");
	}
	void TickFunctionMain(double delta) override {
		//auto target = GetClosestActiveVehicle(GetLocalPlayerVehicle(), true, inFrontThreshold);
		auto target = GetMostInFrontActiveVehicle(GetLocalPlayerVehicle(), 200, inFrontThreshold);
		if (target) {
			bVector3 screenPos;
			auto worldPos = WorldToRenderCoords(*target->GetPosition());
			eViewPlatInterface::GetScreenPosition(&eViews[EVIEW_PLAYER1], &screenPos, (bVector3*)&worldPos);

			screenPos.x /= (double)nResX;
			screenPos.y /= (double)nResY;

			static auto texture = LoadTexture("CwoeeChaos/data/textures/firework_crosshair.png");
			DrawRectangle(screenPos.x - crosshairSize * GetAspectRatioInv(), screenPos.x + crosshairSize * GetAspectRatioInv(), screenPos.y - crosshairSize, screenPos.y + crosshairSize, {0,255,0,255}, 0, texture);
		}

		tNyaStringData data;
		data.x = 0.5;
		data.y = 0.85;
		data.size = 0.04;
		data.XCenterAlign = true;
		data.outlinea = 255;
		data.outlinedist = 0.025;
		DrawString(data, "Press X to fire a rocket!");

		GetLocalPlayer()->ResetGameBreaker(false);
		if (!IsKeyJustPressed('X') && !IsPadKeyJustPressed(NYA_PAD_KEY_X)) return;

		if (auto veh = GetLocalPlayerInterface<IRigidBody>()) {
			auto mat = UMath::Matrix4::kIdentity;
			veh->GetMatrix4(&mat);
			auto pos = *veh->GetPosition();
			pos += mat.x * offX;
			pos += mat.y * offY;
			pos += mat.z * offZ;

			UMath::Matrix4 rotation;
			if (target) {
				rotation.Rotate(NyaVec3(rotOffX * 0.01745329, rotOffY * 0.01745329, rotOffZ * 0.01745329));
			}
			else {
				rotation.Rotate(NyaVec3(rotOffXNoTarget * 0.01745329, rotOffY * 0.01745329, rotOffZ * 0.01745329));
			}
			mat = (UMath::Matrix4)(mat * rotation);
			mat.p = pos;
			SpawnBomb(mat, target);
		}
	}
	void DeinitFunction() override {
		GetLocalPlayer()->ChargeGameBreaker(100);
	}
	bool HasTimer() override { return true; }
	bool CanQuickTrigger() override { return false; }
} E_ReVoltFirework;