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

	struct CustomArticle {
		std::vector<WCollisionTri> aTriStrips;
		std::vector<WCollisionTri> aBarriers;
		b3MeshData* pB3Mesh;
		b3BodyId nB3Body;
		b3ShapeId nB3Shape;
	};
	CustomArticle aCollisionArticles[2700];

	void ProcessCollisionBarriers(int articleId, WCollisionBarrier* list, int count, NyaVec3 offset) {
		WriteLog(std::format("Adding {} barriers for article {}", count, articleId));

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

			aCollisionArticles[articleId].aBarriers.push_back(tri);

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
			aCollisionArticles[articleId].aBarriers.push_back(tri);

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

			aCollisionArticles[articleId].aBarriers.push_back(tri);

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
			aCollisionArticles[articleId].aBarriers.push_back(tri);
		}
	}

	void ProcessCollisionArticle(int articleId, WCollisionInstance* inst) {
		if (!inst) return;

		auto article = inst->fCollisionArticle;
		if (!article) return;

		UMath::Matrix4 instMat;
		inst->MakeMatrix(&instMat, true);

		// filter out unused stuff
		if (inst->fGroupNumber && !SceneryGroupEnabledTable[inst->fGroupNumber]) return;

		auto articles_end_ptr = (uintptr_t)(&article[1]);

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

				aCollisionArticles[articleId].aTriStrips.push_back(tri);

				auto flip = tri;
				flip.fPt0 = tri.fPt2;
				flip.fPt1 = tri.fPt1;
				flip.fPt2 = tri.fPt0;
				aCollisionArticles[articleId].aTriStrips.push_back(flip);
			}
			strip += strip->numTrisOrSurfaceId;
			stripSphere++;
		}

		ProcessCollisionBarriers(articleId, (WCollisionBarrier*)(articles_end_ptr + article->fStripsSize), article->fNumEdges, instMat.p);
	}

	void ConvertCollisionArticle(int articleId) {
		auto& article = aCollisionArticles[articleId];

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

		WriteLog(std::format("Processing {} verts ({} tri, {} barrier)", vertices.size(), article.aTriStrips.size()*3, article.aBarriers.size()*3));

		if (!vertices.empty()) {
			article.pB3Mesh = CreateMesh(&vertices[0], vertices.size(), &indices[0], indices.size());
			WriteLog(std::format("article.pB3Mesh {:X}", (uintptr_t)article.pB3Mesh));

			b3BodyDef def = b3DefaultBodyDef();
			def.type = b3_staticBody;
			def.position = {0,0,0};
			article.nB3Body = b3CreateBody(m_worldId, &def);
			WriteLog(std::format("article.nB3Body {}", article.nB3Body.index1));

			b3ShapeDef shapeDef = b3DefaultShapeDef();
			//shapeDef.materials = materials;
			//shapeDef.materialCount = 7;
			article.nB3Shape = b3CreateMeshShape(article.nB3Body, &shapeDef, article.pB3Mesh, b3Vec3_one);
			WriteLog(std::format("article.nB3Shape {}", article.nB3Shape.index1));
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

	b3BodyId PlayerBodyTemp;

	bool bEnabled = false;
	bool bDoReset = false;

	float fMoveSpeed = 5.0;
	float fMaxMoveSpeed = 50.0;

	bool bDrawThisFrame = false;
	void OnWorldTick() {
		if (!bEnabled) {
			bDoReset = true;
			return;
		}
		if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_RACING) {
			bDoReset = true;
			return;
		}
		if (IsInLoadingScreen() || IsInMovie()) {
			bDoReset = true;
			return;
		}

		static CNyaTimer gTimer;
		gTimer.Process();

		for (int i = 0; i < 2700; i++) {
			auto pack = WCollisionAssets::mCollisionPackList[i];
			if (!pack) continue;

			if (!aCollisionArticles[i].aTriStrips.empty()) continue;
			if (!aCollisionArticles[i].aBarriers.empty()) continue;

			WriteLog(std::format("Processing article {}", i));

			for (int j = 0; j < pack->mInstanceNum; j++) {
				WriteLog(std::format("Processing instance {}", j));
				ProcessCollisionArticle(i, &pack->mInstanceList[j]);
			}

			WriteLog(std::format("Converting article {}", i));

			ConvertCollisionArticle(i);
		}

		if (auto ply = GetLocalPlayerInterface<IRigidBody>()) {
			if (bDoReset) {
				auto pos = *ply->GetPosition();
				pos.y += 5;
				auto q = *ply->GetOrientation();
				b3Body_SetTransform(PlayerBodyTemp, {pos.x, pos.y, pos.z}, {q.x, q.y, q.z, q.w});
				b3Body_SetLinearVelocity(PlayerBodyTemp, {0,0,0});
				b3Body_SetAngularVelocity(PlayerBodyTemp, {0,0,0});
				bDoReset = false;
			}

			auto pos = b3Body_GetPosition(PlayerBodyTemp);
			auto quat = b3Body_GetRotation(PlayerBodyTemp);
			auto vel = b3Body_GetLinearVelocity(PlayerBodyTemp);
			auto avel = b3Body_GetAngularVelocity(PlayerBodyTemp);
			if (auto ply = GetLocalPlayerInterface<IRigidBody>()) {
				UMath::Vector3 v = {pos.x, pos.y, pos.z};
				ply->SetPosition(&v);
				v = {vel.x, vel.y, vel.z};
				ply->SetLinearVelocity(&v);
				v = {avel.x, avel.y, avel.z};
				ply->SetAngularVelocity(&v);
				UMath::Vector4 q = {quat.v.x, quat.v.y, quat.v.z, quat.s};
				ply->SetOrientation(&q);
			}

			auto mat = PrepareCameraMatrix(GetLocalPlayerCamera());
			auto fwd = RenderToWorldCoords(mat.z);
			auto side = RenderToWorldCoords(mat.x);
			fwd.y = 0;
			side.y = 0;
			fwd.Normalize();
			side.Normalize();
			auto input = GetLocalPlayerInterface<IInput>()->GetControls();
			float gas = input->fGas;
			float brake = input->fBrake;
			if (GetLocalPlayerInterface<ITransmission>()->GetGear() == G_REVERSE) {
				gas = input->fBrake;
				brake = input->fGas;
			}
			auto len = b3Length(vel);
			vel.x += fwd.x * gas * fMoveSpeed * gTimer.fDeltaTime;
			vel.y += fwd.y * gas * fMoveSpeed * gTimer.fDeltaTime;
			vel.z += fwd.z * gas * fMoveSpeed * gTimer.fDeltaTime;
			vel.x -= fwd.x * brake * fMoveSpeed * gTimer.fDeltaTime;
			vel.y -= fwd.y * brake * fMoveSpeed * gTimer.fDeltaTime;
			vel.z -= fwd.z * brake * fMoveSpeed * gTimer.fDeltaTime;
			vel.x += side.x * input->fSteering * fMoveSpeed * gTimer.fDeltaTime;
			vel.y += side.y * input->fSteering * fMoveSpeed * gTimer.fDeltaTime;
			vel.z += side.z * input->fSteering * fMoveSpeed * gTimer.fDeltaTime;
			auto newLen = b3Length(vel);
			if (newLen > fMaxMoveSpeed) {
				vel.x /= newLen;
				vel.y /= newLen;
				vel.z /= newLen;
				vel.x *= len;
				vel.y *= len;
				vel.z *= len;
			}
			b3Body_SetLinearVelocity(PlayerBodyTemp, vel);
		}

		if (!FEManager::mPauseRequest) {
			b3World_Step(m_worldId, gTimer.fDeltaTime, 4);
		}

		bDrawThisFrame = true;
	}

	void OnTick() {
		if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_RACING) {
			bDoReset = true;
			return;
		}
		if (IsInLoadingScreen() || IsInMovie()) {
			bDoReset = true;
			return;
		}

		if (!bDrawThisFrame) return;

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

		static auto mdl = Render3D::CreateModels("beachball.fbx");
		if (!mdl.empty()) {
			auto rb = GetLocalPlayerInterface<ICollisionBody>();

			UMath::Matrix4 mat = *rb->GetMatrix4();
			mat.p = *rb->GetPosition();

			mat.x *= 2.0;
			mat.y *= 2.0;
			mat.z *= 2.0;

			mdl[0]->RenderAt(WorldToRenderMatrix(mat));
		}
	}

	ChloeHook Init([]{
		aDrawing3DLoopFunctions.push_back(OnTick);
		aMainLoopFunctions.push_back(OnWorldTick);

		b3WorldDef worldDef = b3DefaultWorldDef();
		worldDef.workerCount = 8;
		worldDef.enableSleep = false;
		m_worldId = b3CreateWorld(&worldDef);

		b3BodyDef def = b3DefaultBodyDef();
		def.type = b3_dynamicBody;
		def.position = {0,0,0};
		PlayerBodyTemp = b3CreateBody(m_worldId, &def);
		WriteLog(std::format("PlayerBodyTemp {}", PlayerBodyTemp.index1));

		b3ShapeDef shapeDef = b3DefaultShapeDef();
		b3Sphere sphere;
		sphere.center = {0,0,0};
		sphere.radius = 2.0;
		auto shape = b3CreateSphereShape(PlayerBodyTemp, &shapeDef, &sphere);
		WriteLog(std::format("PlayerBodyShape {}", shape.index1));
	});
}