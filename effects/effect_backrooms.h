class Effect_Backrooms : public ChaosEffect {
public:
	Effect_Backrooms() : ChaosEffect("Uncategorized") {
		sName = "Teleport To Backrooms";
		bCanQuickTrigger = false;
		bRigProportionalChances = true;
	}

	static inline bool invert = false;
	static inline float rX = 90;
	static inline float rY = -90;
	static inline float rZ = 0;
	static inline float scale = 10.0;
	static inline float playerX = 4;
	static inline float playerY = -12;
	static inline float playerZ = -37;

	void InitFunction() override {
		Render3D::sTextureSubdir = "backrooms/";
		static auto models = Render3D::CreateModels("backrooms/1.fbx");
		static auto modelsCol = Render3D::CreateModels("backrooms/backrooms_col.fbx");
		Render3D::sTextureSubdir = "";

		//TeleportPlayer({-2485.08, 208.38, -1348.54}, {0.0,0.0,1.0}); // teleport behind old bridge

		static bool bOnce = true;
		if (bOnce) {
			auto mat = UMath::Matrix4::kIdentity;
			mat.x *= scale;
			mat.y *= scale;
			mat.z *= scale;
			mat.p = BACKROOMS_COORDS;

			UMath::Matrix4 rotation;
			rotation.Rotate(NyaVec3(rX * 0.01745329, rY * 0.01745329, rZ * 0.01745329));
			mat = (UMath::Matrix4)(mat * rotation);

			if (invert) {
				mat.x *= -1;
			}

			Render3DObjects::aObjects.push_back(new Render3DObjects::Object("map", models, mat, mat.p));

			int id = Render3DObjects::aObjects.size();
			Render3DObjects::aObjects.push_back(new Render3DObjects::Object("mapCol", modelsCol, mat, mat.p));
			Render3DObjects::aObjects[id]->bDontRender = true;
			Render3DObjects::aObjects[id]->bTriCollidable = true;

			bOnce = false;
		}

		if (auto veh = GetLocalPlayerInterface<IRigidBody>()) {
			UMath::Vector3 playerPos = BACKROOMS_COORDS;
			playerPos.x += playerX;
			playerPos.y += playerY;
			playerPos.z += playerZ;
			TeleportPlayer(playerPos, {-0.96,0.0,-0.3});
		}
	}
} E_Backrooms;