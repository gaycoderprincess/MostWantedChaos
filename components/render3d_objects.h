namespace Render3DObjects {
	class CustomBarrier {
	public:
		WCollisionBarrier data;
		NyaVec3 midpoint;

		CustomBarrier() {}
		CustomBarrier(NyaVec3 min, NyaVec3 max) {
			data.fPts[0].x = min.x;
			data.fPts[0].y = min.y;
			data.fPts[0].z = min.z;
			data.fPts[1].x = max.x;
			data.fPts[1].y = max.y;
			data.fPts[1].z = max.z;

			auto min2d = min;
			auto max2d = max;
			min2d.y = 0;
			max2d.y = 0;
			data.fPts[1].w = 1.0 / (max2d - min2d).length();
			midpoint = (max2d + min2d) * 0.5;
		}
	};
	std::vector<CustomBarrier> aBarriers;

	float CollisionStrength = 10;

	class Object {
	public:
		std::vector<Render3D::tModel*> aModels;
		NyaMat4x4 mMatrix = UMath::Matrix4::kIdentity;
		NyaVec3 vColPosition = UMath::Vector3::kZero;
		float fColSize = 1;
		void(*pTickFunction)(Object*, double) = nullptr;
		void* CustomData = nullptr;

		NyaVec3 vLastColPosition = UMath::Vector3::kZero;
		std::vector<CustomBarrier> Barriers = {};

		void RegenerateBarriers() {
			if (fColSize <= 0) return;
			if (vColPosition.x == vLastColPosition.x && vColPosition.y == vLastColPosition.y && vColPosition.z == vLastColPosition.z) return;
			vLastColPosition = vColPosition;

			// points:
			// -1 -1
			// -1 1
			// 1 1
			// 1 -1

			// 1 2
			// 2 3
			// 3 4
			// 4 1

			auto v1 = NyaVec3(-1, 0, -1);
			auto v2 = NyaVec3(-1, 0, 1);
			auto v3 = NyaVec3(1, 0, 1);
			auto v4 = NyaVec3(1, 0, -1);
			v1.x = vColPosition.x + (fColSize * 0.5 * v1.x);
			v1.z = vColPosition.z + (fColSize * 0.5 * v1.z);
			v2.x = vColPosition.x + (fColSize * 0.5 * v2.x);
			v2.z = vColPosition.z + (fColSize * 0.5 * v2.z);
			v3.x = vColPosition.x + (fColSize * 0.5 * v3.x);
			v3.z = vColPosition.z + (fColSize * 0.5 * v3.z);
			v4.x = vColPosition.x + (fColSize * 0.5 * v4.x);
			v4.z = vColPosition.z + (fColSize * 0.5 * v4.z);

			Barriers.clear();
			Barriers.push_back(CustomBarrier(NyaVec3(v1), NyaVec3(v2)));
			Barriers.push_back(CustomBarrier(NyaVec3(v2), NyaVec3(v3)));
			Barriers.push_back(CustomBarrier(NyaVec3(v3), NyaVec3(v4)));
			Barriers.push_back(CustomBarrier(NyaVec3(v4), NyaVec3(v1)));

			for (auto& barrier : Barriers) {
				barrier.data.fPts[0].y = vColPosition.y - (fColSize * 0.5);
				barrier.data.fPts[1].y = vColPosition.y + (fColSize * 0.5);
			}
		}

		Object(std::vector<Render3D::tModel*> models, NyaMat4x4 matrix, NyaVec3 colPosition = {0,0,0}, float collisionSize = 0, void(*tickFunction)(Object*, double) = nullptr) : aModels(models), mMatrix(matrix), vColPosition(colPosition), fColSize(collisionSize), pTickFunction(tickFunction) {
			RegenerateBarriers();
		}

		void CheckCollision(IRigidBody* other, double delta) {
			return; // not needed anymore with the new barrier system

			if (fColSize <= 0) return;

			UMath::Vector3 dim;
			other->GetDimension(&dim);
			UMath::Vector3 fwd;
			other->GetForwardVector(&fwd);
			fwd.y = 0;
			fwd.Normalize();
			auto otherPos = *other->GetPosition();
			auto velocity = *other->GetLinearVelocity();

			auto colSize1 = fColSize + dim.x;
			auto colSize2 = fColSize + dim.z;
			auto dir = (otherPos - vColPosition);
			auto dirNorm = dir;
			dirNorm.Normalize();
			auto dirNormXZ = dirNorm;
			dirNormXZ.y = 0;
			dirNormXZ.Normalize();

			auto colSize = std::lerp(colSize1, colSize2, abs(dirNormXZ.Dot(fwd)));
			if ((otherPos - vColPosition).length() < colSize) {
				velocity += dir * CollisionStrength * delta * velocity.length();
				other->SetLinearVelocity(&velocity);
				auto newPos = vColPosition + (dirNorm * colSize);
				other->SetPosition((UMath::Vector3*)&newPos);
			}
		}

		bool IsActive() {
			return !aModels.empty() && !aModels[0]->bInvalidated;
		}

		bool IsEmpty() {
			return aModels.empty();
		}
	};
	std::vector<Object> aObjects;

	void OnTick() {
		static CNyaTimer gTimer;
		gTimer.Process();

		if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_RACING) return;

		auto cars = GetActiveVehicles();
		for (auto& obj : aObjects) {
			if (!obj.IsActive()) continue;
			obj.RegenerateBarriers();
			//for (auto& car: cars) {
			//	obj.CheckCollision(car->mCOMObject->Find<IRigidBody>(), gTimer.fDeltaTime);
			//}
			if (obj.pTickFunction) {
				obj.pTickFunction(&obj, gTimer.fDeltaTime);
			}
		}
	}

	void OnTick3D() {
		if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_RACING) return;

		for (auto& obj : aObjects) {
			for (auto& model : obj.aModels) {
				model->RenderAt(WorldToRenderMatrix(obj.mMatrix));
			}
		}
	}

	void AddBarrier(NyaVec3 min, NyaVec3 max) {
		aBarriers.push_back(CustomBarrier(min, max));
		aBarriers.push_back(CustomBarrier(max, min)); // inverted barrier so it's always double sided
	}

	void ProcessBarriers(WCollider* pCollider) {
		auto potentialBarriers = aBarriers;
		for (auto& obj : aObjects) {
			if (!obj.IsActive()) continue;
			if (obj.fColSize <= 0) continue;

			for (auto& barrier : obj.Barriers) {
				potentialBarriers.push_back(barrier);

				auto inv = barrier;
				inv.data.fPts[0].x = barrier.data.fPts[1].x;
				inv.data.fPts[0].y = barrier.data.fPts[1].y;
				inv.data.fPts[0].z = barrier.data.fPts[1].z;
				inv.data.fPts[1].x = barrier.data.fPts[0].x;
				inv.data.fPts[1].y = barrier.data.fPts[0].y;
				inv.data.fPts[1].z = barrier.data.fPts[0].z;
				potentialBarriers.push_back(inv);
			}
		}

		auto collider2d = pCollider->fPosition;
		collider2d.y = 0;

		// reserve memory for our barriers
		pCollider->fBarrierList.reserve(pCollider->fBarrierList.size()+potentialBarriers.size());

		// then add our barriers
		for (auto& barrier : potentialBarriers) {
			auto size = 1.0 / barrier.data.fPts[1].w;
			if ((collider2d - barrier.midpoint).length() >= size + 25.0) continue; // disregard barriers too far away

			WCollisionBarrierListEntry tmp;
			tmp.fB = barrier.data;
			pCollider->fBarrierList.push_back(tmp);
		}
	}

	float AvgTriY(const WCollisionTri* tri) {
		float f = tri->fPt0.y + tri->fPt1.y + tri->fPt2.y;
		return f / 3.0;
	}

	void ProcessTris(WCollider* pCollider) {
		auto pos = pCollider->fPosition;

		WCollisionTri tri = {};
		tri.fFlags = 0;
		tri.fSurface.fSurface = 0;
		tri.fSurface.fFlags = 0;
		tri.PAD = 0;

		tri.fPt2 = {pos.x - 25, 150, pos.z - 25};
		tri.fPt1 = {pos.x - 25, 150, pos.z + 25};
		tri.fPt0 = {pos.x + 25, 150, pos.z - 25};
		//tri.fPt0 = {-3500,150,-3500};
		//tri.fPt1 = {-3500,150,3500};
		//tri.fPt2 = {3500,150,-3500};
		tri.fSurfaceRef = Attrib::FindCollection(Attrib::StringHash32("simsurface"), Attrib::StringHash32("asphalt_no_leaves"));

		auto tri3 = tri;
		tri3.fPt2 = {pos.x + 25, 150, pos.z + 25};

		if (pCollider->fTriList.empty()) {
			auto tmp = (WCollisionTriBlock*)gFastMem.Alloc(sizeof(WCollisionTriBlock), nullptr);
			memset(tmp, 0, sizeof(WCollisionTriBlock));
			//tmp->clear();
			tmp->push_back(tri);
			tmp->push_back(tri3);
			pCollider->fTriList.push_back(tmp);
		}
		else {
			for (int i = 0; i < pCollider->fTriList.size(); i++) {
				auto tmp = pCollider->fTriList[i];
				//tmp->clear();
				tmp->push_back(tri);
				tmp->push_back(tri3);

				std::sort(tmp->mpBegin, tmp->mpEnd, [](const WCollisionTri& a, const WCollisionTri& b) { return AvgTriY(&a) > AvgTriY(&b); });
				if (tmp->mpBegin[0].fPt0.y == 150.0f) {
					tmp->clear();
					tmp->push_back(tri);
					tmp->push_back(tri3);
				}
			}
		}
	}

	void __thiscall ProcessCollider(WCollider* pCollider, uint32_t updateMask) {
		pCollider->PrepareRegion(updateMask);

		if ((updateMask & 4) != 0) ProcessBarriers(pCollider);
		//if ((updateMask & 12) != 0) ProcessTris(pCollider); // doesn't work consistently
	}

	ChloeHook Init([](){
		//AddBarrier(NyaVec3(-2135, 150, 1184), NyaVec3(-2117, 200, 1187)); // test

		aMainLoopFunctions.push_back(OnTick);
		aDrawing3DLoopFunctions.push_back(OnTick3D);

		NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x789FC1, &ProcessCollider);
	});
}