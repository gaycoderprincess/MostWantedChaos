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