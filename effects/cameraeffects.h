class EffectBase_CameraHelper : public ChaosEffect {
public:
	EffectBase_CameraHelper() : ChaosEffect() {
		sName = "(EFFECT BASE) Camera Helper";
	}

	// view to world
	static NyaMat4x4 PrepareCameraMatrix() {
		auto camMatrix = *(NyaMat4x4*)&GetLocalPlayerCamera()->CurrentKey.Matrix;
		return camMatrix.Invert();
	}
	// world to view
	static void ApplyCameraMatrix(NyaMat4x4 mat) {
		*(NyaMat4x4*)&GetLocalPlayerCamera()->CurrentKey.Matrix = mat.Invert();
	}
};

class Effect_LiftCamera : public EffectBase_CameraHelper {
public:
	Effect_LiftCamera() : EffectBase_CameraHelper() {
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

class Effect_LowerCamera : public EffectBase_CameraHelper {
public:
	Effect_LowerCamera() : EffectBase_CameraHelper() {
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

/*class Effect_RoofCamera : public EffectBase_CameraHelper {
public:
	Effect_RoofCamera() : EffectBase_CameraHelper() {
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

class Effect_GTCamera : public EffectBase_CameraHelper {
public:
	Effect_GTCamera() : EffectBase_CameraHelper() {
		sName = "Gran Turismo Camera";
		fTimerLength = 60;
		IncompatibilityGroups.push_back(Attrib::StringHash32("camera_replace"));
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