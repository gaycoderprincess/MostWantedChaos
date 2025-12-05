#define EFFECT_CATEGORY_TEMP "Camera"

/*class Effect_LiftCamera : public ChaosEffect {
public:
	Effect_LiftCamera() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Lift Camera Up";
		fTimerLength = 30;
		IncompatibilityGroups.push_back(Attrib::StringHash32("camera_height"));
	}

	static inline float LiftOffset = 3;

	void TickFunctionCamera(Camera* pCamera, double delta) override {
		auto camMatrix = PrepareCameraMatrix();
		camMatrix.p.z += LiftOffset;
		ApplyCameraMatrix(camMatrix);
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
} E_LiftCamera;

class Effect_LowerCamera : public ChaosEffect {
public:
	Effect_LowerCamera() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Low Camera Height";
		fTimerLength = 60;
		IncompatibilityGroups.push_back(Attrib::StringHash32("camera_height"));
	}

	static inline float LiftOffset = -0.4;

	void TickFunctionCamera(Camera* pCamera, double delta) override {
		auto camMatrix = PrepareCameraMatrix();
		camMatrix.p.z += LiftOffset;
		ApplyCameraMatrix(camMatrix);
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
} E_LowerCamera;

class Effect_RoofCamera : public ChaosEffect {
public:
	Effect_RoofCamera() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "00Roof Camera";
		fTimerLength = 60;
		ActivateIncompatibilityGroups.push_back(Attrib::StringHash32("camera_height"));
	}

	static inline float RoofOffset = 0.5;

	void TickFunctionCamera(Camera* pCamera, double delta) override {
		auto camMatrix = PrepareCameraMatrix();
		UMath::Matrix4 playerMatrix;
		GetLocalPlayerInterface<IRigidBody>()->GetMatrix4(&playerMatrix);
		playerMatrix.p = *GetLocalPlayerVehicle()->GetPosition();
		camMatrix = WorldToRenderMatrix(playerMatrix);
		camMatrix.p += RoofOffset * -camMatrix.y;
		ApplyCameraMatrix(camMatrix);
	}
	bool HasTimer() override { return true; }
} E_RoofCamera;*/

class Effect_GTCamera : public ChaosEffect {
public:
	Effect_GTCamera() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Gran Turismo Camera";
		fTimerLength = 60;
		IncompatibilityGroups.push_back(Attrib::StringHash32("camera_replace"));
		ActivateIncompatibilityGroups.push_back(Attrib::StringHash32("camera_height"));
	}

	static inline float XOffset = 0;
	static inline float YOffset = -1;
	static inline float ZOffset = -5;

	void TickFunctionCamera(Camera* pCamera, double delta) override {
		UMath::Matrix4 playerMatrix;
		GetLocalPlayerInterface<IRigidBody>()->GetMatrix4(&playerMatrix);
		playerMatrix.p = *GetLocalPlayerVehicle()->GetPosition();
		auto camMatrix = WorldToRenderMatrix(playerMatrix);
		camMatrix.p += XOffset * camMatrix.x;
		camMatrix.p += YOffset * camMatrix.y;
		camMatrix.p += ZOffset * camMatrix.z;
		ApplyCameraMatrix(pCamera, camMatrix);
	}
	bool HasTimer() override { return true; }
} E_GTCamera;

class Effect_FreezeCamera : public ChaosEffect {
public:
	Effect_FreezeCamera() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Freeze Camera";
		fTimerLength = 10;
		IncompatibilityGroups.push_back(Attrib::StringHash32("camera_replace"));
		ActivateIncompatibilityGroups.push_back(Attrib::StringHash32("camera_height"));
	}

	void InitFunction() override {
		Camera::StopUpdating = true;
	}
	void DeinitFunction() override {
		Camera::StopUpdating = false;
	}
	bool HasTimer() override { return true; }
} E_FreezeCamera;

class Effect_SecondPersonCamera : public ChaosEffect {
public:
	Effect_SecondPersonCamera() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Second Person View";
		fTimerLength = 45;
		IncompatibilityGroups.push_back(Attrib::StringHash32("camera_replace"));
		ActivateIncompatibilityGroups.push_back(Attrib::StringHash32("camera_height"));
	}

	void TickFunctionCamera(Camera* pCamera, double delta) override {
		auto closest = GetClosestActiveVehicle(GetLocalPlayerVehicle());
		if (!IsAvailable()) return;
		CustomCamera::SetTargetCar(closest, GetLocalPlayerVehicle());
		CustomCamera::ProcessCam(pCamera, delta);
	}
	bool HasTimer() override { return true; }
	bool IsAvailable() override {
		auto closest = GetClosestActiveVehicle(GetLocalPlayerVehicle());
		if (!closest || (*closest->GetPosition() - *GetLocalPlayerVehicle()->GetPosition()).length() > 150) return false;
		return true;
	}
	bool IsRehideable() override { return true; }
	bool IsConditionallyAvailable() override { return true; }
} E_SecondPersonCamera;

class Effect_CinematicCamera : public ChaosEffect {
public:
	Effect_CinematicCamera() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Wheel Camera";
		fTimerLength = 60;
		IncompatibilityGroups.push_back(Attrib::StringHash32("camera_replace"));
		ActivateIncompatibilityGroups.push_back(Attrib::StringHash32("camera_height"));
	}

	static inline float rx = -0.15;
	static inline float ry = 0.2;
	static inline float rz = -0.1;
	static inline float x = -1.5;
	static inline float y = 0.0;
	static inline float z = -2.0;

	void TickFunctionCamera(Camera* pCamera, double delta) override {
		UMath::Matrix4 playerMatrix;
		GetLocalPlayerInterface<IRigidBody>()->GetMatrix4(&playerMatrix);
		playerMatrix.p = *GetLocalPlayerVehicle()->GetPosition();
		NyaMat4x4 offsetMatrix;
		offsetMatrix.Rotate({rx, ry, rz});
		offsetMatrix.p.x = x;
		offsetMatrix.p.y = y;
		offsetMatrix.p.z = z;
		playerMatrix = (UMath::Matrix4)(playerMatrix * offsetMatrix);
		ApplyCameraMatrix(pCamera, WorldToRenderMatrix(playerMatrix));
	}
	bool HasTimer() override { return true; }
} E_CinematicCamera;

class Effect_ReplayCamera : public ChaosEffect {
public:
	Effect_ReplayCamera() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Cinematic Camera";
		fTimerLength = 45;
		IncompatibilityGroups.push_back(Attrib::StringHash32("camera_replace"));
		ActivateIncompatibilityGroups.push_back(Attrib::StringHash32("camera_height"));
	}

	void InitFunction() override {
		Tweak_ForceICEReplay = true;
		NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x479B96, 0x479CD5);
	}
	void DeinitFunction() override {
		Tweak_ForceICEReplay = false;
		NyaHookLib::Patch<uint64_t>(0x479B96, 0x5E3900000139850F);
	}
	bool HasTimer() override { return true; }
} E_ReplayCamera;

class Effect_TopDownCamera : public ChaosEffect {
public:
	float speed = 0;

	Effect_TopDownCamera() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Top-Down Camera";
		fTimerLength = 90;
		IncompatibilityGroups.push_back(Attrib::StringHash32("camera_replace"));
		ActivateIncompatibilityGroups.push_back(Attrib::StringHash32("camera_height"));
	}

	static inline float yOffset = 10;
	static inline float yOffsetScale = 0.5;
	static inline float fwdOffsetScale = 0.3;
	static inline float speedDecay = 5;
	static inline float speedCap = 200;

	void InitFunction() override {
		speed = 0;
	}
	void TickFunctionCamera(Camera* pCamera, double delta) override {
		auto ply = GetLocalPlayerInterface<IRigidBody>();

		UMath::Matrix4 camMatrix;
		camMatrix.Rotate(NyaVec3(-90 * 0.01745329, 0, 0));
		camMatrix.p = *ply->GetPosition();

		UMath::Vector3 fwd;
		ply->GetForwardVector(&fwd);
		fwd.y = 0;
		fwd.Normalize();

		auto velocity = *ply->GetLinearVelocity();
		velocity.y = 0;
		if (velocity.length() > TOMPS(speedCap)) {
			velocity.Normalize();
			velocity *= TOMPS(speedCap);
		}

		float currSpeed = velocity.length() > 1 ? fwd.Dot(velocity) : 0;
		if (currSpeed != speed) {
			if (currSpeed > speed) {
				speed += speedDecay * delta * abs(currSpeed - speed);
				if (currSpeed < speed) speed = currSpeed;
			}
			else if (currSpeed < speed) {
				speed -= speedDecay * delta * abs(currSpeed - speed);
				if (currSpeed > speed) speed = currSpeed;
			}
		}

		camMatrix.p.y += yOffset;
		camMatrix.p.y += abs(speed) * yOffsetScale;
		camMatrix.p += speed * fwdOffsetScale * fwd;
		ApplyCameraMatrix(pCamera, WorldToRenderMatrix(camMatrix));
	}
	bool HasTimer() override { return true; }
} E_TopDownCamera;