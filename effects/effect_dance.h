/*class Effect_DanceGame : public ChaosEffect {
public:
	double state = 0;
	bool goBack = false;

	static inline float GroovySpeed = 4.5;

	Effect_DanceGame() : ChaosEffect("Uncategorized") {
		sName = "Why's This Dealer?";
		fTimerLength = 60;
		AddToIncompatiblityGroup("car_scale");
		bCanQuickTrigger = false;
	}

	static inline NyaMat4x4 mCameraMatrix = {};
	static inline uint16_t nCameraFOV = 0;
	static inline NyaMat4x4 mCarMatrix = {};
	static inline NyaVec3 vPlayerCarPos = {0,0,0};
	static inline float fYOffset = -0.6;

	void InitFunction() override {
		SetSoundControlState(true, SNDSTATE_STOP_MUSIC);

		state = 0;

		auto cam = std::ifstream("CwoeeChaos/data/captures/dance_camera.dat", std::ios::in | std::ios::binary);
		if (cam.is_open()) {
			cam.read((char*)&mCameraMatrix, sizeof(mCameraMatrix));
			cam.read((char*)&nCameraFOV, sizeof(nCameraFOV));
		}
		auto car = std::ifstream("CwoeeChaos/data/captures/dance_car.dat", std::ios::in | std::ios::binary);
		if (car.is_open()) {
			car.read((char*)&mCarMatrix, sizeof(mCarMatrix));
		}

		FEManager::mPauseRequest = 1;
		DrawLightFlares = false;
		if (auto ply = GetLocalPlayerVehicle()) {
			vPlayerCarPos = WorldToRenderCoords(*ply->GetPosition());
		}
	}
	void TickFunction(eChaosHook hook, double delta) override {
		if (hook != HOOK_CAMERA) return;

		auto tmp = mCameraMatrix;
		tmp.p += vPlayerCarPos;
		tmp.p.z += fYOffset;
		ApplyCameraMatrix(pMoverCamera, tmp);
		if (nCameraFOV) pMoverCamera->CurrentKey.FieldOfView = nCameraFOV;
	}
	void TickFunctionMain(double delta) override {
		if (auto ply = GetLocalPlayerVehicle()) {
			vPlayerCarPos = WorldToRenderCoords(*ply->GetPosition());
		}

		auto tmp = mCarMatrix;
		tmp.p += vPlayerCarPos;
		tmp.p.z += fYOffset;
		CarRender_ForceMatrix = tmp;

		state += delta * (goBack ? -GroovySpeed : GroovySpeed);
		if (state > 1) goBack = true;
		if (state < 0) goBack = false;

		CarScaleMatrix = UMath::Matrix4::kIdentity;

		CarScaleMatrix.x.x *= 1 + (easeInOutQuart(1 - state) * 0.33);
		CarScaleMatrix.x.y *= 1 + (easeInOutQuart(1 - state) * 0.33);
		CarScaleMatrix.x.z *= 1 + (easeInOutQuart(1 - state) * 0.33);
		CarScaleMatrix.y.x *= 1 + (easeInOutQuart(1 - state) * 0.33);
		CarScaleMatrix.y.y *= 1 + (easeInOutQuart(1 - state) * 0.33);
		CarScaleMatrix.y.z *= 1 + (easeInOutQuart(1 - state) * 0.33);
		CarScaleMatrix.z.x *= 1 + (easeInOutQuart(state) * 0.33);
		CarScaleMatrix.z.y *= 1 + (easeInOutQuart(state) * 0.33);
		CarScaleMatrix.z.z *= 1 + (easeInOutQuart(state) * 0.33);
	}
	void DeinitFunction() override {
		SetSoundControlState(false, SNDSTATE_STOP_MUSIC);

		CarScaleMatrix = UMath::Matrix4::kIdentity;
		CarRender_ForceMatrix = {};
		DrawLightFlares = true;
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
	bool RunWhenBlocked() override { return true; }
} E_DanceGame;*/