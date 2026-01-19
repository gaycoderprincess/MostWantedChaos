bool CarRender_Truncate = false;
float CarRender_TruncateAccuracy = 2;
bool CarRender_TruncateRotation = false;
float CarRender_TruncateRotationAccuracy = 10;
bool CarRender_Billboard = false;
bool CarRender_DontRenderPlayer = false;
bool CarRender_BillboardEachOther = false;

float TruncateFloat(float in, int accuracy) {
	in *= accuracy;
	in = (int)in;
	in /= accuracy;
	return in;
}

auto CarGetVisibleStateOrig = (int(__thiscall*)(eView*, const bVector3*, const bVector3*, bMatrix4*))nullptr;
int __thiscall CarGetVisibleStateHooked(eView* a1, const bVector3* a2, const bVector3* a3, bMatrix4* a4) {
	auto carMatrix = (NyaMat4x4*)a4;
	if (CarRender_DontRenderPlayer && TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_RACING && !IsInLoadingScreen()) {
		if (GetClosestActiveVehicle(RenderToWorldCoords(carMatrix->p)) == GetLocalPlayerVehicle()) {
			// hacky solution!! it works but checking some CarRenderInfo ptr against the player and disabling DrawCars would be way better
			carMatrix->p = {0,0,0};
			return CarGetVisibleStateOrig(a1, a2, a3, a4);
		}
	}
	if (CarRender_Billboard) {
		auto cameraMatrix = PrepareCameraMatrix(eViews[EVIEW_PLAYER1].pCamera);
		auto cameraPos = cameraMatrix.p;
		auto carPos = carMatrix->p;

		auto lookat = carPos - cameraPos;
		lookat.Normalize();
		auto lookatMatrix = NyaMat4x4::LookAt(lookat, {0,0,1});
		carMatrix->x = lookatMatrix.x;
		carMatrix->y = lookatMatrix.y;
		carMatrix->z = lookatMatrix.z;

		NyaMat4x4 offsetMatrix;
		offsetMatrix.Rotate(NyaVec3(90 * 0.01745329, 0 * 0.01745329, -90 * 0.01745329));
		*carMatrix = *carMatrix * offsetMatrix;
	}
	if (CarRender_BillboardEachOther && TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_RACING) {
		if (auto renderVeh = GetClosestActiveVehicle(RenderToWorldCoords(carMatrix->p))) {
			auto closest = (renderVeh->GetDriverClass() == DRIVER_COP && renderVeh->mCOMObject->Find<IVehicleAI>()->GetPursuit()) ? GetLocalPlayerVehicle() : GetClosestActiveVehicle(renderVeh);
			if (closest) {
				UMath::Matrix4 cameraMatrix;
				closest->mCOMObject->Find<IRigidBody>()->GetMatrix4(&cameraMatrix);
				cameraMatrix.p = *closest->GetPosition();

				auto cameraPos = WorldToRenderCoords(cameraMatrix.p);
				auto carPos = carMatrix->p;

				auto lookat = carPos - cameraPos;
				lookat.Normalize();
				auto lookatMatrix = NyaMat4x4::LookAt(lookat, {0,0,1});
				carMatrix->x = lookatMatrix.x;
				carMatrix->y = lookatMatrix.y;
				carMatrix->z = lookatMatrix.z;

				NyaMat4x4 offsetMatrix;
				offsetMatrix.Rotate(NyaVec3(90 * 0.01745329, 0 * 0.01745329, -90 * 0.01745329));
				*carMatrix = *carMatrix * offsetMatrix;
			}
		}
	}
	if (CarRender_TruncateRotation) {
		double lengths[3] = {
				carMatrix->x.length(),
				carMatrix->y.length(),
				carMatrix->z.length(),
		};

		carMatrix->x.x = TruncateFloat(carMatrix->x.x, CarRender_TruncateRotationAccuracy);
		carMatrix->x.y = TruncateFloat(carMatrix->x.y, CarRender_TruncateRotationAccuracy);
		carMatrix->x.z = TruncateFloat(carMatrix->x.z, CarRender_TruncateRotationAccuracy);
		carMatrix->y.x = TruncateFloat(carMatrix->y.x, CarRender_TruncateRotationAccuracy);
		carMatrix->y.y = TruncateFloat(carMatrix->y.y, CarRender_TruncateRotationAccuracy);
		carMatrix->y.z = TruncateFloat(carMatrix->y.z, CarRender_TruncateRotationAccuracy);
		carMatrix->z.x = TruncateFloat(carMatrix->z.x, CarRender_TruncateRotationAccuracy);
		carMatrix->z.y = TruncateFloat(carMatrix->z.y, CarRender_TruncateRotationAccuracy);
		carMatrix->z.z = TruncateFloat(carMatrix->z.z, CarRender_TruncateRotationAccuracy);

		carMatrix->x.Normalize();
		carMatrix->y.Normalize();
		carMatrix->z.Normalize();
		carMatrix->x *= lengths[0];
		carMatrix->y *= lengths[1];
		carMatrix->z *= lengths[2];
	}
	if (CarRender_Truncate) {
		carMatrix->p.x = TruncateFloat(carMatrix->p.x, CarRender_TruncateAccuracy);
		carMatrix->p.y = TruncateFloat(carMatrix->p.y, CarRender_TruncateAccuracy);
		carMatrix->p.z = TruncateFloat(carMatrix->p.z, CarRender_TruncateAccuracy);
	}
	return CarGetVisibleStateOrig(a1, a2, a3, a4);
}

ChloeHook Hook_CarRender([]() {
	CarGetVisibleStateOrig = (int(__thiscall*)(eView*, const bVector3*, const bVector3*, bMatrix4*))NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x74E346, &CarGetVisibleStateHooked);
	NyaHookLib::PatchRelative(NyaHookLib::JMP, 0x73786A, 0x73790A); // don't reset CarScaleMatrix when exiting to menu
	NyaHooks::LateInitHook::aFunctions.push_back([]() { CarScaleMatrix = UMath::Matrix4::kIdentity; });
});