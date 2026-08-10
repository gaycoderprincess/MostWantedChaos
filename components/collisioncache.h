// one big collision cache to reduce overall memory use by only storing one copy
namespace CollisionCache {
	struct CachedInstance {
		int nSceneryGroupId = 0;
		std::vector<WCollisionTri> aTriStrips;
		std::vector<WCollisionTri> aBarriers;
		NyaVec3 vAABBMin;
		NyaVec3 vAABBMax;

		bool IsInsideAABB(NyaVec3 pt) const {
			if (pt.x < vAABBMin.x - 5) return false;
			if (pt.y < vAABBMin.y - 5) return false;
			if (pt.z < vAABBMin.z - 5) return false;
			if (pt.x > vAABBMax.x + 5) return false;
			if (pt.y > vAABBMax.y + 5) return false;
			if (pt.z > vAABBMax.z + 5) return false;
			return true;
		}

		bool IsActive() const {
			return !nSceneryGroupId || SceneryGroupEnabledTable[nSceneryGroupId];
		}

		void CalculateAABB() {
			vAABBMin = {99999,99999,99999};
			vAABBMax = {-99999,-99999,-99999};
			for (auto& v : aTriStrips) {
				vAABBMin.x = std::min(v.fPt0.x, vAABBMin.x);
				vAABBMin.x = std::min(v.fPt1.x, vAABBMin.x);
				vAABBMin.x = std::min(v.fPt2.x, vAABBMin.x);
				vAABBMin.y = std::min(v.fPt0.y, vAABBMin.y);
				vAABBMin.y = std::min(v.fPt1.y, vAABBMin.y);
				vAABBMin.y = std::min(v.fPt2.y, vAABBMin.y);
				vAABBMin.z = std::min(v.fPt0.z, vAABBMin.z);
				vAABBMin.z = std::min(v.fPt1.z, vAABBMin.z);
				vAABBMin.z = std::min(v.fPt2.z, vAABBMin.z);
				vAABBMax.x = std::max(v.fPt0.x, vAABBMax.x);
				vAABBMax.x = std::max(v.fPt1.x, vAABBMax.x);
				vAABBMax.x = std::max(v.fPt2.x, vAABBMax.x);
				vAABBMax.y = std::max(v.fPt0.y, vAABBMax.y);
				vAABBMax.y = std::max(v.fPt1.y, vAABBMax.y);
				vAABBMax.y = std::max(v.fPt2.y, vAABBMax.y);
				vAABBMax.z = std::max(v.fPt0.z, vAABBMax.z);
				vAABBMax.z = std::max(v.fPt1.z, vAABBMax.z);
				vAABBMax.z = std::max(v.fPt2.z, vAABBMax.z);
			}
			for (auto& v : aBarriers) {
				vAABBMin.x = std::min(v.fPt0.x, vAABBMin.x);
				vAABBMin.x = std::min(v.fPt1.x, vAABBMin.x);
				vAABBMin.x = std::min(v.fPt2.x, vAABBMin.x);
				vAABBMin.y = std::min(v.fPt0.y, vAABBMin.y);
				vAABBMin.y = std::min(v.fPt1.y, vAABBMin.y);
				vAABBMin.y = std::min(v.fPt2.y, vAABBMin.y);
				vAABBMin.z = std::min(v.fPt0.z, vAABBMin.z);
				vAABBMin.z = std::min(v.fPt1.z, vAABBMin.z);
				vAABBMin.z = std::min(v.fPt2.z, vAABBMin.z);
				vAABBMax.x = std::max(v.fPt0.x, vAABBMax.x);
				vAABBMax.x = std::max(v.fPt1.x, vAABBMax.x);
				vAABBMax.x = std::max(v.fPt2.x, vAABBMax.x);
				vAABBMax.y = std::max(v.fPt0.y, vAABBMax.y);
				vAABBMax.y = std::max(v.fPt1.y, vAABBMax.y);
				vAABBMax.y = std::max(v.fPt2.y, vAABBMax.y);
				vAABBMax.z = std::max(v.fPt0.z, vAABBMax.z);
				vAABBMax.z = std::max(v.fPt1.z, vAABBMax.z);
				vAABBMax.z = std::max(v.fPt2.z, vAABBMax.z);
			}
		}
	};

	struct CachedArticle {
		std::vector<CachedInstance> aInstances;
	};
	CachedArticle aCachedCollisions[2700];

	CachedArticle gTempArticle;
	void ClearTempArticle() {
		gTempArticle.aInstances.clear();
	}

	void ProcessCollisionBarriers(CachedInstance* article, WCollisionBarrier* list, int count, NyaVec3 offset) {
		for (int i = 0; i < count; i++) {
			auto ptMin = list[i].fPts[0];
			auto ptMax = list[i].fPts[1];
			ptMin -= offset;
			ptMax -= offset;

			// first tri
			WCollisionTri tri;
			tri.fPt2.x = ptMin.x;
			tri.fPt2.y = ptMin.y;
			tri.fPt2.z = ptMin.z;
			tri.fPt1.x = ptMin.x;
			tri.fPt1.y = ptMax.y;
			tri.fPt1.z = ptMin.z;
			tri.fPt0.x = ptMax.x;
			tri.fPt0.y = ptMax.y;
			tri.fPt0.z = ptMax.z;

			article->aBarriers.push_back(tri);

			// second tri
			tri.fPt2.x = ptMin.x;
			tri.fPt2.y = ptMin.y;
			tri.fPt2.z = ptMin.z;
			tri.fPt1.x = ptMax.x;
			tri.fPt1.y = ptMin.y;
			tri.fPt1.z = ptMax.z;
			tri.fPt0.x = ptMax.x;
			tri.fPt0.y = ptMax.y;
			tri.fPt0.z = ptMax.z;
			article->aBarriers.push_back(tri);
		}
	}

	void ProcessCollisionArticle(int articleId, WCollisionInstance* inst) {
		auto& cache = articleId >= 2700 ? gTempArticle : aCachedCollisions[articleId];
		if (!inst) return;

		auto article = inst->fCollisionArticle;
		if (!article) return;

		UMath::Matrix4 instMat;
		inst->MakeMatrix(&instMat, true);

		// filter out unused stuff
		//if (inst->fGroupNumber && !SceneryGroupEnabledTable[inst->fGroupNumber]) return;

		auto articles_end_ptr = (uintptr_t)(&article[1]);

		cache.aInstances.push_back({});
		auto articleInst = &cache.aInstances[cache.aInstances.size()-1];

		articleInst->nSceneryGroupId = inst->fGroupNumber;

		auto stripSphere = (WCollisionStripSphere*)articles_end_ptr;
		auto strip = (WCollisionStrip*)(&stripSphere[article->fNumStrips]);
		for (int i = 0; i < article->fNumStrips; i++) {
			int numToIterate = strip->numTrisOrSurfaceId - 2;
			for (int j = 0; j < numToIterate; j++) {
				WCollisionTri tri;
				WCollisionStrip::MakeFace(strip, j, &stripSphere->fPos, &tri);
				tri.fSurfaceRef = *(Attrib::Collection**)(articles_end_ptr + (4 * tri.fSurface.fSurface) + article->fStripsSize + article->fEdgesSize);

				tri.fPt0 -= instMat.p;
				tri.fPt1 -= instMat.p;
				tri.fPt2 -= instMat.p;

				articleInst->aTriStrips.push_back(tri);
			}
			strip += strip->numTrisOrSurfaceId;
			stripSphere++;
		}

		ProcessCollisionBarriers(articleInst, (WCollisionBarrier*)(articles_end_ptr + article->fStripsSize), article->fNumEdges, instMat.p);

		articleInst->CalculateAABB();
	}

	void OnTick() {
		if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_RACING) return;

		for (int i = 0; i < 2700; i++) {
			auto pack = WCollisionAssets::mCollisionPackList[i];
			if (!pack) continue;

			if (!aCachedCollisions[i].aInstances.empty()) continue;

			aCachedCollisions[i].aInstances.reserve(128);
			for (int j = 0; j < pack->mInstanceNum && j < 128; j++) {
				ProcessCollisionArticle(i, &pack->mInstanceList[j]);
			}
		}
	}

	ChloeHook OnInit([]() {
		aMainLoopFunctions.push_back(OnTick);
	});
}