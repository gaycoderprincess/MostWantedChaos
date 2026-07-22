namespace CustomPhysicsObjects {
	enum eColliderType {
		SPHERE,
		BOX
	};

	const int NUM_CONTACTS_CHECK = 8;
	float fObjectSFXRange = 100;
	float fObjectSFXVolume = 0.66;

	struct CustomPhysicsObject {
		std::vector<Render3D::tModel*> aModels;
		NyaVec3 vModelSize = {1,1,1};
		b3BodyId nB3Body;
		bool bRenderFlat = false;
		bool bRemoveOnSafehouse = false;
		bool bRemoveOnOutOfBounds = false;
		bool bRemoveOnOutOfRange = false;
		bool bUseExpensiveCollisionCheck = false;
		bool bAffectGamePhysics = false;
		std::string sDebugName;

		NyaVec3 vSpawnPosition = {0,0,0};
		NyaAudio::NyaSound pCollisionSound = 0;

		struct CollidedObject {
			IRigidBody* body;
			double time;
		};
		double fTimeSinceCollidedWorld = 0.0;
		std::vector<CollidedObject> aLastCollidedGameObject;
		int nLazyLastCollided = 0;

		void AddCollision(IRigidBody* body) {
			for (auto& obj : aLastCollidedGameObject) {
				if (obj.body == body) {
					obj.time = 0.0;
					return;
				}
			}
			aLastCollidedGameObject.push_back({body,0.0});
		}

		bool HasHadCollision(IRigidBody* body) {
			for (auto& obj : aLastCollidedGameObject) {
				if (obj.body == body) return obj.time < 1.0;
			}
			return false;
		}

		UMath::Vector3 GetPosition() {
			auto v = b3Body_GetPosition(nB3Body);
			return {v.x,v.y,v.z};
		}

		UMath::Vector3 GetLinearVelocity() {
			auto v = b3Body_GetLinearVelocity(nB3Body);
			return {v.x,v.y,v.z};
		}

		UMath::Vector3 GetAngularVelocity() {
			auto v = b3Body_GetAngularVelocity(nB3Body);
			return {v.x,v.y,v.z};
		}

		void SetLinearVelocity(const UMath::Vector3* v) {
			b3Body_SetLinearVelocity(nB3Body, {v->x,v->y,v->z});
		}

		void SetAngularVelocity(const UMath::Vector3* v) {
			b3Body_SetAngularVelocity(nB3Body, {v->x,v->y,v->z});
		}

		void Respawn() {
			b3Body_SetTransform(nB3Body, {vSpawnPosition.x,vSpawnPosition.y,vSpawnPosition.z}, b3Quat_identity);
			b3Body_SetLinearVelocity(nB3Body, b3Vec3_zero);
			b3Body_SetAngularVelocity(nB3Body, b3Vec3_zero);
		}

		void PlayCollisionSound() {
			auto dist = (*GetLocalPlayerVehicle()->GetPosition() - GetPosition());
			auto volume = (fObjectSFXRange - dist.length()) / fObjectSFXRange;
			if (volume > 1) volume = 1;
			if (volume <= 0) {
				volume = 0;
				return;
			}
			volume *= fObjectSFXVolume;
			NyaAudio::SetVolume(pCollisionSound, GetSFXVolume() * volume);
			NyaAudio::SkipTo(pCollisionSound, 0, false);
			NyaAudio::Play(pCollisionSound);
		}

		void ProcessLazyCollisionSound() {
			if (!pCollisionSound) return;

			b3ContactData contactData[NUM_CONTACTS_CHECK];
			int num = b3Body_GetContactData(nB3Body, contactData, NUM_CONTACTS_CHECK);
			//if (num > nLazyLastCollided) { // this results in too many false positives
			if (num && !nLazyLastCollided) {
				PlayCollisionSound();
			}
			nLazyLastCollided = num;
		}

		void ProcessExpensiveCollisionSound(double delta) {
			b3ContactData contactData[NUM_CONTACTS_CHECK];
			int num = b3Body_GetContactData(nB3Body, contactData, NUM_CONTACTS_CHECK);
			bool collidedWorld = false;
			bool collidedNewObject = false;
			for (int i = 0; i < num; i++) {
				auto game = CustomPhysics::GetGameBodyForB3Body(b3Shape_GetBody(contactData->shapeIdA));
				if (!game) game = CustomPhysics::GetGameBodyForB3Body(b3Shape_GetBody(contactData->shapeIdB));

				if (game) {
					if (!HasHadCollision(game)) {
						collidedNewObject = true;
					}
					AddCollision(game);
				}
				else {
					collidedWorld = true;
				}
			}
			if ((collidedWorld && fTimeSinceCollidedWorld > 0.25) || collidedNewObject) {
				PlayCollisionSound();
			}

			if (collidedWorld) {
				fTimeSinceCollidedWorld = 0.0;
			}
			else {
				fTimeSinceCollidedWorld += delta;
			}
			for (auto& collided : aLastCollidedGameObject) {
				collided.time += delta;
			}
		}

		void ProcessGamePhysicsIntegration() {
			b3ContactData contactData[8];
			int num = b3Body_GetContactData(nB3Body, contactData, 8);
			for (int i = 0; i < num; i++) {
				auto body = b3Shape_GetBody(contactData[i].shapeIdA);

				auto gameObj = CustomPhysics::GetGameObjectInstanceForB3Body(body);
				if (!gameObj) {
					body = b3Shape_GetBody(contactData[i].shapeIdB);
					gameObj = CustomPhysics::GetGameObjectInstanceForB3Body(body);
				}
				if (!gameObj) continue;

				gameObj->bReturnChangesToGame = true;
			}
		}
	};
	std::vector<CustomPhysicsObject*> aPhysicsObjects;

	void CreatePhysicsObject(CustomPhysicsObject data, eColliderType collider, NyaVec3 position, NyaVec3 velocity) {
		data.vSpawnPosition = position;
		if (collider == BOX) {
			b3BodyDef def = b3DefaultBodyDef();
			def.type = b3_dynamicBody;
			def.position = {0,0,0};
			data.nB3Body = b3CreateBody(CustomPhysics::m_worldId, &def);

			b3ShapeDef shapeDef = b3DefaultShapeDef();
			auto hull = b3MakeBoxHull(data.vModelSize.x, data.vModelSize.y, data.vModelSize.z);
			b3HullData hullData;
			b3CreateHullShape(data.nB3Body, &shapeDef, &hull.base);
		}
		else if (collider == SPHERE) {
			b3BodyDef def = b3DefaultBodyDef();
			def.type = b3_dynamicBody;
			def.position = {0,0,0};
			data.nB3Body = b3CreateBody(CustomPhysics::m_worldId, &def);

			b3ShapeDef shapeDef = b3DefaultShapeDef();
			b3Sphere sphere;
			sphere.center = {0,0,0};
			sphere.radius = data.vModelSize.x;
			b3CreateSphereShape(data.nB3Body, &shapeDef, &sphere);
		}

		b3Body_SetTransform(data.nB3Body, {position.x,position.y,position.z}, b3Quat_identity);
		b3Body_SetLinearVelocity(data.nB3Body, {velocity.x,velocity.y,velocity.z});

		auto obj = new CustomPhysicsObject;
		*obj = data;
		aPhysicsObjects.push_back(obj);
	}

	void DeletePhysicsObject(CustomPhysicsObject* obj) {
		for (auto& search : aPhysicsObjects) {
			if (search == obj) {
				b3DestroyBody(search->nB3Body);
				delete search;
				aPhysicsObjects.erase(aPhysicsObjects.begin() + (&search - &aPhysicsObjects[0]));
				return;
			}
		}
	}

	bool PurgeRemovables() {
		for (auto& obj : aPhysicsObjects) {
			if (obj->bRemoveOnSafehouse) {
				DeletePhysicsObject(obj);
				return true;
			}
		}
		return false;
	}

	bool PurgeOutOfWorld() {
		for (auto& obj : aPhysicsObjects) {
			if (obj->GetPosition().y < -20) {
				if (obj->bRemoveOnOutOfBounds) {
					DeletePhysicsObject(obj);
					return true;
				}
				else {
					obj->Respawn();
				}
			}
		}
		return false;
	}

	bool PurgeByRange() {
		auto plyPos = *GetLocalPlayerVehicle()->GetPosition();

		for (auto& obj : aPhysicsObjects) {
			if (!obj->bRemoveOnOutOfRange) continue;

			auto dist = (plyPos - obj->GetPosition());
			if (dist.length() > 1000) {
				DeletePhysicsObject(obj);
				return true;
			}
		}
		return false;
	}

	void OnTick() {
		PerformanceBenchmarker _perf("CustomPhysicsObjects::OnTick");

		static CNyaTimer gTimer;
		gTimer.Process();

		if (!GetLocalPlayerVehicle()) return;

		if (!aPhysicsObjects.empty()) CustomPhysics::bEnabled = true;

		while (PurgeOutOfWorld()) {}
		while (PurgeByRange()) {}

		for (auto& pObj : aPhysicsObjects) {
			auto& obj = *pObj;
			if (obj.bAffectGamePhysics) {
				obj.ProcessGamePhysicsIntegration();
			}

			if (obj.pCollisionSound) {
				if (obj.bUseExpensiveCollisionCheck) {
					obj.ProcessExpensiveCollisionSound(gTimer.fDeltaTime);
				}
				else {
					obj.ProcessLazyCollisionSound();
				}
			}
		}
	}

	void OnTick3D() {
		PerformanceBenchmarker _perf("CustomPhysicsObjects::OnTick3D");

		if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_RACING) {
			while (PurgeRemovables()) {}
			return;
		}
		if (IsInLoadingScreen() || IsInMovie()) return;

		auto plyPos = *GetLocalPlayerVehicle()->GetPosition();

		for (auto& pObj : aPhysicsObjects) {
			auto& obj = *pObj;
			auto pos = obj.GetPosition();
			auto dist = (plyPos - pos);
			if (dist.length() > 250) continue; // don't render far away objects

			UMath::Matrix4 mat;
			auto m = b3MakeMatrixFromQuat(b3Body_GetRotation(obj.nB3Body));

			mat.x.x = m.cx.x;
			mat.x.y = m.cx.y;
			mat.x.z = m.cx.z;
			mat.y.x = m.cy.x;
			mat.y.y = m.cy.y;
			mat.y.z = m.cy.z;
			mat.z.x = m.cz.x;
			mat.z.y = m.cz.y;
			mat.z.z = m.cz.z;
			mat.p.x = pos.x;
			mat.p.y = pos.y;
			mat.p.z = pos.z;

			mat.x *= obj.vModelSize.x;
			mat.y *= obj.vModelSize.y;
			mat.z *= obj.vModelSize.z;

			for (auto& mdl : obj.aModels) {
				if (obj.bRenderFlat) {
					mdl->RenderAt_NoEffect(WorldToRenderMatrix(mat));
				}
				else {
					mdl->RenderAt(WorldToRenderMatrix(mat));
				}
			}
		}
	}

	ChloeHook Init([]{
		aDrawing3DLoopFunctions.push_back(OnTick3D);
		aMainLoopFunctions.push_back(OnTick);
	});
}

class CwoeeSharedRigidBody {
public:
	IRigidBody* pGameObject;
	CustomPhysicsObjects::CustomPhysicsObject* pCustomObject;
	Render3DObjects::Object* pCustomStaticObject;

	CwoeeSharedRigidBody() {
		pGameObject = nullptr;
		pCustomObject = nullptr;
		pCustomStaticObject = nullptr;
	}
	CwoeeSharedRigidBody(IRigidBody* obj) : pGameObject(obj) {}
	CwoeeSharedRigidBody(CustomPhysicsObjects::CustomPhysicsObject* obj) : pCustomObject(obj) {}
	CwoeeSharedRigidBody(Render3DObjects::Object* obj) : pCustomStaticObject(obj) {}

	bool IsValid() {
		if (pGameObject && IsRigidBodyValidAndActive(pGameObject)) return true;
		if (pCustomObject) {
			for (auto& obj : CustomPhysicsObjects::aPhysicsObjects) {
				if (obj == pCustomObject) return true;
			}
		}
		if (pCustomStaticObject) {
			for (auto& obj : Render3DObjects::aObjects) {
				if (obj == pCustomStaticObject) return true;
			}
		}
		return false;
	}

	void InvalidError() {
		MessageBoxA(0, std::format("Attempted to index invalid rigidbody {:X} {:X} {:X}", (uintptr_t)pGameObject, (uintptr_t)pCustomObject, (uintptr_t)pCustomStaticObject).c_str(), "nya?!~", MB_ICONERROR);
		exit(0);
	}

	UMath::Vector3 GetPosition() {
		if (pGameObject) return *pGameObject->GetPosition();
		if (pCustomObject) return pCustomObject->GetPosition();
		if (pCustomStaticObject) return pCustomStaticObject->mMatrix.p;
		InvalidError();
	}

	UMath::Vector3 GetLinearVelocity() {
		if (pGameObject) return *pGameObject->GetLinearVelocity();
		if (pCustomObject) return pCustomObject->GetLinearVelocity();
		if (pCustomStaticObject) return {0,0,0};
		InvalidError();
	}

	UMath::Vector3 GetAngularVelocity() {
		if (pGameObject) return *pGameObject->GetAngularVelocity();
		if (pCustomObject) return pCustomObject->GetAngularVelocity();
		if (pCustomStaticObject) return {0,0,0};
		InvalidError();
	}

	void SetLinearVelocity(UMath::Vector3 v) {
		if (pGameObject) pGameObject->SetLinearVelocity(&v);
		if (pCustomObject) pCustomObject->SetLinearVelocity(&v);
		if (pCustomStaticObject) {
			pCustomStaticObject->mMatrix.p += v * RealTimeElapsedFrame;
			if (pCustomStaticObject->fColSize > 0.0) {
				pCustomStaticObject->vColPosition += v * RealTimeElapsedFrame;
			}
		}
	}

	void SetAngularVelocity(UMath::Vector3 v) {
		if (pGameObject) pGameObject->SetAngularVelocity(&v);
		if (pCustomObject) pCustomObject->SetAngularVelocity(&v);
	}
};

std::vector<CwoeeSharedRigidBody> GetActiveSharedRigidBodies(bool includeStaticObjects = false) {
	std::vector<CwoeeSharedRigidBody> out;
	auto game = GetActiveRigidBodies();
	for (auto& rb : game) {
		out.push_back(rb);
	}
	auto cwoee = CustomPhysicsObjects::aPhysicsObjects;
	for (auto& rb : cwoee) {
		out.push_back(rb);
	}
	if (includeStaticObjects) {
		auto render3d = Render3DObjects::aObjects;
		for (auto& obj : render3d) {
			if (obj->sDebugName == "bomb") continue;
			if (obj->sDebugName == "firework") continue;
			out.push_back(obj);
		}
	}
	return out;
}