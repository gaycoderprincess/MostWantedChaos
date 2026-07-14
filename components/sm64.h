namespace SM64 {
	uint8_t *utils_read_file_alloc( const char *path, size_t *fileLength )
	{
		FILE *f = fopen( path, "rb" );

		if( !f ) return NULL;

		fseek( f, 0, SEEK_END );
		size_t length = (size_t)ftell( f );
		rewind( f );
		uint8_t *buffer = (uint8_t*)malloc( length + 1 );
		fread( buffer, 1, length, f );
		buffer[length] = 0;
		fclose( f );

		if( fileLength ) *fileLength = length;

		return buffer;
	}

	int32_t marioId;

	SM64MarioInputs marioInputs;
	SM64MarioState marioState;
	SM64MarioGeometryBuffers marioGeometry;

	// interpolation
	float lastPos[3] = {};
	float currPos[3] = {};
	float lastGeoPos[9 * SM64_GEO_MAX_TRIANGLES] = {};
	float currGeoPos[9 * SM64_GEO_MAX_TRIANGLES] = {};

	float marioScalar = 100;

	// i dont get it why is this different??????
	NyaVec3 MarioToWorld_Render(NyaVec3 v) {
		auto out = v / marioScalar;
		out.y *= -1;
		out.z *= -1;
		return out;
	}

	NyaVec3 MarioToWorld_RenderNormal(NyaVec3 v) {
		auto out = v;
		out.y *= -1;
		out.z *= -1;
		return out;
	}

	NyaVec3 MarioToWorld(NyaVec3 v) {
		auto out = v / marioScalar;
		//out.x *= -1;
		out.z *= -1;
		return out;
	}

	NyaVec3 WorldToMario(NyaVec3 v) {
		auto out = v * marioScalar;
		//out.x *= -1;
		out.z *= -1;
		return out;
	}

	int marioLightness = 255;
	int marioLightnessMenu = 255;

	bool bInvincibleFlash = false;

	template<int bufId, bool textured>
	void RenderMario(SM64MarioGeometryBuffers marioBuffers) {
		if (marioState.invincTimer > 0 && bInvincibleFlash) return;

		int numFaces = SM64_GEO_MAX_TRIANGLES;
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

		int lightness = TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_IN_FRONTEND ? marioLightnessMenu : marioLightness;

		int numFacesUsed = marioBuffers.numTrianglesUsed;
		int numVerticesUsed = marioBuffers.numTrianglesUsed*3;
		for (int i = 0; i < numVerticesUsed; i++) {
			auto src = &marioBuffers.position[i*3];
			auto srcNormal = &marioBuffers.normal[i*3];
			//auto srcTangent = &tangents[i*3];
			auto srcUV = &marioBuffers.uv[i*2];
			auto srcColor = &marioBuffers.color[i*3];
			auto dest = &verticesOut[i];

			auto tmpPos = MarioToWorld_Render({src[0], src[1], src[2]});
			dest->vPos[0] = tmpPos[0];
			dest->vPos[1] = tmpPos[1];
			dest->vPos[2] = tmpPos[2];

			auto tmpNormal = MarioToWorld_RenderNormal({srcNormal[0], srcNormal[1], srcNormal[2]});

			dest->vNormals[0] = tmpNormal[0];
			dest->vNormals[1] = tmpNormal[1];
			dest->vNormals[2] = tmpNormal[2];

			dest->vTangents[0] = tmpNormal[0];
			dest->vTangents[1] = tmpNormal[1];
			dest->vTangents[2] = tmpNormal[2];

			// todo?
			//if (tangents) {
			//	dest->vTangents[0] = srcTangent[0];
			//	dest->vTangents[1] = srcTangent[1];
			//	dest->vTangents[2] = srcTangent[2];
			//}

			auto tmp = NyaDrawing::CNyaRGBA32();
			if (textured) {
				tmp.b = lightness;
				tmp.g = lightness;
				tmp.r = lightness;
			}
			else {
				tmp.b = srcColor[0] * lightness;
				tmp.g = srcColor[1] * lightness;
				tmp.r = srcColor[2] * lightness;
			}
			tmp.a = 255;
			dest->Color = *(uint32_t*)&tmp;

			if (textured && (srcUV[0] != 1 && srcUV[1] != 1)) {
				dest->vUV[0] = srcUV[0];
				dest->vUV[1] = srcUV[1];
			}
			else {
				dest->vUV[0] = 0.5;
				dest->vUV[1] = 0.5;
			}
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

		auto mat = NyaMat4x4();
		mat.SetIdentity();
		if (textured) {
			static auto marioTextured = LoadTexture("CwoeeChaos/data/models/letsago.png");
			tmpModel.pTexture = marioTextured;
			tmpModel.RenderAt(WorldToRenderMatrix(mat), true);
		}
		else {
			static auto marioColored = LoadTexture("CwoeeChaos/data/models/letsago_white.png");
			tmpModel.pTexture = marioColored;
			tmpModel.RenderAt(WorldToRenderMatrix(mat), false);
		}
	}

	std::vector<WCollisionTri> aCollisionTris;
	std::vector<WCollisionTri> aCollisionBarriers;
	std::vector<int> aCollisionTriMarios;

	void ProcessCollisionBarriers(WCollisionBarrier* list, int count, NyaVec3 offset) {
		if (*(uint8_t*)0x6BB796 == 0xEB) return; // disable wall collision chaos effect

		auto marioPos = MarioToWorld({marioState.position[0], marioState.position[1], marioState.position[2]});
		marioPos.y = 0;

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

			// normal
			auto faceNormal = (tri.fPt1 - tri.fPt0).Cross(tri.fPt2 - tri.fPt0);
			faceNormal.y = 0;
			faceNormal.Normalize();

			auto faceCenter = (tri.fPt0 + tri.fPt1 + tri.fPt2) / 3.0;
			faceCenter.y = 0;

			auto marioNormal = (marioPos - faceCenter);
			marioNormal.Normalize();

			// flip barrier face if mario is behind it
			bool flip = marioNormal.Dot(faceNormal) > 0.0;
			if (flip) {
				tri.fPt0.x = ptMin.x;
				tri.fPt0.y = ptMin.y;
				tri.fPt0.z = ptMin.z;
				tri.fPt1.x = ptMin.x;
				tri.fPt1.y = ptMax.y;
				tri.fPt1.z = ptMin.z;
				tri.fPt2.x = ptMax.x;
				tri.fPt2.y = ptMax.y;
				tri.fPt2.z = ptMax.z;
			}
			aCollisionBarriers.push_back(tri);

			// second tri
			if (flip) {
				tri.fPt2.x = ptMin.x;
				tri.fPt2.y = ptMin.y;
				tri.fPt2.z = ptMin.z;
				tri.fPt1.x = ptMax.x;
				tri.fPt1.y = ptMin.y;
				tri.fPt1.z = ptMax.z;
				tri.fPt0.x = ptMax.x;
				tri.fPt0.y = ptMax.y;
				tri.fPt0.z = ptMax.z;
			}
			else {
				tri.fPt0.x = ptMin.x;
				tri.fPt0.y = ptMin.y;
				tri.fPt0.z = ptMin.z;
				tri.fPt1.x = ptMax.x;
				tri.fPt1.y = ptMin.y;
				tri.fPt1.z = ptMax.z;
				tri.fPt2.x = ptMax.x;
				tri.fPt2.y = ptMax.y;
				tri.fPt2.z = ptMax.z;
			}
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

	void ClearMarioCollision() {
		for (auto& id : aCollisionTriMarios) {
			sm64_surface_object_delete(id);
		}
		aCollisionTriMarios.clear();
	}

	void AddDynamicDummyFloor(NyaVec3 center, int width) {
		SM64SurfaceObject obj;
		obj.surfaces = new SM64Surface[2];
		obj.surfaceCount = 2;
		obj.transform.position[0] = 0;
		obj.transform.position[1] = 0;
		obj.transform.position[2] = 0;
		obj.transform.eulerRotation[0] = 0;
		obj.transform.eulerRotation[1] = 0;
		obj.transform.eulerRotation[2] = 0;

		obj.surfaces[0].type = SURFACE_DEFAULT;
		obj.surfaces[0].force = 0;
		obj.surfaces[0].terrain = TERRAIN_GRASS;
		obj.surfaces[0].vertices[0][0] = center.x + width;
		obj.surfaces[0].vertices[0][1] = center.y;
		obj.surfaces[0].vertices[0][2] = center.z + width;
		obj.surfaces[0].vertices[1][0] = center.x - width;
		obj.surfaces[0].vertices[1][1] = center.y;
		obj.surfaces[0].vertices[1][2] = center.z - width;
		obj.surfaces[0].vertices[2][0] = center.x - width;
		obj.surfaces[0].vertices[2][1] = center.y;
		obj.surfaces[0].vertices[2][2] = center.z + width;

		obj.surfaces[1].type = SURFACE_DEFAULT;
		obj.surfaces[1].force = 0;
		obj.surfaces[1].terrain = TERRAIN_GRASS;
		obj.surfaces[1].vertices[0][0] = center.x - width;
		obj.surfaces[1].vertices[0][1] = center.y;
		obj.surfaces[1].vertices[0][2] = center.z - width;
		obj.surfaces[1].vertices[1][0] = center.x + width;
		obj.surfaces[1].vertices[1][1] = center.y;
		obj.surfaces[1].vertices[1][2] = center.z + width;
		obj.surfaces[1].vertices[2][0] = center.x + width;
		obj.surfaces[1].vertices[2][1] = center.y;
		obj.surfaces[1].vertices[2][2] = center.z - width;

		auto id = sm64_surface_object_create(&obj);
		if (id != -1) {
			aCollisionTriMarios.push_back(id);
		}
	}

	void AddMarioStaticObject(std::vector<WCollisionTri>* collisions, bool doubleSided) {
		SM64SurfaceObject obj;
		obj.surfaces = new SM64Surface[collisions->size()];
		obj.surfaceCount = collisions->size();
		obj.transform.position[0] = 0;
		obj.transform.position[1] = 0;
		obj.transform.position[2] = 0;
		obj.transform.eulerRotation[0] = 0;
		obj.transform.eulerRotation[1] = 0;
		obj.transform.eulerRotation[2] = 0;

		auto objFlip = obj;
		objFlip.surfaces = new SM64Surface[collisions->size()];

		for (int i = 0; i < collisions->size(); i++) {
			auto in = &(*collisions)[i];
			auto out = &obj.surfaces[i];
			auto out2 = &objFlip.surfaces[i];

			out->type = SURFACE_DEFAULT;
			out->force = 0;
			out->terrain = TERRAIN_GRASS;

			auto pt0 = WorldToMario({in->fPt0[0],in->fPt0[1],in->fPt0[2]});
			auto pt1 = WorldToMario({in->fPt1[0],in->fPt1[1],in->fPt1[2]});
			auto pt2 = WorldToMario({in->fPt2[0],in->fPt2[1],in->fPt2[2]});

			out->vertices[0][0] = pt0[0];
			out->vertices[0][1] = pt0[1];
			out->vertices[0][2] = pt0[2];
			out->vertices[1][0] = pt1[0];
			out->vertices[1][1] = pt1[1];
			out->vertices[1][2] = pt1[2];
			out->vertices[2][0] = pt2[0];
			out->vertices[2][1] = pt2[1];
			out->vertices[2][2] = pt2[2];

			*out2 = *out;
			out2->vertices[0][0] = pt2[0];
			out2->vertices[0][1] = pt2[1];
			out2->vertices[0][2] = pt2[2];
			out2->vertices[1][0] = pt1[0];
			out2->vertices[1][1] = pt1[1];
			out2->vertices[1][2] = pt1[2];
			out2->vertices[2][0] = pt0[0];
			out2->vertices[2][1] = pt0[1];
			out2->vertices[2][2] = pt0[2];
		}

		auto id = sm64_surface_object_create(&obj);
		if (id != -1) {
			aCollisionTriMarios.push_back(id);
		}

		if (doubleSided) {
			auto id2 = sm64_surface_object_create(&objFlip);
			if (id2 != -1) {
				aCollisionTriMarios.push_back(id2);
			}
		}

		delete[] obj.surfaces;
		delete[] objFlip.surfaces;
	}

	void UpdateMarioCollision() {
		PerformanceBenchmarker _perf("UpdateMarioCollision");

		if (aCollisionTris.empty()) return; // always keep old collision if empty

		for (auto& id : aCollisionTriMarios) {
			sm64_surface_object_delete(id);
		}
		aCollisionTriMarios.clear();

		// fix invisible walls
		AddDynamicDummyFloor({marioState.position[0],-500 * marioScalar,marioState.position[2]}, 16384);

		AddMarioStaticObject(&aCollisionTris, true);
		AddMarioStaticObject(&aCollisionBarriers, false);
	}

	void InitAudio();

	void LoadDummyFloor(NyaVec3 center, int width) {
		SM64Surface surfaces[2];

		surfaces[0].type = SURFACE_DEFAULT;
		surfaces[0].force = 0;
		surfaces[0].terrain = TERRAIN_GRASS;
		surfaces[0].vertices[0][0] = center.x + width;
		surfaces[0].vertices[0][1] = center.y;
		surfaces[0].vertices[0][2] = center.z + width;
		surfaces[0].vertices[1][0] = center.x - width;
		surfaces[0].vertices[1][1] = center.y;
		surfaces[0].vertices[1][2] = center.z - width;
		surfaces[0].vertices[2][0] = center.x - width;
		surfaces[0].vertices[2][1] = center.y;
		surfaces[0].vertices[2][2] = center.z + width;

		surfaces[1].type = SURFACE_DEFAULT;
		surfaces[1].force = 0;
		surfaces[1].terrain = TERRAIN_GRASS;
		surfaces[1].vertices[0][0] = center.x - width;
		surfaces[1].vertices[0][1] = center.y;
		surfaces[1].vertices[0][2] = center.z - width;
		surfaces[1].vertices[1][0] = center.x + width;
		surfaces[1].vertices[1][1] = center.y;
		surfaces[1].vertices[1][2] = center.z + width;
		surfaces[1].vertices[2][0] = center.x + width;
		surfaces[1].vertices[2][1] = center.y;
		surfaces[1].vertices[2][2] = center.z - width;

		sm64_static_surfaces_load(surfaces, 2);
	}

	void ResetMario(NyaVec3 worldPos) {
		if (marioId >= 0) sm64_mario_delete(marioId);

		auto pos = WorldToMario(worldPos);

		LoadDummyFloor(pos, 128);

		marioId = sm64_mario_create(pos.x, pos.y + 300, pos.z);
		marioState.position[0] = pos.x;
		marioState.position[1] = pos.y;
		marioState.position[2] = pos.z;
	}

	bool bDoReset = false;
	bool bEnabled = false;
	bool bAvailable = false;
	double fTimeSinceLastAttacked = 0.0;

	void EnableMario() {
		bEnabled = true;
		NyaHookLib::Patch<uint16_t>(0x6B1A02, 0x9090); // disable player causality check for cop flipping
	}

	void DisableMario() {
		bEnabled = false;
		CarRender_DontRenderPlayer = false;
		DrawLightFlares = true;
		DrawCars = true;
		NyaHookLib::Patch<uint16_t>(0x6B1A02, 0x0974);
	}

	NyaVec3 GetMarioWorldPos() {
		return MarioToWorld({marioState.position[0], marioState.position[1], marioState.position[2]});
	}

	NyaVec3 GetMarioWorldVelocity() {
		return MarioToWorld({marioState.velocity[0], marioState.velocity[1], marioState.velocity[2]}) / (1.0 / 30.0);
	}

	NyaVec3 GetMarioWorldFacing() {
		NyaMat4x4 mat;
		mat.SetIdentity();
		mat.Rotate({0,0,marioState.faceAngle});
		return -mat.z;
	}

	float GetMarioScale() {
		return 100.0f / SM64::marioScalar;
	}

	void MarioInteract_KnockAway(IRigidBody* body) {
		UMath::Vector3 marioPos = GetMarioWorldPos();
		auto objPos = *body->GetPosition();

		UMath::Vector3 dir = (objPos - marioPos);
		dir.Normalize();

		dir *= 15;

		body->SetLinearVelocity(&dir);
	}

	template<typename T>
	void MarioInteract_KnockFwd(T* body) {
		UMath::Vector3 dir = GetMarioWorldFacing();
		dir *= 25;
		dir.y = 5;
		body->SetLinearVelocity(&dir);

		dir *= 0.15;
		body->SetAngularVelocity(&dir);
	}

	bool IsCustomObjectKillable(const Render3DObjects::Object& obj) {
		if (obj.sDebugName == "vergil") return true;
		if (obj.sDebugName == "scientist") return true;
		return false;
	}

	bool IsCustomObjectPunchKillable(const Render3DObjects::Object& obj) {
		if (obj.sDebugName == "scientist") return true;
		return false;
	}

	void KillCustomObject(Render3DObjects::Object& obj) {
		if (obj.sDebugName == "vergil") { // teleport vergil, cant remove his music from here
			obj.vColPosition = obj.mMatrix.p = {0,-100,0};
		}
		else {
			obj.aModels.clear();
		}
	}

	void MarioCustomObjectInteractions() {
		UMath::Vector3 marioPos = GetMarioWorldPos();

		for (auto& car : Render3DObjects::aObjects) {
			if (car.fColSize <= 0.0) continue;

			float dimY = car.fColSize;

			const float fAttackRange = 6.0 * GetMarioScale();
			const float fJumpAttackRange = 2.5 * GetMarioScale();

			float dist = (car.vColPosition - marioPos).length();
			if (dist < fAttackRange) {
				auto pos = WorldToMario(car.vColPosition);
				auto interaction = sm64_fake_determine_interaction(marioId, pos.x, pos.y, pos.z);

				// ground pound is an instakill
				if (interaction == INT_GROUND_POUND_OR_TWIRL) {
					if (dist < fJumpAttackRange && marioState.velocity[1] < -50.0f && IsCustomObjectKillable(car)) { // only kill while moving downwards
						KillCustomObject(car);
						sm64_play_sound_global(SOUND_GENERAL_BREAK_BOX);
					}
				}
				// bounce off if needed
				else if (interaction == INT_HIT_FROM_ABOVE || interaction == INT_HIT_FROM_BELOW) {
					if (dist < fJumpAttackRange) {
						sm64_mario_attack(marioId, pos.x, pos.y, pos.z, dimY * marioScalar);
					}
				}
				// punches & kicks throw forward
				else if (interaction == INT_PUNCH || interaction == INT_KICK) {
					if (IsCustomObjectPunchKillable(car)) {
						KillCustomObject(car);
					}

					// bounce_back_from_attack

					if (marioState.action == ACT_PUNCHING) {
						sm64_set_mario_action(marioId, ACT_MOVE_PUNCHING);
					}

					if (marioState.action & ACT_FLAG_AIR) {
						sm64_set_mario_forward_velocity(marioId, -16.0f);
					} else {
						sm64_set_mario_forward_velocity(marioId, -48.0f);
					}

					sm64_play_sound_global(SOUND_ACTION_HIT_2);
				}
				// all else throws away
				//else if (interaction) {
				//	MarioInteract_KnockAway(rb);
				//}
			}
		}
	}

	void MarioCustomPhysicsObjectInteractions() {
		UMath::Vector3 marioPos = GetMarioWorldPos();

		auto objs = CustomPhysicsObjects::aPhysicsObjects;
		for (auto& rb : objs) {
			auto dim = rb.vModelSize;

			const float fAttackRange = 6.0 * GetMarioScale();
			const float fJumpAttackRange = 2.5 * GetMarioScale();

			float dist = (rb.GetPosition() - marioPos).length();
			if (dist < fAttackRange) {
				auto pos = WorldToMario(rb.GetPosition());
				auto interaction = sm64_fake_determine_interaction(marioId, pos.x, pos.y, pos.z);

				// ground pound is an instakill
				if (interaction == INT_GROUND_POUND_OR_TWIRL) {
					// todo
				}
				// bounce off if needed
				else if (interaction == INT_HIT_FROM_ABOVE || interaction == INT_HIT_FROM_BELOW) {
					if (dist < fJumpAttackRange) {
						sm64_mario_attack(marioId, pos.x, pos.y, pos.z, dim.y * marioScalar);
					}
				}
				// punches & kicks throw forward
				else if (interaction == INT_PUNCH || interaction == INT_KICK) {
					MarioInteract_KnockFwd(&rb);

					// bounce_back_from_attack

					if (marioState.action == ACT_PUNCHING) {
						sm64_set_mario_action(marioId, ACT_MOVE_PUNCHING);
					}

					if (marioState.action & ACT_FLAG_AIR) {
						sm64_set_mario_forward_velocity(marioId, -16.0f);
					} else {
						sm64_set_mario_forward_velocity(marioId, -48.0f);
					}

					sm64_play_sound_global(SOUND_ACTION_HIT_2);
				}
				// all else throws away
				//else if (interaction) {
				//	MarioInteract_KnockAway(rb);
				//}
			}
		}
	}

	void MarioObjectInteractions() {
		UMath::Vector3 marioPos = GetMarioWorldPos();

		auto cars = GetActiveVehicles();
		for (auto& car : cars) {
			if (car == GetLocalPlayerVehicle()) continue;

			auto rb = car->GetSimable()->GetRigidBody();
			if (!rb) continue;

			UMath::Vector3 dim;
			rb->GetDimension(&dim);

			const float fAttackRange = 6.0 * GetMarioScale();
			const float fJumpAttackRange = 2.5 * GetMarioScale();

			float dist = (*car->GetPosition() - marioPos).length();
			if (dist < fAttackRange) {
				auto pos = WorldToMario(*car->GetPosition());
				auto interaction = sm64_fake_determine_interaction(marioId, pos.x, pos.y, pos.z);

				// ground pound is an instakill
				if (interaction == INT_GROUND_POUND_OR_TWIRL) {
					if (dist < fJumpAttackRange && marioState.velocity[1] < -50.0f) { // only kill while moving downwards
						if (auto dam = car->mCOMObject->Find<IEngineDamage>()) {
							if (!dam->IsBlown()) dam->Blow();
						}
						if (auto dam = car->mCOMObject->Find<IDamageable>()) {
							if (!dam->IsDestroyed()) {
								dam->Destroy();
								sm64_play_sound_global(SOUND_GENERAL_BREAK_BOX);
							}
						}
					}
				}
				// bounce off if needed
				else if (interaction == INT_HIT_FROM_ABOVE || interaction == INT_HIT_FROM_BELOW) {
					if (dist < fJumpAttackRange) {
						sm64_mario_attack(marioId, pos.x, pos.y, pos.z, dim.y * marioScalar);

						// jumping on weak cops kills them
						auto name = car->GetVehicleName();
						if (!strcmp(name, "copmidsize") || !strcmp(name, "copghost")) {
							if (auto dam = car->mCOMObject->Find<IEngineDamage>()) {
								if (!dam->IsBlown()) dam->Blow();
							}
							if (auto dam = car->mCOMObject->Find<IDamageable>()) {
								if (!dam->IsDestroyed()) {
									dam->Destroy();
									sm64_play_sound_global(SOUND_GENERAL_BREAK_BOX);
								}
							}
						}
					}
				}
				// punches & kicks throw forward
				else if (interaction == INT_PUNCH || interaction == INT_KICK) {
					MarioInteract_KnockFwd(rb);

					// bounce_back_from_attack

					if (marioState.action == ACT_PUNCHING) {
						sm64_set_mario_action(marioId, ACT_MOVE_PUNCHING);
					}

					if (marioState.action & ACT_FLAG_AIR) {
						sm64_set_mario_forward_velocity(marioId, -16.0f);
					} else {
						sm64_set_mario_forward_velocity(marioId, -48.0f);
					}

					sm64_play_sound_global(SOUND_ACTION_HIT_2);
				}
				// all else throws away
				//else if (interaction) {
				//	MarioInteract_KnockAway(rb);
				//}
			}
		}
	}

	void MarioCarInheritance() {
		auto ply = GetLocalPlayerInterface<ISpikeable>();
		if (ply && ply->GetNumBlowouts() > 0) {
			GetLocalPlayerInterface<IDamageable>()->ResetDamage();

			sm64_set_mario_action_arg(SM64::marioId, ACT_LAVA_BOOST, 1);
		}
	}

	void OnTick() {
		PerformanceBenchmarker _perf("SM64::OnTick");

		if (!bEnabled) {
			bDoReset = true;
			return;
		}
		if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_RACING && TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_IN_FRONTEND) {
			bDoReset = true;
			return;
		}
		if (IsInLoadingScreen() || IsInMovie()) {
			bDoReset = true;
			return;
		}

		UMath::Vector3 marioPos = GetMarioWorldPos();
		UMath::Vector3 marioVel = GetMarioWorldVelocity();
		if (marioPos.y < -100) {
			bDoReset = true;
		}
		if (TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_IN_FRONTEND && marioPos.length() > 50) {
			bDoReset = true;
		}

		if (auto ply = GetLocalPlayerInterface<IRigidBody>()) {
			static CNyaTimer gCollisionTimer;
			gCollisionTimer.Process();

			if ((gCollisionTimer.fTotalTime >= 0.5 && GetMarioWorldVelocity().length() > 0.0) || bDoReset) {
				gCollisionTimer.fTotalTime -= 0.5;

				aCollisionTris.clear();
				aCollisionBarriers.clear();

				for (int i = 0; i < 2700; i++) {
					auto pack = WCollisionAssets::mCollisionPackList[i];
					if (!pack) continue;

					for (int j = 0; j < pack->mInstanceNum; j++) {
						ProcessCollisionArticle(&pack->mInstanceList[j]);
					}
				}

				// custom spawned barriers from chaos objects
				std::vector<WCollisionBarrier> barriers;
				auto customBarriers = Render3DObjects::GetFullBarrierList();
				for (auto& barrier : customBarriers) {
					barriers.push_back(barrier.data);
				}
				ProcessCollisionBarriers(&barriers[0], barriers.size(), {0,0,0});

				UpdateMarioCollision();
			}

			if (!bDoReset && marioPos.length() > 50) {
				marioPos.y += 1;
				ply->SetPosition(&marioPos);

				UMath::Matrix4 lookatMatrix = NyaMat4x4::LookAt(GetMarioWorldFacing(), {0,1,0});
				ply->SetOrientation(&lookatMatrix);

				UMath::Vector3 tmp = {0,0,0};

				ply->SetLinearVelocity(&marioVel);
				ply->SetAngularVelocity(&tmp);

				CarRender_DontRenderPlayer = true;
				DrawLightFlares = false;
			}
		}
		else {
			CarRender_DontRenderPlayer = false;
			DrawLightFlares = true;
			if (TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_IN_FRONTEND) {
				DrawCars = false;
			}

			if (bDoReset) {
				ClearMarioCollision();
			}
		}

		if (bDoReset) {
			DrawCars = true;

			NyaVec3 v = {0,0,0};
			if (auto ply = GetLocalPlayerInterface<IRigidBody>()) {
				v = *ply->GetPosition();
				v.y -= 1;
			}
			ResetMario(v);
		}
		bDoReset = false;

		if (TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_IN_FRONTEND) {
			marioInputs.buttonA = 0;
			marioInputs.buttonB = 0;
			marioInputs.buttonZ = 0;
		}
		else {
			marioInputs.buttonA = IsPadKeyPressed(NYA_PAD_KEY_A);
			marioInputs.buttonB = IsPadKeyPressed(NYA_PAD_KEY_B);
			marioInputs.buttonZ = IsPadKeyPressed(NYA_PAD_KEY_X) || IsPadKeyPressed(NYA_PAD_KEY_LB) || IsPadKeyPressed(NYA_PAD_KEY_RB);
		}

		float cameraPos[3] = {};

		auto cameraMatReal = PrepareCameraMatrix(GetLocalPlayerCamera());
		auto cameraPosReal = WorldToMario(RenderToWorldCoords(cameraMatReal.p));
		cameraPos[0] = cameraPosReal[0];
		cameraPos[1] = cameraPosReal[1];
		cameraPos[2] = cameraPosReal[2];

		//cameraPos[0] = marioState.position[0] + 1000.0f * cosf(cameraRot);
		//cameraPos[1] = marioState.position[1] + 200.0f;
		//cameraPos[2] = marioState.position[2] + 1000.0f * sinf(cameraRot);

		marioInputs.camLookX = marioState.position[0] - cameraPos[0];
		marioInputs.camLookZ = marioState.position[2] - cameraPos[2];
		marioInputs.stickX = GetPadKeyState(NYA_PAD_KEY_LSTICK_X) / 32767.0;
		marioInputs.stickY = GetPadKeyState(NYA_PAD_KEY_LSTICK_Y) / -32767.0;

		// basic keyboard controls
		if (TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_RACING) {
			if (IsKeyPressed(VK_LEFT)) {
				marioInputs.stickX = -1.0;
			}
			if (IsKeyPressed(VK_RIGHT)) {
				marioInputs.stickX = 1.0;
			}
			if (IsKeyPressed(VK_UP)) {
				marioInputs.stickY = -1.0;
			}
			if (IsKeyPressed(VK_DOWN)) {
				marioInputs.stickY = 1.0;
			}
			if (IsKeyPressed(VK_SPACE)) {
				marioInputs.buttonA = 1;
			}
			if (IsKeyPressed(VK_CONTROL)) {
				marioInputs.buttonZ = 1;
			}
			if (IsKeyPressed(VK_LBUTTON) || IsKeyPressed(VK_SHIFT)) {
				marioInputs.buttonB = 1;
			}
		}

		auto stick = NyaVec3(marioInputs.stickX, marioInputs.stickY, 0);
		if (stick.length() > 1.0) {
			stick.Normalize();
			marioInputs.stickX = stick.x;
			marioInputs.stickY = stick.y;
		}

		if (!FEManager::mPauseRequest) {
			MarioObjectInteractions();
			MarioCustomObjectInteractions();
			MarioCustomPhysicsObjectInteractions();
			MarioCarInheritance();

			sm64_set_sound_volume(GetSFXVolume());

			static CNyaTimer gTimer;
			gTimer.Process();

			fTimeSinceLastAttacked += gTimer.fDeltaTime;
			if (fTimeSinceLastAttacked > 2.0) {
				sm64_set_mario_health(marioId, 0x880);
			}

			while (gTimer.fTotalTime >= 1.f/30)
			{
				memcpy(lastPos, currPos, sizeof(currPos));
				memcpy(lastGeoPos, currGeoPos, sizeof(currGeoPos));

				gTimer.fTotalTime -= 1.f/30;
				sm64_mario_tick( marioId, &marioInputs, &marioState, &marioGeometry );

				memcpy(currPos, marioState.position, sizeof(currPos));
				memcpy(currGeoPos, marioGeometry.position, sizeof(currGeoPos));
			}

			for (int i=0; i<3; i++) marioState.position[i] = std::lerp(lastPos[i], currPos[i], gTimer.fTotalTime / (1.f/30));
			for (int i=0; i<marioGeometry.numTrianglesUsed*9; i++) marioGeometry.position[i] = std::lerp(lastGeoPos[i], currGeoPos[i], gTimer.fTotalTime / (1.f/30));
		}

		static CNyaTimer gInvincibilityTimer;
		gInvincibilityTimer.Process();
		while (gInvincibilityTimer.fTotalTime > 1.0 / 30.0) {
			gInvincibilityTimer.fTotalTime -= 1.0 / 30.0;
			bInvincibleFlash = !bInvincibleFlash;
		}

		static bool bOnce = true;
		if (bOnce) {
			aDrawing3DLoopFunctionsOnce.push_back(InitAudio);
			bOnce = false;
		}
	}

	void OnTick3D() {
		PerformanceBenchmarker _perf("SM64::OnTick3D");

		if (!bEnabled) return;
		if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_RACING && TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_IN_FRONTEND) return;
		if (IsInLoadingScreen() || IsInMovie()) return;

		RenderMario<0, false>(marioGeometry);
		RenderMario<0, true>(marioGeometry);
	}

	void OnAudioTick() {
		int numDesiredSamples = 1100;
		int sampleRate = 32000;

		auto audioStream = BASS_StreamCreate(sampleRate, 2, 0, STREAMPROC_PUSH, nullptr);

		while (true) {
			int16_t audioBuffer[numDesiredSamples*2]; // ??????????
			uint32_t numSamples = sm64_audio_tick(0, numDesiredSamples, audioBuffer);
			BASS_StreamPutData(audioStream, audioBuffer, numSamples * 8);
			BASS_ChannelPlay(audioStream, false);

			// busy wait seems to be the only reasonable way to make this accurate
			// 29.0 is too slow and causes crackling, 30.0 is too fast and adds delay
			auto start = std::chrono::high_resolution_clock::now();
			while ((std::chrono::high_resolution_clock::now() - start).count() / 1e9 < (1.0 / 29.15)) {}
		}
	}

	void InitAudio() {
		WriteLog("InitAudio");
		std::thread(OnAudioTick).detach();
	}

	ChloeHook Init([](){
		DLLDirSetter _setdir;

		// init mario
		size_t romSize;

		uint8_t *rom = utils_read_file_alloc( "baserom.us.z64", &romSize );

		if(rom == NULL) {
			//MessageBoxA(nullptr, "Failed to read ROM file \"baserom.us.z64\"", "nya?!~", MB_ICONERROR);
			return;
		}

		uint8_t *texture = (uint8_t*)malloc( 4 * SM64_TEXTURE_WIDTH * SM64_TEXTURE_HEIGHT );

		sm64_global_terminate();
		sm64_global_init(rom, texture);
		sm64_audio_init(rom);

		ResetMario({0,0,0});

		free( rom );

		//audio_init();

		//sm64_play_music(0, 0x05 | 0x80, 0); // from decomp/include/seq_ids.h: SEQ_LEVEL_WATER | SEQ_VARIATION
		//sm64_play_music(0, 0x03, 0);

		marioGeometry.position = new float[9 * SM64_GEO_MAX_TRIANGLES];
		marioGeometry.color    = new float[9 * SM64_GEO_MAX_TRIANGLES];
		marioGeometry.normal   = new float[9 * SM64_GEO_MAX_TRIANGLES];
		marioGeometry.uv       = new float[6 * SM64_GEO_MAX_TRIANGLES];
		marioGeometry.numTrianglesUsed = 0;

		memset(marioGeometry.position, 0, sizeof(float)*9*SM64_GEO_MAX_TRIANGLES);
		memset(marioGeometry.color, 0, sizeof(float)*9*SM64_GEO_MAX_TRIANGLES);
		memset(marioGeometry.normal, 0, sizeof(float)*9*SM64_GEO_MAX_TRIANGLES);
		memset(marioGeometry.uv, 0, sizeof(float)*6*SM64_GEO_MAX_TRIANGLES);

		aDrawing3DLoopFunctions.push_back(OnTick3D);
		aDrawingLoopFunctions.push_back(OnTick);
		//aDrawing3DLoopFunctionsOnce.push_back(InitAudio);

		bAvailable = true;
	});

	void OnTakeDamage(int damage, NyaVec3 pos, bool heavyDamage) {
		if (!bEnabled) return;

		fTimeSinceLastAttacked = 0;

		//sm64_set_mario_action_arg(SM64::marioId, ACT_BURNING_JUMP, 1);

		NyaVec3 mario = SM64::WorldToMario(pos);
		sm64_mario_take_damage(SM64::marioId, 1, heavyDamage ? 8 : 0, mario.x, mario.y, mario.z);

		// doesnt work
		//if (heavyDamage) {
		//	sm64_set_mario_forward_velocity(SM64::marioId, 150);
		//}
	}

	void TakeLavaDamage() {
		if (!bEnabled) return;

		sm64_set_mario_action_arg(SM64::marioId, ACT_LAVA_BOOST, 1);
	}
}