/*class Effect_LiftCamera : public ChaosEffect {
public:
	Effect_LiftCamera() : ChaosEffect() {
		sName = "Lift Camera Up";
		fTimerLength = 30;
		IncompatibilityGroups.push_back(Attrib::StringHash32("camera_height"));
	}

	static inline float LiftOffset = 3;

	void TickFunctionCamera(double delta) override {
		auto camMatrix = PrepareCameraMatrix();
		camMatrix.p.z += LiftOffset;
		ApplyCameraMatrix(camMatrix);
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
} E_LiftCamera;

class Effect_LowerCamera : public ChaosEffect {
public:
	Effect_LowerCamera() : ChaosEffect() {
		sName = "Low Camera Height";
		fTimerLength = 60;
		IncompatibilityGroups.push_back(Attrib::StringHash32("camera_height"));
	}

	static inline float LiftOffset = -0.4;

	void TickFunctionCamera(double delta) override {
		auto camMatrix = PrepareCameraMatrix();
		camMatrix.p.z += LiftOffset;
		ApplyCameraMatrix(camMatrix);
	}
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
} E_LowerCamera;

class Effect_RoofCamera : public ChaosEffect {
public:
	Effect_RoofCamera() : ChaosEffect() {
		sName = "00Roof Camera";
		fTimerLength = 60;
		ActivateIncompatibilityGroups.push_back(Attrib::StringHash32("camera_height"));
	}

	static inline float RoofOffset = 0.5;

	void TickFunctionCamera(double delta) override {
		auto camMatrix = PrepareCameraMatrix();
		NyaMat4x4 playerMatrix;
		GetLocalPlayerInterface<IRigidBody>()->GetMatrix4((UMath::Matrix4*)&playerMatrix);
		playerMatrix.p = *(NyaVec3*)GetLocalPlayerVehicle()->GetPosition();
		camMatrix = WorldToRenderMatrix(playerMatrix);
		camMatrix.p += RoofOffset * -camMatrix.y;
		ApplyCameraMatrix(camMatrix);
	}
	bool HasTimer() override { return true; }
} E_RoofCamera;*/

class Effect_GTCamera : public ChaosEffect {
public:
	Effect_GTCamera() : ChaosEffect() {
		sName = "Gran Turismo Camera";
		fTimerLength = 60;
		IncompatibilityGroups.push_back(Attrib::StringHash32("camera_replace"));
		ActivateIncompatibilityGroups.push_back(Attrib::StringHash32("camera_height"));
	}

	static inline float XOffset = 0;
	static inline float YOffset = -1;
	static inline float ZOffset = -5;

	void TickFunctionCamera(double delta) override {
		NyaMat4x4 playerMatrix;
		GetLocalPlayerInterface<IRigidBody>()->GetMatrix4((UMath::Matrix4*)&playerMatrix);
		playerMatrix.p = *(NyaVec3*)GetLocalPlayerVehicle()->GetPosition();
		auto camMatrix = WorldToRenderMatrix(playerMatrix);
		camMatrix.p += XOffset * camMatrix.x;
		camMatrix.p += YOffset * camMatrix.y;
		camMatrix.p += ZOffset * camMatrix.z;
		ApplyCameraMatrix(camMatrix);
	}
	bool HasTimer() override { return true; }
} E_GTCamera;

class Effect_FreezeCamera : public ChaosEffect {
public:
	Effect_FreezeCamera() : ChaosEffect() {
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
	Effect_SecondPersonCamera() : ChaosEffect() {
		sName = "Second Person View";
		fTimerLength = 45;
		IncompatibilityGroups.push_back(Attrib::StringHash32("camera_replace"));
		ActivateIncompatibilityGroups.push_back(Attrib::StringHash32("camera_height"));
	}

	void TickFunctionCamera(double delta) override {
		CustomCamera::SetTargetCar(GetClosestActiveVehicle(GetLocalPlayerVehicle()), GetLocalPlayerVehicle());
		CustomCamera::ProcessCam(eViews[EVIEW_PLAYER1].pCamera, delta);
	}
	bool HasTimer() override { return true; }
} E_SecondPersonCamera;