namespace MapSpawner {
	class MapData {
	public:
		NyaVec3 vMapPos = BACKROOMS_COORDS;
		bool bUseAlpha = false;
		bool bInvertX = false;
		float fRotateX = 90;
		float fRotateY = -90;
		float fRotateZ = 0;
		float fScale = 1.0;
		NyaVec3 vPlayerOffset = {0,0,0};

		std::string sTextureBasePath;
		std::vector<std::string> aModelPaths;
		std::vector<std::string> aCollisionPaths;

		std::vector<Render3D::tModel*> aLoadedModels;
		std::vector<Render3D::tModel*> aLoadedCollisions;
		std::vector<int> aSpawnedModels;
		std::vector<int> aSpawnedCollisions;

		MapData(NyaVec3 mapPos, NyaVec3 playerPos, float scale, bool alpha, const std::string& textureBasePath, const std::vector<std::string>& modelPaths, const std::vector<std::string>& collisionPaths) {
			aModelPaths = modelPaths;
			aCollisionPaths = collisionPaths;
			sTextureBasePath = textureBasePath;
			bUseAlpha = alpha;
			vPlayerOffset = playerPos;
			vMapPos = mapPos;
			fScale = scale;
		}

		bool IsSpawned() {
			if (aSpawnedModels.empty()) return false;
			if (aLoadedModels.empty()) return false;
			if (aLoadedModels[0]->bInvalidated) return false;
			return true;
		}

		bool IsInvalidated() {
			if (aLoadedModels.empty()) return false;
			return aLoadedModels[0]->bInvalidated;
		}

		void Despawn() {
			for (auto& id : aSpawnedModels) {
				Render3DObjects::aObjects[id]->Destroy(true);
			}
			aSpawnedModels.clear();
			for (auto& id : aSpawnedCollisions) {
				Render3DObjects::aObjects[id]->Destroy(true);
			}
			aSpawnedCollisions.clear();

			aLoadedModels.clear();
			aLoadedCollisions.clear();
		}

		void Spawn() {
			auto mat = UMath::Matrix4::kIdentity;
			mat.x *= fScale;
			mat.y *= fScale;
			mat.z *= fScale;
			mat.p = vMapPos;

			UMath::Matrix4 rotation;
			rotation.Rotate(NyaVec3(fRotateX * 0.01745329, fRotateY * 0.01745329, fRotateZ * 0.01745329));
			mat = (UMath::Matrix4)(mat * rotation);

			if (bInvertX) {
				mat.x *= -1;
			}

			if (aLoadedModels.empty() || aLoadedModels[0]->bInvalidated) {
				aLoadedModels.clear();
				Render3D::sTextureSubdir = sTextureBasePath;
				for (auto& modelPath : aModelPaths) {
					auto models = Render3D::CreateModels(modelPath);
					for (auto& mdl : models) {
						aLoadedModels.push_back(mdl);
					}
				}
				Render3D::sTextureSubdir = "";
			}
			for (auto& model : aLoadedModels) {
				aSpawnedModels.push_back(Render3DObjects::aObjects.size());
				Render3DObjects::aObjects.push_back(new Render3DObjects::Object("map", {model}, mat, mat.p));
			}
			for (auto& id : aSpawnedModels) {
				Render3DObjects::aObjects[id]->bUseAlpha = bUseAlpha;
			}

			if (aLoadedCollisions.empty() || aLoadedCollisions[0]->bInvalidated) {
				aLoadedCollisions.clear();
				Render3D::sTextureSubdir = sTextureBasePath;
				for (auto& modelPath : aCollisionPaths) {
					auto models = Render3D::CreateModels(modelPath);
					for (auto& mdl : models) {
						aLoadedCollisions.push_back(mdl);
					}
				}
				Render3D::sTextureSubdir = "";
			}
			for (auto& model : aLoadedCollisions) {
				aSpawnedCollisions.push_back(Render3DObjects::aObjects.size());
				Render3DObjects::aObjects.push_back(new Render3DObjects::Object("mapCol", {model}, mat, mat.p));
			}
			for (auto& id : aSpawnedCollisions) {
				Render3DObjects::aObjects[id]->bDontRender = true;
				Render3DObjects::aObjects[id]->bTriCollidable = true;
			}

			if (auto veh = GetLocalPlayerInterface<IRigidBody>()) {
				UMath::Vector3 playerPos = vMapPos + vPlayerOffset;
				TeleportPlayer(playerPos, {-0.96,0.0,-0.3});
			}
		}
	};

	NyaVec3 TEMP_CUSTOMMAP_COORDS = {BACKROOMS_COORDS.x, BACKROOMS_COORDS.y+50, BACKROOMS_COORDS.z};

	enum eMap {
		MAP_BACKROOMS,
		MAP_AQUATICA,
		MAP_UG1,
		MAP_COTEDAZUR,
	};

	MapData aMaps[] = {
		MapData(BACKROOMS_COORDS, {4,-12,-37}, 10.0, false, "backrooms/", {"backrooms/1.fbx"}, {"backrooms/backrooms_col.fbx"}),
		MapData(TEMP_CUSTOMMAP_COORDS, {0,5,0}, 1.0, true, "aquatica/",
				{
						"aquatica/aquatica.fbx_Section1.fbx",
						"aquatica/aquatica.fbx_Section2.fbx",
						"aquatica/aquatica.fbx_Section3.fbx",
						"aquatica/aquatica.fbx_Section4.fbx",
						"aquatica/aquatica.fbx_Section5.fbx",
						"aquatica/aquatica.fbx_Section6.fbx",
						"aquatica/aquatica.fbx_Section7.fbx",
						"aquatica/aquatica.fbx_Section8.fbx",
						"aquatica/aquatica.fbx_Section9.fbx",
						"aquatica/aquatica.fbx_Section10.fbx",
						"aquatica/aquatica.fbx_Section11.fbx",
						//"aquatica/aquatica.fbx_Seabox.fbx",
				},
				{
						"aquatica/aquatica.fbx_Section1_col.fbx",
						"aquatica/aquatica.fbx_Section2_col.fbx",
						"aquatica/aquatica.fbx_Section3_col.fbx",
						"aquatica/aquatica.fbx_Section4_col.fbx",
						"aquatica/aquatica.fbx_Section5_col.fbx",
						"aquatica/aquatica.fbx_Section6_col.fbx",
						"aquatica/aquatica.fbx_Section7_col.fbx",
						"aquatica/aquatica.fbx_Section8_col.fbx",
						"aquatica/aquatica.fbx_Section9_col.fbx",
						"aquatica/aquatica.fbx_Section10_col.fbx",
						"aquatica/aquatica.fbx_Section11_col.fbx",
				}),
		MapData(TEMP_CUSTOMMAP_COORDS, {-4231.55 - TEMP_CUSTOMMAP_COORDS.x, 102.52 - TEMP_CUSTOMMAP_COORDS.y, -4537.59 - TEMP_CUSTOMMAP_COORDS.z}, 1.0, true, "OlympicCity/",
				{
						"OlympicCity/01.fbx",
						"OlympicCity/02.fbx",
						"OlympicCity/03.fbx",
						"OlympicCity/04.fbx",
						"OlympicCity/05.fbx",
						"OlympicCity/06.fbx",
						"OlympicCity/07.fbx",
						"OlympicCity/08.fbx",
						"OlympicCity/09.fbx",
						"OlympicCity/10.fbx",
						"OlympicCity/11.fbx",
						"OlympicCity/12.fbx",
						"OlympicCity/13.fbx",
						"OlympicCity/14.fbx",
						"OlympicCity/15.fbx",
						"OlympicCity/16.fbx",
						"OlympicCity/17.fbx",
						"OlympicCity/18.fbx",
						"OlympicCity/19.fbx",
						"OlympicCity/20.fbx",
						"OlympicCity/21.fbx",
						"OlympicCity/22.fbx",
						"OlympicCity/23.fbx",
						"OlympicCity/24.fbx",
						"OlympicCity/25.fbx",
						"OlympicCity/26.fbx",
						"OlympicCity/27.fbx",
						"OlympicCity/28.fbx",
						"OlympicCity/29.fbx",
						"OlympicCity/30.fbx",
						"OlympicCity/31.fbx",
						"OlympicCity/32.fbx",
						"OlympicCity/33.fbx",
						"OlympicCity/34.fbx",
						"OlympicCity/35.fbx",
						"OlympicCity/36.fbx",
						"OlympicCity/37.fbx",
						"OlympicCity/38.fbx",
						"OlympicCity/39.fbx",
						"OlympicCity/40.fbx",
						"OlympicCity/41.fbx",
						"OlympicCity/42.fbx",
						"OlympicCity/43.fbx",
						"OlympicCity/44.fbx",
						"OlympicCity/45.fbx",
						"OlympicCity/46.fbx",
						"OlympicCity/47.fbx",
						"OlympicCity/48.fbx",
						"OlympicCity/49.fbx",
						"OlympicCity/50.fbx",
				},
				{
						"OlympicCity/01_col.fbx",
						"OlympicCity/02_col.fbx",
						"OlympicCity/03_col.fbx",
						"OlympicCity/04_col.fbx",
						"OlympicCity/05_col.fbx",
						"OlympicCity/06_col.fbx",
						"OlympicCity/07_col.fbx",
						"OlympicCity/08_col.fbx",
						"OlympicCity/09_col.fbx",
						"OlympicCity/10_col.fbx",
						"OlympicCity/11_col.fbx",
						"OlympicCity/12_col.fbx",
						"OlympicCity/13_col.fbx",
						"OlympicCity/14_col.fbx",
						"OlympicCity/15_col.fbx",
						"OlympicCity/16_col.fbx",
						"OlympicCity/17_col.fbx",
						"OlympicCity/18_col.fbx",
						"OlympicCity/19_col.fbx",
						"OlympicCity/20_col.fbx",
						"OlympicCity/21_col.fbx",
						"OlympicCity/22_col.fbx",
						"OlympicCity/23_col.fbx",
						"OlympicCity/24_col.fbx",
						"OlympicCity/25_col.fbx",
						"OlympicCity/26_col.fbx",
						"OlympicCity/27_col.fbx",
						"OlympicCity/28_col.fbx",
						"OlympicCity/29_col.fbx",
						"OlympicCity/30_col.fbx",
						"OlympicCity/31_col.fbx",
						"OlympicCity/32_col.fbx",
						"OlympicCity/33_col.fbx",
						"OlympicCity/34_col.fbx",
						"OlympicCity/35_col.fbx",
						"OlympicCity/36_col.fbx",
						"OlympicCity/37_col.fbx",
						"OlympicCity/38_col.fbx",
						"OlympicCity/39_col.fbx",
						"OlympicCity/40_col.fbx",
						"OlympicCity/41_col.fbx",
						"OlympicCity/42_col.fbx",
						"OlympicCity/43_col.fbx",
						"OlympicCity/44_col.fbx",
						"OlympicCity/45_col.fbx",
						"OlympicCity/46_col.fbx",
						"OlympicCity/47_col.fbx",
						"OlympicCity/48_col.fbx",
						"OlympicCity/49_col.fbx",
						"OlympicCity/50_col.fbx",
				}),
		MapData(TEMP_CUSTOMMAP_COORDS, {-185,230,138}, 1.0, true, "cotedazur/", {"cotedazur/1.fbx"}, {"cotedazur/col.fbx"}),
	};
}