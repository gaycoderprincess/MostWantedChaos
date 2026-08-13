namespace Render3D {
//#define RENDER3D_NOEFFECT
	const uint32_t nDefaultVertexColor = 0xFF404040;

	struct {
		uint32_t nVertexColorValue = nDefaultVertexColor;
		bool bColorByNormals = false;
		float fColorByNormalsScale = 1.0;
		uint8_t nAlphaValue = 255;
		std::string sTextureSubdir;

		void Reset() {
			nVertexColorValue = nDefaultVertexColor;
			bColorByNormals = false;
			fColorByNormalsScale = 1.0;
			nAlphaValue = 255;
			sTextureSubdir.clear();
		}
	} ModelLoaderConfig;

	struct {
		bool bForceNoEffect = false;
		bool bForceNoCulling = false;

		bool bNoEffect_ReadVertexColor = false;

		D3DXVECTOR4 fDIFFUSEMIN = {0.4,0.4,0.4,1};
		D3DXVECTOR4 fDIFFUSERANGE = {0.6,0.6,0.6,0};
		D3DXVECTOR4 fSPECULARMIN = {0.16,0.2,0.16,0};
		D3DXVECTOR4 fSPECULARRANGE = {0.04,0.0,0.04,0};
		D3DXVECTOR4 fENVMAPMIN = {1.75,1.75,1.75,0};
		D3DXVECTOR4 fENVMAPANGE = {-1.65,-1.65,-1.65,0};
		float fSPECULARPOWER = 8.0;
		float fENVMAPPOWER = 0.15;

		IDirect3DTexture9* pOverrideDiffuse = nullptr;

		void Reset() {
			bForceNoEffect = false;
			bForceNoCulling = false;

			bNoEffect_ReadVertexColor = false;

			fDIFFUSEMIN = {0.4,0.4,0.4,1};
			fDIFFUSERANGE = {0.6,0.6,0.6,0};
			fSPECULARMIN = {0.16,0.2,0.16,0};
			fSPECULARRANGE = {0.04,0.0,0.04,0};
			fENVMAPMIN = {1.75,1.75,1.75,0};
			fENVMAPANGE = {-1.65,-1.65,-1.65,0};
			fSPECULARPOWER = 8.0;
			fENVMAPPOWER = 0.15;

			pOverrideDiffuse = nullptr;
		}
	} RendererConfig;

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

	eView* pViewToDraw = nullptr;

	bool bForceNoEnvmap = false;
	bool bForceNoShadows = false;

	bool bUserForceNoEffect = false;
	bool bUserForceNoEnvmap = false;
	bool bUserForceNoShadows = false;

	bool bShadowsAvailable = false;

	bool IsEffectDisabled() {
		return RendererConfig.bForceNoEffect || bUserForceNoEffect;
	}
	bool IsEnvmapDisabled() {
		return bForceNoEnvmap || bUserForceNoEnvmap;
	}
	bool IsShadowingDisabled() {
		return bForceNoShadows || bUserForceNoShadows;
	}

	eEffect* pLastUsedEffect = nullptr;
	IDirect3DTexture9* pLastUsedTexture = nullptr;
	IDirect3DVertexBuffer9* pLastUsedVBuffer = nullptr;
	IDirect3DIndexBuffer9* pLastUsedIBuffer = nullptr;
	void BeginRendering() {
		pLastUsedEffect = nullptr;
		pLastUsedTexture = (IDirect3DTexture9*)0xCDCDCDCD;
		pLastUsedVBuffer = (IDirect3DVertexBuffer9*)0xCDCDCDCD;
		pLastUsedIBuffer = (IDirect3DIndexBuffer9*)0xCDCDCDCD;
		*(uint32_t*)0x982CB4 = 0; // set last texture to null, fixes sky
	}

	void FinalizeRendering() {
		if (!pLastUsedEffect) return;
		pLastUsedEffect->End();
		pLastUsedEffect->hD3DXEffect->EndPass();
		pLastUsedEffect->hD3DXEffect->End();
		pLastUsedEffect = nullptr;
		pLastUsedTexture = (IDirect3DTexture9*)0xCDCDCDCD;
	}

	struct tRenderProperties {
		bool useAlpha;
		int effectId;
		bool zwrite;
		int cullMode;
		bool noeffect_vertexColor = RendererConfig.bNoEffect_ReadVertexColor;
	};
	tRenderProperties LastRenderProperties;
	bool ShouldRefreshRenderProperties(bool useAlpha, int effectId, bool zwrite, int cullMode) {
		auto newProp = tRenderProperties();
		newProp.useAlpha = useAlpha;
		newProp.effectId = effectId;
		newProp.zwrite = zwrite;
		newProp.cullMode = cullMode;
		if (!memcmp(&LastRenderProperties, &newProp, sizeof(LastRenderProperties))) return false;
		LastRenderProperties = newProp;
		return true;
	}

	struct tModel {
		IDirect3DIndexBuffer9* pIndexBuffer = nullptr;
		IDirect3DVertexBuffer9* pVertexBuffer = nullptr;
		IDirect3DTexture9* pTextureDiffuse = nullptr;
		IDirect3DTexture9* pTextureNormal = nullptr;
		IDirect3DTexture9* pTextureSpecular = nullptr;
		std::string sTextureName;
		std::string sNormalTextureName;
		std::string sSpecularTextureName;
		uint32_t nVertexCount;
		uint32_t nFaceCount;
		bool bInvalidated = false;

		// for collision checks
		std::vector<NyaVec3> aVertices;
		std::vector<int> aIndices;

		void RenderAt(NyaMat4x4 matrix, bool useAlpha = false, int effectId = EEFFECT_WORLD, bool zwrite = true) const {
#ifdef RENDER3D_NOEFFECT
			return RenderAt_NoEffect(matrix, useAlpha, zwrite);
#else
			if (bInvalidated) return;

			bool isShadow = IsRenderingShadows(pViewToDraw);
			bool isEnvmap = IsRenderingEnvmap(pViewToDraw);

			if (IsEnvmapDisabled() && isEnvmap) return;
			if (IsShadowingDisabled() && isShadow) return;

			if (IsEffectDisabled() && !isShadow) {
				return RenderAt_NoEffect(matrix, useAlpha, zwrite);
			}

			int cullMode = D3DCULL_CW;
			if (isEnvmap) cullMode = D3DCULL_CCW;
			if (RendererConfig.bForceNoCulling) cullMode = D3DCULL_NONE;

			if (isShadow) {
				effectId = effectId == EEFFECT_CAR ? EEFFECT_CARSHADOWMAP : EEFFECT_WORLDNOFOG;
			}
			else {
				if (pTextureNormal && effectId == EEFFECT_WORLD) {
					effectId = EEFFECT_WORLDNORMALMAP;
				}
			}

			bool shouldRefresh = ShouldRefreshRenderProperties(useAlpha, effectId, zwrite, cullMode);

			auto effect = eEffectStaticState::pCurrentEffect = eEffects[effectId];

			if (pLastUsedEffect != effect) {
				shouldRefresh = true;

				// finish last pass if needed
				FinalizeRendering();

				// begin new pass
				effect->Start();
				effect->hD3DXEffect->Begin(nullptr, 0);
				effect->hD3DXEffect->BeginPass(0);
				pLastUsedEffect = effect;

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

					effect->hD3DXEffect->SetFloat(effect->mParamTable->mParamMappingTable[CParamHashTable::SPECULARPOWER].mHandle, RendererConfig.fSPECULARPOWER);
					effect->hD3DXEffect->SetFloat(effect->mParamTable->mParamMappingTable[CParamHashTable::ENVMAPPOWER].mHandle, RendererConfig.fENVMAPPOWER);

					D3DXVECTOR4 v = RendererConfig.fDIFFUSEMIN;
					effect->hD3DXEffect->SetVector(effect->mParamTable->mParamMappingTable[CParamHashTable::DIFFUSEMIN].mHandle, &v);
					v = RendererConfig.fDIFFUSERANGE;
					effect->hD3DXEffect->SetVector(effect->mParamTable->mParamMappingTable[CParamHashTable::DIFFUSERANGE].mHandle, &v);
					v = RendererConfig.fSPECULARMIN;
					effect->hD3DXEffect->SetVector(effect->mParamTable->mParamMappingTable[CParamHashTable::SPECULARMIN].mHandle, &v);
					v = RendererConfig.fSPECULARRANGE;
					effect->hD3DXEffect->SetVector(effect->mParamTable->mParamMappingTable[CParamHashTable::SPECULARRANGE].mHandle, &v);

					v = RendererConfig.fENVMAPMIN;
					effect->hD3DXEffect->SetVector(effect->mParamTable->mParamMappingTable[CParamHashTable::ENVMAPMIN].mHandle, &v);
					v = RendererConfig.fENVMAPANGE;
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

				g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

				g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
				g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
				g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
				//g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
				//g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
				//g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

				g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
				g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
				g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
			}

			g_pd3dDevice->SetVertexDeclaration(effect->VertexDecl);

			// this is kinda nasty but this function will refuse to update the matrix if its pointer is identical to the previous caller's
			static UMath::Matrix4 matrixTemp[2];
			static bool matrixTempSecond = false;
			UMath::Matrix4* pMatrix = matrixTempSecond ? &matrixTemp[1] : &matrixTemp[0];
			*pMatrix = (UMath::Matrix4)matrix;
			matrixTempSecond = !matrixTempSecond;
			ParticleSetTransform(pMatrix, pViewToDraw->ID);

			if (shouldRefresh) {
				effect->hD3DXEffect->SetInt(effect->mParamTable->mParamMappingTable[CParamHashTable::CULL_MODE].mHandle, cullMode);
				g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, zwrite);

				int blendState[5] = {};
				if (useAlpha) {
					blendState[0] = TRUE; // D3DRS_ALPHATESTENABLE
					blendState[1] = 1; // D3DRS_ALPHAREF
					g_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
				}
				else {
					blendState[0] = 0; // D3DRS_ALPHATESTENABLE
					blendState[1] = 0; // D3DRS_ALPHAREF
				}
				blendState[2] = useAlpha; // D3DRS_ALPHABLENDENABLE
				blendState[3] = D3DBLEND_SRCALPHA; // D3DRS_SRCBLEND
				blendState[4] = D3DBLEND_INVSRCALPHA; // D3DRS_DESTBLEND
				effect->hD3DXEffect->SetIntArray(effect->mParamTable->mParamMappingTable[CParamHashTable::BLENDSTATE].mHandle, blendState, 5);
			}

			if (effectId == EEFFECT_CAR) {
				auto light = (eDynamicLightContext*)eFrameMalloc(sizeof(eDynamicLightContext));
				elSetupLightContext(light, &ShaperLightsCarsInGame, pMatrix, &pViewToDraw->pCamera->CurrentKey.Matrix, (UMath::Vector4*)&pViewToDraw->pCamera->CurrentKey.Position, pViewToDraw);
				eEffect::SetLightContext(light, pMatrix);
			}

			if (pLastUsedVBuffer != pVertexBuffer) {
				g_pd3dDevice->SetStreamSource(0, pVertexBuffer, 0, sizeof(CwoeeVertexData));
				pLastUsedVBuffer = pVertexBuffer;
			}
			if (pLastUsedIBuffer != pIndexBuffer) {
				g_pd3dDevice->SetIndices(pIndexBuffer);
				pLastUsedIBuffer = pIndexBuffer;
			}

			auto diffuse = RendererConfig.pOverrideDiffuse ? RendererConfig.pOverrideDiffuse : pTextureDiffuse;
			if (pLastUsedTexture != diffuse) {
				effect->hD3DXEffect->SetTexture(effect->mParamTable->mParamMappingTable[CParamHashTable::DiffuseMap].mHandle, diffuse);
				pLastUsedTexture = diffuse;
			}
			if (effectId == EEFFECT_WORLDNORMALMAP || effectId == EEFFECT_CAR) {
				effect->hD3DXEffect->SetTexture(effect->mParamTable->mParamMappingTable[CParamHashTable::NormalMapTexture].mHandle, pTextureNormal ? pTextureNormal : pTextureDiffuse);
			}
			if (effectId == EEFFECT_WORLDNORMALMAP && pTextureSpecular) {
				effect->hD3DXEffect->SetTexture(effect->mParamTable->mParamMappingTable[CParamHashTable::SPECULARMAPTEXTURE].mHandle, pTextureSpecular);
			}
			effect->hD3DXEffect->CommitChanges();

			g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, nVertexCount, 0, nFaceCount);
#endif
		}

		void RenderAt_NoEffect(NyaMat4x4 matrix, bool useAlpha = false, bool zwrite = true, bool useZ = true) const {
			if (bInvalidated) return;

			bool isShadow = IsRenderingShadows(pViewToDraw);
			bool isEnvmap = IsRenderingEnvmap(pViewToDraw);

			if (IsEnvmapDisabled() && isEnvmap) return;
			if (IsShadowingDisabled() && isShadow) return;

			if (isShadow) {
				return RenderAt(matrix);
			}

			// finish last pass if needed
			FinalizeRendering();

			int cullMode = D3DCULL_CW;
			if (isEnvmap) cullMode = D3DCULL_CCW;
			if (RendererConfig.bForceNoCulling) cullMode = D3DCULL_NONE;

			ShouldRefreshRenderProperties(useAlpha, (int)useZ + 64, zwrite, cullMode);

			g_pd3dDevice->SetPixelShader(nullptr);
			g_pd3dDevice->SetVertexShader(nullptr);

			auto view = pViewToDraw->PlatInfo->ViewMatrix;
			auto proj = pViewToDraw->PlatInfo->ProjectionMatrix;
			g_pd3dDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&view);
			g_pd3dDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&proj);

			g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, useZ);
			g_pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, zwrite);
			g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, cullMode);
			if (useAlpha) {
				g_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 127);
				g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
				g_pd3dDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
			}
			else {
				g_pd3dDevice->SetRenderState(D3DRS_ALPHATESTENABLE, 0);
				g_pd3dDevice->SetRenderState(D3DRS_ALPHAREF, 0);
			}
			g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, useAlpha);
			g_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			g_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

			g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, RendererConfig.bNoEffect_ReadVertexColor ? D3DTOP_MODULATE : D3DTOP_SELECTARG1);
			g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			g_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
			g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
			g_pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

			g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
			g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
			g_pd3dDevice->SetSamplerState(0, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);

			g_pd3dDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&matrix);
			g_pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1);
			if (pLastUsedVBuffer != pVertexBuffer) {
				g_pd3dDevice->SetStreamSource(0, pVertexBuffer, 0, sizeof(CwoeeVertexData));
				pLastUsedVBuffer = pVertexBuffer;
			}
			if (pLastUsedIBuffer != pIndexBuffer) {
				g_pd3dDevice->SetIndices(pIndexBuffer);
				pLastUsedIBuffer = pIndexBuffer;
			}
			g_pd3dDevice->SetTexture(0, RendererConfig.pOverrideDiffuse ? RendererConfig.pOverrideDiffuse : pTextureDiffuse);
			g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, nVertexCount, 0, nFaceCount);
		}

		void Invalidate() {
			if (bInvalidated) return;

			if (pVertexBuffer) {
				pVertexBuffer->Release();
				pVertexBuffer = nullptr;
			}
			if (pIndexBuffer) {
				pIndexBuffer->Release();
				pIndexBuffer = nullptr;
			}
			// textures are stored and shared in aAllTextures
			//if (pTexture) {
			//	pTexture->Release();
			//	pTexture = nullptr;
			//}
			aVertices.clear();
			aVertices.shrink_to_fit();
			aIndices.clear();
			aIndices.shrink_to_fit();
			bInvalidated = true;
		}
	};
	std::vector<tModel*> aAllModels;
	std::vector<tTextureInfo> aAllTextures;
	std::vector<std::string> aFailedTextures;

	IDirect3DTexture9* LoadOrFindTexture(const std::string& material, bool addToFail) {
		auto baseTextureName = material;
		if (!baseTextureName.empty() && baseTextureName.find('.') == std::string::npos) {
			baseTextureName += ".png";
		}

		auto textureName = ModelLoaderConfig.sTextureSubdir + baseTextureName;
		if (baseTextureName.empty()) {
			textureName = "white.png";
		}

		for (auto& texture : aAllTextures) {
			if (texture.sFile == textureName) {
				return texture.pTexture;
			}
		}

		if (auto tex = LoadTexture_SetDir(std::format("CwoeeChaos/data/models/{}", textureName).c_str())) {
			aAllTextures.push_back({textureName, tex});
			return tex;
		}

		if (addToFail) {
			bool isNew = true;
			for (auto& name : aFailedTextures) {
				if (name == textureName) isNew = false;
			}
			if (isNew) {
				aFailedTextures.push_back(textureName);
			}
		}
		return nullptr;
	}

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
			auto srcColor = &colors[i];
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
			if (!ModelLoaderConfig.nVertexColorValue) {
				auto tmp = NyaDrawing::CNyaRGBA32();
				tmp.b = srcColor->r;
				tmp.g = srcColor->g;
				tmp.r = srcColor->b;
				tmp.a = ModelLoaderConfig.nAlphaValue;
				dest->Color = *(uint32_t*)&tmp;
			}
			else {
				dest->Color = overrideVertexColor ? 0xFFFFFFFF : ModelLoaderConfig.nVertexColorValue;
			}
			if (uvs) {
				dest->vUV[0] = srcUV->x;
				dest->vUV[1] = srcUV->y * -1;
			}
			
			if (ModelLoaderConfig.bColorByNormals && normals) {
				auto tmp = NyaDrawing::CNyaRGBA32();
				tmp.b = srcNormal->x * 255 * ModelLoaderConfig.fColorByNormalsScale;
				tmp.g = srcNormal->y * 255 * ModelLoaderConfig.fColorByNormalsScale;
				tmp.r = srcNormal->z * 255 * ModelLoaderConfig.fColorByNormalsScale;
				tmp.a = ModelLoaderConfig.nAlphaValue;
				dest->Color = *(uint32_t*)&tmp;
			}
		}

		memcpy(indicesOut, indices, indexTotalSize);

		model->nVertexCount = numVertices;
		model->nFaceCount = numFaces;

		model->pVertexBuffer->Unlock();
		model->pIndexBuffer->Unlock();

		model->sTextureName = material;
		model->sNormalTextureName = material + "_normal";
		model->sSpecularTextureName = material + "_specular";

		model->pTextureDiffuse = LoadOrFindTexture(model->sTextureName, true);
		//model->pTextureNormal = LoadOrFindTexture(model->sNormalTextureName, false);
		//model->pTextureSpecular = LoadOrFindTexture(model->sSpecularTextureName, false);
		//if (model->pTextureNormal && !model->pTextureSpecular) {
		//	model->pTextureSpecular = LoadOrFindTexture("black.png", true);
		//}

		aAllModels.push_back(model);
		return model;
	}

	std::vector<tModel*> CreateModels(const std::string& path) {
		DLLDirSetter _setdir;

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
		std::string textureFails;
		for (auto& textureName : aFailedTextures) {
			textureFails += "\n";
			textureFails += textureName;
		}
		if (!textureFails.empty()) {
			MessageBoxA(nullptr, std::format("Failed to load textures:{}", textureFails).c_str(), "nya?!~", MB_ICONERROR);
		}
		aFailedTextures.clear();
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