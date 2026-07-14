class Effect_MouseDrag : public EffectBase_PursuitNoRaceConditional {
public:
	Effect_MouseDrag() : EffectBase_PursuitNoRaceConditional("Uncategorized") {
		sName = "Throw Cars With Mouse";
		fTimerLength = 60;
		bAbortOnConditionFailed = true;
		AddToIncompatiblityGroup("control_mode");
		bCanQuickTrigger = false;
		bRigProportionalChances = true; // todo remove
		nFrequency *= 3; // todo remove
	}

	static inline NyaMat4x4 mCameraMatrix;

	static inline float crosshairSize = 0.015;
	static inline float selectRange = 0.2;
	static inline float dragSpeed = 100.0;
	static inline float throwSpeed = 50.0;
	static inline float maxDistance = 200.0;
	static inline IRigidBody* selectedBody = nullptr;

	static NyaVec3 GetScreenPosition(NyaVec3 world) {
		bVector3 screenPos;
		auto worldPos = WorldToRenderCoords(world);
		eViewPlatInterface::GetScreenPosition(&eViews[EVIEW_PLAYER1], &screenPos, (bVector3*)&worldPos);

		screenPos.x /= (double)nResX;
		screenPos.y /= (double)nResY;
		return screenPos;
	}

	static void DrawCrosshair(IRigidBody* target, bool isDragging) {
		auto screenPos = GetScreenPosition(*target->GetPosition());

		static auto texture = LoadTexture("CwoeeChaos/data/textures/firework_crosshair.png");
		DrawRectangle(screenPos.x - crosshairSize * GetAspectRatioInv(), screenPos.x + crosshairSize * GetAspectRatioInv(), screenPos.y - crosshairSize, screenPos.y + crosshairSize, isDragging ? NyaDrawing::CNyaRGBA32(0,255,0,255) : NyaDrawing::CNyaRGBA32(255,0,0,255), 0, texture);
	}

	static IRigidBody* GetClosestCarToCursor() {
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

		IRigidBody* closest = nullptr;
		float closestDist = 9999.0;

		auto objs = GetActiveRigidBodies();
		for (auto& obj : objs) {
			auto pos = *obj->GetPosition();
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
		selectedBody = nullptr;
		NyaHookLib::Patch<uint16_t>(0x6B1A02, 0x9090); // disable player causality check for cop flipping

		CwoeeHints::AddHint("Drag stuff around using the mouse!");
		CwoeeHints::AddHint("Reset your car to move the camera");
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

		if (auto car = GetClosestCarToCursor()) {
			auto pos = *car->GetPosition();
			auto screenPos = GetScreenPosition(pos);
			screenPos.x *= nResX;
			screenPos.y *= nResY;
			screenPos.z = 0.0;

			if ((cursorPos - screenPos).length() < nResY * selectRange) {
				if (IsKeyJustPressed(VK_LBUTTON)) selectedBody = car;

				if (!selectedBody) {
					DrawCrosshair(car, true);
				}
			}
		}

		if (!IsRigidBodyValidAndActive(selectedBody)) {
			selectedBody = nullptr;
		}

		if (IsKeyPressed(VK_LBUTTON) && selectedBody) {
			auto cam = PrepareCameraMatrix(GetLocalPlayerCamera());
			auto camPos = RenderToWorldCoords(cam.p);
			auto fwd = RenderToWorldCoords(cam.z);
			auto side = RenderToWorldCoords(cam.x);
			auto up = RenderToWorldCoords(cam.y);

			DrawCrosshair(selectedBody, false);

			auto cursorMove = cursorPos - lastCursorPos;
			cursorMove.x /= nResX;
			cursorMove.y /= nResY;

			auto distFromCamera = (camPos - *selectedBody->GetPosition()).length();

			auto vel = *selectedBody->GetLinearVelocity();
			vel = cursorMove.x * side * dragSpeed * distFromCamera;
			vel += cursorMove.y * up * dragSpeed * distFromCamera;

			if (IsKeyPressed(VK_RBUTTON)) {
				vel += fwd * throwSpeed;
				selectedBody->SetLinearVelocity(&vel);
				selectedBody = nullptr;
			}
			else {
				selectedBody->SetLinearVelocity(&vel);
			}
		}
		else if (selectedBody) {
			selectedBody = nullptr;
		}

		lastCursorPos = cursorPos;
	}
	void DeinitFunction() override {
		NyaHookLib::Patch<uint16_t>(0x6B1A02, 0x0974);
	}
	bool HasTimer() override { return true; }
} E_MouseDrag;