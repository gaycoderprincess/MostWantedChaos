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

	bool bForceNoEffect = false;

	//D3DXVECTOR4 fDIFFUSEMIN = {1,1,1,1};
	//D3DXVECTOR4 fDIFFUSERANGE = {1,1,1,-0.65};
	//D3DXVECTOR4 fSPECULARMIN = {0,0,0,0};
	//D3DXVECTOR4 fSPECULARRANGE = {0.3,0.3,0.3,0};
	//D3DXVECTOR4 fENVMAPMIN = {0,0,0,0};
	//D3DXVECTOR4 fENVMAPANGE = {1,1,1,0};
	//float fSPECULARPOWER = 2.5;
	//float fENVMAPPOWER = 1.0;

	D3DXVECTOR4 fDIFFUSEMIN = {0.4,0.4,0.4,1};
	D3DXVECTOR4 fDIFFUSERANGE = {0.6,0.6,0.6,0};
	D3DXVECTOR4 fSPECULARMIN = {0.16,0.2,0.16,0};
	D3DXVECTOR4 fSPECULARRANGE = {0.04,0.0,0.04,0};
	D3DXVECTOR4 fENVMAPMIN = {1.75,1.75,1.75,0};
	D3DXVECTOR4 fENVMAPANGE = {-1.65,-1.65,-1.65,0};
	float fSPECULARPOWER = 8.0;
	float fENVMAPPOWER = 0.15;

	struct tModel {
		IDirect3DIndexBuffer9* pIndexBuffer = nullptr;
		IDirect3DVertexBuffer9* pVertexBuffer = nullptr;
		IDirect3DTexture9* pTexture = nullptr;
		std::string sTextureName;
		uint32_t nVertexCount;
		uint32_t nFaceCount;
		bool bInvalidated = false;

		// for collision checks
		std::vector<NyaVec3> aVertices;
		std::vector<int> aIndices;

		void RenderAt(NyaMat4x4 matrix, bool useAlpha = false, int effectId = EEFFECT_WORLD, bool zwrite = true) const {
			if (bInvalidated) return;

			if (bForceNoEffect) {
				return RenderAt_NoEffect(matrix, useAlpha, zwrite);
			}

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

			//static bool bOnce = true;
			//if (bOnce) {
			//	for (int i = 0; i < CParamHashTable::NUM_SHADER_PARAM; i++) {
			//		WriteLog(std::format("{}: {}", i, effect->mParamTable->mParamMappingTable[i].mName));
			//	}
			//	bOnce = false;
			//}

			g_pd3dDevice->SetVertexDeclaration(effect->VertexDecl);

			// this is kinda nasty but this function will refuse to update the matrix if its pointer is identical to the previous caller's
			static UMath::Matrix4 matrixTemp[2];
			static bool matrixTempSecond = false;
			UMath::Matrix4* pMatrix = matrixTempSecond ? &matrixTemp[1] : &matrixTemp[0];
			*pMatrix = (UMath::Matrix4)matrix;
			matrixTempSecond = !matrixTempSecond;
			ParticleSetTransform(pMatrix, EVIEW_PLAYER1);

			D3DXVECTOR4 textureOffset = {0,0,0,0};
			effect->hD3DXEffect->SetMatrix(effect->mParamTable->mParamMappingTable[CParamHashTable::TEXTUREOFFSETMATRIX].mHandle, (D3DXMATRIX*)&UMath::Matrix4::kIdentity);
			effect->hD3DXEffect->SetVector(effect->mParamTable->mParamMappingTable[CParamHashTable::TEXTUREOFFSET].mHandle, &textureOffset);

			// desperate attempts to make stuff not carry over from the last drawn car (which is almost always traffic)
			if (effectId == EEFFECT_CAR) {
				// for all effects, game sets:
				// DIFFUSEMIN vector
				// DIFFUSERANGE vector

				// if car, the game sets:
				// SPECULARMIN vector
				// SPECULARRANGE vector
				// ENVMAPMIN vector
				// ENVMAPANGE vector
				// SPECULARPOWER float
				// ENVMAPPOWER float

				effect->hD3DXEffect->SetFloat(effect->mParamTable->mParamMappingTable[CParamHashTable::SPECULARPOWER].mHandle, fSPECULARPOWER);
				effect->hD3DXEffect->SetFloat(effect->mParamTable->mParamMappingTable[CParamHashTable::ENVMAPPOWER].mHandle, fENVMAPPOWER);

				D3DXVECTOR4 v = fDIFFUSEMIN;
				effect->hD3DXEffect->SetVector(effect->mParamTable->mParamMappingTable[CParamHashTable::DIFFUSEMIN].mHandle, &v);
				v = fDIFFUSERANGE;
				effect->hD3DXEffect->SetVector(effect->mParamTable->mParamMappingTable[CParamHashTable::DIFFUSERANGE].mHandle, &v);
				v = fSPECULARMIN;
				effect->hD3DXEffect->SetVector(effect->mParamTable->mParamMappingTable[CParamHashTable::SPECULARMIN].mHandle, &v);
				v = fSPECULARRANGE;
				effect->hD3DXEffect->SetVector(effect->mParamTable->mParamMappingTable[CParamHashTable::SPECULARRANGE].mHandle, &v);

				v = fENVMAPMIN;
				effect->hD3DXEffect->SetVector(effect->mParamTable->mParamMappingTable[CParamHashTable::ENVMAPMIN].mHandle, &v);
				v = fENVMAPANGE;
				effect->hD3DXEffect->SetVector(effect->mParamTable->mParamMappingTable[CParamHashTable::ENVMAPANGE].mHandle, &v);

				//static D3DXHANDLE SpecularHotSpot = effect->hD3DXEffect->GetParameterByName(0, "SpecularHotSpot");
				//if (SpecularHotSpot) {
				//	effect->hD3DXEffect->SetFloat(SpecularHotSpot, 1.0);
				//}
				//static D3DXHANDLE Desaturation = effect->hD3DXEffect->GetParameterByName(0, "Desaturation");
				//if (Desaturation) {
				//	effect->hD3DXEffect->SetFloat(Desaturation, 0.0);
				//}
				//static D3DXHANDLE g_bDoCarShadowMap = effect->hD3DXEffect->GetParameterByName(0, "g_bDoCarShadowMap");
				//if (g_bDoCarShadowMap) {
				//	effect->hD3DXEffect->SetInt(g_bDoCarShadowMap, 1);
				//}
			}

			effect->hD3DXEffect->Begin(nullptr, 0);
			effect->hD3DXEffect->BeginPass(0);
#endif

			g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
			g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, zwrite);
			g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
			g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, 0);
			g_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0);
			g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, useAlpha);
			g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

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

		void RenderAt_NoEffect(NyaMat4x4 matrix, bool useAlpha = false, bool zwrite = true, bool useZ = true) const {
			if (bInvalidated) return;

			g_pd3dDevice->SetPixelShader(nullptr);
			g_pd3dDevice->SetVertexShader(nullptr);

			auto view = eViews[EVIEW_PLAYER1].PlatInfo->ViewMatrix;
			auto proj = eViews[EVIEW_PLAYER1].PlatInfo->ProjectionMatrix;
			g_pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&view);
			g_pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&proj);

			g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, useZ);
			g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, zwrite);
			g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
			g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, 0);
			g_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0);
			g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, useAlpha);
			g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

			g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
			g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			//g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
			//g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			//g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

			g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
			g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
			g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);

			g_pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&matrix);
			g_pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1);
			g_pd3dDevice->SetStreamSource(0, pVertexBuffer, 0, sizeof(CwoeeVertexData));
			g_pd3dDevice->SetIndices(pIndexBuffer);
			g_pd3dDevice->SetTexture(0, pTexture);
			g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, nVertexCount, 0, nFaceCount);
		}

		void Invalidate() {
			if (bInvalidated) return;
			pVertexBuffer->Release();
			pIndexBuffer->Release();
			pVertexBuffer = nullptr;
			pIndexBuffer = nullptr;
			if (pTexture) {
				pTexture->Release();
				pTexture = nullptr;
			}
			aVertices.clear();
			aVertices.shrink_to_fit();
			aIndices.clear();
			aIndices.shrink_to_fit();
			bInvalidated = true;
		}
	};
	std::vector<tModel*> aAllModels;
	std::vector<tTextureInfo> aAllTextures;

	tModel* CreateOneModel(int numVertices, int numFaces, const NyaVec3* vertices, const NyaVec3* normals, const NyaVec3* tangents, const NyaVec3* bitangents, const NyaVec3* uvs, const NyaDrawing::CNyaRGBA32* colors, const uint32_t* indices, const std::string& material) {
		auto model = new tModel;

		model->aVertices.reserve(numVertices);
		model->aIndices.reserve(numFaces*3);
		for (int i = 0; i < numVertices; i++) {
			model->aVertices.push_back(vertices[i]);
		}
		for (int i = 0; i < numFaces*3; i++) {
			model->aIndices.push_back(indices[i]);
		}

		size_t vertexTotalSize = numVertices * sizeof(CwoeeVertexData);
		size_t indexTotalSize = numFaces * 3 * 4;
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

		CwoeeVertexData* verticesOut = nullptr;
		int* indicesOut = nullptr;
		hr = model->pVertexBuffer->Lock(0, vertexTotalSize, (void**)&verticesOut, D3DLOCK_DISCARD);
		if (hr != D3D_OK) {
			model->pVertexBuffer->Release();
			model->pIndexBuffer->Release();
			delete model;
			return nullptr;
		}
		hr = model->pIndexBuffer->Lock(0, indexTotalSize, (void**)&indicesOut, D3DLOCK_DISCARD);
		if (hr != D3D_OK) {
			model->pVertexBuffer->Release();
			model->pIndexBuffer->Release();
			delete model;
			return nullptr;
		}

		bool overrideVertexColor = false;
		if (material.find("CARSKIN") != std::string::npos) overrideVertexColor = true;

		for (int i = 0; i < numVertices; i++) {
			auto src = &vertices[i];
			auto srcNormal = &normals[i];
			auto srcTangent = &tangents[i];
			auto srcUV = &uvs[i];
			auto dest = &verticesOut[i];
			dest->vPos[0] = src->x;
			dest->vPos[1] = src->y;
			dest->vPos[2] = src->z;
			if (normals) {
				dest->vNormals[0] = srcNormal->x;
				dest->vNormals[1] = srcNormal->y;
				dest->vNormals[2] = srcNormal->z;
			}
			if (tangents) {
				dest->vTangents[0] = srcTangent->x;
				dest->vTangents[1] = srcTangent->y;
				dest->vTangents[2] = srcTangent->z;
			}
			dest->Color = overrideVertexColor ? 0xFFFFFFFF : nVertexColorValue;
			if (uvs) {
				dest->vUV[0] = srcUV->x;
				dest->vUV[1] = srcUV->y * -1;
			}
		}

		memcpy(indicesOut, indices, indexTotalSize);

		model->nVertexCount = numVertices;
		model->nFaceCount = numFaces;

		model->pVertexBuffer->Unlock();
		model->pIndexBuffer->Unlock();

		auto baseTextureName = material;
		auto textureName = sTextureSubdir + baseTextureName;
		if (baseTextureName.empty()) {
			textureName = "white.png";
		}
		model->sTextureName = baseTextureName;

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
		auto fullPathCwo = std::format("CwoeeChaos/data/models/{}.cwo", path);
		if (!std::filesystem::exists(fullPathCwo)) {
			MessageBoxA(0, std::format("Failed to find model {}!", fullPathCwo).c_str(), "nya?!~", MB_ICONERROR);
			exit(0);
		}

		WriteLog(std::format("loading file {}", fullPathCwo));

		auto modelsCwo = ReadCwoeeModel(fullPathCwo);
		std::vector<tModel*> models;
		for (auto& mesh : modelsCwo) {
			auto model = CreateOneModel(mesh.nNumVertices, mesh.nNumFaces, mesh.aVertices, mesh.aNormals, mesh.aTangents, mesh.aBitangents, mesh.aUVs1, mesh.aColors, mesh.aIndices, mesh.sMaterialName);
			if (!model) continue;
			models.push_back(model);
			mesh.Destroy();
		}
		return models;
	}

	void OnD3DReset() {
		for (auto& model : aAllModels) {
			model->Invalidate();
		}
	}

	ChloeHook Init([](){
		NyaHooks::D3DResetHook::aFunctions.push_back(OnD3DReset);
	});
}

struct CarShaderParams {
	// SPECULARMIN vector
	// SPECULARRANGE vector
	// ENVMAPMIN vector
	// ENVMAPANGE vector
	// SPECULARPOWER float
	// ENVMAPPOWER float

	D3DXVECTOR4 DIFFUSEMIN;
	D3DXVECTOR4 DIFFUSERANGE;
	D3DXVECTOR4 SPECULARMIN;
	D3DXVECTOR4 SPECULARRANGE;
	D3DXVECTOR4 ENVMAPMIN;
	D3DXVECTOR4 ENVMAPANGE;
	float SPECULARPOWER;
	float ENVMAPPOWER;

	bool operator==(CarShaderParams& other) {
		if (DIFFUSEMIN != other.DIFFUSEMIN) return false;
		if (DIFFUSERANGE != other.DIFFUSERANGE) return false;
		if (SPECULARMIN != other.SPECULARMIN) return false;
		if (SPECULARRANGE != other.SPECULARRANGE) return false;
		if (ENVMAPMIN != other.ENVMAPMIN) return false;
		if (ENVMAPANGE != other.ENVMAPANGE) return false;
		if (SPECULARPOWER != other.SPECULARPOWER) return false;
		if (ENVMAPPOWER != other.ENVMAPPOWER) return false;
		return true;
	}

	void Collect() {
		auto effect = eEffectStaticState::pCurrentEffect;
		effect->hD3DXEffect->GetVector(effect->mParamTable->mParamMappingTable[CParamHashTable::DIFFUSEMIN].mHandle, &DIFFUSEMIN);
		effect->hD3DXEffect->GetVector(effect->mParamTable->mParamMappingTable[CParamHashTable::DIFFUSERANGE].mHandle, &DIFFUSERANGE);
		effect->hD3DXEffect->GetVector(effect->mParamTable->mParamMappingTable[CParamHashTable::SPECULARMIN].mHandle, &SPECULARMIN);
		effect->hD3DXEffect->GetVector(effect->mParamTable->mParamMappingTable[CParamHashTable::SPECULARRANGE].mHandle, &SPECULARRANGE);
		effect->hD3DXEffect->GetVector(effect->mParamTable->mParamMappingTable[CParamHashTable::ENVMAPMIN].mHandle, &ENVMAPMIN);
		effect->hD3DXEffect->GetVector(effect->mParamTable->mParamMappingTable[CParamHashTable::ENVMAPANGE].mHandle, &ENVMAPANGE);
		effect->hD3DXEffect->GetFloat(effect->mParamTable->mParamMappingTable[CParamHashTable::SPECULARPOWER].mHandle, &SPECULARPOWER);
		effect->hD3DXEffect->GetFloat(effect->mParamTable->mParamMappingTable[CParamHashTable::ENVMAPPOWER].mHandle, &ENVMAPPOWER);
	}

	void Log() {
		auto v = DIFFUSEMIN;
		auto v2 = DIFFUSERANGE;
		auto v3 = SPECULARMIN;
		auto v4 = SPECULARRANGE;
		AddLogPopup(std::format("DIFFUSEMIN {:.2f} {:.2f} {:.2f} {:.2f} DIFFUSERANGE {:.2f} {:.2f} {:.2f} {:.2f} SPECULARMIN {:.2f} {:.2f} {:.2f} {:.2f} SPECULARRANGE {:.2f} {:.2f} {:.2f} {:.2f}", v.x, v.y, v.z, v.w, v2.x, v2.y, v2.z, v2.w, v3.x, v3.y, v3.z, v3.w, v4.x, v4.y, v4.z, v4.w));
		v = ENVMAPMIN;
		v2 = ENVMAPANGE;
		AddLogPopup(std::format("ENVMAPMIN {:.2f} {:.2f} {:.2f} {:.2f} ENVMAPANGE {:.2f} {:.2f} {:.2f} {:.2f} SPECULARPOWER {:.2f} ENVMAPPOWER {:.2f}", v.x, v.y, v.z, v.w, v2.x, v2.y, v2.z, v2.w, SPECULARPOWER, ENVMAPPOWER));
	}
};
std::vector<CarShaderParams> aCarShaderParamsCollected;

void CollectCarShader() {
	if (aCarShaderParamsCollected.size() > 255) aCarShaderParamsCollected.clear();

	auto effect = eEffectStaticState::pCurrentEffect;
	if (!effect) return;

	if (effect->ID == EEFFECT_CAR) {
		CarShaderParams temp;
		temp.Collect();

		for (auto& params : aCarShaderParamsCollected) {
			if (params == temp) return;
		}

		aCarShaderParamsCollected.push_back(temp);

		temp.Log();
	}
}

static inline uintptr_t testhook_jmp = 0x6E0497;
static inline uintptr_t testhook_jmp2 = 0x6E0630;
static void __attribute__((naked)) testhook() {
	__asm__ (
		"pushad\n\t"
		"call %2\n\t"
		"popad\n\t"
		"cmp [esi+4], ebp\n\t"
		"jnz loc_6E0630\n\t"
		"jmp %0\n\t"
		"loc_6E0630:\n\t"
		"jmp %1\n\t"
			:
			:  "m" (testhook_jmp), "m" (testhook_jmp2), "i" (CollectCarShader)
	);
}