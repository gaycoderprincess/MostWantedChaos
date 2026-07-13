namespace CustomPhysicsObjects {
	enum eColliderType {
		SPHERE,
		BOX
	};

	struct CustomPhysicsObject {
		std::vector<Render3D::tModel*> aModels;
		NyaVec3 vModelSize = {1,1,1};
		b3BodyId nB3Body;
		bool bRenderFlat = false;
		bool bRemoveOnSafehouse = false;
		bool bRemoveOnOutOfBounds = false;
		bool bRemoveOnOutOfRange = false;

		NyaVec3 vSpawnPosition = {0,0,0};
		int nLastCollided = 0;
		NyaAudio::NyaSound pCollisionSound = 0;

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
	};
	std::vector<CustomPhysicsObject> aPhysicsObjects;

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

		aPhysicsObjects.push_back(data);
	}

	bool PurgeRemovables() {
		for (auto& obj : aPhysicsObjects) {
			if (obj.bRemoveOnSafehouse) {
				b3DestroyBody(obj.nB3Body);
				aPhysicsObjects.erase(aPhysicsObjects.begin() + (&obj - &aPhysicsObjects[0]));
				return true;
			}
		}
		return false;
	}

	bool PurgeOutOfWorld() {
		for (auto& obj : aPhysicsObjects) {
			if (obj.GetPosition().y < -100) {
				if (obj.bRemoveOnOutOfBounds) {
					b3DestroyBody(obj.nB3Body);
					aPhysicsObjects.erase(aPhysicsObjects.begin() + (&obj - &aPhysicsObjects[0]));
					return true;
				}
				else {
					obj.Respawn();
				}
			}
		}
		return false;
	}

	bool PurgeByRange() {
		auto plyPos = *GetLocalPlayerVehicle()->GetPosition();

		for (auto& obj : aPhysicsObjects) {
			if (!obj.bRemoveOnOutOfRange) continue;

			auto dist = (plyPos - obj.GetPosition());
			if (dist.length() > 1000) {
				b3DestroyBody(obj.nB3Body);
				aPhysicsObjects.erase(aPhysicsObjects.begin() + (&obj - &aPhysicsObjects[0]));
				return true;
			}
		}
		return false;
	}

	float fObjectSFXRange = 100;
	float fObjectSFXVolume = 0.66;

	void OnTick() {
		if (!GetLocalPlayerVehicle()) return;

		if (!aPhysicsObjects.empty()) CustomPhysics::bEnabled = true;

		while (PurgeOutOfWorld()) {}
		while (PurgeByRange()) {}

		b3ContactData contactData[8];

		for (auto& obj : aPhysicsObjects) {
			if (!obj.pCollisionSound) continue;

			int num = b3Body_GetContactData(obj.nB3Body, contactData, 8);
			//if (num > obj.nLastCollided) { // this results in too many false positives
			if (num && !obj.nLastCollided) {
				obj.nLastCollided = num;

				auto dist = (*GetLocalPlayerVehicle()->GetPosition() - obj.GetPosition());
				auto volume = (fObjectSFXRange - dist.length()) / fObjectSFXRange;
				if (volume > 1) volume = 1;
				if (volume < 0) {
					volume = 0;
					break;
				}
				volume *= fObjectSFXVolume;
				NyaAudio::SetVolume(obj.pCollisionSound, GetSFXVolume() * volume);
				NyaAudio::Play(obj.pCollisionSound);
			}
			obj.nLastCollided = num;
		}
	}

	void OnTick3D() {
		if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_RACING) {
			while (PurgeRemovables()) {}
			return;
		}
		if (IsInLoadingScreen() || IsInMovie()) return;

		auto plyPos = *GetLocalPlayerVehicle()->GetPosition();

		for (auto& obj : aPhysicsObjects) {
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