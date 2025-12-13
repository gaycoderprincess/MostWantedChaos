#define EFFECT_CATEGORY_TEMP "3D Render"

class Effect_Shark : public ChaosEffect {
public:
	Effect_Shark() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Friends Of Blahaj";
		sFriendlyName = "Blahaj Roof Racks";
		fTimerLength = 90;
	}

	std::vector<Render3D::tModel*> models;

	static inline float rX = 0;
	static inline float rY = 180;
	static inline float rZ = 90;
	static inline float offX = 0;
	static inline float offY = -0.1;
	static inline float offZ = -0.5;
	static inline float scale = 0.5;

	void TickFunction(eChaosHook hook, double delta) override {
		if (hook != HOOK_3D) return;

		if (models.empty() || models[0]->bInvalidated) {
			models = Render3D::CreateModels("shork.fbx");
		}

		auto cars = GetActiveVehicles();
		for (auto& model : models) {
			for (auto& car : cars) {
				auto mat = UMath::Matrix4::kIdentity;
				if (auto veh = car->mCOMObject->Find<IRigidBody>()) {
					veh->GetMatrix4(&mat);
					UMath::Vector3 dim;
					veh->GetDimension(&dim);
					mat.p = *veh->GetPosition();
					mat.p += mat.x * offX;
					mat.p += mat.y * (dim.y + offY);
					mat.p += mat.z * offZ;
				}
				mat.x *= scale;
				mat.y *= scale;
				mat.z *= scale;

				UMath::Matrix4 rotation;
				rotation.Rotate(NyaVec3(rX * 0.01745329, rY * 0.01745329, rZ * 0.01745329));
				mat = (UMath::Matrix4)(mat * rotation);
				model->RenderAt(WorldToRenderMatrix(mat));
			}
		}
	}
	bool HasTimer() override { return true; }
	bool RunWhenBlocked() override { return true; }
} E_Shark;

class Effect_Shark2 : public ChaosEffect {
public:
	Effect_Shark2() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Become Fish";
		sFriendlyName = "All Cars Are Blahaj";
		fTimerLength = 60;
	}

	std::vector<Render3D::tModel*> models;

	static inline float rX = 0;
	static inline float rY = 180;
	static inline float rZ = 90;
	static inline float offX = 0;
	static inline float offY = -0.9;
	static inline float offZ = -0.5;
	static inline float scale = 1.5;

	static inline float stareX = 0;
	static inline float stareY = 0;
	static inline float stareZ = 90;

	void DrawAFish(const Render3D::tModel* model, UMath::Matrix4 mat) {
		mat.p += mat.x * offX;
		mat.p += mat.y * offY;
		mat.p += mat.z * offZ;
		mat.x *= scale;
		mat.y *= scale;
		mat.z *= scale;

		UMath::Matrix4 rotation;
		rotation.Rotate(NyaVec3(rX * 0.01745329, rY * 0.01745329, rZ * 0.01745329));
		mat = (UMath::Matrix4)(mat * rotation);

		auto carMatrix = WorldToRenderMatrix(mat);
		if (CarRender_Billboard) {
			auto cameraMatrix = PrepareCameraMatrix(eViews[EVIEW_PLAYER1].pCamera);
			auto cameraPos = cameraMatrix.p;
			auto carPos = carMatrix.p;

			auto lookat = carPos - cameraPos;
			lookat.Normalize();
			auto lookatMatrix = NyaMat4x4::LookAt(lookat, {0, 0, 1});
			carMatrix.x = lookatMatrix.x;
			carMatrix.y = lookatMatrix.y;
			carMatrix.z = lookatMatrix.z;

			NyaMat4x4 offsetMatrix;
			offsetMatrix.Rotate(NyaVec3(stareX * 0.01745329, stareY * 0.01745329, stareZ * 0.01745329));
			carMatrix = carMatrix * offsetMatrix;
		}
		carMatrix.x *= CarScaleMatrix.x.x;
		carMatrix.y *= CarScaleMatrix.z.z;
		carMatrix.z *= CarScaleMatrix.y.y;
		model->RenderAt(carMatrix);
	}

	void TickFunction(eChaosHook hook, double delta) override {
		if (hook != HOOK_3D) return;
		
		DrawCars = false;

		if (models.empty() || models[0]->bInvalidated) {
			models = Render3D::CreateModels("shork.fbx");
		}

		if (TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_IN_FRONTEND) {
			for (auto& model : models) {
				auto mat = UMath::Matrix4::kIdentity;
				mat.p.y += 1;
				DrawAFish(model, mat);
			}
		}
		else {
			auto cars = GetActiveVehicles();
			for (auto& model : models) {
				for (auto& car : cars) {
					if (auto veh = car->mCOMObject->Find<IRigidBody>()) {
						auto mat = UMath::Matrix4::kIdentity;
						veh->GetMatrix4(&mat);
						mat.p = *veh->GetPosition();
						DrawAFish(model, mat);
					}
				}
			}
		}
	}
	void DeinitFunction() override {
		DrawCars = true;
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
	bool RunWhenBlocked() override { return true; }
} E_Shark2;

class Effect_Teddie : public ChaosEffect {
public:
	Effect_Teddie() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "pee.";
		sFriendlyName = "Spawn Teddie From Persona 4";
	}

	std::vector<Render3D::tModel*> models;

	static inline float rX = 90;
	static inline float rY = 0;
	static inline float rZ = 0;
	static inline float offX = 0;
	static inline float offY = -1;
	static inline float offZ = 4;
	static inline float scale = 1.0;
	static inline float colScale = 0.65;

	void InitFunction() override {
		if (models.empty() || models[0]->bInvalidated) {
			models = Render3D::CreateModels("teddie.fbx");
		}

		if (auto veh = GetLocalPlayerInterface<IRigidBody>()) {
			auto mat = UMath::Matrix4::kIdentity;
			veh->GetMatrix4(&mat);
			mat.p = *veh->GetPosition();
			auto colPos = mat.p;
			colPos += mat.x * offX;
			colPos += mat.z * offZ;
			mat.p += mat.x * offX;
			mat.p += mat.y * offY;
			mat.p += mat.z * offZ;
			mat.x *= scale;
			mat.y *= scale;
			mat.z *= scale;

			UMath::Matrix4 rotation;
			rotation.Rotate(NyaVec3(rX * 0.01745329, rY * 0.01745329, rZ * 0.01745329));
			mat = (UMath::Matrix4)(mat * rotation);
			Render3DObjects::aObjects.push_back(Render3DObjects::Object(models, mat, colPos, colScale));
		}
	}
} E_Teddie;

class Effect_Pep : public ChaosEffect {
public:
	Effect_Pep() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Pep";
		sFriendlyName = "Change Car To FO1 Pepper";
		fTimerLength = 180;
		IncompatibilityGroups.push_back(Attrib::StringHash32("customplayercar"));
	}

	void InitFunction() override {
		gCustomCar_Pepper.Reset(GetLocalPlayerVehicle());
	}
	void TickFunction(eChaosHook hook, double delta) override {
		if (hook != HOOK_3D) return;
		
		CarRender_DontRenderPlayer = true;
		DrawLightFlares = false;

		gCustomCar_Pepper.Update(GetLocalPlayerVehicle(), delta);
		gCustomCar_Pepper.Render(GetLocalPlayerVehicle());
	}
	void DeinitFunction() override {
		CarRender_DontRenderPlayer = false;
		DrawLightFlares = true;
	}
	bool HasTimer() override { return true; }
	bool RunWhenBlocked() override { return true; }
} E_Pep;

class Effect_Greenwood : public ChaosEffect {
public:
	Effect_Greenwood() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Bucket-Ass Car";
		sFriendlyName = "Change Car To GTA Greenwood";
		fTimerLength = 120;
		IncompatibilityGroups.push_back(Attrib::StringHash32("customplayercar"));
	}

	void InitFunction() override {
		gCustomCar_Greenwood.Reset(GetLocalPlayerVehicle());
	}
	void TickFunction(eChaosHook hook, double delta) override {
		if (hook != HOOK_3D) return;
		
		CarRender_DontRenderPlayer = true;
		DrawLightFlares = false;

		gCustomCar_Greenwood.Update(GetLocalPlayerVehicle(), delta);
		gCustomCar_Greenwood.Render(GetLocalPlayerVehicle());
	}
	void DeinitFunction() override {
		CarRender_DontRenderPlayer = false;
		DrawLightFlares = true;
	}
	bool HasTimer() override { return true; }
	bool RunWhenBlocked() override { return true; }
} E_Greenwood;

class Effect_Mona : public ChaosEffect {
public:
	Effect_Mona() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Not A Cat";
		sFriendlyName = "Change Car To Morgana";
		fTimerLength = 60;
		IncompatibilityGroups.push_back(Attrib::StringHash32("customplayercar"));
	}

	void InitFunction() override {
		gCustomCar_Mona.Reset(GetLocalPlayerVehicle());
	}
	void TickFunction(eChaosHook hook, double delta) override {
		if (hook != HOOK_3D) return;
		
		CarRender_DontRenderPlayer = true;
		DrawLightFlares = false;

		gCustomCar_Mona.Update(GetLocalPlayerVehicle(), delta);
		gCustomCar_Mona.Render(GetLocalPlayerVehicle());
	}
	void DeinitFunction() override {
		CarRender_DontRenderPlayer = false;
		DrawLightFlares = true;
	}
	bool HasTimer() override { return true; }
	bool RunWhenBlocked() override { return true; }
} E_Mona;