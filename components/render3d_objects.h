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
	std::vector<CustomBarrier> aSM64Barriers;

	float CollisionStrength = 10;

	class Object {
	public:
		std::vector<Render3D::tModel*> aModels;
		NyaMat4x4 mMatrix = UMath::Matrix4::kIdentity;
		NyaVec3 vColPosition = UMath::Vector3::kZero;
		float fColSize = 1;
		void(*pTickFunction)(Object*, double) = nullptr;
		void* CustomData = nullptr;
		std::string sDebugName;

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

		Object(const std::string& debugName, std::vector<Render3D::tModel*> models, NyaMat4x4 matrix, NyaVec3 colPosition = {0,0,0}, float collisionSize = 0, void(*tickFunction)(Object*, double) = nullptr) : sDebugName(debugName), aModels(models), mMatrix(matrix), vColPosition(colPosition), fColSize(collisionSize), pTickFunction(tickFunction) {
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
	std::vector<Object*> aObjects;

	void OnTick() {
		static CNyaTimer gTimer;
		gTimer.Process();

		if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_RACING) return;

		auto cars = GetActiveVehicles();
		for (auto& obj : aObjects) {
			if (!obj->IsActive()) continue;
			obj->RegenerateBarriers();
			//for (auto& car: cars) {
			//	obj->CheckCollision(car->mCOMObject->Find<IRigidBody>(), gTimer.fDeltaTime);
			//}
			if (obj->pTickFunction) {
				obj->pTickFunction(obj, gTimer.fDeltaTime);
			}
		}
	}

	void OnTick3D() {
		PerformanceBenchmarker _perf("Render3DObjects::OnTick3D");

		if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_RACING) return;

		for (auto& obj : aObjects) {
			for (auto& model : obj->aModels) {
				model->RenderAt(WorldToRenderMatrix(obj->mMatrix));
			}
		}
	}

	void AddBarrier(NyaVec3 min, NyaVec3 max) {
		aBarriers.push_back(CustomBarrier(min, max));
		aBarriers.push_back(CustomBarrier(max, min)); // inverted barrier so it's always double sided
	}

	std::vector<CustomBarrier> GetFullBarrierList(bool includeMarios = true) {
		auto potentialBarriers = aBarriers;
		if (includeMarios) {
			for (auto& barrier : aSM64Barriers) {
				potentialBarriers.push_back(barrier);
			}
		}
		for (auto& obj : aObjects) {
			if (!obj->IsActive()) continue;
			if (obj->fColSize <= 0) continue;

			for (auto& barrier : obj->Barriers) {
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
		return potentialBarriers;
	}

	void ProcessBarriers(WCollider* pCollider) {
		auto potentialBarriers = GetFullBarrierList();

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

		//auto pt = pCollider->fPosition;
		//auto radius = pCollider->fRadius;

		WCollisionTri tri = {};
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

	float fHoverPlatform = -999.0;
	void ProcessTrisNew(WCollider* pCollider) {
		if (pCollider->fInstanceCacheList.empty()) return;

		auto pos = pCollider->fPosition;
		if (auto ply = GetLocalPlayerInterface<IRigidBody>()) {
			pos = *ply->GetPosition();
		}
		pos.y = fHoverPlatform;

		float width = 16.0;
		float length = 16.0;

		WCollisionTri tri = {};
		tri.fPt2 = {pos.x - (width/2.0), fHoverPlatform, pos.z - (length/2.0)};
		tri.fPt1 = {pos.x - (width/2.0), fHoverPlatform, pos.z + (length/2.0)};
		tri.fPt0 = {pos.x + (width/2.0), fHoverPlatform, pos.z - (length/2.0)};
		tri.fSurface.fSurface = 0;
		tri.fSurfaceRef = Attrib::FindCollection(Attrib::StringHash32("simsurface"), Attrib::StringHash32("unknown"));

		auto tri3 = tri;
		tri3.fPt2 = {pos.x + (width/2.0), fHoverPlatform, pos.z + (length/2.0)};

		auto triRev = tri;
		triRev.fPt0 = tri.fPt2;
		triRev.fPt1 = tri.fPt1;
		triRev.fPt2 = tri.fPt0;
		auto tri3Rev = tri3;
		tri3Rev.fPt0 = tri3.fPt2;
		tri3Rev.fPt1 = tri3.fPt1;
		tri3Rev.fPt2 = tri3.fPt0;

		//if (pCollider->fInstanceCacheList.empty())
		{
			NyaVec3 tmp = {width,length,0};

			static auto inst = new WCollisionInstance;
			inst->fInvMatRow0Width.x = 1.0;
			inst->fInvMatRow0Width.y = 0.0;
			inst->fInvMatRow0Width.z = 0.0;
			inst->fInvMatRow0Width.w = width;
			inst->fInvMatRow2Length.x = 0.0;
			inst->fInvMatRow2Length.y = 0.0;
			inst->fInvMatRow2Length.z = 1.0;
			inst->fInvMatRow2Length.w = length;
			inst->fInvPosRadius.x = -pos.x;
			inst->fInvPosRadius.y = -pos.y;
			inst->fInvPosRadius.z = -pos.z;
			inst->fInvPosRadius.w = tmp.length();
			inst->fIterStamp = 0;
			inst->fFlags = 0;
			inst->fHeight = 0.0; // temp value
			inst->fGroupNumber = 0;
			inst->fRenderInstanceInd = 0; // todo?

			size_t numStrips = 4;
			size_t numTris = numStrips*3;
			size_t stripSphere_begin = sizeof(WCollisionArticle);
			size_t strips_begin = stripSphere_begin+(sizeof(WCollisionStripSphere)*numStrips);
			size_t surfaces_begin = strips_begin+(sizeof(WCollisionStrip)*numTris);

			size_t dataSize = sizeof(WCollisionArticle)+4+(sizeof(WCollisionStripSphere)*numStrips)+(sizeof(WCollisionStrip)*numTris);
			static auto data = new uint8_t[dataSize];
			memset(data,0,dataSize);

			auto article = (WCollisionArticle*)data;
			article->fNumStrips = numStrips;
			article->fStripsSize = (sizeof(WCollisionStripSphere)*numStrips)+(sizeof(WCollisionStrip)*numTris);
			article->fNumEdges = 0;
			article->fEdgesSize = 0;
			article->fResolvedFlag = 0;
			article->fNumSurfaces = 1;
			article->fSurfacesSize = 4;
			article->fIntermediatObjInd = 0; // ??
			article->fFlags = 0;

			auto stripSphere = (WCollisionStripSphere*)(data+stripSphere_begin);

			auto stripList = (WCollisionStrip*)(data+strips_begin);

			float stripMult = 128.0; // 0.0078125
			float stripSphereMult = 16.0; // 0.0625

			std::vector<WCollisionTri> tris;
			tris.push_back(tri);
			tris.push_back(tri3);
			tris.push_back(triRev);
			tris.push_back(tri3Rev);
			for (int i = 0; i < tris.size(); i++) {
				// todo is this correct?
				tris[i].fPt0.x += inst->fInvPosRadius.x;
				tris[i].fPt0.y += inst->fInvPosRadius.y;
				tris[i].fPt0.z += inst->fInvPosRadius.z;
				tris[i].fPt1.x += inst->fInvPosRadius.x;
				tris[i].fPt1.y += inst->fInvPosRadius.y;
				tris[i].fPt1.z += inst->fInvPosRadius.z;
				tris[i].fPt2.x += inst->fInvPosRadius.x;
				tris[i].fPt2.y += inst->fInvPosRadius.y;
				tris[i].fPt2.z += inst->fInvPosRadius.z;

				//WriteLog(std::format("tris[{}].fPt0 {:.2f} {:.2f} {:.2f}", i, tris[i].fPt0.x, tris[i].fPt0.y, tris[i].fPt0.z));
				//WriteLog(std::format("tris[{}].fPt1 {:.2f} {:.2f} {:.2f}", i, tris[i].fPt1.x, tris[i].fPt1.y, tris[i].fPt1.z));
				//WriteLog(std::format("tris[{}].fPt2 {:.2f} {:.2f} {:.2f}", i, tris[i].fPt2.x, tris[i].fPt2.y, tris[i].fPt2.z));

				tris[i].fPt0 *= stripMult;
				tris[i].fPt1 *= stripMult;
				tris[i].fPt2 *= stripMult;

				stripSphere->fPos = {0,0,0}; // ?? this is still relative right?
				stripSphere->fOffset = ((uintptr_t)stripList)-((uintptr_t)data)-sizeof(WCollisionArticle); // offset to strip from start of strip data?
				stripSphere->fRadius = stripSphereMult * inst->fInvPosRadius.w;
				stripSphere++;

				// one tri per strip, very inefficient but alas i am stupid
				stripList->numTrisOrSurfaceId = 3;
				stripList->pt[0] = tris[i].fPt0.x;
				stripList->pt[1] = tris[i].fPt0.y;
				stripList->pt[2] = tris[i].fPt0.z;
				stripList++;
				stripList->numTrisOrSurfaceId = 0;
				stripList->pt[0] = tris[i].fPt1.x;
				stripList->pt[1] = tris[i].fPt1.y;
				stripList->pt[2] = tris[i].fPt1.z;
				stripList++;
				stripList->numTrisOrSurfaceId = 0;
				stripList->pt[0] = tris[i].fPt2.x;
				stripList->pt[1] = tris[i].fPt2.y;
				stripList->pt[2] = tris[i].fPt2.z;
				stripList++;
			}

			auto surfaceList = (Attrib::Collection**)(data+surfaces_begin);
			surfaceList[0] = tri.fSurfaceRef;

			inst->fCollisionArticle = article;

			// fInvMatRow0Width 1.0 0.0 0.0 33.14
			// fInvMatRow2Length 0.0 0.0 1.0 44.8
			// fInvPosRadius 2499.75 -448.0 -1757.5
			// fHeight 576 - actual height from top to bottom / 2 ? or amount to move by? no that'd be 57.6
			// fFlags 0
			// fGroupNumber 0
			// player pos -2508 148 1762

			bool alreadyAdded = false;
			for (int i = 0; i < pCollider->fInstanceCacheList.size(); i++) {
				if (pCollider->fInstanceCacheList[i] == inst) {
					alreadyAdded = true;
				}
			}
			if (!alreadyAdded) {
				pCollider->fInstanceCacheList.push_back(inst);
			}
		}
	}

	void __thiscall ProcessCollider(WCollider* pCollider, uint32_t updateMask) {
		//pCollider->PrepareRegion(updateMask);

		auto maskNoTris = updateMask;
		maskNoTris &= ~8;
		pCollider->PrepareRegion(maskNoTris);

		if ((updateMask & 4) != 0) ProcessBarriers(pCollider);
		if ((updateMask & 12) != 0) {
			if (fHoverPlatform > -100.0) {
				ProcessTrisNew(pCollider);
			}

			// manually do GetTriList if required, as inserting stuff into the instance list doesn't work otherwise
			// (they're called right after one another)
			if ((updateMask & 12) != 0 && (updateMask & 8) != 0) {
				WCollisionMgr::GetTriList(&pCollider->fInstanceCacheList, &pCollider->fPosition, pCollider->fRadius, &pCollider->fTriList);
				//pCollider->PrepareRegion(8);
			}
		}
		//if ((updateMask & 12) != 0) ProcessTris(pCollider); // doesn't work consistently
	}

	ChloeHook Init([](){
		//AddBarrier(NyaVec3(-2135, 150, 1184), NyaVec3(-2117, 200, 1187)); // test

		aMainLoopFunctions.push_back(OnTick);
		aDrawing3DLoopFunctions.push_back(OnTick3D);

		NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x789FC1, &ProcessCollider);

		// WWorldPos::FindClosestFace quitIfOnSameFace = false
		NyaHookLib::Patch<uint8_t>(0x7867B2, 0xEB);
		NyaHookLib::Fill(0x7791CD, 0x90, 6);

		// WWorldPos::Update use different FindClosestFace that uses the collision instance cache
		NyaHookLib::Patch<uint8_t>(0x789CDD + 2, 0x3C);
		NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x789CE3, 0x786750);
	});
}