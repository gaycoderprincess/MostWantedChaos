class Effect_MouseDrag : public EffectBase_PursuitNoRaceConditional {
public:
	Effect_MouseDrag() : EffectBase_PursuitNoRaceConditional("Uncategorized") {
		sName = "Throw Cars With Mouse";
		fTimerLength = 60;
		bAbortOnConditionFailed = true;
		AddToIncompatiblityGroup("control_mode");
		bCanQuickTrigger = false;
		nFrequency *= 2;
	}

	static inline NyaMat4x4 mCameraMatrix;

	static inline float crosshairSize = 0.015;
	static inline float selectRange = 0.2;
	static inline float dragSpeed = 100.0;
	static inline float throwSpeed = 50.0;
	static inline float maxDistance = 200.0;
	static inline auto selectedBody = CwoeeSharedRigidBody();

	static NyaVec3 GetScreenPosition(NyaVec3 world) {
		bVector3 screenPos;
		auto worldPos = WorldToRenderCoords(world);
		eViewPlatInterface::GetScreenPosition(&eViews[EVIEW_PLAYER1], &screenPos, (bVector3*)&worldPos);

		screenPos.x /= (double)nResX;
		screenPos.y /= (double)nResY;
		return screenPos;
	}

	static void DrawCrosshair(CwoeeSharedRigidBody target, bool isDragging) {
		auto screenPos = GetScreenPosition(target.GetPosition());

		static auto texture = LoadTexture("CwoeeChaos/data/textures/firework_crosshair.png");
		DrawRectangle(screenPos.x - crosshairSize * GetAspectRatioInv(), screenPos.x + crosshairSize * GetAspectRatioInv(), screenPos.y - crosshairSize, screenPos.y + crosshairSize, isDragging ? NyaDrawing::CNyaRGBA32(0,255,0,255) : NyaDrawing::CNyaRGBA32(255,0,0,255), 0, texture);
	}

	static CwoeeSharedRigidBody GetClosestCarToCursor() {
		POINT point;
		GetCursorPos(&point);

		NyaVec3 cursorPos = {point.x / (float)nResX,point.y / (float)nResY,0};

		//tNyaStringData data;
		//data.x = 0.5;
		//data.y = 0.5;
		//data.size = 0.02;
		//data.XCenterAlign = true;
		//DrawString(data, std::format("cursor {:.2f} {:.2f}", cursorPos.x, cursorPos.y));

		auto cam = PrepareCameraMatrix(GetLocalPlayerCamera());
		auto camPos = RenderToWorldCoords(cam.p);
		auto fwd = RenderToWorldCoords(cam.z);

		CwoeeSharedRigidBody closest = CwoeeSharedRigidBody();
		float closestDist = 9999.0;

		auto objs = GetActiveSharedRigidBodies();
		for (auto& obj : objs) {
			auto pos = obj.GetPosition();
			auto dirFromCam = (pos - camPos);
			if (dirFromCam.length() > maxDistance) continue;
			dirFromCam.Normalize();
			if (dirFromCam.Dot(fwd) < 0.0) continue;

			auto screenPos = GetScreenPosition(pos);
			//screenPos.x *= GetAspectRatioInv();
			screenPos.z = 0.0;

			auto dist = (cursorPos - screenPos).length();
			if (dist < closestDist) {
				closest = obj;
				closestDist = dist;
			}
		}
		return closest;
	}

	void InitFunction() override {
		mCameraMatrix = PrepareCameraMatrix(GetLocalPlayerCamera());
		selectedBody = CwoeeSharedRigidBody();
		NyaHookLib::Patch<uint16_t>(0x6B1A02, 0x9090); // disable player causality check for cop flipping

		CwoeeHints::AddHint("Drag stuff around using the mouse!", 15);
		CwoeeHints::AddHint("Reset your car to move the camera", 15);
	}
	void TickFunction(eChaosHook hook, double delta) override {
		if (hook != HOOK_CAMERA) return;

		if (auto veh = GetLocalPlayerInterface<IRBVehicle>()) {
			if (veh->GetInvulnerability() == INVULNERABLE_FROM_MANUAL_RESET) return;
		}
		ApplyCameraMatrix(pMoverCamera, mCameraMatrix);
	}
	void TickFunctionMain(double delta) override {
		ICopMgr::mInstance->SetAllBustedTimersToZero(); // never busted

		POINT point;
		GetCursorPos(&point);

		static NyaVec3 lastCursorPos;
		NyaVec3 cursorPos = {(float)point.x,(float)point.y,0};

		if (auto veh = GetLocalPlayerInterface<IRBVehicle>()) {
			if (veh->GetInvulnerability() == INVULNERABLE_FROM_MANUAL_RESET) {
				mCameraMatrix = PrepareCameraMatrix(GetLocalPlayerCamera());
			}
		}

		auto closestCar = GetClosestCarToCursor();
		if (closestCar.IsValid()) {
			auto pos = closestCar.GetPosition();
			auto screenPos = GetScreenPosition(pos);
			screenPos.x *= nResX;
			screenPos.y *= nResY;
			screenPos.z = 0.0;

			if ((cursorPos - screenPos).length() < nResY * selectRange) {
				if (IsKeyJustPressed(VK_LBUTTON)) selectedBody = closestCar;

				if (!selectedBody.IsValid()) {
					DrawCrosshair(closestCar, true);
				}
			}
		}

		if (!selectedBody.IsValid()) {
			selectedBody = CwoeeSharedRigidBody();
		}

		if (IsKeyPressed(VK_LBUTTON) && selectedBody.IsValid()) {
			auto cam = PrepareCameraMatrix(GetLocalPlayerCamera());
			auto camPos = RenderToWorldCoords(cam.p);
			auto fwd = RenderToWorldCoords(cam.z);
			auto side = RenderToWorldCoords(cam.x);
			auto up = RenderToWorldCoords(cam.y);

			if (selectedBody.pGameObject) {
				auto cb = selectedBody.pGameObject->mCOMObject->Find<ICollisionBody>();
				if (cb && cb->IsAttachedToWorld()) {
					cb->AttachedToWorld(false, 50.0);
				}
			}

			DrawCrosshair(selectedBody, false);

			auto cursorMove = cursorPos - lastCursorPos;
			cursorMove.x /= nResX;
			cursorMove.y /= nResY;

			auto distFromCamera = (camPos - selectedBody.GetPosition()).length();

			NyaVec3 vel = (cursorMove.x * side * dragSpeed * distFromCamera) + (cursorMove.y * up * dragSpeed * distFromCamera);
			if (IsKeyPressed(VK_RBUTTON)) {
				vel += fwd * throwSpeed;
				selectedBody.SetLinearVelocity(vel);
				selectedBody = CwoeeSharedRigidBody();
			}
			else {
				selectedBody.SetLinearVelocity(vel);
			}
		}
		else if (selectedBody.IsValid()) {
			selectedBody = CwoeeSharedRigidBody();
		}

		lastCursorPos = cursorPos;
	}
	void DeinitFunction() override {
		NyaHookLib::Patch<uint16_t>(0x6B1A02, 0x0974);
	}
	bool HasTimer() override { return true; }
} E_MouseDrag;