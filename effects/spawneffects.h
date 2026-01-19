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
		sFriendlyName = "Spawn Truck On Player";
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

				static auto sound = NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/truck.mp3");
				if (sound) {
					NyaAudio::SetVolume(sound, FEDatabase->mUserProfile->TheOptionsSettings.TheAudioSettings.SoundEffectsVol);
					NyaAudio::Play(sound);
				}
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
			DoChaosSave();
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
			DoChaosSave();

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
		DoChaosSave();
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
			DoChaosSave();
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
		static auto sound = NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/pickgen.wav");
		if (sound) {
			NyaAudio::SetVolume(sound, FEDatabase->mUserProfile->TheOptionsSettings.TheAudioSettings.SoundEffectsVol);
			NyaAudio::Play(sound);
		}

		if (auto veh = GetLocalPlayerInterface<IRigidBody>()) {
			auto mat = UMath::Matrix4::kIdentity;
			veh->GetMatrix4(&mat);
			mat.p = *veh->GetPosition();
			WCollisionMgr::GetWorldHeightAtPointRigorous((UMath::Vector3*)&mat.p, &mat.p.y, nullptr);
			mat.p += mat.x * offX;
			mat.p += mat.y * offY;
			mat.p += mat.z * offZ;
			SpawnBomb(mat);
			DoChaosSave();
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
	static inline float inFrontThreshold = 0.6;
	static inline float crosshairSize = 0.02;
	static inline float sfxVolume = 0.5;

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
				NyaAudio::SetVolume(ExplodeSound, FEDatabase->mUserProfile->TheOptionsSettings.TheAudioSettings.SoundEffectsVol * sfxVolume);
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
	}

	static void LaunchRocketFromPlayer(IRigidBody* veh, IVehicle* target) {
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

		if (veh == GetLocalPlayerInterface<IRigidBody>() && FireSound) {
			NyaAudio::Stop(FireSound);
			NyaAudio::SkipTo(FireSound, 0, false);
			NyaAudio::SetVolume(FireSound, FEDatabase->mUserProfile->TheOptionsSettings.TheAudioSettings.SoundEffectsVol * sfxVolume);
			NyaAudio::Play(FireSound);
		}
	}

	static void DrawCrosshair(IVehicle* target, bool isPlayerCrosshair) {
		bVector3 screenPos;
		auto worldPos = WorldToRenderCoords(*target->GetPosition());
		eViewPlatInterface::GetScreenPosition(&eViews[EVIEW_PLAYER1], &screenPos, (bVector3*)&worldPos);

		screenPos.x /= (double)nResX;
		screenPos.y /= (double)nResY;

		static auto texture = LoadTexture("CwoeeChaos/data/textures/firework_crosshair.png");
		DrawRectangle(screenPos.x - crosshairSize * GetAspectRatioInv(), screenPos.x + crosshairSize * GetAspectRatioInv(), screenPos.y - crosshairSize, screenPos.y + crosshairSize, isPlayerCrosshair ? NyaDrawing::CNyaRGBA32(0,255,0,255) : NyaDrawing::CNyaRGBA32(255,0,0,255), 0, texture);
	}

	static void LoadSounds() {
		if (!FireSound) FireSound = NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/firefire.wav");
		if (!ExplodeSound) ExplodeSound = NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/firebang.wav");
	}

	void InitFunction() override {
		CwoeeHints::AddHint("Press X to fire a rocket.");
		CwoeeHints::AddHint("The green reticule displays your lock-on target.");
		LoadSounds();
	}
	void TickFunctionMain(double delta) override {
		//auto target = GetClosestActiveVehicle(GetLocalPlayerVehicle(), true, inFrontThreshold);
		auto target = GetMostInFrontActiveVehicle(GetLocalPlayerVehicle(), 200, inFrontThreshold);
		if (target) {
			DrawCrosshair(target, true);
		}

		GetLocalPlayer()->ResetGameBreaker(false);
		if (IsKeyJustPressed('X') || IsPadKeyJustPressed(NYA_PAD_KEY_X)) {
			LaunchRocketFromPlayer(GetLocalPlayerInterface<IRigidBody>(), target);
		}
	}
	void DeinitFunction() override {
		GetLocalPlayer()->ChargeGameBreaker(100);
	}
	bool HasTimer() override { return true; }
	bool CanQuickTrigger() override { return false; }
} E_ReVoltFirework;

class Effect_ReVoltFirework2 : public ChaosEffect {
public:
	Effect_ReVoltFirework2() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Give Other Cars Firework Rockets";
		fTimerLength = 120;
	}

	double timer = 0;

	void InitFunction() override {
		timer = 0;
		Effect_ReVoltFirework::LoadSounds();
	}
	void TickFunctionMain(double delta) override {
		auto cars = GetActiveVehicles();
		for (auto& veh : cars) {
			if (veh == GetLocalPlayerVehicle()) continue;
			if (IsCarDestroyed(veh)) continue;
			auto target = GetMostInFrontActiveVehicle(veh, 200, Effect_ReVoltFirework::inFrontThreshold);
			if (target == GetLocalPlayerVehicle()) {
				Effect_ReVoltFirework::DrawCrosshair(target, false);
			}
		}

		timer += delta;
		if (timer > 1) {
			for (auto& veh : cars) {
				if (veh == GetLocalPlayerVehicle()) continue;
				if (IsCarDestroyed(veh)) continue;
				if (PercentageChanceCheck(25)) {
					auto target = GetMostInFrontActiveVehicle(veh, 200, Effect_ReVoltFirework::inFrontThreshold);
					if (!target) continue;
					if (veh->GetDriverClass() == DRIVER_COP && target->GetDriverClass() == DRIVER_COP) continue; // prevent cop friendly fire
					Effect_ReVoltFirework::LaunchRocketFromPlayer(veh->mCOMObject->Find<IRigidBody>(), target);
				}
			}
			timer -= 1;
		}
	}
	bool HasTimer() override { return true; }
} E_ReVoltFirework2;

class Effect_Vergil : public ChaosEffect {
public:
	Effect_Vergil() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Spawn Griefer Vergil";
		nFrequency *= 3;
	}

	static inline std::vector<Render3D::tModel*> models;

	static inline float rX = 90;
	static inline float rY = 0;
	static inline float rZ = 0;
	static inline float offX = 0;
	static inline float offY = -1;
	static inline float offZ = 4;
	static inline float scale = 1.5;
	static inline float colScale = 0.65;
	static inline float attackFrequency = 0.5;
	static inline float sfxRange = 250;
	static inline float sfxVolume = 1;
	static inline float attackVolume = 4;
	static inline float styleVolume = 5;
	static inline float targetRange = 20;
	static inline float attackRange = 25;
	static inline float attackStingerRange = 15;
	static inline float attackPower = 100;
	static inline float attackPowerAng = 25;

	static inline float attackStyleIncrease = 0.75;
	static inline float styleDecay = 1;

	static inline std::vector<int> aVergilsInWorld;

	enum eStyle {
		STYLE_NONE,
		STYLE_D,
		STYLE_C,
		STYLE_B,
		STYLE_A,
		STYLE_S,
		STYLE_SS,
		STYLE_SSS,
		NUM_STYLES,
	};

	struct tVergilData {
		NyaVec3 direction = {};
		double attackTimer = 0;
		NyaAudio::NyaSound audio = 0;
		double styleRanking = 0;
		bool stylesCalledOut[NUM_STYLES] = {};
	};

	static bool CanCarBeTargeted(IVehicle* veh) {
		auto invuln = veh->mCOMObject->Find<IRBVehicle>()->GetInvulnerability();
		if (invuln != INVULNERABLE_NONE && invuln != INVULNERABLE_FROM_MANUAL_RESET) return false;
		if (veh->IsStaging()) return false;
		// don't target racers during NISs
		//if (veh->GetDriverClass() == DRIVER_RACER || veh->GetDriverClass() == DRIVER_HUMAN) {
		//	if (INIS::mInstance) return false;
		//}
		return true;
	}

	static void VergilStyleAnnouncer(Render3DObjects::Object* obj) {
		auto data = (tVergilData*)obj->CustomData;

		int currentStyle = (int)data->styleRanking;
		if ((int)data->styleRanking >= STYLE_B && !data->stylesCalledOut[currentStyle]) {
			for (int i = 0; i < currentStyle; i++) {
				data->stylesCalledOut[i] = true;
			}
			data->stylesCalledOut[currentStyle] = true;

			static NyaAudio::NyaSound styles[] = {
					0,
					NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/vergil/style/d.wav"),
					NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/vergil/style/c.wav"),
					NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/vergil/style/b.wav"),
					NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/vergil/style/a.wav"),
					NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/vergil/style/s.wav"),
					NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/vergil/style/ss.wav"),
					NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/vergil/style/sss.wav"),
			};

			if (auto sound = styles[currentStyle]) {
				NyaAudio::SetVolume(sound, FEDatabase->mUserProfile->TheOptionsSettings.TheAudioSettings.SoundEffectsVol * styleVolume);
				NyaAudio::Play(sound);
			}
		}

		if (data->styleRanking > NUM_STYLES) data->styleRanking = NUM_STYLES; // cap at top of SSS
	}

	static void VergilGenericAttack(Render3DObjects::Object* obj, float range, float extraUp) {
		auto data = (tVergilData*)obj->CustomData;

		auto cars = GetActiveVehicles();
		for (auto& car: cars) {
			if (!CanCarBeTargeted(car)) continue;

			auto pos = *car->GetPosition();
			auto dist = (pos - obj->vColPosition).length();
			if (dist < range) {
				auto dir = (pos - obj->vColPosition);
				dir.Normalize();

				auto rb = car->mCOMObject->Find<IRigidBody>();

				auto vel = *rb->GetLinearVelocity();
				vel += dir * attackPower;
				vel.y += extraUp;
				rb->SetLinearVelocity(&vel);

				auto avel = *rb->GetAngularVelocity();
				avel += dir * attackPowerAng;
				rb->SetAngularVelocity(&avel);

				data->styleRanking += attackStyleIncrease;

				if (car->GetDriverClass() == DRIVER_COP) {
					car->mCOMObject->Find<IDamageable>()->Destroy();
				}
			}
		}
	}

	static void VergilForwardAttack(Render3DObjects::Object* obj) {
		VergilGenericAttack(obj, attackStingerRange, 0);

		auto data = (tVergilData*)obj->CustomData;
		auto newPosition = obj->vColPosition + data->direction * 7.5;
		if (WCollisionMgr::GetWorldHeightAtPointRigorous((UMath::Vector3*)&newPosition, &newPosition.y, nullptr)) {
			obj->vColPosition = newPosition;
			VergilGenericAttack(obj, attackStingerRange, 0);
		}

		static auto sound1 = NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/vergil/stinger.mp3");
		static auto sound2 = NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/vergil/stinger_2.mp3");

		auto sound = PercentageChanceCheck(50) ? sound1 : sound2;
		if (sound) {
			NyaAudio::SetVolume(sound, FEDatabase->mUserProfile->TheOptionsSettings.TheAudioSettings.SoundEffectsVol * attackVolume);
			NyaAudio::Play(sound);
		}
	}

	static void VergilTeleport(Render3DObjects::Object* obj) {
		auto ply = *GetLocalPlayerVehicle()->GetPosition();
		UMath::Vector3 fwd;
		GetLocalPlayerInterface<IRigidBody>()->GetForwardVector(&fwd);

		// check ground exists before teleporting
		auto newPosition = ply + fwd * 5;
		if (!WCollisionMgr::GetWorldHeightAtPointRigorous((UMath::Vector3*)&newPosition, &newPosition.y, nullptr)) return;
		obj->vColPosition = newPosition;

		static auto sound = NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/vergil/teleport.mp3");
		if (sound) {
			NyaAudio::SetVolume(sound, FEDatabase->mUserProfile->TheOptionsSettings.TheAudioSettings.SoundEffectsVol * attackVolume);
			NyaAudio::Play(sound);
		}
	}

	static void VergilOnTick(Render3DObjects::Object* obj, double delta) {
		auto data = (tVergilData*)obj->CustomData;

		NyaVec3 plyPos = {0,0,0};
		auto ply = GetLocalPlayerVehicle();
		if (ply) plyPos = *ply->GetPosition();

		auto plyDist = (obj->vColPosition - plyPos).length();

		if (auto closestCar = GetClosestActiveVehicle(obj->vColPosition)) {
			WCollisionMgr::GetWorldHeightAtPointRigorous((UMath::Vector3*)&obj->vColPosition, &obj->vColPosition.y, nullptr);

			auto dir = (obj->vColPosition - *closestCar->GetPosition());
			dir.y = 0;
			dir.Normalize();
			data->direction = -dir;

			obj->mMatrix = NyaMat4x4::LookAt(dir);

			UMath::Matrix4 rotation;
			rotation.Rotate(NyaVec3(rX * 0.01745329, rY * 0.01745329, rZ * 0.01745329));
			obj->mMatrix = (UMath::Matrix4)(obj->mMatrix * rotation);

			obj->mMatrix.x *= scale;
			obj->mMatrix.y *= scale;
			obj->mMatrix.z *= scale;
			obj->mMatrix.p = obj->vColPosition;
		}

		data->styleRanking -= styleDecay * delta * Sim::Internal::mSystem->mSpeed;
		if (data->styleRanking < 0) {
			data->styleRanking = 0;
			memset(data->stylesCalledOut, 0, sizeof(data->stylesCalledOut));
		}

		data->attackTimer -= delta * Sim::Internal::mSystem->mSpeed;
		if (data->attackTimer <= 0) {
			auto cars = GetActiveVehicles();
			for (auto& car: cars) {
				if (!CanCarBeTargeted(car)) continue;

				auto pos = *car->GetPosition();
				auto dist = (pos - obj->vColPosition).length();
				if (dist < targetRange) {
					// sometimes teleport to player instead of attacking if far away
					auto ply = *GetLocalPlayerVehicle()->GetPosition();
					if ((obj->vColPosition - ply).length() > attackRange && PercentageChanceCheck(20)) {
						VergilTeleport(obj);
					}
					else {
						if (PercentageChanceCheck(75)) {
							bool up = PercentageChanceCheck(50);
							VergilGenericAttack(obj, attackRange, up ? 15 : 0);

							static auto sound1 = NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/vergil/swing_hard.mp3");
							static auto sound2 = NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/vergil/up.mp3");

							auto sound = up ? sound2 : sound1;
							if (sound) {
								NyaAudio::SetVolume(sound, FEDatabase->mUserProfile->TheOptionsSettings.TheAudioSettings.SoundEffectsVol * attackVolume);
								NyaAudio::Play(sound);
							}
						}
						else {
							VergilForwardAttack(obj);
						}
						VergilStyleAnnouncer(obj);
					}
					data->attackTimer = attackFrequency;
					break;
				}
			}
		}

		if (data->audio) {
			auto volume = (sfxRange - plyDist) / sfxRange;
			volume *= sfxVolume;
			if (volume > 1) volume = 1;
			if (volume < 0) volume = 0;
			if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_RACING) volume = 0;
			NyaAudio::SetVolume(data->audio, FEDatabase->mUserProfile->TheOptionsSettings.TheAudioSettings.SoundEffectsVol * volume);
			if (NyaAudio::IsFinishedPlaying(data->audio)) {
				NyaAudio::Play(data->audio);
			}
		}
		else {
			data->audio = NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/vergil/btl.mp3");
		}
	}

	static void SpawnVergil(UMath::Matrix4 mat) {
		if (models.empty() || models[0]->bInvalidated) {
			models = Render3D::CreateModels("vergil.fbx");
		}

		int id = Render3DObjects::aObjects.size();
		aVergilsInWorld.push_back(id);
		Render3DObjects::aObjects.push_back(Render3DObjects::Object(models, mat, mat.p, colScale, VergilOnTick));
		Render3DObjects::aObjects[id].CustomData = new tVergilData;
	}

	void InitFunction() override {
		if (auto veh = GetLocalPlayerInterface<IRigidBody>()) {
			auto mat = UMath::Matrix4::kIdentity;
			veh->GetMatrix4(&mat);
			mat.p = *veh->GetPosition();
			mat.p += mat.x * offX;
			mat.p += mat.y * offY;
			mat.p += mat.z * offZ;

			UMath::Matrix4 rotation;
			rotation.Rotate(NyaVec3(rX * 0.01745329, rY * 0.01745329, rZ * 0.01745329));
			mat = (UMath::Matrix4)(mat * rotation);
			SpawnVergil(mat);
			DoChaosSave();
		}
	}
	bool RigProportionalChances() override { return true; }
} E_Vergil;

/*class Effect_Franklin : public ChaosEffect {
public:
	Effect_Franklin() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Franklin shows up to defend his home";
		nFrequency *= 3;
	}

	static inline std::vector<Render3D::tModel*> models;

	static inline float rX = 90;
	static inline float rY = 0;
	static inline float rZ = 0;
	static inline float offX = 0;
	static inline float offY = -1;
	static inline float offZ = 4;
	static inline float scale = 1.5;
	static inline float colScale = 0.65;
	static inline float attackFrequency = 0.5;
	static inline float sfxRange = 250;
	static inline float sfxVolume = 1;
	static inline float attackVolume = 4;
	static inline float targetRange = 20;
	static inline float attackRange = 25;

	static inline std::vector<int> aFranklinsInWorld;

	struct tFranklinData {
		NyaVec3 direction = {};
		double attackTimer = 0;
		NyaAudio::NyaSound audio = 0;
	};

	static void FranklinOnTick(Render3DObjects::Object* obj, double delta) {
		auto data = (tFranklinData*)obj->CustomData;

		NyaVec3 plyPos = {0,0,0};
		auto ply = GetLocalPlayerVehicle();
		if (ply) plyPos = *ply->GetPosition();

		auto plyDist = (obj->vColPosition - plyPos).length();

		if (auto closestCar = GetClosestActiveVehicle(obj->vColPosition)) {
			WCollisionMgr::GetWorldHeightAtPointRigorous((UMath::Vector3*)&obj->vColPosition, &obj->vColPosition.y, nullptr);

			auto dir = (obj->vColPosition - *closestCar->GetPosition());
			dir.y = 0;
			dir.Normalize();
			data->direction = -dir;

			obj->mMatrix = NyaMat4x4::LookAt(dir);

			UMath::Matrix4 rotation;
			rotation.Rotate(NyaVec3(rX * 0.01745329, rY * 0.01745329, rZ * 0.01745329));
			obj->mMatrix = (UMath::Matrix4)(obj->mMatrix * rotation);

			obj->mMatrix.x *= scale;
			obj->mMatrix.y *= scale;
			obj->mMatrix.z *= scale;
			obj->mMatrix.p = obj->vColPosition;
		}

		data->attackTimer -= delta * Sim::Internal::mSystem->mSpeed;
		if (data->attackTimer <= 0) {
			auto cars = GetActiveVehicles();
			for (auto& car: cars) {
				//if (!CanCarBeTargeted(car)) continue;

				auto pos = *car->GetPosition();
				auto dist = (pos - obj->vColPosition).length();
				if (dist < targetRange) {

					data->attackTimer = attackFrequency;
					break;
				}
			}
		}

		if (data->audio) {
			auto volume = (sfxRange - plyDist) / sfxRange;
			volume *= sfxVolume;
			if (volume > 1) volume = 1;
			if (volume < 0) volume = 0;
			if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_RACING) volume = 0;
			NyaAudio::SetVolume(data->audio, FEDatabase->mUserProfile->TheOptionsSettings.TheAudioSettings.SoundEffectsVol * volume);
			if (NyaAudio::IsFinishedPlaying(data->audio)) {
				NyaAudio::Play(data->audio);
			}
		}
		else {
			data->audio = NyaAudio::LoadFile("CwoeeChaos/data/sound/effect/franklin/bugatti.mp3");
		}
	}

	static void SpawnFranklin(UMath::Matrix4 mat) {
		if (models.empty() || models[0]->bInvalidated) {
			models = Render3D::CreateModels("franklin.fbx");
		}

		int id = Render3DObjects::aObjects.size();
		aFranklinsInWorld.push_back(id);
		Render3DObjects::aObjects.push_back(Render3DObjects::Object(models, mat, mat.p, colScale, FranklinOnTick));
		Render3DObjects::aObjects[id].CustomData = new tFranklinData;
	}

	void InitFunction() override {
		if (auto veh = GetLocalPlayerInterface<IRigidBody>()) {
			auto mat = UMath::Matrix4::kIdentity;
			veh->GetMatrix4(&mat);
			mat.p = *veh->GetPosition();
			mat.p += mat.x * offX;
			mat.p += mat.y * offY;
			mat.p += mat.z * offZ;

			UMath::Matrix4 rotation;
			rotation.Rotate(NyaVec3(rX * 0.01745329, rY * 0.01745329, rZ * 0.01745329));
			mat = (UMath::Matrix4)(mat * rotation);
			SpawnFranklin(mat);
			DoChaosSave();
		}
	}
	bool RigProportionalChances() override { return true; }
} E_Franklin;*/