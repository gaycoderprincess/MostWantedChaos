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

struct tCustomCarPart {
	std::string sModelName;
	bool bPartDetachable = false;
	bool bPartHinged = false;
	UMath::Vector3 RestPosition = {0,0,0};
	UMath::Vector3 HingePosition = {0,0,0};
	UMath::Vector3 HingeMin = {0,0,0};
	UMath::Vector3 HingeMax = {0,0,0};
	UMath::Vector3 CrashNormal = {0,0,0};
	std::string sParentName;
	std::vector<Render3D::tModel*> aModels;

	bool bIsDetached = false;
	bool bIsUnlatched = false;
	UMath::Matrix4 LastRawTransform = UMath::Matrix4::kIdentity;
	UMath::Matrix4 DetachedTransform = UMath::Matrix4::kIdentity;
	NyaVec3 DetachedVelocity = {0,0,0};
	NyaVec3 UnlatchedRotation = {0,0,0};
	NyaVec3 UnlatchedVelocity = {0,0,0};

	tCustomCarPart(const std::string& name, bool detachable, UMath::Vector3 restPosition, bool hinged = false, UMath::Vector3 hingePosition = {0,0,0}, UMath::Vector3 hingeMin = {0,0,0}, UMath::Vector3 hingeMax = {0,0,0}, const std::string& parentName = "") : sModelName(name), bPartDetachable(detachable), RestPosition(restPosition), bPartHinged(hinged), HingePosition(hingePosition), HingeMin(hingeMin), HingeMax(hingeMax), sParentName(parentName) {
		auto temp = RestPosition;
		RestPosition.x = temp.x;
		RestPosition.y = -temp.z;
		RestPosition.z = -temp.y;

		temp = HingePosition;
		HingePosition.x = temp.x;
		HingePosition.y = -temp.z;
		HingePosition.z = -temp.y;

		HingeMin.x *= 0.01745329;
		HingeMin.y *= 0.01745329;
		HingeMin.z *= 0.01745329;

		HingeMax.x *= 0.01745329;
		HingeMax.y *= 0.01745329;
		HingeMax.z *= 0.01745329;

		CrashNormal = RestPosition;
		if (CrashNormal.length() > 0) CrashNormal.Normalize();
	}

	void Reset() {
		bIsDetached = false;
		bIsUnlatched = false;
		LastRawTransform = UMath::Matrix4::kIdentity;
		DetachedTransform = UMath::Matrix4::kIdentity;
		DetachedVelocity = {0,0,0};
		UnlatchedRotation = {0,0,0};
		UnlatchedVelocity = {0,0,0};
	}

	tCustomCarPart* GetParent(tCustomCarPart* list) {
		if (sParentName.empty()) return nullptr;

		while (true) {
			if (list->sModelName == sParentName) return list;
			list++;
		}
	}

	static inline float doorTest = 45;
	void Render(NyaMat4x4 carMatrix, double delta, bool windows) {
		if (aModels.empty() || aModels[0]->bInvalidated) {
			aModels = Render3D::CreateModels(sModelName);
		}

		for (auto& model : aModels) {
			if (windows != (model->sTextureName == "windows.dds")) continue;

			if (bIsDetached) {
				if (DetachedTransform.p.y > -500) {
					model->RenderAt(WorldToRenderMatrix(DetachedTransform), model->sTextureName == "windows.dds");
				}
			}
			else {
				UMath::Matrix4 offset;
				if (bIsUnlatched) {
					auto restToHinge = RestPosition - HingePosition;
					UMath::Matrix4 latchOffset;
					latchOffset.Rotate(UnlatchedRotation);
					offset = (UMath::Matrix4)(offset * latchOffset);
					auto latchPos = latchOffset * restToHinge;
					offset.p = HingePosition + latchPos;
				} else {
					offset.p = RestPosition;
				}
				auto partMat = (UMath::Matrix4)(carMatrix * offset);
				LastRawTransform = partMat;
				model->RenderAt(WorldToRenderMatrix(partMat), model->sTextureName == "windows.dds");
			}
		}
	}

	static inline float latchBreakageThreshold = 10;
	bool CheckHingeBroken() {
		if (HingeMax.x != 0 && UnlatchedRotation.x > HingeMax.x && UnlatchedVelocity.x > latchBreakageThreshold) return true;
		if (HingeMax.y != 0 && UnlatchedRotation.y > HingeMax.y && UnlatchedVelocity.y > latchBreakageThreshold) return true;
		if (HingeMax.z != 0 && UnlatchedRotation.z > HingeMax.z && UnlatchedVelocity.z > latchBreakageThreshold) return true;
		if (HingeMin.x != 0 && UnlatchedRotation.x < HingeMin.x && UnlatchedVelocity.x < -latchBreakageThreshold) return true;
		if (HingeMin.y != 0 && UnlatchedRotation.y < HingeMin.y && UnlatchedVelocity.y < -latchBreakageThreshold) return true;
		if (HingeMin.z != 0 && UnlatchedRotation.z < HingeMin.z && UnlatchedVelocity.z < -latchBreakageThreshold) return true;
		return false;
	}

	static inline float latchMoveFactor = 1;
	static inline float latchRotateFactor = 15;
	static inline float latchBounceFactor = 0.5;
	void Update(NyaVec3 absPlayerVelocity, NyaVec3 playerVelocity, NyaVec3 angularVelocity, double delta) {
		if (bIsDetached && DetachedTransform.p.y > -500) {
			DetachedVelocity.y -= 9.8 * delta;
			DetachedTransform.p += DetachedVelocity * delta;
		}
		if (bIsUnlatched) {
			UnlatchedVelocity.x += playerVelocity.z * latchMoveFactor * delta;
			UnlatchedVelocity.y += playerVelocity.y * latchMoveFactor * delta;
			UnlatchedVelocity.z += playerVelocity.x * latchMoveFactor * delta;
			UnlatchedVelocity.z += angularVelocity.y * latchRotateFactor * delta;

			UnlatchedRotation += UnlatchedVelocity * delta;
			if (CheckHingeBroken()) Detach(absPlayerVelocity);
			else {
				if (UnlatchedRotation.x > HingeMax.x) { UnlatchedRotation.x = HingeMax.x; UnlatchedVelocity.x *= -latchBounceFactor; }
				if (UnlatchedRotation.y > HingeMax.y) { UnlatchedRotation.y = HingeMax.y; UnlatchedVelocity.y *= -latchBounceFactor; }
				if (UnlatchedRotation.z > HingeMax.z) { UnlatchedRotation.z = HingeMax.z; UnlatchedVelocity.z *= -latchBounceFactor; }
				if (UnlatchedRotation.x < HingeMin.x) { UnlatchedRotation.x = HingeMin.x; UnlatchedVelocity.x *= -latchBounceFactor; }
				if (UnlatchedRotation.y < HingeMin.y) { UnlatchedRotation.y = HingeMin.y; UnlatchedVelocity.y *= -latchBounceFactor; }
				if (UnlatchedRotation.z < HingeMin.z) { UnlatchedRotation.z = HingeMin.z; UnlatchedVelocity.z *= -latchBounceFactor; }
			}
		}
	}

	void UpdateChild(tCustomCarPart* parent) {
		if (parent->bIsDetached && !bIsDetached) Detach(parent->DetachedVelocity);
		bIsUnlatched = parent->bIsUnlatched;
		UnlatchedRotation = parent->UnlatchedRotation;
		UnlatchedVelocity = parent->UnlatchedVelocity;
	}

	void Detach(NyaVec3 playerVelocity) {
		bIsDetached = true;
		DetachedTransform = LastRawTransform;
		DetachedVelocity = playerVelocity;
	}

	static inline float detachThreshold = 16.0;
	static inline float unlatchThreshold = 2.0;
	static inline UMath::Vector3 lastPassedColNorm = {0,0,0};
	void OnCollision(UMath::Vector3 colVector, UMath::Vector3 playerVelocity, float collisionStrength) {
		if (bIsDetached) return;

		UMath::Vector3 colNorm = colVector;
		colNorm.Normalize();
		if (bPartDetachable && colNorm.Dot(CrashNormal) > 0.8 && collisionStrength > detachThreshold) {
			lastPassedColNorm = colNorm;
			Detach(playerVelocity);
		}
		else if (bPartHinged && colNorm.Dot(CrashNormal) > 0.5 && collisionStrength > unlatchThreshold) {
			bIsUnlatched = true;
		}
	}
};

class Effect_Pep : public ChaosEffect {
public:
	Effect_Pep() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Pep";
		sFriendlyName = "Change Car To FO1 Pepper";
		fTimerLength = 120;
	}

	static inline tCustomCarPart aCarParts[] = {
		tCustomCarPart("pep/body.fbx", false, {0,0,0}),
		tCustomCarPart("pep/coolingfan_1.fbx", false, {-0.01, 1.56308, 0.415157}),
		tCustomCarPart("pep/dash_lo.fbx", false, {0,0,0}),
		tCustomCarPart("pep/door_l.fbx", true, {-0.6605, 0.17438, 0.65242}, true, {-0.692432, 0.68049, 0.338087}, {0,0,0}, {0,0,60}),
		tCustomCarPart("pep/door_r.fbx", true, {0.6605, 0.17438, 0.65242}, true, {0.692432, 0.68049, 0.338087}, {0,0,-60}, {0,0,0}),
		tCustomCarPart("pep/driveshaft.fbx", false, {-0.000871, 0.776944, 0.134779}),
		tCustomCarPart("pep/engine_1.fbx", false, {0.034055, 1.09684, 0.353421}),
		tCustomCarPart("pep/exhaust_l.fbx", false, {-0.3485, -1.3371, 0.134766}),
		tCustomCarPart("pep/hood.fbx", true, {0, 1.23717, 0.696299}, true, {0, 0.788376, 0.703607}, {0,0,0}, {80,0,0}),
		tCustomCarPart("pep/nitro.fbx", false, {0.226277, -1.54027, 0.392936}),
		tCustomCarPart("pep/part_1_grille.fbx", true, {0, 1.70436, 0.497551}),
		tCustomCarPart("pep/part_2_mask.fbx", true, {0, 1.68417, 0.450264}),
		tCustomCarPart("pep/part_3_frontspoiler.fbx", true, {0, 1.57301, 0.154975}),
		tCustomCarPart("pep/part_4_rearbumper.fbx", true, {0, -1.60887, 0.181289}),
		tCustomCarPart("pep/part_5_frontbumper.fbx", true, {0, 1.707, 0.246704}), // todo rotate 180 degrees
		tCustomCarPart("pep/part_6_roofspoiler.fbx", true, {0, -1.11109, 1.13381}),
		tCustomCarPart("pep/part_7_mirror_l.fbx", true, {-0.754491, 0.454667, 0.742712}, true, {-0.692432, 0.68049, 0.338087}, {0,0,0}, {0,0,60}, "pep/door_l.fbx"),
		tCustomCarPart("pep/part_8_mirror_r.fbx", true, {0.754491, 0.454667, 0.742712}, true, {0.692432, 0.68049, 0.338087}, {0,0,-60}, {0,0,0}, "pep/door_r.fbx"),
		tCustomCarPart("pep/part_9_fenderleft.fbx", true, {-0.691875, 1.16569, 0.384644}),
		tCustomCarPart("pep/part_10_fenderright.fbx", true, {0.691875, 1.16569, 0.384644}),
		tCustomCarPart("pep/part_11_wheelarc_l.fbx", true, {-0.489442, 1.15724, 0.318175}),
		tCustomCarPart("pep/part_12_wheelarc_r.fbx", true, {0.489442, 1.15724, 0.318175}),
		tCustomCarPart("pep/part_13_lightframe.fbx", true, {0, 1.66222, 0.489103}),
		//tCustomCarPart("pep/steering_wheel_lo.fbx", false, {}), // todo!
		tCustomCarPart("pep/susp_rear.fbx", false, {-0.000909, -1.07368, 0.13446}),
		tCustomCarPart("pep/trunk.fbx", true, {0, -1.6189, 0.583304}, true, {0, -1.49874, 0.786098}, {-45,0,0}, {0,0,0}),
	};

	std::vector<Render3D::tModel*> model_tire_l;
	std::vector<Render3D::tModel*> model_tire_r;

	static inline float rX = 180;
	static inline float rY = 0;
	static inline float rZ = 0;
	static inline float offX = 0;
	static inline float offY = 0.01;
	static inline float offZ = 0;
	static inline float wheelOffY = -0.1;

	UMath::Vector3 LastPlayerVelocity = {0,0,0};

	double fWheelState[4] = {};

	void InitFunction() override {
		for (auto& part : aCarParts) {
			part.Reset();
		}
		CarRender_DontRenderPlayer = true;
		DrawLightFlares = false;
		LastPlayerVelocity = {0,0,0};
	}
	void TickFunction3D(double delta) override {
		auto veh = GetLocalPlayerInterface<IRigidBody>();
		auto currentPlayerVelocity = *veh->GetLinearVelocity();
		if (IsInLoadingScreen() || IsInNIS()) {
			for (auto& part : aCarParts) {
				part.Reset();
			}
			LastPlayerVelocity = currentPlayerVelocity;
		}

		if (model_tire_l.empty() || model_tire_l[0]->bInvalidated) {
			model_tire_l = Render3D::CreateModels("pep_wheel_l.fbx");
		}
		if (model_tire_r.empty() || model_tire_r[0]->bInvalidated) {
			model_tire_r = Render3D::CreateModels("pep_wheel_r.fbx");
		}

		if (IsCarDestroyed(GetLocalPlayerVehicle())) {
			for (auto& part : aCarParts) {
				if (!part.bPartDetachable) continue;
				if (part.bIsDetached) continue;
				part.Detach(*veh->GetLinearVelocity());
			}
		}

		if (LastPlayerVelocity.length() > currentPlayerVelocity.length() && currentPlayerVelocity.length() > 0.1) {
			auto colVelocity = (LastPlayerVelocity - currentPlayerVelocity);
			colVelocity = GetRelativeCarOffset(GetLocalPlayerVehicle(), colVelocity);
			colVelocity *= -1;
			for (auto& part : aCarParts) {
				part.OnCollision((UMath::Vector3)colVelocity, currentPlayerVelocity, colVelocity.length());
			}
		}
		LastPlayerVelocity = currentPlayerVelocity;

		for (auto& part : aCarParts) {
			if (auto parent = part.GetParent(aCarParts)) {
				part.UpdateChild(parent);
			}
			else {
				part.Update(currentPlayerVelocity, GetRelativeCarOffset(GetLocalPlayerVehicle(), currentPlayerVelocity), GetRelativeCarOffset(GetLocalPlayerVehicle(), *veh->GetAngularVelocity()), delta);
			}
		}

		auto sus = GetLocalPlayerInterface<ISuspension>();
		if (!FEManager::mPauseRequest) {
			for (int i = 0; i < 4; i++) {
				fWheelState[i] += sus->GetWheelAngularVelocity(i) * delta;
			}
		}

		auto mat = UMath::Matrix4::kIdentity;
		veh->GetMatrix4(&mat);
		mat.p = *veh->GetPosition();
		mat.p.y += sus->GetWheelLocalPos(0)->y;
		mat.p += mat.x * offX;
		mat.p += mat.y * offY;
		mat.p += mat.z * offZ;
		UMath::Matrix4 rotation;
		rotation.Rotate(NyaVec3(rX * 0.01745329, rY * 0.01745329, rZ * 0.01745329));
		mat = (UMath::Matrix4)(mat * rotation);
		for (auto& part : aCarParts) {
			part.Render(mat, delta, false);
		}
		for (auto& part : aCarParts) {
			part.Render(mat, delta, true);
		}

		NyaVec3 wheelOffsets[] = {
				{-0.006627, 0.0, -0.011281}, // fl
				{0.006627, 0.0, -0.011281}, // fr
				{-0.006639, 0.0, 0.011069}, // rl
				{0.006639, 0.0, 0.011069}, // rr
		};
		for (int i = 0; i < 4; i++) {
			std::vector<Render3D::tModel*>* models = i % 2 == 0 ? &model_tire_l : &model_tire_r;
			for (auto& model : *models) {
				UMath::Matrix4 rotation;
				rotation.Rotate(NyaVec3(-fWheelState[i], 0, sus->GetWheelSteer(i) * 4));
				rotation.p = wheelOffsets[i] * 100;
				//rotation.p.y = -sus->GetWheelLocalPos(i)->y;
				rotation.p.y += wheelOffY;
				auto wheelMat = (UMath::Matrix4)(mat * rotation);
				model->RenderAt(WorldToRenderMatrix(wheelMat));
			}
		}
	}
	void DeinitFunction() override {
		CarRender_DontRenderPlayer = false;
		DrawLightFlares = true;
	}
	bool HasTimer() override { return true; }
	bool RunWhenBlocked() override { return true; }
} E_Pep;