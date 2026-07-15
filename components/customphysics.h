namespace CustomPhysics {
	b3WorldId m_worldId;

	b3MeshData* CreateMesh(b3Vec3* vertices, int numVertices, int* indices, int numIndices) {
		b3MeshDef def = {};
		def.vertices = vertices;
		def.vertexCount = numVertices;
		def.indices = indices;
		def.triangleCount = numIndices / 3;
		def.materialIndices = nullptr;
		def.useMedianSplit = false; // todo?
		def.identifyEdges = true;
		def.weldVertices = true;
		def.weldTolerance = 0.002f;

		b3MeshData* meshData = b3CreateMesh( &def, nullptr, 0 );
		return meshData;
	}

	struct CustomArticleInstance {
		int nSceneryGroupId;
		std::vector<WCollisionTri> aTriStrips;
		std::vector<WCollisionTri> aBarriers;
		b3MeshData* pB3Mesh;
		b3BodyId nB3Body;
		bool bB3MeshEnabled;
	};

	struct CustomArticle {
		std::vector<CustomArticleInstance> aInstances;
	};
	CustomArticle aCollisionArticles[2700];

	struct CustomObjectInstance {
		b3BodyId nB3Body;
		IRigidBody* pGameBody;
		bool bReturnChangesToGame = false;
	};
	std::vector<CustomObjectInstance> aB3Objects;

	IVehicle* GetVehicleForB3Body(b3BodyId body) {
		for (auto& obj : aB3Objects) {
			if (B3_ID_EQUALS(obj.nB3Body, body)) {
				return obj.pGameBody->mCOMObject->Find<IVehicle>();
			}
		}
		return nullptr;
	}

	void ProcessCollisionBarriers(CustomArticleInstance* article, WCollisionBarrier* list, int count, NyaVec3 offset) {
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

			// first tri
			tri.fPt0.x = ptMin.x;
			tri.fPt0.y = ptMin.y;
			tri.fPt0.z = ptMin.z;
			tri.fPt1.x = ptMin.x;
			tri.fPt1.y = ptMax.y;
			tri.fPt1.z = ptMin.z;
			tri.fPt2.x = ptMax.x;
			tri.fPt2.y = ptMax.y;
			tri.fPt2.z = ptMax.z;

			article->aBarriers.push_back(tri);

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
			article->aBarriers.push_back(tri);
		}
	}

	void ProcessCollisionArticle(int articleId, WCollisionInstance* inst) {
		if (!inst) return;

		auto article = inst->fCollisionArticle;
		if (!article) return;

		UMath::Matrix4 instMat;
		inst->MakeMatrix(&instMat, true);

		// filter out unused stuff
		//if (inst->fGroupNumber && !SceneryGroupEnabledTable[inst->fGroupNumber]) return;

		auto articles_end_ptr = (uintptr_t)(&article[1]);

		aCollisionArticles[articleId].aInstances.push_back({});
		auto articleInst = &aCollisionArticles[articleId].aInstances[aCollisionArticles[articleId].aInstances.size()-1];

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

				auto flip = tri;
				flip.fPt0 = tri.fPt2;
				flip.fPt1 = tri.fPt1;
				flip.fPt2 = tri.fPt0;
				articleInst->aTriStrips.push_back(flip);
			}
			strip += strip->numTrisOrSurfaceId;
			stripSphere++;
		}

		ProcessCollisionBarriers(articleInst, (WCollisionBarrier*)(articles_end_ptr + article->fStripsSize), article->fNumEdges, instMat.p);
	}

	void ConvertCollisionArticle(CustomArticleInstance& article) {
		if (article.pB3Mesh) {
			b3DestroyMesh(article.pB3Mesh);
		}

		std::vector<b3Vec3> vertices;
		std::vector<int> indices;
		for (auto& tri : article.aTriStrips) {
			int id = &tri - &article.aTriStrips[0];
			indices.push_back(vertices.size());
			vertices.push_back({tri.fPt0.x, tri.fPt0.y, tri.fPt0.z});
			indices.push_back(vertices.size());
			vertices.push_back({tri.fPt1.x, tri.fPt1.y, tri.fPt1.z});
			indices.push_back(vertices.size());
			vertices.push_back({tri.fPt2.x, tri.fPt2.y, tri.fPt2.z});
		}

		for (auto& tri : article.aBarriers) {
			int id = &tri - &article.aBarriers[0];
			indices.push_back(vertices.size());
			vertices.push_back({tri.fPt0.x, tri.fPt0.y, tri.fPt0.z});
			indices.push_back(vertices.size());
			vertices.push_back({tri.fPt1.x, tri.fPt1.y, tri.fPt1.z});
			indices.push_back(vertices.size());
			vertices.push_back({tri.fPt2.x, tri.fPt2.y, tri.fPt2.z});
		}

		if (!vertices.empty()) {
			article.pB3Mesh = CreateMesh(&vertices[0], vertices.size(), &indices[0], indices.size());

			b3BodyDef def = b3DefaultBodyDef();
			def.type = b3_staticBody;
			def.position = {0,0,0};
			article.nB3Body = b3CreateBody(m_worldId, &def);

			b3ShapeDef shapeDef = b3DefaultShapeDef();
			//shapeDef.materials = materials;
			//shapeDef.materialCount = 7;
			auto nB3Shape = b3CreateMeshShape(article.nB3Body, &shapeDef, article.pB3Mesh, b3Vec3_one);

			article.bB3MeshEnabled = true;
		}
	}

	struct CollisionGeometryBuffer {
		float *position;
		int* index;
		uint16_t numVertices;
		uint16_t numFaces;
	};

	NyaVec3 WorldToRender(NyaVec3 in) {
		auto out = in;
		out.y *= -1;
		return out;
	}

	const int COLLVIEW_MAX_TRIANGLES = 16384;

	template<int bufId>
	void DebugRender(CollisionGeometryBuffer marioBuffers, NyaMat4x4 matrix) {
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

		int numFacesUsed = marioBuffers.numFaces;
		int numVerticesUsed = marioBuffers.numVertices;
		for (int i = 0; i < numVerticesUsed; i++) {
			auto src = &marioBuffers.position[i*3];
			auto dest = &verticesOut[i];

			auto tmpPos = WorldToRender({src[0], src[1], src[2]});
			dest->vPos[0] = tmpPos[0];
			dest->vPos[1] = tmpPos[1];
			dest->vPos[2] = tmpPos[2];

			dest->vNormals[0] = 0;
			dest->vNormals[1] = 1;
			dest->vNormals[2] = 0;

			dest->vTangents[0] = 0;
			dest->vTangents[1] = 1;
			dest->vTangents[2] = 0;

			auto tmp = NyaDrawing::CNyaRGBA32();
			tmp.b = 0;
			tmp.g = 255;
			tmp.r = 0;
			tmp.a = 255;
			dest->Color = *(uint32_t*)&tmp;

			dest->vUV[0] = 0.0;
			dest->vUV[1] = 0.0;
		}
		for (int i = 0; i < numFacesUsed*3; i++) {
			indicesOut[i] = marioBuffers.index[i];
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
		tmpModel.RenderAt_NoEffect(WorldToRenderMatrix(matrix), false);
	}

	bool bCollectLocalPlayerCar = true;
	float fWorldObjectMassScale = 100.0;
	float fWorldObjectMassMinimum = 400.0;
	void CollectWorldObjects() {
		for (auto& obj : aB3Objects) {
			if (obj.pGameBody && obj.bReturnChangesToGame) {
				auto m = b3MakeMatrixFromQuat(b3Body_GetRotation(obj.nB3Body));

				UMath::Matrix4 mat;
				mat.x.x = m.cx.x;
				mat.x.y = m.cx.y;
				mat.x.z = m.cx.z;
				mat.y.x = m.cy.x;
				mat.y.y = m.cy.y;
				mat.y.z = m.cy.z;
				mat.z.x = m.cz.x;
				mat.z.y = m.cz.y;
				mat.z.z = m.cz.z;
				obj.pGameBody->SetOrientation(&mat);

				auto p = b3Body_GetPosition(obj.nB3Body);
				auto v = b3Body_GetLinearVelocity(obj.nB3Body);
				auto av = b3Body_GetAngularVelocity(obj.nB3Body);

				UMath::Vector3 pos;
				pos.x = p.x;
				pos.y = p.y;
				pos.z = p.z;
				obj.pGameBody->SetPosition(&pos);

				UMath::Vector3 vel;
				vel.x = v.x;
				vel.y = v.y;
				vel.z = v.z;
				obj.pGameBody->SetLinearVelocity(&vel);

				UMath::Vector3 avel;
				avel.x = av.x;
				avel.y = av.y;
				avel.z = av.z;
				obj.pGameBody->SetAngularVelocity(&avel);
			}

			b3DestroyBody(obj.nB3Body);
		}
		aB3Objects.clear();

		auto objs = GetActiveRigidBodies();
		for (auto& rb : objs) {
			if (!bCollectLocalPlayerCar && rb == GetLocalPlayerInterface<IRigidBody>()) continue;
			if (rb->GetMass() < fWorldObjectMassMinimum) continue;

			auto objInst = CustomObjectInstance();

			UMath::Vector3 dim;
			rb->GetDimension(&dim);

			auto p = *rb->GetPosition();

			UMath::Matrix4 m;
			rb->GetMatrix4(&m);

			b3Matrix3 m3;
			m3.cx.x = m.x.x;
			m3.cx.y = m.x.y;
			m3.cx.z = m.x.z;
			m3.cy.x = m.y.x;
			m3.cy.y = m.y.y;
			m3.cy.z = m.y.z;
			m3.cz.x = m.z.x;
			m3.cz.y = m.z.y;
			m3.cz.z = m.z.z;

			b3BodyDef def = b3DefaultBodyDef();
			def.type = b3_dynamicBody;
			def.position = {p.x,p.y,p.z};
			def.rotation = b3MakeQuatFromMatrix(&m3);
			objInst.nB3Body = b3CreateBody(m_worldId, &def);

			b3ShapeDef shapeDef = b3DefaultShapeDef();
			auto hull = b3MakeBoxHull(dim.x, dim.y, dim.z);
			b3CreateHullShape(objInst.nB3Body, &shapeDef, &hull.base);

			auto vel = *rb->GetLinearVelocity();
			auto avel = *rb->GetAngularVelocity();
			auto mass = rb->GetMass() * fWorldObjectMassScale;
			b3Body_SetLinearVelocity(objInst.nB3Body, {vel.x,vel.y,vel.z});
			b3Body_SetAngularVelocity(objInst.nB3Body, {avel.x,avel.y,avel.z});
			b3MassData massData;
			massData.mass = mass;
			massData.inertia = {mass*dim.x,mass*dim.y,mass*dim.z};
			massData.center = {0,0,0};
			b3Body_SetMassData(objInst.nB3Body, massData);

			objInst.pGameBody = rb;
			aB3Objects.push_back(objInst);
		}
	}

	bool bEnabled = false;
	void OnWorldTick() {
		PerformanceBenchmarker _perf("CustomPhysics::OnWorldTick");

		if (!bEnabled) return;
		if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_RACING) return;
		if (IsInLoadingScreen() || IsInMovie()) return;

		static CNyaTimer gTimer;
		gTimer.Process();

		for (int i = 0; i < 2700; i++) {
			auto pack = WCollisionAssets::mCollisionPackList[i];
			if (!pack) continue;

			if (!aCollisionArticles[i].aInstances.empty()) continue;

			for (int j = 0; j < pack->mInstanceNum; j++) {
				ProcessCollisionArticle(i, &pack->mInstanceList[j]);
			}

			for (auto& inst : aCollisionArticles[i].aInstances) {
				ConvertCollisionArticle(inst);
			}
		}

		for (int i = 0; i < 2700; i++) {
			auto pack = WCollisionAssets::mCollisionPackList[i];
			if (!pack) continue;

			for (auto& inst : aCollisionArticles[i].aInstances) {
				auto enabled = !inst.nSceneryGroupId || SceneryGroupEnabledTable[inst.nSceneryGroupId];
				if (enabled != inst.bB3MeshEnabled) {
					if (enabled) {
						b3Body_Enable(inst.nB3Body);
					}
					else {
						b3Body_Disable(inst.nB3Body);
					}
					inst.bB3MeshEnabled = enabled;
				}
			}
		}

		CollectWorldObjects();

		if (!FEManager::mPauseRequest) {
			b3World_Step(m_worldId, gTimer.fDeltaTime, 4);
		}
	}

	void OnTick3D() {
		/*for (int i = 0; i < 2700; i++) {
			auto mesh = aCollisionArticles[i].pB3Mesh;
			if (!mesh) continue;

			auto verts = b3GetMeshVertices(mesh);
			auto tris = b3GetMeshTriangles(mesh);
			if (!verts || !tris) continue;

			CollisionGeometryBuffer tmp;
			tmp.position = (float*)verts;
			tmp.index = (int*)tris;
			tmp.numVertices = mesh->vertexCount;
			tmp.numFaces = mesh->triangleCount;

			NyaMat4x4 mat;
			mat.SetIdentity();
			DebugRender<6969>(tmp, mat);
		}*/
	}

	ChloeHook Init([]{
		aMainLoopFunctions.push_back(OnWorldTick);

		b3WorldDef worldDef = b3DefaultWorldDef();
		worldDef.workerCount = 8;
		m_worldId = b3CreateWorld(&worldDef);
	});
}