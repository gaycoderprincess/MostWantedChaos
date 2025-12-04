#define EFFECT_CATEGORY_TEMP "3D Render"

class Effect_Shark : public ChaosEffect {
public:
	Effect_Shark() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Friends Of Blahaj";
		sFriendlyName = "Blahaj Roof Racks";
		fTimerLength = 60;
	}

	std::vector<Render3D::tModel*> models;

	static inline float rX = 0;
	static inline float rY = 180;
	static inline float rZ = 90;
	static inline float offX = 0;
	static inline float offY = -0.1;
	static inline float offZ = -0.5;
	static inline float scale = 0.5;

	void TickFunction3D(double delta) override {
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
} E_Shark;

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
	}

	void InitFunction() override {
		gCustomCar_Pepper.Reset();
		CarRender_DontRenderPlayer = true;
		DrawLightFlares = false;
	}
	void TickFunction3D(double delta) override {
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