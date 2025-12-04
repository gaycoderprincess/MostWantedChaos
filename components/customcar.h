class CustomCarPart {
public:
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
	NyaVec3 DetachedTurnVelocity = {0,0,0};
	NyaVec3 UnlatchedRotation = {0,0,0};
	NyaVec3 UnlatchedVelocity = {0,0,0};
	NyaVec3 InitialDetachTurnVelocity = {0,0,0};

	CustomCarPart(const std::string& name, bool detachable, UMath::Vector3 restPosition, bool hinged = false, UMath::Vector3 hingePosition = {0,0,0}, UMath::Vector3 hingeMin = {0,0,0}, UMath::Vector3 hingeMax = {0,0,0}, const std::string& parentName = "") : sModelName(name), bPartDetachable(detachable), RestPosition(restPosition), bPartHinged(hinged), HingePosition(hingePosition), HingeMin(hingeMin), HingeMax(hingeMax), sParentName(parentName) {
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

	virtual void Reset() {
		bIsDetached = false;
		bIsUnlatched = false;
		LastRawTransform = UMath::Matrix4::kIdentity;
		DetachedTransform = UMath::Matrix4::kIdentity;
		DetachedVelocity = {0,0,0};
		DetachedTurnVelocity = {0,0,0};
		UnlatchedRotation = {0,0,0};
		UnlatchedVelocity = {0,0,0};
		InitialDetachTurnVelocity = {0,0,0};
	}

	void Load() {
		if (aModels.empty() || aModels[0]->bInvalidated) {
			aModels = Render3D::CreateModels(sModelName);
		}
	}

	virtual void Render(IVehicle* playerCar, NyaMat4x4 carMatrix, bool windows) {
		Load();

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

	static inline float latchBreakageThreshold = 10; // todo this needs tweaked
	bool CheckHingeBroken() {
		if (HingeMax.x != 0 && UnlatchedRotation.x > HingeMax.x && UnlatchedVelocity.x > latchBreakageThreshold) return true;
		if (HingeMax.y != 0 && UnlatchedRotation.y > HingeMax.y && UnlatchedVelocity.y > latchBreakageThreshold) return true;
		if (HingeMax.z != 0 && UnlatchedRotation.z > HingeMax.z && UnlatchedVelocity.z > latchBreakageThreshold) return true;
		if (HingeMin.x != 0 && UnlatchedRotation.x < HingeMin.x && UnlatchedVelocity.x < -latchBreakageThreshold) return true;
		if (HingeMin.y != 0 && UnlatchedRotation.y < HingeMin.y && UnlatchedVelocity.y < -latchBreakageThreshold) return true;
		if (HingeMin.z != 0 && UnlatchedRotation.z < HingeMin.z && UnlatchedVelocity.z < -latchBreakageThreshold) return true;
		return false;
	}

	static inline float latchMoveFactor[3] = {3.0, 3.0, 1.0};
	static inline float latchInitialDetachFactor = 0.01;
	static inline float latchBounceFactor = 0.5;
	static inline float latchDecayFactor = 3.0;
	virtual void Update(IVehicle* playerCar, const NyaVec3& absPlayerVelocity, const NyaVec3& playerVelocityChange, double delta) {
		InitialDetachTurnVelocity.x = playerVelocityChange.z * latchInitialDetachFactor;
		InitialDetachTurnVelocity.y = playerVelocityChange.y * latchInitialDetachFactor;
		InitialDetachTurnVelocity.z = playerVelocityChange.x * latchInitialDetachFactor;
		if (bIsDetached && DetachedTransform.p.y > -500) {
			DetachedVelocity.y -= 9.8 * delta;
			DetachedTurnVelocity *= 1.0 - (latchDecayFactor * delta);
			DetachedTransform.Rotate(DetachedTurnVelocity * delta);
			DetachedTransform.p += DetachedVelocity * delta;
		}
		if (!bIsDetached && bIsUnlatched) {
			UnlatchedVelocity.x += playerVelocityChange.z * latchMoveFactor[0] * delta;
			UnlatchedVelocity.y += playerVelocityChange.y * latchMoveFactor[1] * delta;
			UnlatchedVelocity.z += playerVelocityChange.x * latchMoveFactor[2] * delta;
			UnlatchedVelocity *= 1.0 - (latchDecayFactor * delta);

			UnlatchedRotation += UnlatchedVelocity * delta;
			if (CheckHingeBroken()) {
				Detach(absPlayerVelocity);
				DetachedTurnVelocity = UnlatchedVelocity;
			}
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

	void UpdateChild(CustomCarPart* parent) {
		if (parent->bIsDetached && !bIsDetached) Detach(parent->DetachedVelocity);
		bIsUnlatched = parent->bIsUnlatched;
		UnlatchedRotation = parent->UnlatchedRotation;
		UnlatchedVelocity = parent->UnlatchedVelocity;
	}

	void Detach(const NyaVec3& playerVelocity) {
		bIsDetached = true;
		DetachedTransform = LastRawTransform;
		DetachedVelocity = playerVelocity;
		DetachedTurnVelocity = InitialDetachTurnVelocity;
	}

	static inline float detachThreshold = 16.0;
	static inline float unlatchThreshold = 2.0;
	static inline UMath::Vector3 lastPassedCol = {0,0,0};
	void OnCollision(UMath::Vector3 colVector, UMath::Vector3 playerVelocity, float collisionStrength) {
		if (bIsDetached) return;

		UMath::Vector3 colNorm = colVector;
		colNorm.Normalize();
		if (bPartDetachable && colNorm.Dot(CrashNormal) > 0.8 && collisionStrength > detachThreshold) {
			lastPassedCol = colVector;
			Detach(playerVelocity);
		}
		else if (bPartHinged && colNorm.Dot(CrashNormal) > 0.5 && collisionStrength > unlatchThreshold) {
			bIsUnlatched = true;
		}
	}
};

class CustomCarTire : public CustomCarPart {
public:
	int nWheelID = 0;
	float fRotation = 0;
	float fRadius = 0;
	float fSuspensionMaxLength = 0.35;

	CustomCarTire(int wheelID, const std::string& name, UMath::Vector3 restPosition, float radius, float suspMaxLength = 0.35) : CustomCarPart(name, false, restPosition), nWheelID(wheelID), fRadius(radius), fSuspensionMaxLength(suspMaxLength) { }

	void Reset() override {
		CustomCarPart::Reset();
		fRotation = 0;
	}

	void Render(IVehicle* playerCar, NyaMat4x4 carMatrix, bool windows) override {
		Load();

		auto sus = playerCar->mCOMObject->Find<ISuspension>();
		for (auto& model : aModels) {
			UMath::Matrix4 rotation;
			rotation.Rotate(NyaVec3(-fRotation, 0, sus->GetWheelSteer(nWheelID) * 4));
			rotation.p = RestPosition;

			auto wheelPos = *sus->GetWheelPos(nWheelID);
			auto wheelPos2 = *sus->GetWheelLocalPos(nWheelID);
			playerCar->mCOMObject->Find<IRigidBody>()->ConvertWorldToLocal(&wheelPos, true);

			//rotation.p.y = -sus->GetWheelLocalPos(i)->y;
			//rotation.p += vMoveOffset;
			//rotation.p.y = vMoveOffset.y + wheelPos.y;
			if (sus->IsWheelOnGround(nWheelID)) {
				rotation.p.y = -fRadius + (wheelPos2.y - wheelPos.y);
			}
			else {
				float f = -sus->GetWheelRoadHeight(nWheelID);
				if (f > fSuspensionMaxLength) f = fSuspensionMaxLength;
				rotation.p.y = -fRadius + f;
			}
			auto partMat = (UMath::Matrix4)(carMatrix * rotation);
			LastRawTransform = partMat;
			model->RenderAt(WorldToRenderMatrix(partMat));
		}
	}

	void Update(IVehicle* playerCar, const NyaVec3& absPlayerVelocity, const NyaVec3& playerVelocityChange, double delta) override {
		CustomCarPart::Update(playerCar, absPlayerVelocity, playerVelocityChange, delta);

		auto sus = playerCar->mCOMObject->Find<ISuspension>();
		fRotation += sus->GetWheelAngularVelocity(nWheelID) * delta;
	}
};

class CustomCar {
public:
	std::string sBasePath;
	std::vector<CustomCarPart*> aParts;

	UMath::Vector3 vRotateOffset = {180,0,0};
	UMath::Vector3 vMoveOffset = {0,0.01,0};

	UMath::Vector3 vLastVelocity = {0,0,0};
	UMath::Vector3 vLastRelativeVelocity = {0,0,0};

	std::string MakeRelativeModelPath(std::string modelPath) {
		return std::format("{}/{}.fbx", sBasePath, modelPath);
	}

	CustomCar(const std::string& basePath, const UMath::Vector3& rotateOffset, const UMath::Vector3& moveOffset, const std::vector<CustomCarPart*>& partList) : sBasePath(basePath), vRotateOffset(rotateOffset), vMoveOffset(moveOffset), aParts(partList) {
		for (auto& part : aParts) {
			part->sModelName = MakeRelativeModelPath(part->sModelName);
		}
		vRotateOffset *= 0.01745329;
	}

	void Load() {
		for (auto part : aParts) {
			part->Load();
		}
	}

	void Render(IVehicle* parentCar) {
		if (!DrawCars) return;

		Load();

		auto rb = parentCar->mCOMObject->Find<IRigidBody>();
		auto sus = parentCar->mCOMObject->Find<ISuspension>();

		auto mat = UMath::Matrix4::kIdentity;
		rb->GetMatrix4(&mat);
		mat.p = *rb->GetPosition();
		mat.p.y += sus->GetWheelLocalPos(0)->y;
		mat.p += mat.x * vMoveOffset.x;
		mat.p += mat.y * vMoveOffset.y;
		mat.p += mat.z * vMoveOffset.z;
		UMath::Matrix4 rotation;
		rotation.Rotate(vRotateOffset);
		mat = (UMath::Matrix4)(mat * rotation);
		for (auto& part : aParts) {
			part->Render(parentCar, mat, false);
		}
		for (auto& part : aParts) {
			part->Render(parentCar, mat, true);
		}
	}

	CustomCarPart* GetParentPart(CustomCarPart* child) {
		if (child->sParentName.empty()) return nullptr;

		for (auto& part : aParts) {
			if (part->sModelName == child->sParentName) return part;
		}
		return nullptr;
	}

	void Update(IVehicle* parentCar, double delta) {
		auto veh = parentCar->mCOMObject->Find<IRigidBody>();
		auto currentVelocity = *veh->GetLinearVelocity();
		if (IsInLoadingScreen() || IsInNIS()) {
			for (auto& part : aParts) {
				part->Reset();
			}
			vLastVelocity = currentVelocity;
			return;
		}
		if (FEManager::mPauseRequest) return;

		if (IsCarDestroyed(parentCar)) {
			for (auto& part : aParts) {
				if (!part->bPartDetachable) continue;
				if (part->bIsDetached) continue;
				part->Detach(*veh->GetLinearVelocity());
			}
		}

		for (auto& part : aParts) {
			if (auto parent = GetParentPart(part)) {
				part->UpdateChild(parent);
			}
			else {
				part->Update(parentCar, currentVelocity, (GetRelativeCarOffset(parentCar, currentVelocity) - GetRelativeCarOffset(parentCar, vLastVelocity)) / delta, delta);
			}
		}

		if (vLastVelocity.length() > currentVelocity.length() && currentVelocity.length() > 0.1) {
			auto colVelocity = (vLastVelocity - currentVelocity);
			colVelocity = GetRelativeCarOffset(parentCar, colVelocity);
			colVelocity.y *= -1;
			colVelocity.z *= -1;
			for (auto& part : aParts) {
				part->OnCollision((UMath::Vector3)colVelocity, currentVelocity, colVelocity.length());
			}
		}
		vLastVelocity = currentVelocity;
	}

	void Reset() {
		for (auto& part : aParts) {
			part->Reset();
		}
		vLastVelocity = {0,0,0};
	}
};
auto gCustomCar_Pepper = CustomCar("pep", {180,0,0}, {0,0.01,0}, {
	new CustomCarPart("body", false, {0,0,0}),
	new CustomCarPart("coolingfan_1", false, {-0.01, 1.56308, 0.415157}),
	new CustomCarPart("dash_lo", false, {0,0,0}),
	new CustomCarPart("door_l", true, {-0.6605, 0.17438, 0.65242}, true, {-0.692432, 0.68049, 0.338087}, {0,0,0}, {0,0,60}),
	new CustomCarPart("door_r", true, {0.6605, 0.17438, 0.65242}, true, {0.692432, 0.68049, 0.338087}, {0,0,-60}, {0,0,0}),
	new CustomCarPart("driveshaft", false, {-0.000871, 0.776944, 0.134779}),
	new CustomCarPart("engine_1", false, {0.034055, 1.09684, 0.353421}),
	new CustomCarPart("exhaust_l", false, {-0.3485, -1.3371, 0.134766}),
	new CustomCarPart("hood", true, {0, 1.23717, 0.696299}, true, {0, 0.788376, 0.703607}, {0,0,0}, {80,0,0}),
	new CustomCarPart("nitro", false, {0.226277, -1.54027, 0.392936}),
	new CustomCarPart("part_1_grille", true, {0, 1.70436, 0.497551}),
	new CustomCarPart("part_2_mask", true, {0, 1.68417, 0.450264}),
	new CustomCarPart("part_3_frontspoiler", true, {0, 1.57301, 0.154975}),
	new CustomCarPart("part_4_rearbumper", true, {0, -1.60887, 0.181289}),
	new CustomCarPart("part_5_frontbumper", true, {0, 1.707, 0.246704}),
	new CustomCarPart("part_6_roofspoiler", true, {0, -1.11109, 1.13381}),
	new CustomCarPart("part_7_mirror_l", true, {-0.754491, 0.454667, 0.742712}, true, {-0.692432, 0.68049, 0.338087}, {0,0,0}, {0,0,60}, "door_l"),
	new CustomCarPart("part_8_mirror_r", true, {0.754491, 0.454667, 0.742712}, true, {0.692432, 0.68049, 0.338087}, {0,0,-60}, {0,0,0}, "door_r"),
	new CustomCarPart("part_9_fenderleft", true, {-0.691875, 1.16569, 0.384644}),
	new CustomCarPart("part_10_fenderright", true, {0.691875, 1.16569, 0.384644}),
	new CustomCarPart("part_11_wheelarc_l", true, {-0.489442, 1.15724, 0.318175}),
	new CustomCarPart("part_12_wheelarc_r", true, {0.489442, 1.15724, 0.318175}),
	new CustomCarPart("part_13_lightframe", true, {0, 1.66222, 0.489103}),
	//CustomCarPart("steering_wheel_lo", false, {}), // todo!
	new CustomCarPart("susp_rear", false, {-0.000909, -1.07368, 0.13446}),
	new CustomCarPart("trunk", true, {0, -1.6189, 0.583304}, true, {0, -1.49874, 0.786098}, {-45,0,0}, {0,0,0}),
	new CustomCarTire(0, "tire_l", {-0.662652, 1.1281, 0.154147}, 0.29),
	new CustomCarTire(1, "tire_r", {0.662652, 1.1281, 0.154147}, 0.29),
	new CustomCarTire(2, "tire_l", {-0.663914, -1.10693, 0.142167}, 0.29),
	new CustomCarTire(3, "tire_r", {0.663914, -1.10693, 0.142167}, 0.29),
});