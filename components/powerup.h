namespace Powerups {
	void PlayAudioFromCar(NyaAudio::NyaSound sound, IRigidBody* veh) {
		if (!sound) return;

		float volume = 1.0;
		if (veh != GetLocalPlayerInterface<IRigidBody>()) {
			auto plyDist = (*GetLocalPlayerVehicle()->GetPosition() - *veh->GetPosition()).length();
			float sfxRange = 200.0;

			volume = (sfxRange - plyDist) / sfxRange;
			if (volume > 1) volume = 1;
			if (volume < 0) volume = 0;
		}

		NyaAudio::SetVolume(sound, volume * GetSFXVolume());
		NyaAudio::Play(sound);
	}

	void PlayAudioFromCar(NyaAudio::NyaSound sound, IVehicle* veh) {
		return PlayAudioFromCar(sound, veh->mCOMObject->Find<IRigidBody>());
	}

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

		std::vector<int> aBombsInWorld;

		void ExplosionSFX() {
			static auto sound = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/puttbang.wav");
			if (sound) {
				NyaAudio::SetVolume(sound, GetSFXVolume());
				NyaAudio::Play(sound);
			}
		}

		void ExplosionSFX(IVehicle* veh) {
			static auto sound = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/puttbang.wav");
			PlayAudioFromCar(sound, veh);
		}

		void ExplodeBomb(Render3DObjects::Object* obj) {
			ExplosionSFX();
			obj->aModels.clear();
		}

		void ExplodeCar(IVehicle* car, bool deadly) {
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
			if (deadly) {
				DestroyCar(car);
			}
		}

		template<bool deadly>
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
					if (deadly) {
						SM64::TakeInstakillDamage();
					}
					ExplodeBomb(obj);
				}
			}

			auto cars = GetActiveVehicles();
			for (auto& car : cars) {
				auto distFromCar = (*car->GetPosition() - obj->mMatrix.p).length();
				if (distFromCar < 5) {
					if (!IsCarDestroyed(car)) {
						ExplodeCar(car, deadly);
						ExplodeBomb(obj);
					}
				}
			}
		}

		template<bool deadly>
		void SpawnBomb(UMath::Matrix4 mat) {
			static std::vector<Render3D::tModel*> models;
			if (models.empty() || models[0]->bInvalidated) {
				Render3D::ModelLoaderConfig.nVertexColorValue = 0xFF808080;
				models = Render3D::CreateModels("pickup.fbx");
				Render3D::ModelLoaderConfig.Reset();
			}

			aBombsInWorld.push_back(Render3DObjects::aObjects.size());
			Render3DObjects::aObjects.push_back(new Render3DObjects::Object("bomb", models, mat, {0,0,0}, 0, BombOnTick<deadly>));
		}

		template<bool deadly>
		void SpawnBomb(IRigidBody* veh) {
			auto mat = UMath::Matrix4::kIdentity;
			veh->GetMatrix4(&mat);
			mat.p = *veh->GetPosition();
			GetWorldHeightAtPoint_WithCustom((UMath::Vector3*)&mat.p, &mat.p.y, nullptr);
			mat.p += mat.x * offX;
			mat.p += mat.y * offY;
			mat.p += mat.z * offZ;
			SpawnBomb<deadly>(mat);

			static auto sound = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/pickgen.wav");
			PlayAudioFromCar(sound, veh);
		}
	}

	namespace ReVoltFirework {
		std::vector<Render3D::tModel*> models;

		float rX = 0;
		float rY = 0;
		float rZ = 0;
		float rotOffX = 10;
		float rotOffXNoTarget = 15;
		float rotOffY = 0;
		float rotOffZ = 0;
		float offX = 0;
		float offY = 1;
		float offZ = 6;
		float scale = 2;
		float moveSpeed = 55;
		float rotSpeed = 2.5;
		float inFrontThreshold = 0.6;
		float crosshairSize = 0.02;
		float sfxVolume = 0.33;

		NyaAudio::NyaSound FireSound = 0;
		NyaAudio::NyaSound ExplodeSound = 0;

		void DrawCrosshair(IVehicle* target, bool isPlayerCrosshair) {
			bVector3 screenPos;
			auto worldPos = WorldToRenderCoords(*target->GetPosition());
			eViewPlatInterface::GetScreenPosition(&eViews[EVIEW_PLAYER1], &screenPos, (bVector3*)&worldPos);

			screenPos.x /= (double)nResX;
			screenPos.y /= (double)nResY;

			static auto texture = LoadTexture_SetDir("CwoeeChaos/data/textures/firework_crosshair.png");
			DrawRectangle(screenPos.x - crosshairSize * GetAspectRatioInv(), screenPos.x + crosshairSize * GetAspectRatioInv(), screenPos.y - crosshairSize, screenPos.y + crosshairSize, isPlayerCrosshair ? NyaDrawing::CNyaRGBA32(0,255,0,255) : NyaDrawing::CNyaRGBA32(255,0,0,255), 0, texture);
		}

		IVehicle* PickTarget(IVehicle* user) {
			//return GetClosestActiveVehicle(user, true, inFrontThreshold);
			return GetMostInFrontActiveVehicle(user, 200, inFrontThreshold);
		}

		void DrawPlayerCrosshair(IVehicle* target) {
			if (!target) return;
			DrawCrosshair(target, true);
		}

		struct tFireworkData {
			IVehicle* target;
			UMath::Vector3 currentDir;
			float speed;
			float timeLeft;
		};

		void FireworkAttack_Box3D(NyaVec3 colPosition, b3BodyId bodyId, float power, float angMult, float maxDist) {
			float objectMass = 250.0; // temp

			auto bodyPos = b3Body_GetPosition(bodyId);

			auto pos = NyaVec3(bodyPos.x,bodyPos.y,bodyPos.z);
			auto dist = (pos - colPosition);
			if (dist.length() < maxDist) {
				auto impulse = dist * (power * objectMass / 1000.0 * std::min((maxDist - dist.length()) * 2.0 / maxDist, 1.0) / std::max(dist.length(), 0.01));

				auto vel = b3Body_GetLinearVelocity(bodyId);
				auto avel = b3Body_GetAngularVelocity(bodyId);
				vel.x += impulse.x;
				vel.y += impulse.y;
				vel.z += impulse.z;
				avel.x += impulse.x * angMult;
				avel.y += impulse.y * angMult;
				avel.z += impulse.z * angMult;
				b3Body_SetLinearVelocity(bodyId, vel);
				b3Body_SetAngularVelocity(bodyId, avel);
			}
		}

		void BombOnTick(Render3DObjects::Object* obj, double delta) {
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

			auto cars = GetActiveRigidBodies();
			for (auto& car : cars) {
				if (!car->mCOMObject->Find<IVehicle>()) continue;

				auto distFromCar = (*car->GetPosition() - obj->mMatrix.p).length();
				if (distFromCar < 4) {
					data->timeLeft = 0;
					if (!NyaAudio::IsFinishedPlaying(FireSound)) {
						NyaAudio::Stop(FireSound);
					}
				}
			}

			float groundY = -9999;
			GetWorldHeightAtPoint_WithCustom((UMath::Vector3*)&obj->mMatrix.p, &groundY, nullptr);
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
					NyaAudio::SetVolume(ExplodeSound, GetSFXVolume() * sfxVolume);
					NyaAudio::Play(ExplodeSound);
				}

				float fExplosionPower = 15;
				float fExplosionAngVelocityMult = 0.25;
				float fExplosionMaxDistance = 10;

				if (CustomPhysicsBall::bEnabled) {
					FireworkAttack_Box3D(obj->mMatrix.p, CustomPhysicsBall::BallBody, fExplosionPower, fExplosionAngVelocityMult, fExplosionMaxDistance);
				}
				for (auto& phys : CustomPhysicsObjects::aPhysicsObjects) {
					FireworkAttack_Box3D(obj->mMatrix.p, phys->nB3Body, fExplosionPower, fExplosionAngVelocityMult, fExplosionMaxDistance);
				}

				for (auto& car : cars) {
					auto dist = (*car->GetPosition() - obj->mMatrix.p);
					if (dist.length() < fExplosionMaxDistance) {
						auto cb = car->mCOMObject->Find<ICollisionBody>();

						auto impulse = dist * (fExplosionPower * car->GetMass() / 1000.0 * std::min((fExplosionMaxDistance - dist.length()) * 2.0 / fExplosionMaxDistance, 1.0) / std::max(dist.length(), 0.01));

						if (cb && cb->IsAttachedToWorld()) {
							cb->AttachedToWorld(false, 50.0);
						}

						auto vel = *car->GetLinearVelocity();
						auto avel = *car->GetAngularVelocity();
						vel += impulse;
						avel += impulse * fExplosionAngVelocityMult;
						car->SetLinearVelocity(&vel);
						car->SetAngularVelocity(&avel);

						if (auto iveh = car->mCOMObject->Find<IVehicle>()) {
							if (iveh->GetDriverClass() == DRIVER_HUMAN && SM64::bEnabled) {
								SM64::OnTakeDamage(1, obj->vColPosition, true);
							}

							if (!IsCarDestroyed(iveh) && iveh->GetDriverClass() == DRIVER_COP) {
								DestroyCar(iveh);
							}
						}
					}
				}

				obj->aModels.clear();
			}
		}

		void SpawnBomb(UMath::Matrix4 mat, IVehicle* target) {
			if (models.empty() || models[0]->bInvalidated) {
				models = Render3D::CreateModels("firework.fbx");
			}

			int id = Render3DObjects::aObjects.size();
			Render3DObjects::aObjects.push_back(new Render3DObjects::Object("firework", models, mat, {0,0,0}, 0, BombOnTick));

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
			Render3DObjects::aObjects[id]->CustomData = data;
		}

		void LaunchRocketFromPlayer(IRigidBody* veh, IVehicle* target) {
			if (!FireSound) FireSound = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/firefire.wav");
			if (!ExplodeSound) ExplodeSound = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/firebang.wav");

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
				NyaAudio::SetVolume(FireSound, GetSFXVolume() * sfxVolume);
				NyaAudio::Play(FireSound);
			}
		}
	}

	namespace HeavyBall {
		void SpawnObject(NyaVec3 pos, NyaVec3 vel) {
			static auto mdl = Render3D::CreateModels("heavyblock.fbx");

			CustomPhysicsObjects::CustomPhysicsObject objData;
			objData.aModels = mdl;
			objData.vModelSize = {1.5,1.5,1.5};
			objData.bRemoveOnSafehouse = false;
			objData.bRemoveOnOutOfBounds = false;
			objData.bRemoveOnOutOfRange = false;
			objData.bAffectGamePhysics = true;
			objData.sDebugName = "metalball_save";
			//objData.bUseExpensiveCollisionCheck = true;
			//objData.pCollisionSound = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/beachball.wav");
			CustomPhysicsObjects::CreatePhysicsObject(objData, CustomPhysicsObjects::BOX, pos, vel);
		}

		void SpawnInFrontOfCar(IRigidBody* rb) {
			auto ply = *rb->GetPosition();
			auto vel = *rb->GetLinearVelocity();
			UMath::Vector3 fwd;
			rb->GetForwardVector(&fwd);

			NyaVec3 pos = ply;
			pos += fwd * 5;
			pos.y += 2;
			SpawnObject(pos, vel);
		}

		void SpawnBehindCar(IRigidBody* rb) {
			auto ply = *rb->GetPosition();
			auto vel = *rb->GetLinearVelocity();
			UMath::Vector3 fwd;
			rb->GetForwardVector(&fwd);

			NyaVec3 pos = ply;
			pos -= fwd * 5;
			pos.y += 2;
			SpawnObject(pos, vel);
		}
	}

	enum ePowerup {
		//POWERUP_SHOCKWAVE,
		POWERUP_PUTTYBOMB,
		POWERUP_FIREWORK,
		POWERUP_FIREWORKPACK,
		//POWERUP_WATERBOMB,
		POWERUP_CLONE,
		//POWERUP_OILSLICK, // todo? copying the relevant collision polys out would work here i think
		POWERUP_ELECTROPULSE,
		POWERUP_CHROMEBALL,
		//POWERUP_TURBO,
		POWERUP_STAR,
		NUM_POWERUPS
	};

	const char* aPowerupSpriteNames[] = {
			//"CwoeeChaos/data/textures/revolt_1.png",
			"CwoeeChaos/data/textures/revolt_2.png",
			"CwoeeChaos/data/textures/revolt_3.png",
			"CwoeeChaos/data/textures/revolt_4.png",
			//"CwoeeChaos/data/textures/revolt_5.png",
			"CwoeeChaos/data/textures/revolt_6.png",
			//"CwoeeChaos/data/textures/revolt_7.png",
			"CwoeeChaos/data/textures/revolt_8.png",
			"CwoeeChaos/data/textures/revolt_9.png",
			//"CwoeeChaos/data/textures/revolt_10.png",
			"CwoeeChaos/data/textures/revolt_12.png",
	};
	IDirect3DTexture9* aPowerupTextures[NUM_POWERUPS] = {};

	int PlayerPowerup = NUM_POWERUPS;
	int PlayerPowerupCount = 0;
	double fPowerupRollTime = 0.0;

	bool ExecutePowerup(IVehicle* user, int powerup) {
		switch (powerup) {
			case POWERUP_CLONE:
				ReVoltBomb::SpawnBomb<false>(user->mCOMObject->Find<IRigidBody>());
				return true;
			case POWERUP_PUTTYBOMB:
				ReVoltBomb::ExplosionSFX(user);
				ReVoltBomb::ExplodeCar(user, false);
				return true;
			case POWERUP_FIREWORK:
			case POWERUP_FIREWORKPACK:
				ReVoltFirework::LaunchRocketFromPlayer(user->mCOMObject->Find<IRigidBody>(), ReVoltFirework::PickTarget(user));
				return true;
			case POWERUP_CHROMEBALL: {
				HeavyBall::SpawnBehindCar(user->mCOMObject->Find<IRigidBody>());

				static auto sound = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/balldrop.wav");
				PlayAudioFromCar(sound, user);
			} return true;
			case POWERUP_STAR: {
				static auto sound = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/starfire.wav");
				if (sound) {
					NyaAudio::SetVolume(sound, GetSFXVolume());
					NyaAudio::Play(sound);
				}

				auto cars = GetActiveVehicles();
				for (auto& car : cars) {
					if (car == user) continue;
					if (IsCarDestroyed(car)) continue;
					ReVoltBomb::ExplodeCar(car, false);
				}
				return true;
			} break;
		}
		return false;
	}

	double fPlayerElectroPulseTime = 0.0;
	bool ProcessPowerup(IVehicle* user, int powerup, double delta) {
		switch (powerup) {
			case POWERUP_PUTTYBOMB:
				return ExecutePowerup(user, powerup);
			case POWERUP_FIREWORK:
			case POWERUP_FIREWORKPACK:
				ReVoltFirework::DrawPlayerCrosshair(ReVoltFirework::PickTarget(GetLocalPlayerVehicle()));
				break;
			case POWERUP_ELECTROPULSE: {
				static auto loop1 = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/electro.wav");
				static auto loop2 = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/electrozap.wav");
				if (loop1) {
					NyaAudio::SetVolume(loop1, GetSFXVolume());
					NyaAudio::Play(loop1);
				}

				fPlayerElectroPulseTime -= delta;
				if (fPlayerElectroPulseTime <= 0.0) {
					NyaAudio::Stop(loop1);
					NyaAudio::Stop(loop2);
					return true;
				}

				auto plyPos = *user->GetPosition();
				auto cars = GetActiveVehicles();
				for (auto& car : cars) {
					if (car == user) continue;
					if (IsCarDestroyed(car)) continue;

					auto dist = (plyPos - *car->GetPosition()).length();
					if (dist < 5) {
						if (!IsCarDestroyed(car)) {
							DestroyCar(car);
						}

						NyaAudio::SetVolume(loop2, GetSFXVolume());
						NyaAudio::Play(loop2);
					}
				}
			} break;
		}
		return false;
	}

	void RollPowerup(IVehicle* veh) {
		if (veh->GetDriverClass() != DRIVER_HUMAN) {
			std::vector<int> powerupsAvailable;
			//powerupsAvailable.push_back(POWERUP_SHOCKWAVE);
			powerupsAvailable.push_back(POWERUP_PUTTYBOMB);
			powerupsAvailable.push_back(POWERUP_FIREWORK);
			//powerupsAvailable.push_back(POWERUP_FIREWORKPACK);
			//powerupsAvailable.push_back(POWERUP_WATERBOMB);
			powerupsAvailable.push_back(POWERUP_CLONE);
			//powerupsAvailable.push_back(POWERUP_OILSLICK);
			//powerupsAvailable.push_back(POWERUP_ELECTROPULSE);
			powerupsAvailable.push_back(POWERUP_CHROMEBALL);
			//powerupsAvailable.push_back(POWERUP_TURBO);
			//powerupsAvailable.push_back(POWERUP_STAR); // too OP
			ExecutePowerup(veh, powerupsAvailable[rand() % powerupsAvailable.size()]);
		}
		else {
			PlayerPowerup = rand() % NUM_POWERUPS;
			//PlayerPowerupCount = (PlayerPowerup == POWERUP_FIREWORKPACK || PlayerPowerup == POWERUP_WATERBOMB) ? 3 : 1;
			PlayerPowerupCount = PlayerPowerup == POWERUP_FIREWORKPACK ? 3 : 1;
			fPowerupRollTime = 4.0;
			fPlayerElectroPulseTime = 10.0;
		}

		static auto sound = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/pickup.wav");
		PlayAudioFromCar(sound, veh);
	}

	bool PlayerHasPowerup() {
		return PlayerPowerup != NUM_POWERUPS && PlayerPowerupCount > 0;
	}

	float fSpriteY = 0.3;
	float fSpriteSize = 0.05;
	void DrawPlayerPowerup() {
		static auto texBase = LoadTexture_SetDir("CwoeeChaos/data/textures/revolt_base.png");
		if (texBase) {
			DrawRectangle(0.5 - (fSpriteSize * GetAspectRatioInv()), 0.5 + (fSpriteSize * GetAspectRatioInv()), fSpriteY - fSpriteSize, fSpriteY + fSpriteSize, {255,255,255,255}, 0, texBase);
		}

		int powerupId = PlayerPowerup;
		uint8_t powerupAlpha = 255;
		if (fPowerupRollTime > 0) {
			if (fPowerupRollTime > 0.5) {
				int scroll = fPowerupRollTime > 1.0 ? 20 : 10;
				powerupId = fPowerupRollTime * scroll;
				powerupId %= NUM_POWERUPS;
			}
			powerupAlpha = 127;
		}

		auto& tex = aPowerupTextures[powerupId];
		if (!tex) {
			tex = LoadTexture_SetDir(aPowerupSpriteNames[powerupId]);
		}

		if (tex) {
			DrawRectangle(0.5 - (fSpriteSize * GetAspectRatioInv()), 0.5 + (fSpriteSize * GetAspectRatioInv()), fSpriteY - fSpriteSize, fSpriteY + fSpriteSize, {255,255,255,powerupAlpha}, 0, tex);
		}
	}

	void OnTick() {
		if (IsChaosBlocked()) return;
		if (!PlayerHasPowerup()) return;

		static CNyaTimer gTimer;
		gTimer.Process();

		fPowerupRollTime -= gTimer.fDeltaTime;
		DrawPlayerPowerup();
		if (fPowerupRollTime > 0) return;

		if (auto ply = GetLocalPlayer()) {
			ply->ResetGameBreaker(false);
		}

		if (ProcessPowerup(GetLocalPlayerVehicle(), PlayerPowerup, gTimer.fDeltaTime) || ((IsKeyJustPressed('X') || IsPadKeyJustPressed(NYA_PAD_KEY_X)) && ExecutePowerup(GetLocalPlayerVehicle(), PlayerPowerup))) {
			PlayerPowerupCount--;
			if (PlayerPowerupCount <= 0) {
				PlayerPowerup = NUM_POWERUPS;
			}
		}
	}

	void OnTick3D() {
		if (IsChaosBlocked()) return;
		if (!PlayerHasPowerup()) return;
		if (fPowerupRollTime > 0) return;

		if (PlayerPowerup == POWERUP_ELECTROPULSE) {
			NyaDrawing::CNyaRGBA32 tmp;
			tmp.b = 0;
			tmp.g = 255;
			tmp.r = 255;
			tmp.a = 255;
			Render3D::ModelLoaderConfig.nVertexColorValue = *(uint32_t*)&tmp;
			static auto models = Render3D::CreateModels("cube.fbx");
			Render3D::ModelLoaderConfig.Reset();

			if (auto rb = GetLocalPlayerInterface<ICollisionBody>()) {
				auto simable = rb->mCOMObject->Find<ISimable>();

				UMath::Vector3 dim;
				rb->GetDimension(&dim);

				UMath::Matrix4 mat = *rb->GetMatrix4();
				mat.p = *rb->GetPosition();

				mat.x *= dim.x;
				mat.y *= dim.y;
				mat.z *= dim.z;

				g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
				for (auto& mdl : models) {
					mdl->RenderAt(WorldToRenderMatrix(mat));
				}
				g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
			}
		}
	}

	ChloeHook Init([](){
		aDrawingLoopFunctions.push_back(OnTick);
		aDrawing3DLoopFunctions.push_back(OnTick3D);
	});
}