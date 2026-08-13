namespace CustomPhysicsObjects {
	enum eColliderType {
		SPHERE,
		BOX
	};

	const int NUM_CONTACTS_CHECK = 8;
	float fObjectSFXRange = 100;
	float fObjectSFXVolume = 0.66;

	bool bEverythingAffectsGame_Player = false;
	bool bEverythingAffectsGame_Opponents = false;

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
		void(*pCollisionFunction)(CustomPhysicsObject*, b3BodyId) = nullptr;

		NyaVec3 vSpawnPosition = {0,0,0};
		NyaAudio::NyaSound pCollisionSound = 0;

		struct CollidedObject {
			IRigidBody* body;
			double time;
		};
		double fTimeSinceCollidedWorld = 0.0;
		std::vector<CollidedObject> aLastCollidedGameObject;
		int nLazyLastCollided = 0;
		double fTimeSinceSpawned = 0.0;
		double fTimeSinceMovedByScript = 0.0;

		b3ContactData aContactData[NUM_CONTACTS_CHECK];
		int nNumContactData = 0;

		bool bQueuedForDeletion = false;

		bool bAABBCalculated = false;
		NyaVec3 vAABBMin;
		NyaVec3 vAABBMax;

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

		void CalculateAABB() {
			if (!bAABBCalculated) {
				auto aabb = b3Body_ComputeAABB(nB3Body);
				vAABBMin.x = aabb.lowerBound.x;
				vAABBMin.y = aabb.lowerBound.y;
				vAABBMin.z = aabb.lowerBound.z;
				vAABBMax.x = aabb.upperBound.x;
				vAABBMax.y = aabb.upperBound.y;
				vAABBMax.z = aabb.upperBound.z;
				bAABBCalculated = true;
			}
		}

		NyaVec3 GetAABBMin() {
			CalculateAABB();
			return vAABBMin;
		}

		NyaVec3 GetAABBMax() {
			CalculateAABB();
			return vAABBMax;
		}

		void SetLinearVelocity(const UMath::Vector3* v) {
			b3Body_SetLinearVelocity(nB3Body, {v->x,v->y,v->z});
		}

		void SetAngularVelocity(const UMath::Vector3* v) {
			b3Body_SetAngularVelocity(nB3Body, {v->x,v->y,v->z});
		}

		bool CanRespawn() {
			return !bRemoveOnOutOfBounds || !bRemoveOnSafehouse;
		}

		void Respawn() {
			b3Body_SetTransform(nB3Body, {vSpawnPosition.x,vSpawnPosition.y,vSpawnPosition.z}, b3Quat_identity);
			b3Body_SetLinearVelocity(nB3Body, b3Vec3_zero);
			b3Body_SetAngularVelocity(nB3Body, b3Vec3_zero);
			fTimeSinceSpawned = 0.0;
		}

		void PlayCollisionSound() {
			if (fTimeSinceSpawned < 1.0) return;
			PlaySoundFromRange(pCollisionSound, GetPosition(), fObjectSFXRange, fObjectSFXVolume);
		}

		void ProcessLazyCollisionSound() {
			//if (nNumContactData > nLazyLastCollided) { // this results in too many false positives
			if (nNumContactData && !nLazyLastCollided) {
				PlayCollisionSound();
			}
			nLazyLastCollided = nNumContactData;
		}

		void ProcessExpensiveCollisionSound(double delta) {
			bool collidedWorld = false;
			bool collidedNewObject = false;
			for (int i = 0; i < nNumContactData; i++) {
				auto game = CustomPhysics::GetGameBodyForB3Body(b3Shape_GetBody(aContactData[i].shapeIdA));
				if (!game) game = CustomPhysics::GetGameBodyForB3Body(b3Shape_GetBody(aContactData[i].shapeIdB));

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

		void ProcessCollisionFunction() {
			for (int i = 0; i < nNumContactData; i++) {
				auto body1 = b3Shape_GetBody(aContactData[i].shapeIdA);
				auto body2 = b3Shape_GetBody(aContactData[i].shapeIdB);
				if (!B3_ID_EQUALS(body1, nB3Body)) {
					pCollisionFunction(this, body1);
				}
				if (!B3_ID_EQUALS(body2, nB3Body)) {
					pCollisionFunction(this, body2);
				}
			}
		}

		void ProcessGamePhysicsIntegration() {
			for (int i = 0; i < nNumContactData; i++) {
				auto body = b3Shape_GetBody(aContactData[i].shapeIdA);

				auto gameObj = CustomPhysics::GetGameObjectInstanceForB3Body(body);
				if (!gameObj) {
					body = b3Shape_GetBody(aContactData[i].shapeIdB);
					gameObj = CustomPhysics::GetGameObjectInstanceForB3Body(body);
				}
				if (!gameObj) continue;

				bool isPlayer = gameObj->pGameBody == GetLocalPlayerInterface<IRigidBody>();
				if (!bAffectGamePhysics) {
					if (isPlayer && bEverythingAffectsGame_Opponents) continue;
					if (!isPlayer && bEverythingAffectsGame_Player) continue;
				}

				gameObj->bReturnChangesToGame = true;
			}
		}
	};
	std::vector<CustomPhysicsObject*> aPhysicsObjects;

	std::vector<b3HullData*> CreateDynamicColliderMeshes(const std::vector<Render3D::tModel*>& models, float scale) {
		std::vector<b3HullData*> meshes;
		for (auto& model : models) {
			auto verts = model->aVertices;
			for (auto& v : verts) {
				v *= scale;
				v.y *= -1;
			}

			auto mesh = b3CreateHull((b3Vec3*)&verts[0], verts.size(), verts.size());
			if (!mesh) continue;
			meshes.push_back(mesh);
		}
		return meshes;
	}

	b3BodyId CreatePhysicsObject(CustomPhysicsObject data, eColliderType collider, NyaVec3 position, NyaVec3 velocity) {
		data.vSpawnPosition = position;
		if (collider == BOX) {
			b3BodyDef def = b3DefaultBodyDef();
			def.type = b3_dynamicBody;
			def.position.x = position.x;
			def.position.y = position.y;
			def.position.z = position.z;
			data.nB3Body = b3CreateBody(CustomPhysics::m_worldId, &def);

			b3ShapeDef shapeDef = b3DefaultShapeDef();
			auto hull = b3MakeBoxHull(data.vModelSize.x, data.vModelSize.y, data.vModelSize.z);
			b3CreateHullShape(data.nB3Body, &shapeDef, &hull.base);
		}
		else if (collider == SPHERE) {
			b3BodyDef def = b3DefaultBodyDef();
			def.type = b3_dynamicBody;
			def.position.x = position.x;
			def.position.y = position.y;
			def.position.z = position.z;
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
		return obj->nB3Body;
	}

	void CreatePhysicsObject(CustomPhysicsObject data, std::vector<b3HullData*>& meshes, NyaVec3 position, NyaVec3 velocity) {
		data.vSpawnPosition = position;

		b3BodyDef def = b3DefaultBodyDef();
		def.type = b3_dynamicBody;
		def.position.x = position.x;
		def.position.y = position.y;
		def.position.z = position.z;
		data.nB3Body = b3CreateBody(CustomPhysics::m_worldId, &def);

		for (auto& mesh : meshes) {
			b3ShapeDef shapeDef = b3DefaultShapeDef();
			b3CreateHullShape(data.nB3Body, &shapeDef, mesh);
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

	bool PurgeMarked() {
		for (auto& obj : aPhysicsObjects) {
			if (obj->bQueuedForDeletion) {
				DeletePhysicsObject(obj);
				return true;
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
		while (PurgeMarked()) {}

		AddLogPopup(std::format("{} physics objects", aPhysicsObjects.size()));

		for (auto& pObj : aPhysicsObjects) {
			auto& obj = *pObj;
			obj.fTimeSinceSpawned += gTimer.fDeltaTime;
			obj.fTimeSinceMovedByScript += gTimer.fDeltaTime;
			obj.nNumContactData = b3Body_GetContactData(obj.nB3Body, obj.aContactData, NUM_CONTACTS_CHECK);
			if (bEverythingAffectsGame_Player || bEverythingAffectsGame_Opponents || obj.bAffectGamePhysics) {
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
			if (obj.pCollisionFunction) {
				obj.ProcessCollisionFunction();
			}

			// change spawn position if the object is stationary
			if (obj.CanRespawn() && obj.fTimeSinceMovedByScript > 2.0) {
				if (obj.nNumContactData > 0 && obj.GetLinearVelocity().length() <= TOMPS(1.0)) {
					obj.vSpawnPosition = obj.GetPosition();
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

		float renderDist = IsRenderingMainView(Render3D::pViewToDraw) || IsRenderingShadows(Render3D::pViewToDraw) ? 250 : 50;
		for (auto& pObj : aPhysicsObjects) {
			auto& obj = *pObj;
			auto pos = obj.GetPosition();
			auto dist = (plyPos - pos);
			if (dist.length() > renderDist) continue; // don't render far away objects

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
	bool bIsBall = false;

	CwoeeSharedRigidBody() {
		pGameObject = nullptr;
		pCustomObject = nullptr;
		pCustomStaticObject = nullptr;
	}
	CwoeeSharedRigidBody(IRigidBody* obj) : pGameObject(obj) {}
	CwoeeSharedRigidBody(CustomPhysicsObjects::CustomPhysicsObject* obj) : pCustomObject(obj) {
		bIsBall = CustomPhysicsBall::bEnabled && obj && B3_ID_EQUALS(obj->nB3Body, CustomPhysicsBall::BallBody);
	}
	CwoeeSharedRigidBody(Render3DObjects::Object* obj) : pCustomStaticObject(obj) {}

	bool IsValid() {
		if (bIsBall) return CustomPhysicsBall::bEnabled;
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

	bool HasPhysics() const {
		return pGameObject || pCustomObject;
	}

	bool IsStatic() const {
		return !HasPhysics();
	}

	bool HasBakedCollisionMesh() const {
		return IsStatic() && pCustomStaticObject->bTriCollidable;
	}

	bool IsVehicle() const {
		return pGameObject && pGameObject->mCOMObject->Find<IVehicle>();
	}

	IVehicle* GetVehicle() const {
		if (!pGameObject) return nullptr;
		return pGameObject->mCOMObject->Find<IVehicle>();
	}

	bool CanBeDeleted() const {
		if (auto veh = GetVehicle()) {
			return veh->GetDriverClass() == DRIVER_TRAFFIC || veh->GetDriverClass() == DRIVER_COP;
		}
		return true;
	}

	void WakeObject() {
		if (pGameObject) {
			auto cb = pGameObject->mCOMObject->Find<ICollisionBody>();
			if (!cb) return;
			if (!cb->IsAttachedToWorld()) return;
			cb->AttachedToWorld(false, 50.0);
		}
		if (pCustomObject) {
			b3Body_SetAwake(pCustomObject->nB3Body, true);
		}
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
		return {};
	}

	UMath::Vector3 GetLinearVelocity() {
		if (pGameObject) return *pGameObject->GetLinearVelocity();
		if (pCustomObject) return pCustomObject->GetLinearVelocity();
		if (pCustomStaticObject) return {0,0,0};
		InvalidError();
		return {};
	}

	UMath::Vector3 GetAngularVelocity() {
		if (pGameObject) return *pGameObject->GetAngularVelocity();
		if (pCustomObject) return pCustomObject->GetAngularVelocity();
		if (pCustomStaticObject) return {0,0,0};
		InvalidError();
		return {};
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

	UMath::Vector3 GetDimension() {
		if (pGameObject) {
			UMath::Vector3 out;
			pGameObject->GetDimension(&out);
			return out;
		}
		if (pCustomObject) {
			auto aabbMin = pCustomObject->GetAABBMin();
			auto aabbMax = pCustomObject->GetAABBMax();

			UMath::Vector3 out;
			out.x = std::max(std::abs(aabbMin.x), std::abs(aabbMax.x));
			out.y = std::max(std::abs(aabbMin.y), std::abs(aabbMax.y));
			out.z = std::max(std::abs(aabbMin.z), std::abs(aabbMax.z));
			return out;
		}
		if (pCustomStaticObject) {
			UMath::Vector3 out;
			out.x = std::max(std::abs(pCustomStaticObject->vAABBMin.x), std::abs(pCustomStaticObject->vAABBMax.x));
			out.y = std::max(std::abs(pCustomStaticObject->vAABBMin.y), std::abs(pCustomStaticObject->vAABBMax.y));
			out.z = std::max(std::abs(pCustomStaticObject->vAABBMin.z), std::abs(pCustomStaticObject->vAABBMax.z));
			return out;
		}
		InvalidError();
		return {};
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
	if (CustomPhysicsBall::bEnabled) {
		static auto obj = new CustomPhysicsObjects::CustomPhysicsObject;
		obj->nB3Body = CustomPhysicsBall::BallBody;
		out.push_back(obj);
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