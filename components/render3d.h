namespace Render3D {
//#define RENDER3D_NOEFFECT
	const uint32_t nDefaultVertexColor = 0xFF404040;
	uint32_t nVertexColorValue = nDefaultVertexColor;
	std::string sTextureSubdir;

	struct CwoeeVertexData {
		float vPos[3];
		float vNormals[3];
		uint32_t Color;
		float vUV[2];
		float vTangents[3];
	};

	struct tTextureInfo {
		std::string sFile;
		IDirect3DTexture9* pTexture;
	};

	struct tModel {
		IDirect3DIndexBuffer9* pIndexBuffer = nullptr;
		IDirect3DVertexBuffer9* pVertexBuffer = nullptr;
		IDirect3DTexture9* pTexture = nullptr;
		std::string sTextureName;
		uint32_t nVertexCount;
		uint32_t nFaceCount;
		bool bInvalidated = false;

		void RenderAt(NyaMat4x4 matrix, bool useAlpha = false, int effectId = EEFFECT_WORLD) const {
			if (bInvalidated) return;

#ifdef RENDER3D_NOEFFECT
			g_pd3dDevice->SetPixelShader(nullptr);
			g_pd3dDevice->SetVertexShader(nullptr);

			auto view = eViews[EVIEW_PLAYER1].PlatInfo->ViewMatrix;
			auto proj = eViews[EVIEW_PLAYER1].PlatInfo->ProjectionMatrix;
			g_pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&view);
			g_pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&proj);
#else

			eEffectStaticState::pCurrentEffect = eEffects[effectId];
			auto effect = eEffectStaticState::pCurrentEffect;

			effect->Start();

			g_pd3dDevice->SetVertexDeclaration(effect->VertexDecl);

			// this is kinda nasty but this function will refuse to update the matrix if its pointer is identical to the previous caller's
			static UMath::Matrix4 matrixTemp[2];
			static bool matrixTempSecond = false;
			UMath::Matrix4* pMatrix = matrixTempSecond ? &matrixTemp[1] : &matrixTemp[0];
			*pMatrix = (UMath::Matrix4)matrix;
			matrixTempSecond = !matrixTempSecond;
			ParticleSetTransform(pMatrix, EVIEW_PLAYER1);

			static D3DXHANDLE TextureOffset = effect->hD3DXEffect->GetParameterByName(0, "TextureOffset");
			static D3DXHANDLE TextureOffset2 = effect->hD3DXEffect->GetParameterByName(0, "TEXTUREOFFSET");
			D3DXVECTOR4 textureOffset = {0,0,0,0};
			if (TextureOffset) effect->hD3DXEffect->SetVector(TextureOffset, &textureOffset);
			if (TextureOffset2) effect->hD3DXEffect->SetVector(TextureOffset2, &textureOffset);

			static D3DXHANDLE BLENDSTATE = effect->hD3DXEffect->GetParameterByName(0, "Blend_State");
			static D3DXHANDLE BLENDSTATE2 = effect->hD3DXEffect->GetParameterByName(0, "BLENDSTATE");
			int blendStates[] = {
					0, // alphatestenable
					0, // alpharef
					useAlpha, // alphablendenable
					D3DBLEND_SRCALPHA, // srcblend
					D3DBLEND_INVSRCALPHA, // destblend
			};
			if (BLENDSTATE) effect->hD3DXEffect->SetIntArray(BLENDSTATE, blendStates, 5);
			if (BLENDSTATE2) effect->hD3DXEffect->SetIntArray(BLENDSTATE2, blendStates, 5);

			// desperate attempts to make stuff not carry over from the last drawn car (which is almost always traffic)
			// none of this worked
			/*if (effectId == EEFFECT_CAR) {
				static D3DXHANDLE SpecularPower = effect->hD3DXEffect->GetParameterByName(0, "SpecularPower");
				if (SpecularPower) {
					effect->hD3DXEffect->SetFloat(SpecularPower, 2.5);
				}
				static D3DXHANDLE EnvmapPower = effect->hD3DXEffect->GetParameterByName(0, "EnvmapPower");
				if (EnvmapPower) {
					effect->hD3DXEffect->SetFloat(EnvmapPower, 1.0);
				}
				static D3DXHANDLE SpecularHotSpot = effect->hD3DXEffect->GetParameterByName(0, "SpecularHotSpot");
				if (SpecularHotSpot) {
					effect->hD3DXEffect->SetFloat(SpecularHotSpot, 1.0);
				}
				static D3DXHANDLE Desaturation = effect->hD3DXEffect->GetParameterByName(0, "Desaturation");
				if (Desaturation) {
					effect->hD3DXEffect->SetFloat(Desaturation, 0.0);
				}
				static D3DXHANDLE DiffuseMin = effect->hD3DXEffect->GetParameterByName(0, "DiffuseMin");
				if (DiffuseMin) {
					D3DXVECTOR4 v = {1,1,1,1};
					effect->hD3DXEffect->SetVector(DiffuseMin, &v);
				}
				static D3DXHANDLE DiffuseRange = effect->hD3DXEffect->GetParameterByName(0, "DiffuseRange");
				if (DiffuseRange) {
					D3DXVECTOR4 v = {0,0,0,-0.65};
					effect->hD3DXEffect->SetVector(DiffuseRange, &v);
				}
				static D3DXHANDLE SpecularMin = effect->hD3DXEffect->GetParameterByName(0, "SpecularMin");
				if (SpecularMin) {
					D3DXVECTOR4 v = {0,0,0,0};
					effect->hD3DXEffect->SetVector(SpecularMin, &v);
				}
				static D3DXHANDLE SpecularRange = effect->hD3DXEffect->GetParameterByName(0, "SpecularRange");
				if (SpecularRange) {
					D3DXVECTOR4 v = {0,0,0,0};
					effect->hD3DXEffect->SetVector(SpecularRange, &v);
				}
				static D3DXHANDLE g_bDoCarShadowMap = effect->hD3DXEffect->GetParameterByName(0, "g_bDoCarShadowMap");
				if (g_bDoCarShadowMap) {
					effect->hD3DXEffect->SetInt(g_bDoCarShadowMap, 1);
				}
			}*/

			effect->hD3DXEffect->Begin(nullptr, 0);
			effect->hD3DXEffect->BeginPass(0);
#endif

			g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
			g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
			g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

			g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			//g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
			//g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			//g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

			g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
			g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
			g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);

#ifdef RENDER3D_NOEFFECT
			g_pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&matrix);
			g_pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1);
#endif
			g_pd3dDevice->SetStreamSource(0, pVertexBuffer, 0, sizeof(CwoeeVertexData));
			g_pd3dDevice->SetIndices(pIndexBuffer);
			g_pd3dDevice->SetTexture(0, pTexture);
			g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, nVertexCount, 0, nFaceCount);

#ifndef RENDER3D_NOEFFECT
			effect->hD3DXEffect->EndPass();
			effect->hD3DXEffect->End();

			effect->End();
#endif
		}
	};
	std::vector<tModel*> aAllModels;
	std::vector<tTextureInfo> aAllTextures;

	std::string GetMaterialFilename(aiMaterial* material) {
		aiString tmp;
		material->GetTexture(aiTextureType_DIFFUSE, 0, &tmp);
		std::string str = tmp.C_Str();
		while (str.find('\\') != std::string::npos) {
			str.erase(str.begin());
		}
		while (str.find('/') != std::string::npos) {
			str.erase(str.begin());
		}
		return str;
	}

	tModel* CreateOneModel(const aiMesh* mesh, aiMaterial* material) {
		auto model = new tModel;

		size_t vertexTotalSize = mesh->mNumVertices * sizeof(CwoeeVertexData);
		size_t indexTotalSize = mesh->mNumFaces * 3 * 4;
		auto hr = g_pd3dDevice->CreateVertexBuffer(vertexTotalSize, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1, D3DPOOL_DEFAULT, &model->pVertexBuffer, nullptr);
		if (hr != D3D_OK) {
			delete model;
			return nullptr;
		}
		hr = g_pd3dDevice->CreateIndexBuffer(indexTotalSize, D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &model->pIndexBuffer, nullptr);
		if (hr != D3D_OK) {
			model->pVertexBuffer->Release();
			delete model;
			return nullptr;
		}

		CwoeeVertexData* vertices = nullptr;
		int* indices = nullptr;
		hr = model->pVertexBuffer->Lock(0, vertexTotalSize, (void**)&vertices, D3DLOCK_DISCARD);
		if (hr != D3D_OK) {
			model->pVertexBuffer->Release();
			model->pIndexBuffer->Release();
			delete model;
			return nullptr;
		}
		hr = model->pIndexBuffer->Lock(0, indexTotalSize, (void**)&indices, D3DLOCK_DISCARD);
		if (hr != D3D_OK) {
			model->pVertexBuffer->Release();
			model->pIndexBuffer->Release();
			delete model;
			return nullptr;
		}

		for (int i = 0; i < mesh->mNumVertices; i++) {
			auto src = &mesh->mVertices[i];
			auto srcNormal = &mesh->mNormals[i];
			auto srcTangent = &mesh->mTangents[i];
			auto srcUV = &mesh->mTextureCoords[0][i];
			auto dest = &vertices[i];
			dest->vPos[0] = src->x;
			dest->vPos[1] = src->y;
			dest->vPos[2] = src->z;
			if (mesh->mNormals) {
				dest->vNormals[0] = srcNormal->x;
				dest->vNormals[1] = srcNormal->y;
				dest->vNormals[2] = srcNormal->z;
			}
			if (mesh->mTangents) {
				dest->vTangents[0] = srcTangent->x;
				dest->vTangents[1] = srcTangent->y;
				dest->vTangents[2] = srcTangent->z;
			}
			dest->Color = nVertexColorValue;
			if (mesh->mTextureCoords[0]) {
				dest->vUV[0] = srcUV->x;
				dest->vUV[1] = srcUV->y * -1;
			}
		}

		int indexCount = 0;
		for (int i = 0; i < mesh->mNumFaces; i++) {
			auto src = mesh->mFaces[i];
			indices[indexCount++] = src.mIndices[0];
			indices[indexCount++] = src.mIndices[1];
			indices[indexCount++] = src.mIndices[2];
		}
		model->nVertexCount = mesh->mNumVertices;
		model->nFaceCount = mesh->mNumFaces;

		model->pVertexBuffer->Unlock();
		model->pIndexBuffer->Unlock();

		auto textureName = sTextureSubdir + GetMaterialFilename(material);
		model->sTextureName = textureName;

		for (auto& texture : aAllTextures) {
			if (texture.sFile == textureName) {
				model->pTexture = texture.pTexture;
				break;
			}
		}
		if (!model->pTexture) {
			if (auto tex = LoadTexture(std::format("CwoeeChaos/data/models/{}", textureName).c_str())) {
				model->pTexture = tex;
				aAllTextures.push_back({textureName, model->pTexture});
			} else {
				MessageBoxA(nullptr, std::format("Failed to load texture {}", textureName).c_str(), "nya?!~", MB_ICONERROR);
			}
		}

		aAllModels.push_back(model);
		return model;
	}

	std::vector<tModel*> CreateModels(const std::string& path) {
		auto fullPath = std::format("CwoeeChaos/data/models/{}", path);
		WriteLog(std::format("loading file {}", fullPath));

		std::vector<tModel*> models;

		Assimp::Logger::LogSeverity severity = Assimp::Logger::VERBOSE;
		Assimp::DefaultLogger::create("model_import_log.txt", severity, aiDefaultLogStream_FILE);

		uint32_t flags = 0;
		flags |= aiProcess_CalcTangentSpace;
		flags |= aiProcess_GenSmoothNormals;
		flags |= aiProcess_JoinIdenticalVertices;
		flags |= aiProcess_RemoveRedundantMaterials;
		flags |= aiProcess_Triangulate;
		flags |= aiProcess_GenUVCoords;
		flags |= aiProcess_GenBoundingBoxes;

		static Assimp::Importer importer;
		if (auto scene = importer.ReadFile(fullPath.c_str(), flags)) {
			for (int i = 0; i < scene->mNumMeshes; i++) {
				auto mesh = scene->mMeshes[i];
				auto model = CreateOneModel(mesh, scene->mMaterials[mesh->mMaterialIndex]);
				if (!model) continue;
				models.push_back(model);
			}
		}
		return models;
	}

	void OnD3DReset() {
		for (auto& model : aAllModels) {
			if (model->bInvalidated) continue;
			model->pVertexBuffer->Release();
			model->pIndexBuffer->Release();
			model->pVertexBuffer = nullptr;
			model->pIndexBuffer = nullptr;
			if (model->pTexture) {
				model->pTexture->Release();
				model->pTexture = nullptr;
			}
			model->bInvalidated = true;
		}
	}

	ChloeHook Init([](){
		NyaHooks::aD3DResetFuncs.push_back(OnD3DReset);
	});
}