namespace SM64 {
//#define RENDER_NFS_COLLISIONS

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

	int marioLightness = 128;

	template<int bufId, bool textured>
	void RenderMario(SM64MarioGeometryBuffers marioBuffers) {
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

			auto tmpNormal = MarioToWorld_Render({srcNormal[0], srcNormal[1], srcNormal[2]});

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
				tmp.b = marioLightness;
				tmp.g = marioLightness;
				tmp.r = marioLightness;
			}
			else {
				tmp.b = srcColor[0] * marioLightness;
				tmp.g = srcColor[1] * marioLightness;
				tmp.r = srcColor[2] * marioLightness;
			}
			tmp.a = 255;
			dest->Color = *(uint32_t*)&tmp;

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

		auto mat = NyaMat4x4();
		mat.SetIdentity();
		if (textured) {
			static auto marioTextured = LoadTexture("CwoeeChaos/data/models/letsago.png");
			tmpModel.pTexture = marioTextured;
			tmpModel.RenderAt(WorldToRenderMatrix(mat), true);
		}
		else {
			static auto marioColored = LoadTexture("CwoeeChaos/data/models/white.png");
			tmpModel.pTexture = marioColored;
			tmpModel.RenderAt(WorldToRenderMatrix(mat), false);
		}
	}

	std::vector<WCollisionTri> aCollisionTris;
	std::vector<int> aCollisionTriMarios;

	void ProcessCollisionArticle(WCollisionInstance* inst) {
		if (!inst) return;

		auto article = inst->fCollisionArticle;
		if (!article) return;

		UMath::Matrix4 instMat;
		inst->MakeMatrix(&instMat, true);

		auto stripSphere = (WCollisionStripSphere*)(&article[1]);
		auto strip = (WCollisionStrip*)(&stripSphere[article->fNumStrips]);
		for (int i = 0; i < article->fNumStrips; i++) {
			int numToIterate = strip->numTrisOrSurfaceId - 2;
			for (int j = 0; j < numToIterate; j++) {
				WCollisionTri tri;
				WCollisionStrip::MakeFace(strip, j, &stripSphere->fPos, &tri);

				tri.fPt0 -= instMat.p;
				tri.fPt1 -= instMat.p;
				tri.fPt2 -= instMat.p;

				// these tri strips seem to be double sided
				//if (j % 2 == 0) {
				//	auto tmp = tri.fPt0;
				//	tri.fPt0 = tri.fPt2;
				//	tri.fPt2 = tmp;
				//}

				//tri.fPt0 = instMat * tri.fPt0;
				//tri.fPt1 = instMat * tri.fPt1;
				//tri.fPt2 = instMat * tri.fPt2;
				//tri.fPt0 *= -1;
				//tri.fPt1 *= -1;
				//tri.fPt2 *= -1;

				aCollisionTris.push_back(tri);
			}
			strip += strip->numTrisOrSurfaceId;
			stripSphere++;
		}
	}

	void ClearMarioCollision() {
		for (auto& id : aCollisionTriMarios) {
			sm64_surface_object_delete(id);
		}
		aCollisionTriMarios.clear();
	}

	void UpdateMarioCollision() {
		if (aCollisionTris.empty()) return; // always keep old collision if empty

		for (auto& id : aCollisionTriMarios) {
			sm64_surface_object_delete(id);
		}
		aCollisionTriMarios.clear();

		SM64SurfaceObject obj;
		obj.surfaces = new SM64Surface[aCollisionTris.size()];
		obj.surfaceCount = aCollisionTris.size();
		obj.transform.position[0] = 0;
		obj.transform.position[1] = 0;
		obj.transform.position[2] = 0;
		obj.transform.eulerRotation[0] = 0;
		obj.transform.eulerRotation[1] = 0;
		obj.transform.eulerRotation[2] = 0;

		auto objFlip = obj;
		objFlip.surfaces = new SM64Surface[aCollisionTris.size()];

#ifdef RENDER_NFS_COLLISIONS
		static SM64MarioGeometryBuffers colBuffers = {};
		static SM64MarioGeometryBuffers colBuffersFlip = {};
		if (!colBuffers.position) {
			colBuffers.position = new float[9 * SM64_GEO_MAX_TRIANGLES];
			colBuffers.normal = colBuffersFlip.normal = new float[9 * SM64_GEO_MAX_TRIANGLES];
			colBuffers.color = colBuffersFlip.color = new float[9 * SM64_GEO_MAX_TRIANGLES];
			colBuffers.uv = colBuffersFlip.uv = new float[6 * SM64_GEO_MAX_TRIANGLES];
			colBuffersFlip.position = new float[9 * SM64_GEO_MAX_TRIANGLES];
		}
		colBuffers.numTrianglesUsed = colBuffersFlip.numTrianglesUsed = std::min((int)aCollisionTris.size(), (int)SM64_GEO_MAX_TRIANGLES);

		auto colDrawPosition = &colBuffers.position[0];
		auto colDrawNormal = &colBuffers.normal[0];
		auto colDrawColor = &colBuffers.color[0];
		auto colDrawUV = &colBuffers.uv[0];

		auto colDrawFlipPosition = &colBuffersFlip.position[0];
#endif

		for (int i = 0; i < aCollisionTris.size(); i++) {
			auto in = &aCollisionTris[i];
			auto out = &obj.surfaces[i];
			auto out2 = &objFlip.surfaces[i];

			out->type = SURFACE_DEFAULT;
			out->force = 0;
			out->terrain = TERRAIN_GRASS;

			auto pt0 = WorldToMario({in->fPt0[0],in->fPt0[1],in->fPt0[2]});
			auto pt1 = WorldToMario({in->fPt1[0],in->fPt1[1],in->fPt1[2]});
			auto pt2 = WorldToMario({in->fPt2[0],in->fPt2[1],in->fPt2[2]});

			out->vertices[0][0] = pt2[0];
			out->vertices[0][1] = pt2[1];
			out->vertices[0][2] = pt2[2];
			out->vertices[1][0] = pt1[0];
			out->vertices[1][1] = pt1[1];
			out->vertices[1][2] = pt1[2];
			out->vertices[2][0] = pt0[0];
			out->vertices[2][1] = pt0[1];
			out->vertices[2][2] = pt0[2];

			*out2 = *out;
			out2->vertices[0][0] = pt0[0];
			out2->vertices[0][1] = pt0[1];
			out2->vertices[0][2] = pt0[2];
			out2->vertices[1][0] = pt1[0];
			out2->vertices[1][1] = pt1[1];
			out2->vertices[1][2] = pt1[2];
			out2->vertices[2][0] = pt2[0];
			out2->vertices[2][1] = pt2[1];
			out2->vertices[2][2] = pt2[2];

#ifdef RENDER_NFS_COLLISIONS
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

				colDrawNormal[0] = 0;
				colDrawNormal[1] = 1;
				colDrawNormal[2] = 0;
				colDrawNormal += 3;
				colDrawNormal[0] = 0;
				colDrawNormal[1] = 1;
				colDrawNormal[2] = 0;
				colDrawNormal += 3;
				colDrawNormal[0] = 0;
				colDrawNormal[1] = 1;
				colDrawNormal[2] = 0;
				colDrawNormal += 3;

				colDrawColor[0] = 0;
				colDrawColor[1] = 1;
				colDrawColor[2] = 0;
				colDrawColor += 3;
				colDrawColor[0] = 0;
				colDrawColor[1] = 1;
				colDrawColor[2] = 0;
				colDrawColor += 3;
				colDrawColor[0] = 0;
				colDrawColor[1] = 1;
				colDrawColor[2] = 0;
				colDrawColor += 3;

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
#endif
		}

		auto id = sm64_surface_object_create(&obj);
		if (id != -1) {
			aCollisionTriMarios.push_back(id);
		}

		auto id2 = sm64_surface_object_create(&objFlip);
		if (id2 != -1) {
			aCollisionTriMarios.push_back(id2);
		}

#ifdef RENDER_NFS_COLLISIONS
		RenderMario<1, false>(colBuffers);
		RenderMario<2, false>(colBuffersFlip);
#endif
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

	void OnTick() {
		if (!bEnabled) {
			bDoReset = true;
			return;
		}
		if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_RACING && TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_IN_FRONTEND) {
			bDoReset = true;
			return;
		}
		if (IsInLoadingScreen()) {
			bDoReset = true;
			return;
		}

		if (auto ply = GetLocalPlayerInterface<IRigidBody>()) {
			static CNyaTimer gCollisionTimer;
			gCollisionTimer.Process();

			if (gCollisionTimer.fTotalTime >= 0.25) {
				gCollisionTimer.fTotalTime -= 0.25;

				aCollisionTris.clear();

				auto col = (WCollider*)ply->GetWCollider();
				for (int i = 0; i < col->fInstanceCacheList.size(); i++) {
					auto inst = col->fInstanceCacheList[i];
					ProcessCollisionArticle(inst);
				}

				UpdateMarioCollision();
			}

			UMath::Vector3 marioPos = MarioToWorld({marioState.position[0], marioState.position[1], marioState.position[2]});
			UMath::Vector3 marioVel = MarioToWorld({marioState.velocity[0], marioState.velocity[1], marioState.velocity[2]}) / (1.0 / 30.0);
			if (marioPos.length() > 50) {
				marioPos.y += 1;
				ply->SetPosition(&marioPos);

				UMath::Vector4 q = {0,0,0,1};
				//ply->SetOrientation(&q);

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

		if (!FEManager::mPauseRequest) {
			static CNyaTimer gTimer;
			gTimer.Process();

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

		RenderMario<0, false>(marioGeometry);
		RenderMario<0, true>(marioGeometry);

		static bool bOnce = true;
		if (bOnce) {
			aDrawing3DLoopFunctionsOnce.push_back(InitAudio);
			bOnce = false;
		}
	}

	void OnAudioTick() {
		int numDesiredSamples = 3200;
		int sampleRate = 32000;

		static CNyaTimer gTimer;
		gTimer.Process();

		auto audioStream = BASS_StreamCreate(sampleRate, 2, 0, STREAMPROC_PUSH, nullptr);

		double currentTime = gTimer.fTotalTime;
		double targetTime = 0;
		while (true) {
			gTimer.Process();

			int16_t audioBuffer[numDesiredSamples*2]; // ??????????
			uint32_t numSamples = sm64_audio_tick(0, numDesiredSamples, audioBuffer);
			//BASS_SampleSetData(sample, audioBuffer); // set the sample's data

			BASS_StreamPutData(audioStream, audioBuffer, numSamples*8);
			BASS_ChannelPlay(audioStream, false);

			targetTime = currentTime + (1.0 / 30.0);
			while (gTimer.fTotalTime < targetTime) {
				Sleep(0);
				gTimer.Process();
			}
			currentTime = gTimer.fTotalTime;
		}
	}

	void InitAudio() {
		WriteLog("InitAudio");
		std::thread(OnAudioTick).detach();
	}

	ChloeHook Init([](){
		aDrawing3DLoopFunctions.push_back(OnTick);
		//aDrawing3DLoopFunctionsOnce.push_back(InitAudio);

		// init mario
		size_t romSize;

		uint8_t *rom = utils_read_file_alloc( "baserom.us.z64", &romSize );

		if(rom == NULL) {
			MessageBoxA(nullptr, "Failed to read ROM file \"baserom.us.z64\"", "nya?!~", MB_ICONERROR);
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

		bAvailable = true;
	});
}