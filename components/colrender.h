namespace CollView {
	const int COLLVIEW_MAX_TRIANGLES = 16384;

	bool bWireframeGround = true;
	bool bWireframeBarriers = false;
	bool bWireframeCars = true;

	int nWallColR = 0;
	int nWallColG = 0;
	int nWallColB = 255;

	int nGroundColR = 0;
	int nGroundColG = 255;
	int nGroundColB = 255;

	int nCarColR = 0;
	int nCarColG = 255;
	int nCarColB = 0;

	int nObjColR = 255;
	int nObjColG = 0;
	int nObjColB = 255;

	struct CollisionGeometryBuffer {
		float *position;
		float *normal;
		float *uv;
		uint16_t numTrianglesUsed;
	};

	NyaVec3 WorldToRender(NyaVec3 in) {
		auto out = in;
		out.y *= -1;
		return out;
	}

	template<int bufId>
	void RenderMario(CollisionGeometryBuffer marioBuffers, NyaMat4x4 matrix) {
		int numFaces = COLLVIEW_MAX_TRIANGLES;
		int numVertices = 3 * numFaces;
		
		size_t vertexTotalSize = numVertices * sizeof(Render3D::CwoeeVertexData);
		size_t indexTotalSize = numFaces * 3 * 4;

		static IDirect3DVertexBuffer9* vertexBuffer = nullptr;
		static IDirect3DIndexBuffer9* indexBuffer = nullptr;

		static auto hr1 = g_pd3dDevice->CreateVertexBuffer(vertexTotalSize, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1, D3DPOOL_DEFAULT, &vertexBuffer, nullptr);
		if (hr1 != D3D_OK) {
			return;
		}
		static auto hr2 = g_pd3dDevice->CreateIndexBuffer(indexTotalSize, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &indexBuffer, nullptr);
		if (hr2 != D3D_OK) {
			return;
		}

		Render3D::CwoeeVertexData* verticesOut = nullptr;
		int* indicesOut = nullptr;
		auto hr = vertexBuffer->Lock(0, vertexTotalSize, (void**)&verticesOut, D3DLOCK_DISCARD);
		if (hr != D3D_OK) {
			return;
		}
		hr = indexBuffer->Lock(0, indexTotalSize, (void**)&indicesOut, D3DLOCK_DISCARD);
		if (hr != D3D_OK) {
			return;
		}

		DWORD fillMode;
		g_pd3dDevice->GetRenderState(D3DRS_FILLMODE, &fillMode);

		auto sunDir = NyaVec3(0.75, 0.0, 0.75);
		sunDir.Normalize();

		int numFacesUsed = marioBuffers.numTrianglesUsed;
		int numVerticesUsed = marioBuffers.numTrianglesUsed*3;
		for (int i = 0; i < numVerticesUsed; i++) {
			auto src = &marioBuffers.position[i*3];
			auto srcNormal = &marioBuffers.normal[i*3];
			auto srcUV = &marioBuffers.uv[i*2];
			auto dest = &verticesOut[i];

			auto tmpPos = WorldToRender({src[0], src[1], src[2]});
			dest->vPos[0] = tmpPos[0];
			dest->vPos[1] = tmpPos[1];
			dest->vPos[2] = tmpPos[2];

			auto tmpNormal = WorldToRender({srcNormal[0], srcNormal[1], srcNormal[2]});

			dest->vNormals[0] = tmpNormal[0];
			dest->vNormals[1] = tmpNormal[1];
			dest->vNormals[2] = tmpNormal[2];

			dest->vTangents[0] = tmpNormal[0];
			dest->vTangents[1] = tmpNormal[1];
			dest->vTangents[2] = tmpNormal[2];

			if (fillMode == D3DFILL_SOLID) {
				float shading = std::clamp(tmpNormal.Dot(sunDir), 0.66, 1.0);

				auto tmp = NyaDrawing::CNyaRGBA32();
				tmp.b = nWallColR * shading;
				tmp.g = nWallColG * shading;
				tmp.r = nWallColB * shading;
				tmp.a = 255;
				dest->Color = *(uint32_t*)&tmp;
			}
			else {
				auto tmp = NyaDrawing::CNyaRGBA32();
				tmp.b = nGroundColR;
				tmp.g = nGroundColG;
				tmp.r = nGroundColB;
				tmp.a = 255;
				dest->Color = *(uint32_t*)&tmp;
			}

			dest->vUV[0] = srcUV[0];
			dest->vUV[1] = srcUV[1];
		}
		for (int i = 0; i < numFacesUsed*3; i++) {
			indicesOut[i] = i;
		}

		vertexBuffer->Unlock();
		indexBuffer->Unlock();

		Render3D::tModel tmpModel = {};
		tmpModel.pVertexBuffer = vertexBuffer;
		tmpModel.pIndexBuffer = indexBuffer;
		tmpModel.nVertexCount = numVerticesUsed;
		tmpModel.nFaceCount = numFacesUsed;

		static auto tex = LoadTexture("CwoeeChaos/data/models/white.png");
		tmpModel.pTexture = tex;
		tmpModel.RenderAt_NoEffect(WorldToRenderMatrix(matrix), false, fillMode == D3DFILL_SOLID, fillMode == D3DFILL_SOLID);
	}

	std::vector<WCollisionTri> aCollisionTris;
	std::vector<WCollisionTri> aCollisionBarriers;

	void ProcessCollisionBarriers(WCollisionBarrier* list, int count, NyaVec3 offset) {
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
			aCollisionBarriers.push_back(tri);

			// second tri
			tri.fPt0.x = ptMin.x;
			tri.fPt0.y = ptMin.y;
			tri.fPt0.z = ptMin.z;
			tri.fPt1.x = ptMax.x;
			tri.fPt1.y = ptMin.y;
			tri.fPt1.z = ptMax.z;
			tri.fPt2.x = ptMax.x;
			tri.fPt2.y = ptMax.y;
			tri.fPt2.z = ptMax.z;
			aCollisionBarriers.push_back(tri);
		}
	}

	void ProcessCollisionArticle(WCollisionInstance* inst) {
		if (!inst) return;

		auto article = inst->fCollisionArticle;
		if (!article) return;

		UMath::Matrix4 instMat;
		inst->MakeMatrix(&instMat, true);

		auto articles_end_ptr = (uintptr_t)(&article[1]);

		auto stripSphere = (WCollisionStripSphere*)articles_end_ptr;
		auto strip = (WCollisionStrip*)(&stripSphere[article->fNumStrips]);
		for (int i = 0; i < article->fNumStrips; i++) {
			int numToIterate = strip->numTrisOrSurfaceId - 2;
			for (int j = 0; j < numToIterate; j++) {
				WCollisionTri tri;
				WCollisionStrip::MakeFace(strip, j, &stripSphere->fPos, &tri);

				tri.fPt0 -= instMat.p;
				tri.fPt1 -= instMat.p;
				tri.fPt2 -= instMat.p;

				aCollisionTris.push_back(tri);
			}
			strip += strip->numTrisOrSurfaceId;
			stripSphere++;
		}

		// filter out unused barriers
		if (inst->fGroupNumber && !SceneryGroupEnabledTable[inst->fGroupNumber]) return;
		ProcessCollisionBarriers((WCollisionBarrier*)(articles_end_ptr + article->fStripsSize), article->fNumEdges, instMat.p);
	}

	template<int debugRenderId>
	void AddMarioStaticObject(std::vector<WCollisionTri>* collisions, bool doubleSided, NyaMat4x4 matrix) {
		static CollisionGeometryBuffer colBuffers = {};
		static CollisionGeometryBuffer colBuffersFlip = {};
		if (!colBuffers.position) {
			colBuffers.position = new float[9 * COLLVIEW_MAX_TRIANGLES];
			colBuffers.normal = colBuffersFlip.normal = new float[9 * COLLVIEW_MAX_TRIANGLES];
			colBuffers.uv = colBuffersFlip.uv = new float[6 * COLLVIEW_MAX_TRIANGLES];
			colBuffersFlip.position = new float[9 * COLLVIEW_MAX_TRIANGLES];
		}
		colBuffers.numTrianglesUsed = colBuffersFlip.numTrianglesUsed = std::min((int)collisions->size(), (int)COLLVIEW_MAX_TRIANGLES);

		auto colDrawPosition = &colBuffers.position[0];
		auto colDrawNormal = &colBuffers.normal[0];
		auto colDrawUV = &colBuffers.uv[0];

		auto colDrawFlipPosition = &colBuffersFlip.position[0];

		for (int i = 0; i < collisions->size(); i++) {
			auto in = &(*collisions)[i];

			auto pt0 = NyaVec3({in->fPt0[0],in->fPt0[1],in->fPt0[2]});
			auto pt1 = NyaVec3({in->fPt1[0],in->fPt1[1],in->fPt1[2]});
			auto pt2 = NyaVec3({in->fPt2[0],in->fPt2[1],in->fPt2[2]});

			if (i < colBuffers.numTrianglesUsed) {
				colDrawPosition[0] = pt0[0];
				colDrawPosition[1] = pt0[1];
				colDrawPosition[2] = pt0[2];
				colDrawPosition[3] = pt1[0];
				colDrawPosition[4] = pt1[1];
				colDrawPosition[5] = pt1[2];
				colDrawPosition[6] = pt2[0];
				colDrawPosition[7] = pt2[1];
				colDrawPosition[8] = pt2[2];
				colDrawPosition += 9;

				colDrawFlipPosition[0] = pt2[0];
				colDrawFlipPosition[1] = pt2[1];
				colDrawFlipPosition[2] = pt2[2];
				colDrawFlipPosition[3] = pt1[0];
				colDrawFlipPosition[4] = pt1[1];
				colDrawFlipPosition[5] = pt1[2];
				colDrawFlipPosition[6] = pt0[0];
				colDrawFlipPosition[7] = pt0[1];
				colDrawFlipPosition[8] = pt0[2];
				colDrawFlipPosition += 9;

				// normal
				auto faceNormal = (pt1 - pt0).Cross(pt2 - pt0);
				faceNormal.y = 0;
				faceNormal.Normalize();

				colDrawNormal[0] = faceNormal[0];
				colDrawNormal[1] = faceNormal[1];
				colDrawNormal[2] = faceNormal[2];
				colDrawNormal += 3;
				colDrawNormal[0] = faceNormal[0];
				colDrawNormal[1] = faceNormal[1];
				colDrawNormal[2] = faceNormal[2];
				colDrawNormal += 3;
				colDrawNormal[0] = faceNormal[0];
				colDrawNormal[1] = faceNormal[1];
				colDrawNormal[2] = faceNormal[2];
				colDrawNormal += 3;

				colDrawUV[0] = 0;
				colDrawUV[1] = 0;
				colDrawUV += 2;
				colDrawUV[0] = 0;
				colDrawUV[1] = 0;
				colDrawUV += 2;
				colDrawUV[0] = 0;
				colDrawUV[1] = 0;
				colDrawUV += 2;
			}
		}

		RenderMario<debugRenderId>(colBuffers, matrix);
		if (doubleSided) {
			RenderMario<debugRenderId+100>(colBuffersFlip, matrix);
		}
	}

	void UpdateMarioCollision() {
		NyaMat4x4 matrix;
		matrix.SetIdentity();

		g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, bWireframeBarriers ? D3DFILL_WIREFRAME : D3DFILL_SOLID);

		AddMarioStaticObject<2>(&aCollisionBarriers, !bWireframeBarriers, matrix);

		g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, bWireframeGround ? D3DFILL_WIREFRAME : D3DFILL_SOLID);

		AddMarioStaticObject<1>(&aCollisionTris, !bWireframeGround, matrix);
	}

	bool bEnabled = false;

	void OnTick() {
		if (!bEnabled) return;
		
		if (auto ply = GetLocalPlayerInterface<IRigidBody>()) {			
			aCollisionTris.clear();
			aCollisionBarriers.clear();

			for (int i = 0; i < 2700; i++) {
				auto pack = WCollisionAssets::mCollisionPackList[i];
				if (!pack) continue;

				for (int j = 0; j < pack->mInstanceNum; j++) {
					ProcessCollisionArticle(&pack->mInstanceList[j]);
				}
			}

			auto tmp = NyaDrawing::CNyaRGBA32();
			tmp.b = nObjColR;
			tmp.g = nObjColG;
			tmp.r = nObjColB;
			tmp.a = 255;
			Render3D::nVertexColorValue = *(uint32_t*)&tmp;
			static auto mdl = Render3D::CreateModels("cube.fbx");
			Render3D::nVertexColorValue = Render3D::nDefaultVertexColor;

			tmp.b = nCarColR;
			tmp.g = nCarColG;
			tmp.r = nCarColB;
			tmp.a = 255;
			Render3D::nVertexColorValue = *(uint32_t*)&tmp;
			static auto mdlCar = Render3D::CreateModels("cube.fbx");
			Render3D::nVertexColorValue = Render3D::nDefaultVertexColor;

			if (!mdl.empty() && !mdlCar.empty()) {
				g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, bWireframeCars ? D3DFILL_WIREFRAME : D3DFILL_SOLID);

				auto list = COLLISIONBODY_LIST::_mTable;
				for (int i = 0; i < list.size(); i++) {
					auto rb = list[i];
					if (!rb) continue;

					auto renderModel = mdl[0];

					auto simable = rb->mCOMObject->Find<ISimable>();
					if (auto veh = simable->mCOMObject->Find<IVehicle>()) {
						if (!veh->IsActive()) continue;
						if (veh->IsLoading()) continue;

						renderModel = mdlCar[0];
					}

					UMath::Vector3 dim;
					rb->GetDimension(&dim);

					UMath::Matrix4 mat = *rb->GetMatrix4();
					mat.p = *rb->GetPosition();

					mat.x *= dim.x;
					mat.y *= dim.y;
					mat.z *= dim.z;

					renderModel->RenderAt(WorldToRenderMatrix(mat));
				}
			}

			UpdateMarioCollision();
		}

		g_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	}

	ChloeHook Init([](){
		aDrawing3DLoopFunctions.push_back(OnTick);
	});
}