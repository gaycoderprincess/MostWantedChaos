namespace CustomCamera {
	bool bRunCustom = false;
	bool bReset = false;
	IVehicle* pTargetPlayerVehicle = nullptr;
	IRigidBody* pTargetPlayerBody = nullptr;
	IRBVehicle* pTargetPlayerRBVehicle = nullptr;
	IVehicle* pTargetPlayerSecondPerson = nullptr;
	IRigidBody* pTargetPlayerBodySecondPerson = nullptr;

	bool bSecondPersonOrbitMode = true;

	const float fPanSpeedBase = 0.005;

	double fMouse[2] = {};
	NyaVec3 vPos = {0, 0, 0};
	NyaVec3 vPosChange = {0, 0, 0};
	float fLookatOffset = 0.7;
	float fFollowOffset = 1.7;
	NyaVec3 vLastPlayerPosition = {0, 0, 0};
	float fMouseRotateSpeed = 1;
	float fStringMinDistance = 3;
	float fStringMaxDistance = 3;
	float fStringVelocityMult = 1;
	float fStringResetTime = 2;
	float fStringCorrectionMult = 0.5;
	float fStringMaxYDiff = -0.5;

	double fMouseTimer = -1;

	bool IsHeliCam() {
		return !strcmp(pTargetPlayerVehicle->GetVehicleName(), "copheli");
	}

	double GetMinStringDistance(IRigidBody* ply) {
		// this makes the heli stop moving??
		//if (IsHeliCam()) return 1;

		UMath::Vector3 dim;
		ply->GetDimension(&dim);
		return abs(dim.z) * fStringMinDistance;
	}

	double GetMaxStringDistance(IRigidBody* ply) {
		// this makes the heli stop moving??
		//if (IsHeliCam()) return 1;

		UMath::Vector3 dim;
		ply->GetDimension(&dim);
		return abs(dim.z) * fStringMaxDistance;
	}

	NyaVec3 GetLookatOffset(IRigidBody* ply) {
		UMath::Vector3 dim;
		ply->GetDimension(&dim);
		return {0, abs(dim.y) * fLookatOffset, 0};
	}

	NyaVec3 GetFollowOffset(IRigidBody* ply) {
		if (IsHeliCam()) return {0, -4, 0};

		UMath::Vector3 dim;
		ply->GetDimension(&dim);
		return {0, abs(dim.y) * fFollowOffset, 0};
	}

	NyaVec3* GetTargetPosition(IRigidBody* ply) {
		if (!ply) return nullptr;

		static NyaVec3 vec;
		vec = *ply->GetPosition();
		vec += GetLookatOffset(ply);
		return &vec;
	}

	NyaVec3* GetFollowPosition(IRigidBody* ply) {
		if (!ply) return nullptr;

		static NyaVec3 vec;
		vec = *ply->GetPosition();
		vec += GetFollowOffset(ply);
		return &vec;
	}

	void SetRotation(Camera* pCam) {
		auto plyPos = GetTargetPosition(pTargetPlayerBody);
		if (!plyPos) return;

		auto lookat = *plyPos - vPos;
		lookat.Normalize();
		auto mat = NyaMat4x4::LookAt(lookat);
		mat.p = vPos;
		mat = WorldToRenderMatrix(mat);
		ApplyCameraMatrix(pCam, mat);
	}

	void SetRotationSecondPerson(Camera* pCam) {
		auto plyPos = GetTargetPosition(pTargetPlayerBodySecondPerson);
		if (!plyPos) return;

		auto lookat = *plyPos - vPos;
		lookat.Normalize();
		auto mat = NyaMat4x4::LookAt(lookat);
		mat.p = vPos;
		mat = WorldToRenderMatrix(mat);
		ApplyCameraMatrix(pCam, mat);
	}

	void DoCamString() {
		auto ply = pTargetPlayerBody;
		auto minDist = GetMinStringDistance(ply);
		auto maxDist = GetMaxStringDistance(ply);

		auto plyPos = GetFollowPosition(ply);
		auto lookatFront = -(vPos - *plyPos);
		auto dist = lookatFront.length();
		lookatFront.Normalize();
		if (dist > maxDist) {
			vPos += lookatFront * dist;
			vPos -= lookatFront * maxDist;
		} else if (dist < minDist) {
			vPos += lookatFront * dist;
			vPos -= lookatFront * minDist;
		}
	}

	// alternate string code for treading towards max dist when moving the mouse
	void DoCamStringAlt() {
		auto ply = pTargetPlayerBody;
		auto maxDist = GetMaxStringDistance(ply);
		auto maxChange = vPosChange.length();

		auto plyPos = GetFollowPosition(ply);
		auto lookatFront = -(vPos - *plyPos);
		auto dist = lookatFront.length();
		lookatFront.Normalize();
		if (dist < maxDist) {
			vPos -= lookatFront * maxChange * fStringCorrectionMult;
		}

		lookatFront = -(vPos - *plyPos);
		dist = lookatFront.length();
		lookatFront.Normalize();
		if (dist > maxDist) {
			vPos += lookatFront * dist;
			vPos -= lookatFront * maxDist;
		}
	}

	void DoMovement(Camera* pCam) {
		auto player = pTargetPlayerBody;
		if (!player) return;

		auto mat = PrepareCameraMatrix(pCam);
		vPosChange = mat.x * fMouse[0] * fMouseRotateSpeed * fPanSpeedBase;
		vPosChange += mat.y * fMouse[1] * -fMouseRotateSpeed * fPanSpeedBase;
		vPos -= vPosChange;

		auto velocity = *player->GetPosition() - vLastPlayerPosition;
		if ((vPos - *GetFollowPosition(player)).length() >= GetMaxStringDistance(player) * 0.999) {
			velocity *= fStringVelocityMult;
		}

		vPos -= vLastPlayerPosition;
		vPos += *player->GetPosition();
		if (fMouseTimer <= 0) {
			vPos -= velocity;
			DoCamString();
		}
		else {
			DoCamStringAlt();
		}

		auto lookat = GetTargetPosition(player);
		if (vPos.y - lookat->y < fStringMaxYDiff) {
			vPos.y = lookat->y + fStringMaxYDiff;
		}

		vLastPlayerPosition = *player->GetPosition();

		mat.p = WorldToRenderCoords(vPos);
		ApplyCameraMatrix(pCam, mat);
	}

	void SetCameraToDefaultPos(IRigidBody* ply) {
		UMath::Matrix4 mat;
		ply->GetMatrix4(&mat);
		vPos = vLastPlayerPosition = *ply->GetPosition();
		vPos += GetFollowOffset(ply);
		vPos -= mat.z * GetMaxStringDistance(ply);
		fMouseTimer = -1;
		fMouse[0] = 0;
		fMouse[1] = 0;
	}

	void ProcessCam(Camera* cam, double delta) {
		//if (auto sensitivity = GetGameSettingByName("MouseSensitivity")) {
		//	fMouseRotateSpeed = std::lerp(0.25, 4.0, *(int*)sensitivity->value / 100.0);
		//}

		fMouseTimer -= delta;

		if (!cam) return;

		auto follow = pTargetPlayerVehicle;
		if (!IsVehicleValidAndActive(follow)) {
			bReset = true;
			pTargetPlayerVehicle = follow = GetLocalPlayerVehicle();
			if (!pTargetPlayerVehicle) return;
			pTargetPlayerBody = GetLocalPlayerInterface<IRigidBody>();
			pTargetPlayerRBVehicle = GetLocalPlayerInterface<IRBVehicle>();
		}
		if (pTargetPlayerSecondPerson && !IsVehicleValidAndActive(pTargetPlayerSecondPerson)) {
			pTargetPlayerSecondPerson = nullptr;
			pTargetPlayerBodySecondPerson = nullptr;
			bReset = true;
		}
		if (!follow || IsInLoadingScreen() || IsInNIS()) {
			bReset = true;
			return;
		}

		static bool bLastGhosting = false;
		if (bReset || (!bLastGhosting && pTargetPlayerRBVehicle->GetInvulnerability() != INVULNERABLE_NONE)) {
			SetCameraToDefaultPos(pTargetPlayerBody);
		}
		bLastGhosting = pTargetPlayerRBVehicle->GetInvulnerability() != INVULNERABLE_NONE;
		bReset = false;

		vPosChange = {0,0,0};
		if (bSecondPersonOrbitMode && pTargetPlayerSecondPerson) {
			SetRotationSecondPerson(cam);
			DoMovement(cam);
			SetRotationSecondPerson(cam);
		}
		else {
			SetRotation(cam);
			DoMovement(cam);
			SetRotation(cam);
			SetRotationSecondPerson(cam);
		}

		if (fMouse[0] != 0.0 || fMouse[1] != 0.0) {
			fMouseTimer = fStringResetTime;
		}
		fMouse[0] = 0;
		fMouse[1] = 0;
	}

	void SetTargetCar(IVehicle* pCar, IVehicle* pSecondPersonCar) {
		if (!pCar) {
			pTargetPlayerVehicle = nullptr;
			pTargetPlayerBody = nullptr;
			pTargetPlayerRBVehicle = nullptr;
			return;
		}
		if (pCar != pTargetPlayerVehicle) bReset = true;
		pTargetPlayerVehicle = pCar;
		pTargetPlayerBody = pCar->mCOMObject->Find<IRigidBody>();
		pTargetPlayerRBVehicle = pCar->mCOMObject->Find<IRBVehicle>();
		pTargetPlayerSecondPerson = pSecondPersonCar;
		pTargetPlayerBodySecondPerson = pSecondPersonCar ? pSecondPersonCar->mCOMObject->Find<IRigidBody>() : nullptr;
	}
}