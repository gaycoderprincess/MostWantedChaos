namespace Powerups {
	namespace ReVoltBomb {
		float rX = 90;
		float rY = 0;
		float rZ = 0;
		float offX = 0;
		float offY = 1.5;
		float offZ = -6;
		float scale = 2;

		float rotSpeedX = 0;
		float rotSpeedY = 0;
		float rotSpeedZ = -1.5;

		float sfxRange = 200;

		std::vector<int> aBombsInWorld;

		void ExplosionSFX() {
			static auto sound = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/puttbang.wav");
			if (sound) {
				NyaAudio::SetVolume(sound, GetSFXVolume());
				NyaAudio::Play(sound);
			}
		}

		void ExplodeBomb(Render3DObjects::Object* obj) {
			ExplosionSFX();
			obj->aModels.clear();
		}

		void ExplodeCar(IVehicle* car) {
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
			if (car->GetDriverClass() == DRIVER_HUMAN && SM64::bEnabled) {
				SM64::TakeLavaDamage();
			}
			DestroyCar(car);
		}

		void BombOnTick(Render3DObjects::Object* obj, double delta) {
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

			if (IsChaosBlocked()) return;

			// instakill enemy mario
			if (SM64::bEnemyEnabled) {
				auto dist = (SM64::GetMarioWorldPos() - obj->mMatrix.p);
				if (dist.length() < 5) {
					SM64::TakeLavaDamage();
					SM64::TakeInstakillDamage();
					ExplodeBomb(obj);
				}
			}

			auto cars = GetActiveVehicles();
			for (auto& car : cars) {
				auto distFromCar = (*car->GetPosition() - obj->mMatrix.p).length();
				if (distFromCar < 5) {
					if (!IsCarDestroyed(car)) {
						ExplodeCar(car);
						ExplodeBomb(obj);
					}
				}
			}
		}

		void SpawnBomb(UMath::Matrix4 mat) {
			static std::vector<Render3D::tModel*> models;
			if (models.empty() || models[0]->bInvalidated) {
				Render3D::ModelLoaderConfig.nVertexColorValue = 0xFF808080;
				models = Render3D::CreateModels("pickup.fbx");
				Render3D::ModelLoaderConfig.Reset();
			}

			aBombsInWorld.push_back(Render3DObjects::aObjects.size());
			Render3DObjects::aObjects.push_back(new Render3DObjects::Object("bomb", models, mat, {0,0,0}, 0, BombOnTick));
		}

		void SpawnBomb(IRigidBody* veh) {
			auto mat = UMath::Matrix4::kIdentity;
			veh->GetMatrix4(&mat);
			mat.p = *veh->GetPosition();
			GetWorldHeightAtPoint_WithCustom((UMath::Vector3*)&mat.p, &mat.p.y, nullptr);
			mat.p += mat.x * offX;
			mat.p += mat.y * offY;
			mat.p += mat.z * offZ;
			SpawnBomb(mat);

			static auto sound = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/pickgen.wav");
			if (sound) {
				float volume = 1.0;
				if (veh != GetLocalPlayerInterface<IRigidBody>()) {
					auto plyDist = (*GetLocalPlayerInterface<IRigidBody>()->GetPosition() - mat.p).length();

					volume = (sfxRange - plyDist) / sfxRange;
					if (volume > 1) volume = 1;
					if (volume < 0) volume = 0;
				}

				NyaAudio::SetVolume(sound, volume * GetSFXVolume());
				NyaAudio::Play(sound);
			}
		}
	}

	enum ePowerup {
		POWERUP_FIREWORK,
		POWERUP_EXPLODE_CAR,
		POWERUP_MAGNET,
		POWERUP_REVOLT_BOMB,
		NUM_POWERUPS
	};
	int PlayerPowerup = NUM_POWERUPS;
	double fPowerupRollTime = 0.0;

	void ExecutePowerup(IVehicle* veh, int powerup) {
		auto rb = veh->mCOMObject->Find<IRigidBody>();
		if (!rb) return;

		switch (powerup) {
			case POWERUP_REVOLT_BOMB:
				ReVoltBomb::SpawnBomb(rb);
				break;
		}
	}

	void RollPowerup(IVehicle* veh) {
		auto powerup = rand() % NUM_POWERUPS;
		if (veh->GetDriverClass() != DRIVER_HUMAN) {
			ExecutePowerup(veh, powerup);
		}
		else {
			PlayerPowerup = powerup;
			fPowerupRollTime = 3.0;
		}
	}

	bool PlayerHasPowerup() {
		return PlayerPowerup != NUM_POWERUPS;
	}

	void OnTick() {
		if (!PlayerHasPowerup()) return;

		static CNyaTimer gTimer;
		gTimer.Process();

		switch (PlayerPowerup) {
			case POWERUP_EXPLODE_CAR:
				ReVoltBomb::ExplosionSFX();
				ReVoltBomb::ExplodeCar(GetLocalPlayerVehicle());
				break;
		}
	}

	ChloeHook Init([](){
		aDrawingLoopFunctions.push_back(OnTick);
	});
}