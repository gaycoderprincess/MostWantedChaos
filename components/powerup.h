namespace Powerups {
	bool bMK64Style = true;
	NyaAudio::NyaSound gPickupSound = 0;
	bool bPickupSoundPaused = false;

	bool bOverheadDisplay = true;

	float sfxVolumeMultiplier = 0.33;
	float sfxVolumeMultiplierMK64 = 1.0;

	double fTimeSinceMarioSpawned = 0.0;

	void PlayAudioFromCar(NyaAudio::NyaSound sound, IRigidBody* veh, bool mk64Volume = false) {
		if (!sound) return;

		float volume = 1.0;
		if (veh != GetLocalPlayerInterface<IRigidBody>()) {
			auto plyDist = (*GetLocalPlayerVehicle()->GetPosition() - *veh->GetPosition()).length();
			float sfxRange = 200.0;

			volume = (sfxRange - plyDist) / sfxRange;
			if (volume > 1) volume = 1;
			if (volume < 0) volume = 0;
		}

		if (volume <= 0.0) return;
		NyaAudio::SetVolume(sound, volume * GetSFXVolume() * (mk64Volume ? sfxVolumeMultiplierMK64 : sfxVolumeMultiplier));
		NyaAudio::SkipTo(sound, 0, false);
		NyaAudio::Play(sound);
	}

	void PlayAudioFromCar(NyaAudio::NyaSound sound, IVehicle* veh, bool mk64Volume = false) {
		return PlayAudioFromCar(sound, veh->mCOMObject->Find<IRigidBody>(), mk64Volume);
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
				NyaAudio::SetVolume(sound, GetSFXVolume() * sfxVolumeMultiplier);
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

		void ExplodeBomb(IVehicle* cause, Render3DObjects::Object* obj) {
			ExplosionSFX(cause);
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
						ExplodeBomb(car, obj);
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
		float inFrontThresholdAI = 0.3;
		float crosshairSize = 0.02;

		float fExplosionPower = 25;
		float fExplosionPowerChaos = 15;
		float fExplosionAngVelocityMult = 0.25;
		float fExplosionMaxDistance = 10;

		NyaAudio::NyaSound FireSound = 0;
		NyaAudio::NyaSound ExplodeSound = 0;

		void DrawCrosshair(IVehicle* target, bool isPlayerCrosshair) {
			if (!target) return;

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

		IVehicle* PickTargetBehind(IVehicle* user) {
			return GetMostInFrontActiveVehicle(user, 200, -inFrontThreshold);
		}

		IVehicle* PickTargetAI(IVehicle* user) {
			return GetMostInFrontActiveVehicle(user, 200, inFrontThresholdAI);
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
					NyaAudio::SetVolume(ExplodeSound, GetSFXVolume() * sfxVolumeMultiplier);
					NyaAudio::Play(ExplodeSound);
				}

#ifdef CWOEECHAOS
				float power = fExplosionPowerChaos;
				if (CustomPhysicsBall::bEnabled) {
					FireworkAttack_Box3D(obj->mMatrix.p, CustomPhysicsBall::BallBody, power, fExplosionAngVelocityMult, fExplosionMaxDistance);
				}
#else
				float power = fExplosionPower;
#endif
				for (auto& phys : CustomPhysicsObjects::aPhysicsObjects) {
					FireworkAttack_Box3D(obj->mMatrix.p, phys->nB3Body, power, fExplosionAngVelocityMult, fExplosionMaxDistance);
				}

				for (auto& car : cars) {
					auto dist = (*car->GetPosition() - obj->mMatrix.p);
					if (dist.length() < fExplosionMaxDistance) {
						auto iveh = car->mCOMObject->Find<IVehicle>();
						if (iveh && !IsCarExplodeable(iveh)) continue;

						auto cb = car->mCOMObject->Find<ICollisionBody>();

						auto impulse = dist * (power * car->GetMass() / 1000.0 * std::min((fExplosionMaxDistance - dist.length()) * 2.0 / fExplosionMaxDistance, 1.0) / std::max(dist.length(), 0.01));

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
				NyaAudio::SetVolume(FireSound, GetSFXVolume() * sfxVolumeMultiplier);
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
			objData.bRemoveOnSafehouse = !save;
			objData.bRemoveOnOutOfBounds = !save;
			objData.bRemoveOnOutOfRange = !save;
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

	namespace OilDrum {
		float explosionRange = 50;
		float explosionPower = 50;
		float explosionPowerAng = 25;
		float explosionPowerWeakMult = 0.1;

		void OnBarrelHit(CustomPhysicsObjects::CustomPhysicsObject* pThis, b3BodyId otherBody) {
			if (b3Body_GetType(otherBody) == b3_staticBody) return;

			auto collidedVeh = CustomPhysics::GetVehicleForB3Body(otherBody);
			if (collidedVeh && !strcmp(collidedVeh->GetVehicleName(), "copheli")) return;

			auto barrelPos = pThis->GetPosition();

			auto objs = GetActiveSharedRigidBodies(true);
			for (auto& obj : objs) {
				if (obj.pCustomObject == pThis) continue;

				if (auto iveh = obj.GetVehicle()) {
					if (!strcmp(iveh->GetVehicleName(), "copheli")) continue;

					if (auto rb = iveh->mCOMObject->Find<IRBVehicle>()) {
						if (rb->GetInvulnerability() != INVULNERABLE_NONE) continue;
					}
				}

				auto pos = obj.GetPosition();
				auto dist = (pos - barrelPos).length();
				if (dist < explosionRange) {
					auto dir = (pos - barrelPos);
					dir.Normalize();

					obj.WakeObject();

					bool doWeak = false;
					if (auto iveh = obj.GetVehicle()) {
						if (iveh->GetDriverClass() == DRIVER_HUMAN) {
							if (SM64::bEnabled) {
								SM64::OnTakeDamage(1, barrelPos, true);
							}

#ifdef CWOEECHAOS
							if (GetEffectRunning("Juggernaut")) {
								doWeak = true;
							}
#endif
						}

						if (iveh->GetDriverClass() == DRIVER_COP) {
							DestroyCar(iveh);
						}
					}

					float powerMult = doWeak ? explosionPowerWeakMult : 1.0;
					powerMult *= (explosionRange - dist) / explosionRange;

					if (obj.pCustomStaticObject) {
						obj.pCustomStaticObject->fHealth -= 25.0 * powerMult;
						continue;
					}
					else {
						auto vel = obj.GetLinearVelocity();
						vel += dir * explosionPower * powerMult;
						obj.SetLinearVelocity(vel);

						auto avel = obj.GetAngularVelocity();
						avel += dir * explosionPowerAng * powerMult;
						obj.SetAngularVelocity(avel);
					}
				}
			}

			static NyaAudio::NyaSound sounds[] = {
					LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/explode3.wav"),
					LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/explode4.wav"),
					LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/explode5.wav"),
			};
			PlaySoundFromRange(sounds[rand() % (sizeof(sounds)/sizeof(sounds[0]))], pThis->GetPosition());
			pThis->bQueuedForDeletion = true;
		}

		float scale = 2.0;

		template<bool save>
		void SpawnObject(NyaVec3 pos, NyaVec3 vel) {
			static auto mdl = Render3D::CreateModels("oildrum001_explosive.fbx");
			static auto mdlCol = Render3D::CreateModels("oildrum001_collider.fbx");
			static auto col = CustomPhysicsObjects::CreateDynamicColliderMeshes(mdlCol, scale);

			CustomPhysicsObjects::CustomPhysicsObject objData;
			objData.aModels = mdl;
			objData.vModelSize = {scale,scale,scale};
			objData.bRemoveOnSafehouse = !save;
			objData.bRemoveOnOutOfBounds = !save;
			objData.bRemoveOnOutOfRange = !save;
			objData.sDebugName = "oildrum_save";
			objData.pCollisionFunction = OnBarrelHit;
			CustomPhysicsObjects::CreatePhysicsObject(objData, col, pos, vel);
		}

		static void SpawnBarrelFromCar(IVehicle* veh) {
			auto plyPos = *GetLocalPlayerVehicle()->GetPosition();
			auto pos = *veh->GetPosition();
			if ((plyPos - pos).length() > 250) return;

			float y = 0.0;
			if (!WCollisionMgr::GetWorldHeightAtPoint(&pos, &y, nullptr)) return;

			auto rb = veh->mCOMObject->Find<IRigidBody>();
			auto vel = *rb->GetLinearVelocity();
			pos.y -= 5.0;
			Powerups::OilDrum::SpawnObject<false>(pos, vel);
		}

		void SpawnBehindCar(IRigidBody* rb) {
			auto ply = *rb->GetPosition();
			auto vel = *rb->GetLinearVelocity();
			UMath::Vector3 fwd;
			rb->GetForwardVector(&fwd);

			NyaVec3 pos = ply;
			pos -= fwd * 5;
			pos.y += 2;
			SpawnObject<false>(pos, vel * 0.9);
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
		POWERUP_TURBO,
		POWERUP_STAR,
		POWERUP_MUSHROOM,
		POWERUP_MUSHROOMPACK,
		POWERUP_INVINCIBLE,
		POWERUP_BEACHBALL,
		POWERUP_MARIO,
		POWERUP_OILDRUM,
		POWERUP_MAGNET,
		POWERUP_BARGE,
		NUM_POWERUPS
	};
	const char* aPowerupNames[] = {
		//POWERUP_SHOCKWAVE,
		"PUTTYBOMB",
		"FIREWORKPACK",
		"FIREWORK",
		//POWERUP_WATERBOMB,
		"CLONE",
		//POWERUP_OILSLICK, // todo? copying the relevant collision polys out would work here i think
		"ELECTROPULSE",
		"CHROMEBALL",
		"TURBO",
		"STAR",
		"MUSHROOM",
		"MUSHROOMPACK",
		"INVINCIBLE",
		"BEACHBALL",
		"MARIO",
		"OILDRUM",
		"MAGNET",
		"BARGE",
	};

	const char* aPowerupSpriteNames1[] = {
			//"CwoeeChaos/data/textures/revolt_1.png",
			"CwoeeChaos/data/textures/revolt_2.png",
			"CwoeeChaos/data/textures/revolt_4.png",
			"CwoeeChaos/data/textures/revolt_3.png",
			//"CwoeeChaos/data/textures/revolt_5.png",
			"CwoeeChaos/data/textures/revolt_6.png",
			//"CwoeeChaos/data/textures/revolt_7.png",
			"CwoeeChaos/data/textures/revolt_8.png",
			"CwoeeChaos/data/textures/powerup_heavyblock.png",
			"CwoeeChaos/data/textures/revolt_10.png",
			"CwoeeChaos/data/textures/revolt_12.png",
			"CwoeeChaos/data/textures/mk64_1.png",
			"CwoeeChaos/data/textures/mk64_4.png",
			"CwoeeChaos/data/textures/mk64_2.png",
			"CwoeeChaos/data/textures/powerup_beachballpack.png",
			"CwoeeChaos/data/textures/powerup_mario.png",
			"CwoeeChaos/data/textures/powerup_oildrum.png",
			"CwoeeChaos/data/textures/powerup_magnet.png",
			"CwoeeChaos/data/textures/powerup_barge.png",
	};
	const char* aPowerupSpriteNames2[] = {
			//"CwoeeChaos/data/textures/revolt_1.png",
			"CwoeeChaos/data/textures/revolt_2.png",
			"CwoeeChaos/data/textures/revolt_4.png",
			"CwoeeChaos/data/textures/revolt_3.png",
			//"CwoeeChaos/data/textures/revolt_5.png",
			"CwoeeChaos/data/textures/mk64_3.png",
			//"CwoeeChaos/data/textures/revolt_7.png",
			"CwoeeChaos/data/textures/revolt_8.png",
			"CwoeeChaos/data/textures/powerup_heavyblock.png",
			"CwoeeChaos/data/textures/revolt_10.png",
			"CwoeeChaos/data/textures/revolt_12.png",
			"CwoeeChaos/data/textures/mk64_1.png",
			"CwoeeChaos/data/textures/mk64_4.png",
			"CwoeeChaos/data/textures/mk64_2.png",
			"CwoeeChaos/data/textures/powerup_beachballpack.png",
			"CwoeeChaos/data/textures/powerup_mario.png",
			"CwoeeChaos/data/textures/powerup_oildrum.png",
			"CwoeeChaos/data/textures/powerup_magnet.png",
			"CwoeeChaos/data/textures/powerup_barge.png",
	};
	IDirect3DTexture9* aPowerupTextures1[NUM_POWERUPS] = {};
	IDirect3DTexture9* aPowerupTextures2[NUM_POWERUPS] = {};

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

		auto obj = CustomPhysicsObjects::CreatePhysicsObject(objData, CustomPhysicsObjects::SPHERE, pos, vel);
		auto massData = b3Body_GetMassData(obj);
		massData.mass *= 50;
		b3Body_SetMassData(obj, massData);
	}

	float fSpriteY = 0.3;
	float fSpriteSize = 0.05;

	void SpawnFakePowerupBlock(IRigidBody* rb);

	struct PowerupState {
		IVehicle* pUser = nullptr;
		bool bIsLocalPlayer = false;
		int PowerupID = NUM_POWERUPS;
		int PowerupCount = 0;
		double fRollTime = 0.0;
		double fTimeSinceLastFire = 0.0;
		double fElectroTime = 0.0;
		double fTurboTime = 0.0;
		double fMagnetTime = 0.0;
		double fBargeTime = 0.0;
		IVehicle* pMagnetTarget = nullptr;
		double aTimeSinceRolled[NUM_POWERUPS] = {};

		// audio
		NyaAudio::NyaSound electro = 0;
		NyaAudio::NyaSound electrozap = 0;
		NyaAudio::NyaSound balldrop = 0;
		NyaAudio::NyaSound starfire = 0;
		NyaAudio::NyaSound wbombfire = 0;
		NyaAudio::NyaSound mushroom = 0;
		NyaAudio::NyaSound starmusic = 0;
		NyaAudio::NyaSound barge = 0;

		static inline float gBargeRange = 30.0;

		bool IsCarTargetable(IVehicle* target) {
			if (auto rb = target->mCOMObject->Find<IRBVehicle>()) {
				if (rb->GetInvulnerability() != INVULNERABLE_NONE) return false;
			}
			if (pUser->GetDriverClass() == DRIVER_COP && target->GetDriverClass() == DRIVER_COP) return false;
			return true;
		}

		bool ExecutePowerup() {
			switch (PowerupID) {
				case POWERUP_ELECTROPULSE:
					fElectroTime = pUser->GetDriverClass() == DRIVER_COP ? 5.0 : 15.0;
					return true;
				case POWERUP_CLONE: {
					SpawnFakePowerupBlock(pUser->mCOMObject->Find<IRigidBody>());
					return true;
				} break;
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
				case POWERUP_OILDRUM: {
					OilDrum::SpawnBehindCar(pUser->mCOMObject->Find<IRigidBody>());

					if (!balldrop) balldrop = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/balldrop.wav");
					PlayAudioFromCar(balldrop, pUser);
					return true;
				} break;
				case POWERUP_MAGNET: {
					bool behind = false;
					if (auto input = pUser->mCOMObject->Find<IInput>()) {
						behind = input->IsLookBackButtonPressed();
					}
					if (auto target = behind ? ReVoltFirework::PickTargetBehind(pUser) : ReVoltFirework::PickTarget(pUser)) {
						fMagnetTime = 10.0;
						pMagnetTarget = target;
					}
					return true;
				} break;
				case POWERUP_BARGE: {
					fBargeTime = 0.33;

					if (!barge) barge = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/barge.mp3");
					PlayAudioFromCar(barge, pUser, true);

					float attackPower = 50;
					float attackPowerAng = 25;

					auto userPos = *pUser->GetPosition();

					auto cars = GetActiveSharedRigidBodies();
					if (SM64::bEnemyEnabled) {
						auto dist = (SM64::GetMarioWorldPos() - userPos).length();
						if (dist < gBargeRange) {
							SM64::OnTakeDamage(1, userPos, true);
						}
					}
					for (auto& car : cars) {
						auto pos = car.GetPosition();
						auto dist = (pos - userPos).length();
						if (dist < gBargeRange) {
							auto dir = (pos - userPos);
							dir.Normalize();

							auto iveh = car.GetVehicle();
							if (iveh) {
								if (iveh == pUser) continue;
								if (!IsCarTargetable(iveh)) continue;
							}

							car.WakeObject();

							auto vel = car.GetLinearVelocity();
							vel += dir * attackPower;
							car.SetLinearVelocity(vel);

							auto avel = car.GetAngularVelocity();
							avel += dir * attackPowerAng;
							car.SetAngularVelocity(avel);

							if (iveh && iveh->GetDriverClass() == DRIVER_COP) {
								DestroyCar(iveh);
							}
						}
					}
					return true;
				} break;
				case POWERUP_TURBO:
					fTurboTime = bIsLocalPlayer ? 10.0 : 20.0;
					return true;
				case POWERUP_MUSHROOM:
				case POWERUP_MUSHROOMPACK:
				{
					pUser->SetSpeed(pUser->GetAIVehiclePtr()->GetTopSpeed());

					if (!mushroom) mushroom = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/mushroom.mp3");
					PlayAudioFromCar(mushroom, pUser, true);
					return true;
				} break;
				case POWERUP_INVINCIBLE: {
					if (auto ply = pUser->mCOMObject->Find<IRBVehicle>()) {
						ply->SetInvulnerability(INVULNERABLE_FROM_RESET, 15.0);

						if (bIsLocalPlayer) {
							if (!starmusic) starmusic = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/starmusic.mp3");
							PlayAudioFromCar(starmusic, pUser, true);
						}
					}
					return true;
				} break;
				case POWERUP_BEACHBALL: {
					if (!wbombfire) wbombfire = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/wbombfire.wav");
					PlayAudioFromCar(wbombfire, pUser);

					auto rb = pUser->mCOMObject->Find<IRigidBody>();

					UMath::Vector3 dim;
					UMath::Vector3 fwd;
					UMath::Vector3 up;
					rb->GetDimension(&dim);
					rb->GetForwardVector(&fwd);
					rb->GetUpVector(&up);

					if (auto input = pUser->mCOMObject->Find<IInput>()) {
						if (input->IsLookBackButtonPressed()) {
							fwd *= -1;
						}
					}

					auto pos = *rb->GetPosition();
					auto vel = *rb->GetLinearVelocity();
					pos += fwd * (dim.z + 2.5);
					pos += up * 0.5;
					vel += fwd * 50;
					SpawnPhysicalBeachBall(pos, vel);
					return true;
				} break;
				case POWERUP_STAR: {
					if (!starfire) starfire = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/starfire.wav");
					if (starfire) {
						NyaAudio::SetVolume(starfire, GetSFXVolume() * sfxVolumeMultiplier);
						NyaAudio::Play(starfire);
					}

					auto cars = GetActiveVehicles();
					for (auto& car : cars) {
						if (car == pUser) continue;
						if (IsCarDestroyed(car)) continue;
						if (!IsCarTargetable(car)) continue;
						ReVoltBomb::ExplodeCar(car, false);
					}
					return true;
				} break;
				case POWERUP_MARIO: {
					if (SM64::bEnabled) return false;
					auto rb = pUser->mCOMObject->Find<IRigidBody>();
					auto pos = *rb->GetPosition();
					UMath::Vector3 fwd;
					rb->GetForwardVector(&fwd);
					pos -= fwd * 5;
					SM64::bEnemyEnabled = true;
					SM64::bEnemyIsNeutral = true;
					SM64::bDoReset = true;
					SM64::vEnemySpawnPosition = pos;
					fTimeSinceMarioSpawned = 0.0;
					return true;
				} break;
			}
			return false;
		}

		bool ProcessPowerup(double delta) {
			switch (PowerupID) {
				case POWERUP_PUTTYBOMB:
					return ExecutePowerup();
				case POWERUP_MAGNET:
				case POWERUP_FIREWORK:
				case POWERUP_FIREWORKPACK: {
					bool behind = false;
					if (bIsLocalPlayer && PowerupID == POWERUP_MAGNET) {
						behind = pUser->mCOMObject->Find<IInput>()->IsLookBackButtonPressed();
					}

					auto target = behind ? ReVoltFirework::PickTargetBehind(pUser) : ReVoltFirework::PickTarget(pUser);
					if (bIsLocalPlayer || target == GetLocalPlayerVehicle()) {
						ReVoltFirework::DrawCrosshair(target, bIsLocalPlayer);
					}
				} break;
			}
			return false;
		}

		void GivePowerup(int id) {
			bool isCop = pUser->GetDriverClass() == DRIVER_COP;

			if (!bIsLocalPlayer && bDebugPrintsEnabled) {
				CwoeeHints::AddHint(std::format("{} rolled {} after {:.2f}", pUser->GetVehicleName(), aPowerupNames[id], aTimeSinceRolled[id]), 5.0);
			}
			aTimeSinceRolled[id] = 0.0;

			PowerupID = id;
			PowerupCount = (PowerupID == POWERUP_FIREWORKPACK || PowerupID == POWERUP_BEACHBALL || PowerupID == POWERUP_MUSHROOMPACK) ? 3 : 1;
			if (bMK64Style) {
				fRollTime = isCop ? 2.0 : 5.0;

				if (!bIsLocalPlayer) return;

				if (!gPickupSound) gPickupSound = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/pickup64.mp3");
				NyaAudio::SetVolume(gPickupSound, GetSFXVolume() * sfxVolumeMultiplierMK64);
				NyaAudio::SkipTo(gPickupSound, 0, false);
				NyaAudio::Play(gPickupSound);
			}
			else {
				fRollTime = isCop ? 2.0 : 4.0;

				static auto sound = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/pickup.wav");
				PlayAudioFromCar(sound, pUser);
			}
		}

		int GetRacePlacement() {
			if (!GRaceStatus::fObj) return -1;
			if (IsInPursuitRace()) return -1;
			if (auto ply = GRaceStatus::fObj->GetRacerInfo(pUser->GetSimable())) {
				return ply->mRanking;
			}
			return -1;
		}

		int GetPlayerRacePlacement() {
			if (!GRaceStatus::fObj) return -1;
			if (IsInPursuitRace()) return -1;
			if (auto ply = GRaceStatus::fObj->GetRacerInfo(GetLocalPlayerSimable())) {
				return ply->mRanking;
			}
			return -1;
		}

		void RollPowerup() {
			bool isPlayer = pUser->GetDriverClass() == DRIVER_HUMAN;
			bool isCop = pUser->GetDriverClass() == DRIVER_COP;
			bool hasNOS = false;
			if (auto engine = pUser->mCOMObject->Find<IEngine>()) {
				hasNOS = engine->HasNOS();
			}

			bool isResetting = false;
			if (auto rb = pUser->mCOMObject->Find<IRBVehicle>()) {
				if (rb->GetInvulnerability() != INVULNERABLE_NONE) isResetting = true;
			}

			bool isPlayerInFirstPlace = false;

			bool isFirstPlace = false;
			bool isLastPlace = false;
			int playerPlacement = GetPlayerRacePlacement();
			int placement = GetRacePlacement();
			if (IsInNormalRace()) {
				isFirstPlace = placement == 1;
				isLastPlace = placement == GRaceStatus::fObj->mRacerCount;
			}
			if (isFirstPlace && isLastPlace) {
				isFirstPlace = false;
				isLastPlace = false;
			}

			std::vector<int> powerupsAvailable;
			if (isCop) {
				float heatLevel = GetLocalPlayerInterface<IPerpetrator>()->GetHeat();
				int numCops = GetActiveVehicles(DRIVER_COP).size();
				for (int i = 0; i < NUM_POWERUPS; i++) {
					if (aTimeSinceRolled[i] < 20.0) continue;

					if (i == POWERUP_MARIO && !SM64::bAvailable) continue;
					if (i == POWERUP_MARIO && fTimeSinceMarioSpawned < 60.0) continue;

					if (i == POWERUP_PUTTYBOMB && numCops <= 3) continue;
					if (i == POWERUP_CLONE && ReVoltFirework::PickTarget(GetLocalPlayerVehicle()) != pUser) continue;
					if (i == POWERUP_OILDRUM && ReVoltFirework::PickTarget(GetLocalPlayerVehicle()) != pUser) continue;

					// these two are pretty OP
					if (i == POWERUP_ELECTROPULSE && heatLevel < 3.0) continue;
					if (i == POWERUP_STAR && heatLevel < 2.0) continue;
					if (i == POWERUP_ELECTROPULSE && heatLevel < 6.0 && !PercentageChanceCheck(25)) continue;
					if (i == POWERUP_STAR && heatLevel < 6.0 && !PercentageChanceCheck(25)) continue;

					if (i == POWERUP_MUSHROOM || i == POWERUP_MUSHROOMPACK) continue;

					if (i == POWERUP_TURBO) continue;

					powerupsAvailable.push_back(i);
				}
			}
			else {
				for (int i = 0; i < NUM_POWERUPS; i++) {
					if (aTimeSinceRolled[i] < 20.0) continue;

					if (i == POWERUP_MARIO && !SM64::bAvailable) continue;
					if (i == POWERUP_MARIO && fTimeSinceMarioSpawned < 30.0) continue;

					if (isLastPlace && i == POWERUP_PUTTYBOMB) continue;
					if (isLastPlace && i == POWERUP_CHROMEBALL) continue;
					if (isLastPlace && i == POWERUP_OILDRUM) continue;
					if (isLastPlace && i == POWERUP_MARIO) continue;
					if (isFirstPlace && i == POWERUP_FIREWORK) continue;
					if (isFirstPlace && i == POWERUP_FIREWORKPACK) continue;
					//if (isFirstPlace && i == POWERUP_BEACHBALL) continue;
					if (isFirstPlace && i == POWERUP_STAR) continue;
					if (isFirstPlace && i == POWERUP_MUSHROOMPACK) continue;

					if (!isPlayer && i == POWERUP_STAR) {
						if (playerPlacement != 1) continue;
						if (!isLastPlace) continue;
					}

					if (isPlayer && !hasNOS && i == POWERUP_TURBO) continue; // turbo is forced infinite nos for player, catchup + nos for ai

					// dont reroll the same stuff multiple times
					if (isResetting && i == POWERUP_INVINCIBLE) continue;
					if (fElectroTime > 0.0 && i == POWERUP_ELECTROPULSE) continue;
					if (fTurboTime > 0.0 && i == POWERUP_TURBO) continue;

					int countToAdd = 1;
					if (i == POWERUP_STAR && !isPlayer && placement >= 4 && playerPlacement == 1) {
						countToAdd = 2;
					}
					if (i == POWERUP_MUSHROOMPACK && !isPlayer && playerPlacement == 1 && placement == 2) {
						countToAdd = 2;
					}
					if (i == POWERUP_TURBO && !isPlayer && playerPlacement == 1) {
						countToAdd = 3;
					}
					for (int j = 0; j < countToAdd; j++) {
						powerupsAvailable.push_back(i);
					}
				}
			}
			if (powerupsAvailable.empty()) return;

			GivePowerup(powerupsAvailable[RandNew(powerupsAvailable.size())]);
		}

		bool HasPowerup() {
			return PowerupID != NUM_POWERUPS && PowerupCount > 0;
		}

		int GetPowerupWithRoll() {
			if (!HasPowerup()) return NUM_POWERUPS;

			int powerupId = PowerupID;
			uint8_t powerupAlpha = 255;
			if (fRollTime > 0) {
				if (bMK64Style) {
					if (fRollTime >= 1) {
						powerupId = fRollTime * 20;
						powerupId %= NUM_POWERUPS;
					}
				}
				else {
					if (fRollTime > 0.75) {
						int scroll = 20;
						if (fRollTime < 2.0) scroll = 10;
						if (fRollTime < 1.5) scroll = 5;

						// always make sure it smoothly lands on the powerup it should
						powerupId = PowerupID + ((fRollTime - 0.75) * scroll);
						powerupId %= NUM_POWERUPS;
					}
				}
			}
			return powerupId;
		}

		uint8_t GetPowerupAlpha() {
			uint8_t powerupAlpha = 255;
			if (fRollTime > 0) {
				if (bMK64Style) {
					if (fRollTime < 1) {
						powerupAlpha = 255;
						if (fRollTime < 0.166 * 5) powerupAlpha = 0;
						if (fRollTime < 0.166 * 4) powerupAlpha = 255;
						if (fRollTime < 0.166 * 3) powerupAlpha = 0;
						if (fRollTime < 0.166 * 2) powerupAlpha = 255;
						if (fRollTime < 0.166 * 1) powerupAlpha = 0;
					}
					else {
						powerupAlpha = 127;
					}
				}
				else {
					powerupAlpha = 127;
				}
			}
			return powerupAlpha;
		}

		IDirect3DTexture9* GetPowerupTexture(int powerupId) {
			if (powerupId == NUM_POWERUPS) return nullptr;

			auto& tex = bMK64Style ? aPowerupTextures2[powerupId] : aPowerupTextures1[powerupId];
			if (!tex) {
				tex = LoadTexture_SetDir(bMK64Style ? aPowerupSpriteNames2[powerupId] : aPowerupSpriteNames1[powerupId]);
			}

			return tex;
		}

		IDirect3DTexture9* GetBaseTexture() {
			static auto texBase1 = LoadTexture_SetDir("CwoeeChaos/data/textures/revolt_base.png");
			static auto texBase2 = LoadTexture_SetDir("CwoeeChaos/data/textures/mk64_base.png");
			return bMK64Style ? texBase2 : texBase1;
		}

		bool PowerupHasCrosshair() {
			if (PowerupID == POWERUP_FIREWORK) return true;
			if (PowerupID == POWERUP_FIREWORKPACK) return true;
			if (PowerupID == POWERUP_MAGNET) return true;
			return false;
		}

		bool PowerupHasFireHint() {
			if (PowerupID == POWERUP_FIREWORK) return true;
			if (PowerupID == POWERUP_FIREWORKPACK) return true;
			if (PowerupID == POWERUP_BEACHBALL) return true;
			if (PowerupID == POWERUP_MAGNET) return true;
			return false;
		}

		bool PowerupHasCrosshairBehind() {
			if (PowerupID == POWERUP_MAGNET) return true;
			return false;
		}

		void Render() {
			if (auto texBase = GetBaseTexture()) {
				DrawRectangle(0.5 - (fSpriteSize * GetAspectRatioInv()), 0.5 + (fSpriteSize * GetAspectRatioInv()), fSpriteY - fSpriteSize, fSpriteY + fSpriteSize, {255,255,255,255}, 0, texBase);
			}

			int powerupId = GetPowerupWithRoll();
			uint8_t powerupAlpha = GetPowerupAlpha();
			if (fRollTime <= 0) {
				if (PowerupHasFireHint()) {
					static bool bOnce = true;
					if (bOnce) {
						CwoeeHints::AddHint("Press X to fire.");
						if (PowerupHasCrosshair()) {
							CwoeeHints::AddHint("The green reticule displays your lock-on target.");
						}
						bOnce = false;
					}
				}
			}

			if (auto tex = GetPowerupTexture(powerupId)) {
				DrawRectangle(0.5 - (fSpriteSize * GetAspectRatioInv()), 0.5 + (fSpriteSize * GetAspectRatioInv()), fSpriteY - fSpriteSize, fSpriteY + fSpriteSize, {255,255,255,powerupAlpha}, 0, tex);
			}
		}

		bool HasFired() {
			if (pUser->IsStaging()) return false;
			if (IsCarDestroyed(pUser)) return false;

			if (pUser == GetLocalPlayerVehicle()) {
				return IsKeyJustPressed('X') || IsPadKeyJustPressed(NYA_PAD_KEY_X);
			}

			if (PowerupID == POWERUP_FIREWORK || PowerupID == POWERUP_FIREWORKPACK || PowerupID == POWERUP_BEACHBALL || PowerupID == POWERUP_MAGNET) {
				if (fTimeSinceLastFire < 0.5) return false;

				auto target = ReVoltFirework::PickTargetAI(pUser); // target to make sure there's something right in front
				auto realTarget = ReVoltFirework::PickTarget(pUser); // real target picked by the powerup on fire
				if (target && realTarget && pUser->GetDriverClass() == DRIVER_COP) {
					// prevent cop friendly fire
					if (realTarget->GetDriverClass() != DRIVER_HUMAN) return false;

					// prevent cops killing themselves
					if ((*realTarget->GetPosition() - *pUser->GetPosition()).length() < 10) return false;
				}

				return target != nullptr;
			}
			if (PowerupID == POWERUP_MUSHROOM || PowerupID == POWERUP_MUSHROOMPACK) {
				if (fTimeSinceLastFire < 1.0) return false;

				return GetLocalPlayerInterface<IInput>()->GetControls()->fGas >= 0.95;
			}
			if (PowerupID == POWERUP_ELECTROPULSE || PowerupID == POWERUP_BARGE) {
				if (!GetZappedCars().empty()) return true; // use pulse if cars are nearby
				if (GetPlayerRacePlacement() == 1) return true; // immediately use pulse if player is in first so we don't hoard it
				if (PowerupID == POWERUP_ELECTROPULSE && pUser->GetDriverClass() == DRIVER_COP) return true; // immediately use if cop, otherwise this is way too OP
				return false;
			}
			return true;
		}

		std::vector<IVehicle*> GetZappedCars() {
			std::vector<IVehicle*> out;

			auto plyPos = *pUser->GetPosition();
			auto cars = GetActiveVehicles();
			for (auto& car : cars) {
				if (car == pUser) continue;
				if (!IsCarTargetable(car)) continue;
				//if (IsCarDestroyed(car)) continue;

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
			if (fTurboTime > 0.0) {
#ifdef CWOEECHAOS
				bForcePlayerNOS = true;
				if (auto ply = pUser->mCOMObject->Find<IEngine>()) {
					ply->ChargeNOS(1.0);
				}

				fTurboTime -= delta;
				if (fTurboTime <= 0.0) {
					bForcePlayerNOS = false;
					fForcePlayerNoNOS = 0.5;
				}
#else
				fTurboTime -= delta;

				if (auto ply = pUser->mCOMObject->Find<IEngine>()) {
					ply->ChargeNOS(1.0);
				}

				if (bIsLocalPlayer) {
					bForcePlayerNOS = true;

					if (fTurboTime <= 0.0) {
						bForcePlayerNOS = false;
						fForcePlayerNoNOS = 0.5;
					}
				}
				else {
					aCatchupCheatCars.push_back(pUser);
					aForceNOSCars.push_back(pUser);
				}
#endif
			}
			if (fElectroTime > 0.0) {
				if (!electro) electro = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/electro.wav");
				if (!electrozap) electrozap = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/electrozap.wav");

				if (electro && NyaAudio::IsFinishedPlaying(electro)) {
					PlayAudioFromCar(electro, pUser);
				}

				bool doZap = false;

				auto cars = GetZappedCars();
				for (auto& car : cars) {
					if (IsCarDestroyed(car)) continue;

					if (car->GetDriverClass() == DRIVER_COP) {
						DestroyCar(car);
					}
					else {
						if (auto sus = car->mCOMObject->Find<ISuspension>()) {
							for (int i = 0; i < 4; i++) {
								sus->SetWheelAngularVelocity(i, 0.0);
							}
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
			if (fBargeTime > 0.0) {
				fBargeTime -= delta;
			}
			if (fMagnetTime > 0.0) {
				auto veh = pMagnetTarget;
				if (!IsVehicleValidAndActive(veh)) {
					fMagnetTime = 0.0;
					pMagnetTarget = nullptr;
					return;
				}

				float force = 25.0;

				auto v = veh->GetPosition();

				auto objs = GetActiveSharedRigidBodies();
				for (auto& obj : objs) {
					if (obj.IsVehicle()) continue;
					//if (obj.bIsBall) continue;

					auto c = obj.GetPosition();
					if ((*v - c).length() > 200) continue;
					obj.WakeObject();

					auto vel = obj.GetLinearVelocity();
					vel.x += (v->x - c.x) * force * delta;
					vel.y += (v->y - c.y) * force * delta;
					vel.z += (v->z - c.z) * force * delta;
					obj.SetLinearVelocity(vel);

					if (obj.pCustomObject) {
						obj.pCustomObject->fTimeSinceMovedByScript = 0.0;
					}
				}

				fMagnetTime -= delta;
			}
		}

		void Process(double delta) {
			ProcessLastingEffects(delta);
			for (auto& time : aTimeSinceRolled) {
				time += delta;
			}

			if (starmusic && !NyaAudio::IsFinishedPlaying(starmusic)) {
				if (auto rb = pUser->mCOMObject->Find<IRBVehicle>()) {
					if (rb->GetInvulnerability() != INVULNERABLE_FROM_RESET) {
						NyaAudio::Stop(starmusic);
					}
				}
			}

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

		static void RenderOverhead(IVehicle* veh, IDirect3DTexture9* tex, float scaleX, float scaleY, float yOffset, float zOffset) {
			if (IsRenderingShadows(Render3D::pViewToDraw)) return;

			static auto models = Render3D::CreateModels("plane2d.fbx");

			Render3D::RendererConfig.pOverrideDiffuse = tex;
			Render3D::RendererConfig.bForceNoCulling = true;
			Render3D::RendererConfig.bForceNoEffect = true;

			if (auto rb = veh->mCOMObject->Find<ICollisionBody>()) {
				UMath::Vector3 dim;
				rb->GetDimension(&dim);

				UMath::Matrix4 mat = *rb->GetMatrix4();
				mat.p = *rb->GetPosition();

				mat.p += mat.y * dim.y;
				mat.p += mat.y * scaleY;

				mat.x *= scaleX;
				mat.z *= scaleY;

				mat.p += mat.y * yOffset;
				mat.p += mat.z * zOffset;

				for (auto& mdl : models) {
					mdl->RenderAt(WorldToRenderMatrix(mat), true);
				}
			}

			Render3D::RendererConfig.Reset();
		}

		static void RenderOnGround(UMath::Vector3 pos, IDirect3DTexture9* tex, float scaleX, float scaleY, float yOffset) {
			if (IsRenderingShadows(Render3D::pViewToDraw)) return;

			static auto models = Render3D::CreateModels("plane2d.fbx");

			GetWorldHeightAtPoint_WithCustom(&pos, &pos.y, nullptr);

			Render3D::RendererConfig.pOverrideDiffuse = tex;
			Render3D::RendererConfig.bForceNoCulling = true;
			Render3D::RendererConfig.bForceNoEffect = true;
			Render3D::RendererConfig.bNoEffect_AlphaCutoff = false;

			NyaMat4x4 mat;
			mat.Rotate(NyaVec3(90 * 0.01745329, 0, 0));
			mat.p = pos;

			mat.x *= scaleX;
			mat.y *= scaleY;

			mat.p.y += yOffset;

			for (auto& mdl : models) {
				mdl->RenderAt(WorldToRenderMatrix(mat), true);
			}

			Render3D::RendererConfig.Reset();
		}

		void Process3D() {
			if (!bIsLocalPlayer && bOverheadDisplay) {
				if (auto tex = GetPowerupTexture(GetPowerupWithRoll())) {
					RenderOverhead(pUser, GetBaseTexture(), 1.0, 1.0, 0.0, 0.0);
					if (GetPowerupAlpha() > 64) {
						RenderOverhead(pUser, tex, -1.0, 1.0, 0.0, 0.05); // viewing from front
						RenderOverhead(pUser, tex, 1.0, 1.0, 0.0, -0.05); // viewing from behind
					}
				}
			}

			static auto magnet = LoadTexture_SetDir("CwoeeChaos/data/textures/powerup_magnet_overhead.png");
			if (magnet && fMagnetTime > 0.0 && IsVehicleValidAndActive(pMagnetTarget)) {
				RenderOverhead(pMagnetTarget, magnet, 1.0, 1.0, 0.5, -0.1);
			}

			static auto barge = LoadTexture_SetDir("CwoeeChaos/data/textures/barge_2d.png");
			if (barge && fBargeTime > 0.0) {
				float scale = (0.33 - fBargeTime) * 3;
				if (scale > 0.33) {
					scale = easeInOutQuart(scale);
				}
				RenderOnGround(*pUser->GetPosition(), barge, scale * gBargeRange, scale * gBargeRange, 0.2);
			}

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
				auto driver = car->GetDriverClass();
				if (driver == DRIVER_COP && IsCarDestroyed(car)) continue;
				if (driver != DRIVER_HUMAN && driver != DRIVER_RACER && driver != DRIVER_COP) continue;

				if (car == pUser) return true;
			}
			return false;
		}

		void Reset() {
			for (auto& time : aTimeSinceRolled) {
				time = 9999;
			}

			if (bIsLocalPlayer && fTurboTime > 0.0) {
				bForcePlayerNOS = false;
				fForcePlayerNoNOS = 0.5;
			}

			PowerupID = NUM_POWERUPS;
			PowerupCount = 0;
			fRollTime = 0.0;
			fTimeSinceLastFire = 0.0;
			fElectroTime = 0.0;
			fTurboTime = 0.0;
			fMagnetTime = 0.0;
			pMagnetTarget = nullptr;
		}

		void StopSounds() {
			NyaAudio::Stop(electro);
			NyaAudio::Stop(electrozap);
			NyaAudio::Stop(starmusic);
			NyaAudio::Stop(barge);
		}

		void DeleteSounds() {
			if (electro) NyaAudio::Delete(&electro);
			if (electrozap) NyaAudio::Delete(&electrozap);
			if (balldrop) NyaAudio::Delete(&balldrop);
			if (starfire) NyaAudio::Delete(&starfire);
			if (wbombfire) NyaAudio::Delete(&wbombfire);
			if (starmusic) NyaAudio::Delete(&starmusic);
			if (barge) NyaAudio::Delete(&barge);
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
		state.Reset();
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
				state.DeleteSounds();
				aPowerupStates.erase(aPowerupStates.begin() + (&state - &aPowerupStates[0]));
				return true;
			}
		}
		return false;
	}

	namespace PowerupBlock {
		bool bLightSpawnMode = false;

		std::vector<Render3D::tModel*> models_RV;
		std::vector<Render3D::tModel*> models_MK;

		float rX = 90;
		float rY = 0;
		float rZ = 0;
		float offY_MK = 2.0;
		float offY_RV = 1.5;
		float offZ = -6;
		float scale_MK = 0.75;
		float scale_RV = 2.0;

		float rotSpeedX = 0;
		float rotSpeedY = 0;
		float rotSpeedZ = -1.5;

		std::vector<int> aObjectsInWorld;
		std::vector<int> aFakeObjectsInWorld;

		struct PowerupBlockData {
			float rotDelta = 0.0;
			float despawnTimer = -1;
		};

		template<bool isFake>
		void BombOnTick(Render3DObjects::Object* obj, double delta) {
			obj->aModels = bMK64Style ? models_MK : models_RV;

			auto data = *(PowerupBlockData**)&obj->CustomData;
			if (data->despawnTimer >= 1.0) {
				obj->aModels.clear();
				return;
			}

			auto p = obj->mMatrix.p;
			if (GetWorldHeightAtPoint_WithCustom((UMath::Vector3*)&p, &p.y, nullptr)) {
				p.y += bMK64Style ? offY_MK : offY_RV;
				//if (data->despawnTimer > 0) {
				//	p.y += data->despawnTimer;
				//}
			}
			else {
				auto ply = GetLocalPlayerVehicle();
				if (ply && (*ply->GetPosition() - p).length() < 100) {
					obj->aModels.clear();
					return;
				}
			}

			obj->mMatrix = UMath::Matrix4::kIdentity;
			obj->mMatrix.Rotate(NyaVec3(data->rotDelta * rotSpeedX, data->rotDelta * rotSpeedY, data->rotDelta * rotSpeedZ));

			float finalScale = bMK64Style ? scale_MK : scale_RV;
			if (data->despawnTimer > 0) {
				finalScale *= 1.0 - data->despawnTimer;
			}

			UMath::Matrix4 rotation;
			rotation.Rotate(NyaVec3(rX * 0.01745329, rY * 0.01745329, rZ * 0.01745329));
			obj->mMatrix = (UMath::Matrix4)(obj->mMatrix * rotation);
			obj->mMatrix.x *= finalScale;
			obj->mMatrix.y *= finalScale;
			obj->mMatrix.z *= finalScale;
			obj->mMatrix.p = p;

			if (IsChaosBlocked()) return;
			data->rotDelta += delta;

			if (data->despawnTimer >= 0.0) {
				data->despawnTimer += delta * 4;
				return;
			}

			bool canPlayerPowerup = true;
			if (SM64::bEnabled) canPlayerPowerup = false;
#ifdef CWOEECHAOS
			if (CustomPhysicsBall::bEnabled) canPlayerPowerup = false;
#endif

			auto cars = GetActiveVehicles();
			for (auto& car : cars) {
				if (IsCarDestroyed(car)) continue;
				if (car->GetDriverClass() != DRIVER_HUMAN && car->GetDriverClass() != DRIVER_RACER) continue;
				if (car->GetDriverClass() == DRIVER_HUMAN && !canPlayerPowerup) continue;

				auto distFromCar = (*car->GetPosition() - obj->mMatrix.p).length();
				if (distFromCar < 5) {
					if (isFake) {
						ReVoltBomb::ExplodeCar(car, false);
						ReVoltBomb::ExplosionSFX(car);
						data->despawnTimer = 0.0;
					}
					else {
						if (PlayerHasPowerup(car)) continue;
						RollPowerup(car);
						data->despawnTimer = 0.0;
					}
				}
			}
		}

		template<bool isFake>
		void SpawnObject(UMath::Matrix4 mat) {
			if (models_MK.empty() || models_MK[0]->bInvalidated) {
				Render3D::ModelLoaderConfig.bColorByNormals = true;
				Render3D::ModelLoaderConfig.fColorByNormalsScale = 0.25;
				Render3D::ModelLoaderConfig.nAlphaValue = 127;
				models_MK = Render3D::CreateModels("powerupblock.fbx");
				Render3D::ModelLoaderConfig.Reset();
			}
			if (models_RV.empty() || models_RV[0]->bInvalidated) {
				Render3D::ModelLoaderConfig.nVertexColorValue = 0xFF808080;
				models_RV = Render3D::CreateModels("pickup.fbx");
				Render3D::ModelLoaderConfig.Reset();
			}

			auto id = Render3DObjects::aObjects.size();
			if (isFake) {
				aFakeObjectsInWorld.push_back(id);
			}
			else {
				aObjectsInWorld.push_back(id);
			}
			Render3DObjects::aObjects.push_back(new Render3DObjects::Object(isFake ? "fakepowerup" : "powerup", bMK64Style ? models_MK : models_RV, mat, {0,0,0}, 0, BombOnTick<isFake>));
			Render3DObjects::aObjects[id]->bUseAlpha = true;
			Render3DObjects::aObjects[id]->CustomData = new PowerupBlockData;
		}

		void SpawnForAllCheckpoints() {
			int count = GRaceStatus::fObj->mCheckpoints.size();
			auto laps = GetRaceNumLaps();
			if (IsInNormalRace() && (!laps || *laps <= 1)) {
				count--; // dont spawn powerups at the finish line
			}
			for (int i = 0; i < count; i++) {
				auto& cp = GRaceStatus::fObj->mCheckpoints[i];
				auto dir = cp->mDirection;
				auto pos = cp->mWorldTrigger.fPosRadius;

				auto lookat = NyaMat4x4::LookAt(dir);

				NyaVec3 center = {pos.x, pos.y, pos.z};
				//auto left = center - lookat.x * pos.w;
				//auto right = center + lookat.x * pos.w;
				center.y += 5.0; // to make sure the height checks will work

				if (bLightSpawnMode) {
					UMath::Matrix4 objMat;
					objMat.p = center;
					SpawnObject<false>(objMat);
					objMat.p = center - lookat.x * (pos.w * 0.33);
					SpawnObject<false>(objMat);
					objMat.p = center + lookat.x * (pos.w * 0.33);
					SpawnObject<false>(objMat);
				}
				else {
					UMath::Matrix4 objMat;
					objMat.p = center;
					SpawnObject<false>(objMat);
					objMat.p = center - lookat.x * (pos.w * 0.33);
					SpawnObject<false>(objMat);
					objMat.p = center + lookat.x * (pos.w * 0.33);
					SpawnObject<false>(objMat);
					objMat.p = center - lookat.x * (pos.w * 0.66);
					SpawnObject<false>(objMat);
					objMat.p = center + lookat.x * (pos.w * 0.66);
					SpawnObject<false>(objMat);
				}
			}
		}

		void SpawnBomb(IRigidBody* veh) {
			auto mat = UMath::Matrix4::kIdentity;
			veh->GetMatrix4(&mat);
			mat.p = *veh->GetPosition();
			GetWorldHeightAtPoint_WithCustom((UMath::Vector3*)&mat.p, &mat.p.y, nullptr);
			mat.p += mat.y * (bMK64Style ? offY_MK : offY_RV);
			mat.p += mat.z * offZ;
			SpawnObject<true>(mat);

			static auto sound = LoadAudioFile_SetDir("CwoeeChaos/data/sound/effect/pickgen.wav");
			PlayAudioFromCar(sound, veh);
		}
	}

	void SpawnFakePowerupBlock(IRigidBody* rb) {
		PowerupBlock::SpawnBomb(rb);
	}

	void OnTick() {
#ifndef CWOEECHAOS
		aCatchupCheatCars.clear();
		aForceNOSCars.clear();
#endif

		while (DespawnCleanup()) {}

		if (IsChaosBlocked()) {
			if (!NyaAudio::IsFinishedPlaying(gPickupSound)) {
				bPickupSoundPaused = true;
				NyaAudio::Stop(gPickupSound);
			}
			for (auto& state : aPowerupStates) {
				state.StopSounds();
			}
			return;
		}

		if (bPickupSoundPaused) {
			NyaAudio::Play(gPickupSound);
			bPickupSoundPaused = false;
		}

		if (Sim::Internal::mSystem->mSpeed < 1.0 && !NyaAudio::IsFinishedPlaying(gPickupSound)) {
			NyaAudio::Stop(gPickupSound);
		}

		static CNyaTimer gTimer;
		gTimer.Process();
		fTimeSinceMarioSpawned += gTimer.fDeltaTime * Sim::Internal::mSystem->mSpeed;

		for (auto& state : aPowerupStates) {
			if (!state.IsValid()) continue;
			state.Process(gTimer.fDeltaTime * Sim::Internal::mSystem->mSpeed);
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

#ifndef CWOEECHAOS
	void CleanupOldPowerups() {
		for (auto& obj : Render3DObjects::aObjects) {
			if (!obj->IsActive()) continue;
			if (obj->sDebugName != "bomb" && obj->sDebugName != "powerup") continue;
			obj->aModels.clear();
		}
	}

	bool bShouldSpawnPowerups = false;
	void PowerupMod_OnTick() {
		if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_RACING) return;
		if (IsInLoadingScreen() || IsInNIS()) return;
		if (FEManager::mPauseRequest) return;

		static double fPursuitPowerupTimer = 0.0;
		static double fPursuitCopPowerupTimer = 0.0;
		if (IsInNormalRace()) {
			fPursuitPowerupTimer = 0.0;
			fPursuitCopPowerupTimer = 0.0;

			if (IsLocalPlayerStaging()) {
				// no starting nos
				if (!IsInDragRace()) {
					if (auto ply = GetLocalPlayerInterface<IEngine>()) {
						ply->ChargeNOS(-1);
					}
				}

				CleanupOldPowerups();
				SM64::bEnemyEnabled = false;
				bShouldSpawnPowerups = true;

				if (!NyaAudio::IsFinishedPlaying(gPickupSound)) {
					NyaAudio::Stop(gPickupSound);
				}

				for (auto& state : aPowerupStates) {
					if (!state.IsValid()) continue;
					state.Reset();
					state.StopSounds();
				}
			}
			else {
				if (bShouldSpawnPowerups) {
					auto laps = GetRaceNumLaps();
					PowerupBlock::bLightSpawnMode = !laps || *laps <= 1;
					PowerupBlock::SpawnForAllCheckpoints();
					bShouldSpawnPowerups = false;
				}
			}
		}
		else {
			CleanupOldPowerups();
			bShouldSpawnPowerups = true;

			// powerups every 30 seconds in pursuits
			static CNyaTimer gTimer;
			gTimer.Process();
			if (IsInAnyPursuit()) {
				// minimum heat 3
				if (auto ply = GetLocalPlayerInterface<IPerpetrator>()) {
					if (auto max = GetMaxHeat()) {
						if (*max >= 3.0 && ply->GetHeat() < 3.0) {
							ply->SetHeat(3.0);
						}
					}
				}

				float heatLevel = GetLocalPlayerInterface<IPerpetrator>()->GetHeat();

				float copPowerupInterval = 10.0;
				if (heatLevel < 2.0) copPowerupInterval = 15.0;
				if (heatLevel >= 5.0) copPowerupInterval = 6.0;
				if (heatLevel >= 6.0) copPowerupInterval = 4.0;

				fPursuitCopPowerupTimer += gTimer.fDeltaTime * Sim::Internal::mSystem->mSpeed;
				if (fPursuitCopPowerupTimer > copPowerupInterval) {
					auto cars = GetActiveVehicles(DRIVER_COP);
					auto validCars = cars;
					validCars.clear();
					for (auto& car : cars) {
						if (IsCarDestroyed(car)) continue;
						if (!strcmp(car->GetVehicleName(), "copheli")) continue;
						if (PlayerHasPowerup(car)) continue;
						if (car->GetAIVehiclePtr()->GetRoadBlock()) continue;
						validCars.push_back(car);
					}
					if (!validCars.empty()) {
						RollPowerup(validCars[rand()%validCars.size()]);
						fPursuitCopPowerupTimer = 0;
					}
				}
				if (!PlayerHasPowerup(GetLocalPlayerVehicle())) {
					fPursuitPowerupTimer += gTimer.fDeltaTime * Sim::Internal::mSystem->mSpeed;
					if (fPursuitPowerupTimer > 30.0) {
						RollPowerup(GetLocalPlayerVehicle());
						fPursuitPowerupTimer = 0;
					}
				}
			}
		}
	}

	void PowerupMod_Heli_OnTick() {
		if (!IsInFocusedPursuit()) return;

		float heatLevel = GetLocalPlayerInterface<IPerpetrator>()->GetHeat();

		float heliInterval = 10.0;
		if (heatLevel >= 6.0) heliInterval = 7.0;

		static CNyaTimer gTimer;
		gTimer.Process();
		if (gTimer.fTotalTime >= heliInterval) {
			auto cars = GetActiveVehicles(DRIVER_COP);
			for (auto& veh : cars) {
				if (!strcmp(veh->GetVehicleName(), "copheli")) {
					OilDrum::SpawnBarrelFromCar(veh);
				}
			}
			gTimer.fTotalTime -= heliInterval;
		}
	}
#endif

	ChloeHook Init([](){
		aDrawingLoopFunctions.push_back(OnTick);
		aDrawing3DLoopFunctions.push_back(OnTick3D);

#ifndef CWOEECHAOS
		aDrawingLoopFunctions.push_back(PowerupMod_OnTick);
		aDrawingLoopFunctions.push_back(PowerupMod_Heli_OnTick);
		NyaHookLib::Patch<uint16_t>(0x6B1A02, 0x9090); // disable player causality check for cop flipping
#endif
	});
}