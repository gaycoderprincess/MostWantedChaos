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
		NyaAudio::SkipTo(sound, 0, false);
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
			if (car->GetDriverClass() == DRIVER_COP || deadly) {
				DestroyCar(car);
			}
		}

		template<bool deadly>
		void BombOnTick(Render3DObjects::Object* obj, double delta) {
			auto& rotDelta = *(float*)&obj->CustomData;

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
			rotDelta += delta;

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

		bool IsCarExplodeable(IVehicle* veh) {
			if (auto rb = veh->mCOMObject->Find<IRBVehicle>()) {
				return rb->GetInvulnerability() == INVULNERABLE_NONE;
			}
			return true;
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
				auto iveh = car->mCOMObject->Find<IVehicle>();
				if (!iveh) continue;
				if (!IsCarExplodeable(iveh)) continue;

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
						auto iveh = car->mCOMObject->Find<IVehicle>();
						if (iveh && !IsCarExplodeable(iveh)) continue;

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

						if (iveh) {
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
		template<bool save>
		void SpawnObject(NyaVec3 pos, NyaVec3 vel) {
			static auto mdl = Render3D::CreateModels("heavyblock.fbx");

			CustomPhysicsObjects::CustomPhysicsObject objData;
			objData.aModels = mdl;
			objData.vModelSize = {1.5,1.5,1.5};
			objData.bRemoveOnSafehouse = false;
			objData.bRemoveOnOutOfBounds = false;
			objData.bRemoveOnOutOfRange = false;
			objData.bAffectGamePhysics = true;
			objData.sDebugName = save ? "metalball_save" : "metalball";
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
			SpawnObject<true>(pos, vel);
		}

		void SpawnBehindCar(IRigidBody* rb) {
			auto ply = *rb->GetPosition();
			auto vel = *rb->GetLinearVelocity();
			UMath::Vector3 fwd;
			rb->GetForwardVector(&fwd);

			NyaVec3 pos = ply;
			pos -= fwd * 5;
			pos.y += 2;
			SpawnObject<false>(pos, vel);
		}
	}

	enum ePowerup {
		//POWERUP_SHOCKWAVE,
		POWERUP_PUTTYBOMB,
		POWERUP_FIREWORKPACK,
		POWERUP_FIREWORK,
		//POWERUP_WATERBOMB,
		POWERUP_CLONE,
		//POWERUP_OILSLICK, // todo? copying the relevant collision polys out would work here i think
		POWERUP_ELECTROPULSE,
		POWERUP_CHROMEBALL,
		POWERUP_STAR,
		POWERUP_TURBO,
		POWERUP_INVINCIBLE,
		POWERUP_BEACHBALL,
		NUM_POWERUPS
	};

	const char* aPowerupSpriteNames[] = {
			//"CwoeeChaos/data/textures/revolt_1.png",
			"CwoeeChaos/data/textures/revolt_2.png",
			"CwoeeChaos/data/textures/revolt_4.png",
			"CwoeeChaos/data/textures/revolt_3.png",
			//"CwoeeChaos/data/textures/revolt_5.png",
			"CwoeeChaos/data/textures/revolt_6.png",
			//"CwoeeChaos/data/textures/revolt_7.png",
			"CwoeeChaos/data/textures/revolt_8.png",
			"CwoeeChaos/data/textures/revolt_9.png",
			"CwoeeChaos/data/textures/revolt_12.png",
			"CwoeeChaos/data/textures/mk64_1.png",
			"CwoeeChaos/data/textures/mk64_2.png",
			"CwoeeChaos/data/textures/powerup_beachball.png",
	};
	IDirect3DTexture9* aPowerupTextures[NUM_POWERUPS] = {};

	void SpawnPhysicalBeachBall(NyaVec3 pos, NyaVec3 vel) {
		static auto sound = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/beachball.wav");

		static auto mdl = Render3D::CreateModels("beachball.fbx");

		CustomPhysicsObjects::CustomPhysicsObject objData;
		objData.aModels = mdl;
		objData.vModelSize = {1,1,1};
		objData.bRemoveOnSafehouse = true;
		objData.bRemoveOnOutOfBounds = true;
		objData.bRemoveOnOutOfRange = true;
		objData.bAffectGamePhysics = true;
		objData.sDebugName = "beachball";
		objData.pCollisionSound = sound;
		CustomPhysicsObjects::CreatePhysicsObject(objData, CustomPhysicsObjects::SPHERE, pos, vel);
	}

	float fSpriteY = 0.3;
	float fSpriteSize = 0.05;

	struct PowerupState {
		IVehicle* pUser = nullptr;
		bool bIsLocalPlayer = false;
		int PowerupID = NUM_POWERUPS;
		int PowerupCount = 0;
		double fRollTime = 0.0;
		double fTimeSinceLastFire = 0.0;
		double fElectroTime = 0.0;

		// audio
		NyaAudio::NyaSound electro = 0;
		NyaAudio::NyaSound electrozap = 0;
		NyaAudio::NyaSound balldrop = 0;
		NyaAudio::NyaSound starfire = 0;
		NyaAudio::NyaSound wbombfire = 0;

		bool ExecutePowerup() {
			switch (PowerupID) {
				case POWERUP_ELECTROPULSE:
					fElectroTime = 15.0;
					return true;
				case POWERUP_CLONE:
					ReVoltBomb::SpawnBomb<false>(pUser->mCOMObject->Find<IRigidBody>());
					return true;
				case POWERUP_PUTTYBOMB:
					ReVoltBomb::ExplosionSFX(pUser);
					ReVoltBomb::ExplodeCar(pUser, false);
					return true;
				case POWERUP_FIREWORK:
				case POWERUP_FIREWORKPACK:
					ReVoltFirework::LaunchRocketFromPlayer(pUser->mCOMObject->Find<IRigidBody>(), ReVoltFirework::PickTarget(pUser));
					return true;
				case POWERUP_CHROMEBALL: {
					HeavyBall::SpawnBehindCar(pUser->mCOMObject->Find<IRigidBody>());

					if (!balldrop) balldrop = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/balldrop.wav");
					PlayAudioFromCar(balldrop, pUser);
					return true;
				} break;
				case POWERUP_TURBO: {
					pUser->SetSpeed(TOMPS(300));
					return true;
				} break;
				case POWERUP_INVINCIBLE: {
					if (auto ply = pUser->mCOMObject->Find<IRBVehicle>()) {
						ply->SetInvulnerability(INVULNERABLE_FROM_RESET, 15.0);
					}
					return true;
				} break;
				case POWERUP_BEACHBALL: {
					if (!wbombfire) wbombfire = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/wbombfire.wav");
					PlayAudioFromCar(wbombfire, pUser);

					auto rb = pUser->mCOMObject->Find<IRigidBody>();

					UMath::Vector3 dim;
					UMath::Vector3 fwd;
					rb->GetDimension(&dim);
					rb->GetForwardVector(&fwd);

					auto pos = *rb->GetPosition();
					auto vel = *rb->GetLinearVelocity();
					pos += fwd * (dim.z + 2.5);
					vel += fwd * 50;
					SpawnPhysicalBeachBall(pos, vel);
					return true;
				} break;
				case POWERUP_STAR: {
					if (!starfire) starfire = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/starfire.wav");
					if (starfire) {
						NyaAudio::SetVolume(starfire, GetSFXVolume());
						NyaAudio::Play(starfire);
					}

					auto cars = GetActiveVehicles();
					for (auto& car : cars) {
						if (car == pUser) continue;
						if (IsCarDestroyed(car)) continue;
						ReVoltBomb::ExplodeCar(car, false);
					}
					return true;
				} break;
			}
			return false;
		}

		bool ProcessPowerup(double delta) {
			switch (PowerupID) {
				case POWERUP_PUTTYBOMB:
					return ExecutePowerup();
				case POWERUP_FIREWORK:
				case POWERUP_FIREWORKPACK: {
					auto target = ReVoltFirework::PickTarget(pUser);
					if (target && (bIsLocalPlayer || target == GetLocalPlayerVehicle())) {
						ReVoltFirework::DrawCrosshair(target, bIsLocalPlayer);
					}
				} break;
			}
			return false;
		}

		void GivePowerup(int id) {
			PowerupID = id;
			//PowerupCount = (PowerupID == POWERUP_FIREWORKPACK || PowerupID == POWERUP_WATERBOMB) ? 3 : 1;
			PowerupCount = PowerupID == POWERUP_FIREWORKPACK ? 3 : 1;
			fRollTime = 4.0;

			static auto sound = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/pickup.wav");
			PlayAudioFromCar(sound, pUser);
		}

		void RollPowerup() {
			std::vector<int> powerupsAvailable;
			for (int i = 0; i < NUM_POWERUPS; i++) {
				if (pUser->GetDriverClass() != DRIVER_HUMAN) {
					if (i == POWERUP_STAR) continue; // too OP
				}
				powerupsAvailable.push_back(i);
			}
			GivePowerup(powerupsAvailable[rand()%powerupsAvailable.size()]);
		}

		bool HasPowerup() {
			return PowerupID != NUM_POWERUPS && PowerupCount > 0;
		}

		void Render() {
			static auto texBase = LoadTexture_SetDir("CwoeeChaos/data/textures/revolt_base.png");
			if (texBase) {
				DrawRectangle(0.5 - (fSpriteSize * GetAspectRatioInv()), 0.5 + (fSpriteSize * GetAspectRatioInv()), fSpriteY - fSpriteSize, fSpriteY + fSpriteSize, {255,255,255,255}, 0, texBase);
			}

			int powerupId = PowerupID;
			uint8_t powerupAlpha = 255;
			if (fRollTime > 0) {
				if (fRollTime > 0.75) {
					int scroll = 20;
					if (fRollTime < 2.0) scroll = 10;
					if (fRollTime < 1.5) scroll = 5;

					// always make sure it smoothly lands on the powerup it should
					powerupId = PowerupID + ((fRollTime - 0.75) * scroll);
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

		bool HasFired() {
			if (pUser == GetLocalPlayerVehicle()) {
				return IsKeyJustPressed('X') || IsPadKeyJustPressed(NYA_PAD_KEY_X);
			}

			if (PowerupID == POWERUP_FIREWORK || PowerupID == POWERUP_FIREWORKPACK || PowerupID == POWERUP_BEACHBALL) {
				if (fTimeSinceLastFire < 0.5) return false;

				return Powerups::ReVoltFirework::PickTarget(pUser) != nullptr;
			}
			if (PowerupID == POWERUP_TURBO) {
				return GetLocalPlayerInterface<IInput>()->GetControls()->fGas >= 0.95;
			}
			return true;
		}

		std::vector<IVehicle*> GetZappedCars() {
			std::vector<IVehicle*> out;

			auto plyPos = *pUser->GetPosition();
			auto cars = GetActiveVehicles();
			for (auto& car : cars) {
				if (car == pUser) continue;
				if (IsCarDestroyed(car)) continue;

				auto dist = (plyPos - *car->GetPosition()).length();
				if (dist < 15) {
					if (car->mCOMObject->Find<ISuspension>()) {
						out.push_back(car);
					}
				}
			}

			return out;
		}

		void ProcessLastingEffects(double delta) {
			if (fElectroTime > 0.0) {
				if (!electro) electro = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/electro.wav");
				if (!electrozap) electrozap = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/electrozap.wav");

				if (electro && NyaAudio::IsFinishedPlaying(electro)) {
					PlayAudioFromCar(electro, pUser);
				}

				bool doZap = false;

				auto cars = GetZappedCars();
				for (auto& car : cars) {
					if (auto sus = car->mCOMObject->Find<ISuspension>()) {
						for (int i = 0; i < 4; i++) {
							sus->SetWheelAngularVelocity(i, 0.0);
						}
					}
				}

				if (!cars.empty()) {
					if (electrozap && NyaAudio::IsFinishedPlaying(electrozap)) {
						PlayAudioFromCar(electrozap, pUser);
					}
				}
				else {
					NyaAudio::Stop(electrozap);
				}

				fElectroTime -= delta;
				if (fElectroTime <= 0.0) {
					NyaAudio::Stop(electro);
					NyaAudio::Stop(electrozap);
				}
			}
		}

		void Process(double delta) {
			ProcessLastingEffects(delta);

			fTimeSinceLastFire += delta;

			fRollTime -= delta;
			if (fRollTime > 0) return;

			if (ProcessPowerup(delta) || (HasFired() && ExecutePowerup())) {
				fTimeSinceLastFire = 0.0;

				PowerupCount--;
				if (PowerupCount <= 0) {
					PowerupID = NUM_POWERUPS;

					//if (bIsLocalPlayer) {
					//	GetLocalPlayer()->ResetGameBreaker(true);
					//}
				}
			}
		}

		static void RenderZappingCar(IVehicle* veh) {
			if (!IsVehicleValidAndActive(veh)) return;

			NyaDrawing::CNyaRGBA32 tmp;
			tmp.b = 0;
			tmp.g = 255;
			tmp.r = 255;
			tmp.a = 255;
			Render3D::ModelLoaderConfig.nVertexColorValue = *(uint32_t*)&tmp;
			static auto models = Render3D::CreateModels("cube.fbx");
			Render3D::ModelLoaderConfig.Reset();

			if (auto rb = veh->mCOMObject->Find<ICollisionBody>()) {
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

		static void RenderZappedCar(IVehicle* veh) {
			if (!IsVehicleValidAndActive(veh)) return;

			NyaDrawing::CNyaRGBA32 tmp;
			tmp.b = 255;
			tmp.g = 0;
			tmp.r = 0;
			tmp.a = 255;
			Render3D::ModelLoaderConfig.nVertexColorValue = *(uint32_t*)&tmp;
			static auto models = Render3D::CreateModels("cube.fbx");
			Render3D::ModelLoaderConfig.Reset();

			if (auto rb = veh->mCOMObject->Find<ICollisionBody>()) {
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

		void Process3D() {
			if (fElectroTime > 0.0) {
				RenderZappingCar(pUser);

				auto cars = GetZappedCars();
				for (auto& car : cars) {
					RenderZappedCar(car);
				}
			}
		}

		bool IsValid() {
			if (bIsLocalPlayer) pUser = GetLocalPlayerVehicle();

			auto cars = GetActiveVehicles();
			for (auto& car : cars) {
				if (car->GetDriverClass() != DRIVER_HUMAN && car->GetDriverClass() != DRIVER_RACER) continue;

				if (car == pUser) return true;
			}
			return false;
		}
	};
	std::vector<PowerupState> aPowerupStates;

	PowerupState* GetPowerupState(IVehicle* veh) {
		for (auto& state : aPowerupStates) {
			if (veh == GetLocalPlayerVehicle() && state.bIsLocalPlayer) {
				state.pUser = veh;
			}

			if (state.pUser == veh) {
				return &state;
			}
		}
		return nullptr;
	}

	void RollPowerup(IVehicle* veh) {
		if (auto state = GetPowerupState(veh)) {
			state->RollPowerup();
			return;
		}

		PowerupState state;
		state.pUser = veh;
		state.bIsLocalPlayer = veh == GetLocalPlayerVehicle();
		state.RollPowerup();
		aPowerupStates.push_back(state);

		if (state.bIsLocalPlayer) {
			CwoeeHints::AddHint("Press X to use powerups.");
		}
	}

	bool PlayerHasPowerup(IVehicle* veh) {
		if (auto state = GetPowerupState(veh)) {
			return state->HasPowerup();
		}
		return false;
	}

	bool DespawnCleanup() {
		for (auto& state : aPowerupStates) {
			if (!state.bIsLocalPlayer && !state.IsValid()) {
				aPowerupStates.erase(aPowerupStates.begin() + (&state - &aPowerupStates[0]));
				return true;
			}
		}
		return false;
	}

	void OnTick() {
		while (DespawnCleanup()) {}

		if (IsChaosBlocked()) return;

		static CNyaTimer gTimer;
		gTimer.Process();

		for (auto& state : aPowerupStates) {
			if (!state.IsValid()) continue;
			state.Process(gTimer.fDeltaTime);
		}

		auto powerup = GetPowerupState(GetLocalPlayerVehicle());
		if (!powerup) return;
		if (!powerup->HasPowerup()) return;

		powerup->Render();

		if (auto ply = GetLocalPlayer()) {
			ply->ResetGameBreaker(false);
		}
	}

	void OnTick3D() {
		if (IsChaosBlocked()) return;

		for (auto& state : aPowerupStates) {
			state.Process3D();
		}
	}

	ChloeHook Init([](){
		aDrawingLoopFunctions.push_back(OnTick);
		aDrawing3DLoopFunctions.push_back(OnTick3D);
	});
}