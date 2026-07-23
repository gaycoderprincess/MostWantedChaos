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
	std::vector<CustomBarrier> aSM64Barriers;

	// one tri is 40 bytes total
	const int MAX_COLLISION_TRI_COUNT = 1638;

	std::vector<WCollisionInstance*> aCustomCollisionInstances;
	void RegisterCustomCollisionInstance(WCollisionInstance* inst) {
		for (auto& stored : aCustomCollisionInstances) {
			if (stored == inst) return;
		}
		aCustomCollisionInstances.push_back(inst);
	}

	void DeRegisterCustomCollisionInstance(WCollisionInstance* inst) {
		for (auto& stored : aCustomCollisionInstances) {
			if (stored == inst) {
				aCustomCollisionInstances.erase(aCustomCollisionInstances.begin() + (&stored - &aCustomCollisionInstances[0]));
				return;
			}
		}
	}

	void ModifyCustomCollisionInstance(WCollisionInstance* inst, const WCollisionTri* tris, int numTris, Attrib::Collection* surfaceRef = nullptr) {
		float width = 0.0;
		float length = 0.0;
		float height = 0.0;

		NyaVec3 centerPos = {0,0,0};
		for (int i = 0; i < numTris; i++) {
			centerPos += tris[i].fPt0;
			centerPos += tris[i].fPt1;
			centerPos += tris[i].fPt2;
		}
		centerPos /= (float)(numTris * 3.0);

		for (int i = 0; i < numTris; i++) {
			auto tri = tris[i];
			width = std::max(std::abs(tri.fPt0.x-centerPos.x), width);
			width = std::max(std::abs(tri.fPt1.x-centerPos.x), width);
			width = std::max(std::abs(tri.fPt2.x-centerPos.x), width);
			height = std::max(std::abs(tri.fPt0.y-centerPos.y), height);
			height = std::max(std::abs(tri.fPt1.y-centerPos.y), height);
			height = std::max(std::abs(tri.fPt2.y-centerPos.y), height);
			length = std::max(std::abs(tri.fPt0.z-centerPos.z), length);
			length = std::max(std::abs(tri.fPt1.z-centerPos.z), length);
			length = std::max(std::abs(tri.fPt2.z-centerPos.z), length);
		}

		NyaVec3 tmp = {width,length,height};

		inst->fInvMatRow0Width.x = 1.0;
		inst->fInvMatRow0Width.y = 0.0;
		inst->fInvMatRow0Width.z = 0.0;
		inst->fInvMatRow0Width.w = width;
		inst->fInvMatRow2Length.x = 0.0;
		inst->fInvMatRow2Length.y = 0.0;
		inst->fInvMatRow2Length.z = 1.0;
		inst->fInvMatRow2Length.w = length;
		inst->fInvPosRadius.x = -centerPos.x;
		inst->fInvPosRadius.y = -centerPos.y;
		inst->fInvPosRadius.z = -centerPos.z;
		inst->fInvPosRadius.w = tmp.length();
		inst->fHeight = height;

		auto article = inst->fCollisionArticle;
		auto data = (uint8_t*)inst->fCollisionArticle;

		if (numTris != article->fNumStrips) {
			MessageBoxA(nullptr, std::format("Attempted to modify collision instance with {} tris to have {} tris", article->fNumStrips, numTris).c_str(), "nya?!~", MB_ICONERROR);
			exit(0);
		}

		auto numStrips = article->fNumStrips;
		auto numVerts = numStrips*3;

		size_t stripSphere_begin = sizeof(WCollisionArticle);
		size_t strips_begin = stripSphere_begin+(sizeof(WCollisionStripSphere)*numStrips);
		size_t surfaces_begin = strips_begin+(sizeof(WCollisionStrip)*numVerts);

		auto stripSphere = (WCollisionStripSphere*)(data+stripSphere_begin);

		auto stripList = (WCollisionStrip*)(data+strips_begin);

		float stripMult = 128.0; // 0.0078125
		float stripSphereMult = 16.0; // 0.0625

		for (int i = 0; i < numTris; i++) {
			auto tri = tris[i];
			tri.fPt0.x += inst->fInvPosRadius.x;
			tri.fPt0.y += inst->fInvPosRadius.y;
			tri.fPt0.z += inst->fInvPosRadius.z;
			tri.fPt1.x += inst->fInvPosRadius.x;
			tri.fPt1.y += inst->fInvPosRadius.y;
			tri.fPt1.z += inst->fInvPosRadius.z;
			tri.fPt2.x += inst->fInvPosRadius.x;
			tri.fPt2.y += inst->fInvPosRadius.y;
			tri.fPt2.z += inst->fInvPosRadius.z;

			tri.fPt0 *= stripMult;
			tri.fPt1 *= stripMult;
			tri.fPt2 *= stripMult;

			auto stripOffset = ((uintptr_t)stripList)-((uintptr_t)data)-sizeof(WCollisionArticle);
			if (stripOffset > 65535) {
				MessageBoxA(nullptr, std::format("Attempted to create collision with too high strip offset {} ({} tris)", stripOffset, numTris).c_str(), "nya?!~", MB_ICONERROR);
				exit(0);
			}

			stripSphere->fPos = {0,0,0}; // ?? this is still relative right?
			stripSphere->fOffset = stripOffset; // offset to strip from start of strip data?
			stripSphere->fRadius = stripSphereMult * inst->fInvPosRadius.w;
			stripSphere++;

			// one tri per strip, very inefficient but alas i am stupid
			stripList->numTrisOrSurfaceId = 3;
			stripList->pt[0] = tri.fPt0.x;
			stripList->pt[1] = tri.fPt0.y;
			stripList->pt[2] = tri.fPt0.z;
			stripList++;
			stripList->numTrisOrSurfaceId = 0;
			stripList->pt[0] = tri.fPt1.x;
			stripList->pt[1] = tri.fPt1.y;
			stripList->pt[2] = tri.fPt1.z;
			stripList++;
			stripList->numTrisOrSurfaceId = 0;
			stripList->pt[0] = tri.fPt2.x;
			stripList->pt[1] = tri.fPt2.y;
			stripList->pt[2] = tri.fPt2.z;
			stripList++;
		}

		if (surfaceRef) {
			auto surfaceList = (Attrib::Collection**)(data + surfaces_begin);
			surfaceList[0] = surfaceRef;
		}

		// fInvMatRow0Width 1.0 0.0 0.0 33.14
		// fInvMatRow2Length 0.0 0.0 1.0 44.8
		// fInvPosRadius 2499.75 -448.0 -1757.5
		// fHeight 576 - actual height from top to bottom / 2 ? or amount to move by? no that'd be 57.6
		// fFlags 0
		// fGroupNumber 0
		// player pos -2508 148 1762
	}

	WCollisionInstance* CreateCustomCollisionInstance(const WCollisionTri* tris, int numTris, Attrib::Collection* surfaceRef = nullptr) {
		if (!surfaceRef) {
			surfaceRef = Attrib::FindCollection(Attrib::StringHash32("simsurface"), Attrib::StringHash32("unknown"));
		}

		if (numTris > 65535) {
			MessageBoxA(nullptr, std::format("Attempted to create collision with {} tris (max is 65535)", numTris).c_str(), "nya?!~", MB_ICONERROR);
			exit(0);
		}

		auto inst = new WCollisionInstance;
		inst->fIterStamp = 0;
		inst->fFlags = 0;
		inst->fHeight = 0.0;
		inst->fGroupNumber = 0;
		inst->fRenderInstanceInd = 0; // todo?
		RegisterCustomCollisionInstance(inst);

		size_t numStrips = numTris;
		size_t numVerts = numStrips*3;
		size_t stripsSize = (sizeof(WCollisionStripSphere)*numStrips)+(sizeof(WCollisionStrip)*numVerts);
		if (stripsSize > 65535) {
			MessageBoxA(nullptr, std::format("Attempted to create collision with too high strip size {} ({} tris)", stripsSize, numTris).c_str(), "nya?!~", MB_ICONERROR);
			exit(0);
		}

		size_t dataSize = sizeof(WCollisionArticle)+4+(sizeof(WCollisionStripSphere)*numStrips)+(sizeof(WCollisionStrip)*numVerts);
		auto data = new uint8_t[dataSize];
		memset(data,0,dataSize);

		auto article = (WCollisionArticle*)data;
		article->fNumStrips = numStrips;
		article->fStripsSize = stripsSize;
		article->fNumEdges = 0;
		article->fEdgesSize = 0;
		article->fResolvedFlag = 0;
		article->fNumSurfaces = 1;
		article->fSurfacesSize = 4;
		article->fIntermediatObjInd = 0; // ??
		article->fFlags = 0;
		inst->fCollisionArticle = article;

		ModifyCustomCollisionInstance(inst, tris, numTris, surfaceRef);

		return inst;
	}

	class Object {
	public:
		std::vector<Render3D::tModel*> aModels;
		NyaMat4x4 mMatrix = UMath::Matrix4::kIdentity;
		NyaVec3 vColPosition = UMath::Vector3::kZero;
		float fColSize = 1;
		void(*pTickFunction)(Object*, double) = nullptr;
		void* CustomData = nullptr;
		bool bTriCollidable = false;
		bool bDontRender = false;
		bool bUseAlpha = false;
		std::string sDebugName;

		NyaVec3 vLastBarrierPosition = UMath::Vector3::kZero;
		NyaVec3 vLastTriPosition = UMath::Vector3::kZero;
		std::vector<CustomBarrier> Barriers = {};
		std::vector<WCollisionInstance*> CollisionInstances;
		std::vector<WCollisionInstance*> CollisionInstancesIgnored;

		float fRadius;

		// skip tris that are flipped or sideways, these will become barriers
		bool ShouldTriBeBarrier(const WCollisionTri& tri) {
			auto faceNormal = (tri.fPt1 - tri.fPt0).Cross(tri.fPt2 - tri.fPt0);
			faceNormal.Normalize();
			return std::abs(faceNormal.y) <= 0.05;
		}

		bool ShouldTriBeIgnored(const WCollisionTri& tri) {
			auto faceNormal = (tri.fPt1 - tri.fPt0).Cross(tri.fPt2 - tri.fPt0);
			faceNormal.Normalize();
			return faceNormal.y <= 0.05;
		}

		NyaMat4x4 GetCollisionMatrix() {
			auto out = mMatrix;
			out.y *= -1;
			return out;
		}

		void CalculateRadius() {
			NyaVec3 max;
			for (auto& model : aModels) {
				for (auto& v : model->aVertices) {
					max.x = std::max(std::abs(v.x), max.x);
					max.y = std::max(std::abs(v.y), max.y);
					max.z = std::max(std::abs(v.z), max.z);
				}
			}
			fRadius = max.length();
		}

		void GenerateCollisionInstances(const std::vector<WCollisionTri>& tris, std::vector<WCollisionInstance*>& insts) {
			if (tris.empty()) return;

			bool genFromScratch = insts.empty();
			int instId = 0;

			int trisLeft = tris.size();
			while (trisLeft > 0) {
				int trisToDo = std::min(trisLeft, MAX_COLLISION_TRI_COUNT);

				WCollisionInstance* inst = nullptr;
				if (genFromScratch) {
					auto surface = Attrib::FindCollection(Attrib::StringHash32("simsurface"), Attrib::StringHash32("asphalt_no_leaves"));
					insts.push_back(CreateCustomCollisionInstance(&tris[tris.size()-trisLeft], trisToDo, surface));
					inst = insts[insts.size()-1];
				}
				else {
					inst = insts[instId++];
				}
				ModifyCustomCollisionInstance(inst, &tris[tris.size()-trisLeft], trisToDo);

				WriteLog(std::format("generated {} tris for {}", trisToDo, sDebugName));
				trisLeft -= trisToDo;
			}
			WriteLog(std::format("finished generating {} tris for {}", tris.size(), sDebugName));
		}

		void RegenerateTris() {
			if (!bTriCollidable) return;

			auto currPos = vColPosition;
			if (currPos.x == vLastTriPosition.x && currPos.y == vLastTriPosition.y && currPos.z == vLastTriPosition.z) return;
			vLastTriPosition = currPos;

			std::vector<WCollisionTri> tris;
			std::vector<WCollisionTri> trisIgnored;
			for (auto& model : aModels) {
				for (int i = 0; i < model->aIndices.size()/3; i++) {
					WCollisionTri tri = {};
					tri.fPt2 = GetCollisionMatrix() * model->aVertices[model->aIndices[i*3]];
					tri.fPt1 = GetCollisionMatrix() * model->aVertices[model->aIndices[(i*3)+1]];
					tri.fPt0 = GetCollisionMatrix() * model->aVertices[model->aIndices[(i*3)+2]];
					tri.fSurface.fSurface = 0;
					if (ShouldTriBeIgnored(tri)) {
						trisIgnored.push_back(tri);

						// backfacing
						auto tri2 = tri;
						tri2.fPt2 = tri.fPt0;
						tri2.fPt1 = tri.fPt1;
						tri2.fPt0 = tri.fPt2;
						trisIgnored.push_back(tri2);
					}
					else {
						tris.push_back(tri);

						// backfacing
						auto tri2 = tri;
						tri2.fPt2 = tri.fPt0;
						tri2.fPt1 = tri.fPt1;
						tri2.fPt0 = tri.fPt2;
						tris.push_back(tri2);
					}
				}
			}

			GenerateCollisionInstances(tris, CollisionInstances);
			GenerateCollisionInstances(trisIgnored, CollisionInstancesIgnored);
		}

		void RegenerateTriBarriers() {
			if (!bTriCollidable) return;
			if (vColPosition.x == vLastBarrierPosition.x && vColPosition.y == vLastBarrierPosition.y && vColPosition.z == vLastBarrierPosition.z) return;
			vLastBarrierPosition = vColPosition;

			Barriers.clear();

			std::vector<WCollisionTri> tris;
			for (auto& model : aModels) {
				for (int i = 0; i < model->aIndices.size()/3; i++) {
					WCollisionTri tri = {};
					tri.fPt2 = GetCollisionMatrix() * model->aVertices[model->aIndices[i*3]];
					tri.fPt1 = GetCollisionMatrix() * model->aVertices[model->aIndices[(i*3)+1]];
					tri.fPt0 = GetCollisionMatrix() * model->aVertices[model->aIndices[(i*3)+2]];
					tri.fSurface.fSurface = 0;
					if (!ShouldTriBeBarrier(tri)) continue;
					tris.push_back(tri);

					std::vector<float> ySorted = {tri.fPt0.y, tri.fPt1.y, tri.fPt2.y};
					std::sort(ySorted.begin(), ySorted.end());

					// bit of leeway so i.e. ramps don't get messed up
					// use second highest Y value for yMax so tilted objects dont have loose barriers
					float yMin = ySorted[0] - 0.25;
					float yMax = ySorted[1] - 0.25;
					if (std::abs(yMin - yMax) < 0.25) continue;

					auto dist01 = tri.fPt0 - tri.fPt1;
					auto dist02 = tri.fPt0 - tri.fPt2;
					auto dist12 = tri.fPt1 - tri.fPt2;
					dist01.y = 0;
					dist02.y = 0;
					dist12.y = 0;
					auto length01 = dist01.length();
					auto length02 = dist02.length();
					auto length12 = dist12.length();
					if (length01 > length02) {
						// length01 longest
						if (length01 > length12) {
							Barriers.push_back(CustomBarrier(tri.fPt0, tri.fPt1));
							Barriers[Barriers.size()-1].data.fPts[0].y = yMin;
							Barriers[Barriers.size()-1].data.fPts[1].y = yMax;
							Barriers.push_back(CustomBarrier(tri.fPt1, tri.fPt0));
							Barriers[Barriers.size()-1].data.fPts[0].y = yMin;
							Barriers[Barriers.size()-1].data.fPts[1].y = yMax;
						}
						// length12 longest
						else {
							Barriers.push_back(CustomBarrier(tri.fPt1, tri.fPt2));
							Barriers[Barriers.size()-1].data.fPts[0].y = yMin;
							Barriers[Barriers.size()-1].data.fPts[1].y = yMax;
							Barriers.push_back(CustomBarrier(tri.fPt2, tri.fPt1));
							Barriers[Barriers.size()-1].data.fPts[0].y = yMin;
							Barriers[Barriers.size()-1].data.fPts[1].y = yMax;
						}
					}
					else {
						// length02 longest
						if (length02 > length12) {
							Barriers.push_back(CustomBarrier(tri.fPt0, tri.fPt2));
							Barriers[Barriers.size()-1].data.fPts[0].y = yMin;
							Barriers[Barriers.size()-1].data.fPts[1].y = yMax;
							Barriers.push_back(CustomBarrier(tri.fPt2, tri.fPt0));
							Barriers[Barriers.size()-1].data.fPts[0].y = yMin;
							Barriers[Barriers.size()-1].data.fPts[1].y = yMax;
						}
						// length12 longest
						else {
							Barriers.push_back(CustomBarrier(tri.fPt1, tri.fPt2));
							Barriers[Barriers.size()-1].data.fPts[0].y = yMin;
							Barriers[Barriers.size()-1].data.fPts[1].y = yMax;
							Barriers.push_back(CustomBarrier(tri.fPt2, tri.fPt1));
							Barriers[Barriers.size()-1].data.fPts[0].y = yMin;
							Barriers[Barriers.size()-1].data.fPts[1].y = yMax;
						}
					}
				}
			}
		}

		void RegenerateBarriers() {
			if (bTriCollidable) return;
			if (fColSize <= 0) return;
			if (vColPosition.x == vLastBarrierPosition.x && vColPosition.y == vLastBarrierPosition.y && vColPosition.z == vLastBarrierPosition.z) return;
			vLastBarrierPosition = vColPosition;

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
			Barriers.push_back(CustomBarrier(NyaVec3(v2), NyaVec3(v1)));
			Barriers.push_back(CustomBarrier(NyaVec3(v3), NyaVec3(v2)));
			Barriers.push_back(CustomBarrier(NyaVec3(v4), NyaVec3(v3)));
			Barriers.push_back(CustomBarrier(NyaVec3(v1), NyaVec3(v4)));

			for (auto& barrier : Barriers) {
				barrier.data.fPts[0].y = vColPosition.y - (fColSize * 0.5);
				barrier.data.fPts[1].y = vColPosition.y + (fColSize * 0.5);
			}
		}

		Object(const std::string& debugName, std::vector<Render3D::tModel*> models, NyaMat4x4 matrix, NyaVec3 colPosition = {0,0,0}, float collisionSize = 0, void(*tickFunction)(Object*, double) = nullptr) : sDebugName(debugName), aModels(models), mMatrix(matrix), vColPosition(colPosition), fColSize(collisionSize), pTickFunction(tickFunction) {
			CalculateRadius();
		}

		bool IsActive() {
			return !aModels.empty() && !aModels[0]->bInvalidated;
		}

		bool IsEmpty() {
			return aModels.empty();
		}

		void Destroy(bool deleteModels) {
			for (auto& col : CollisionInstances) {
				Render3DObjects::DeRegisterCustomCollisionInstance(col);
				delete[] col->fCollisionArticle;
				delete col;
			}
			for (auto& col : CollisionInstancesIgnored) {
				Render3DObjects::DeRegisterCustomCollisionInstance(col);
				delete[] col->fCollisionArticle;
				delete col;
			}

			if (deleteModels) {
				for (auto& mdl : aModels) {
					mdl->Invalidate();
				}
			}

			aModels.clear();
			Barriers.clear();
			CollisionInstances.clear();
			CollisionInstancesIgnored.clear();

			Barriers.shrink_to_fit();
			CollisionInstances.shrink_to_fit();
			CollisionInstancesIgnored.shrink_to_fit();
		}
	};
	std::vector<Object*> aObjects;

	void OnTick() {
		static CNyaTimer gTimer;
		gTimer.Process();

		if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_RACING) return;

		for (auto& obj : aObjects) {
			if (!obj->IsActive()) continue;
			obj->RegenerateBarriers();
			obj->RegenerateTris();
			obj->RegenerateTriBarriers();
			if (obj->pTickFunction) {
				obj->pTickFunction(obj, gTimer.fDeltaTime);
			}
		}
	}

	void OnTick3D() {
		PerformanceBenchmarker _perf("Render3DObjects::OnTick3D");

		if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_RACING) return;

		auto camPos = RenderToWorldCoords(PrepareCameraMatrix(GetLocalPlayerCamera()).p);
		for (auto& obj : aObjects) {
			if (obj->bDontRender) continue;

			auto dist = (obj->mMatrix.p - camPos).length();
			auto radius = obj->fRadius * obj->mMatrix.x.length();
			if (dist > (radius * 2) + 200) continue;

			for (auto& model : obj->aModels) {
				model->RenderAt(WorldToRenderMatrix(obj->mMatrix), obj->bUseAlpha);
			}
		}
	}

	std::vector<CustomBarrier> GetFullBarrierList(bool includeMarios = true, bool includeTriBarriers = true) {
		std::vector<CustomBarrier> potentialBarriers;
		if (includeMarios) {
			for (auto& barrier : aSM64Barriers) {
				potentialBarriers.push_back(barrier);
			}
		}
		for (auto& obj : aObjects) {
			if (!obj->IsActive()) continue;
			if (!includeTriBarriers && obj->bTriCollidable) continue;

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

	std::vector<WCollisionInstance*> GetFullTriList(bool includeIgnored = false) {
		std::vector<WCollisionInstance*> potentialInsts;
		for (auto& obj : aObjects) {
			if (!obj->IsActive()) continue;
			if (!obj->bTriCollidable) continue;

			for (auto& inst : obj->CollisionInstances) {
				potentialInsts.push_back(inst);
			}
			if (includeIgnored) {
				for (auto& inst : obj->CollisionInstancesIgnored) {
					potentialInsts.push_back(inst);
				}
			}
		}
		return potentialInsts;
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

	float fHoverPlatform = -999.0;
	WCollisionInstance* pHoverPlatform = nullptr;
	void ProcessHoverPlatform(WCollider* pCollider) {
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

		std::vector<WCollisionTri> tris;
		tris.push_back(tri);
		tris.push_back(tri3);
		tris.push_back(triRev);
		tris.push_back(tri3Rev);

		static auto inst = CreateCustomCollisionInstance(&tris[0], tris.size());
		ModifyCustomCollisionInstance(inst, &tris[0], tris.size());
		pHoverPlatform = inst;
	}

	void AddToWCollider(WCollider* pCollider, WCollisionInstance* inst) {
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

	void ProcessWColliderTris(WCollider* pCollider) {
		PerformanceBenchmarker _perf("Render3DObjects::ProcessWColliderTris");

		if (fHoverPlatform > -100.0) {
			ProcessHoverPlatform(pCollider);
			AddToWCollider(pCollider, pHoverPlatform);
		}

		auto pos = pCollider->fPosition;
		for (auto& obj : aObjects) {
			if (!obj->IsActive()) continue;
			if (!obj->bTriCollidable) continue;

			for (auto& inst : obj->CollisionInstances) {
				auto objPos = inst->fInvPosRadius;
				objPos.x *= -1;
				objPos.y *= -1;
				objPos.z *= -1;

				auto dist = (objPos - pos).length();
				if (dist > inst->fInvPosRadius.w + pCollider->fRadius) continue;

				AddToWCollider(pCollider, inst);
			}
		}
	}

	bool IsColliderInSpecialArea(NyaVec3 v) {
		return true;

		//v.y = 0.0;
		//if ((v - BACKROOMS_COORDS).length() < BACKROOMS_DISTANCE) return true;
		//return false;
	}

	void __thiscall ProcessCollider(WCollider* pCollider, uint32_t updateMask) {
		//pCollider->PrepareRegion(updateMask);

		auto maskNoTris = updateMask;
		maskNoTris &= ~8;
		pCollider->PrepareRegion(maskNoTris);

		if ((updateMask & 4) != 0) ProcessBarriers(pCollider);
		if ((updateMask & 12) != 0) {
			if (!pCollider->fInstanceCacheList.empty() || IsColliderInSpecialArea(pCollider->fPosition)) {
				ProcessWColliderTris(pCollider);
			}

			// manually do GetTriList if required, as inserting stuff into the instance list doesn't work otherwise
			// (they're called right after one another)
			if ((updateMask & 8) != 0) {
				WCollisionMgr::GetTriList(&pCollider->fInstanceCacheList, &pCollider->fPosition, pCollider->fRadius, &pCollider->fTriList);
				//pCollider->PrepareRegion(8);
			}
		}
		//if ((updateMask & 12) != 0) ProcessTris(pCollider); // doesn't work consistently
	}

	// check through all collision instances to see if the reference is valid
	bool IsCollisionInstanceValid(WCollisionInstance* inst) {
		for (auto& custom : aCustomCollisionInstances) {
			if (inst == custom) return true;
		}
		for (int i = 0; i < 2700; i++) {
			auto pack = WCollisionAssets::mCollisionPackList[i];
			if (!pack) continue;

			for (int j = 0; j < pack->mInstanceNum; j++) {
				if (&pack->mInstanceList[j] == inst) return true;
			}
		}
		return false;
	}

	bool IsInstanceListValid(WCollisionInstanceCacheList &instList) {
		for (int i = 0; i < instList.size(); i++) {
			if (!IsCollisionInstanceValid(instList[i])) return false;
		}
		return true;
	}

	bool __thiscall FindClosestFaceCheck(WWorldPos* pThis, WCollisionInstanceCacheList *instList, UMath::Vector3 *ptRaw, bool quitIfOnSameFace) {
		if (!IsInstanceListValid(*instList)) {
			AddLogPopup("WCollisionInstanceCacheList invalid");
			instList = nullptr;
		}
		return WWorldPos::FindClosestFace(pThis, instList, ptRaw, quitIfOnSameFace);
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
		//NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x789CE3, 0x786750);
		NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x789CE3, &FindClosestFaceCheck);
	});
}