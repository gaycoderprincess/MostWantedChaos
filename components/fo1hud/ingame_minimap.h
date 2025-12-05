class CHUD_Minimap : public CIngameHUDElement {
public:
	static inline IDirect3DTexture9* pMapTexture = nullptr;
	static constexpr float fArrowSize = 0.011;
	static constexpr float fResetSize = 0.015;

	static inline bool bFO2Minimap = true;
	static inline NyaVec3 vLocalPlayerPosition = {0, 0, 0};
	static inline float fLocalPlayerHeading = 0;

	static inline float fFO2MapPos[2] = {95, 340};
	static inline float fFO2MapSize = 0.001;
	static inline float fFO2MapSizeRace = 0.001;
	static inline float fFO2MapSizeDerby = 0.001;
	static inline float fFO2MapClipSize = 128;

	static inline float fWorldTopLeft[2] = {-5065,5445};
	static inline float fWorldBottomRight[2] = {1595,-1215};
	static inline float fScreenPos[2] = {32,300};
	static inline float fScreenSize[2] = {128,128};

	static NyaDrawing::CNyaRGBA32 GetPlayerColor(bool isPlayer) {
		if (isPlayer) return {253,193,114,255};
		return {192,240,66,255};
	}

	static void GetMapExtents(float* left, float* right, float* top, float* bottom) {
		auto posX = fScreenPos[0];
		auto posY = fScreenPos[1];
		auto sizeX = fScreenSize[0];
		auto sizeY = fScreenSize[1];

		posX /= 640.0;
		posY /= 480.0;
		sizeX /= 640.0;
		sizeY /= 480.0;
		posX *= 1440.0;
		posY *= 1080.0;
		sizeX *= 1440.0;
		sizeY *= 1080.0;

		auto justify = JUSTIFY_LEFT;
		DoJustify(justify, posX, posY);
		DoJustify(justify, sizeX, sizeY);

		*left = posX;
		*right = posX + sizeX;
		*top = posY;
		*bottom = posY + sizeY;
	}

	static void GetFO2MapPosition(float* x, float* y) {
		auto posX = fFO2MapPos[0];
		auto posY = fFO2MapPos[1];
		DoJustify(JUSTIFY_480P_LEFT, posX, posY);

		*x = posX;
		*y = posY;
	}

	static NyaVec3 GetPositionOnMap(NyaVec3 pos, bool useOffset = true) {
		if (bFO2Minimap) {
			NyaMat4x4 mat;
			mat.SetIdentity();
			mat.p = pos;

			NyaMat4x4 plyMat;
			plyMat.SetIdentity();
			plyMat.Rotate({0, 0, -fLocalPlayerHeading});
			plyMat.p = vLocalPlayerPosition;
			plyMat = plyMat.Invert();
			mat = plyMat * mat;

			pos = mat.p;

			float x, y;
			GetFO2MapPosition(&x, &y);

			if (!useOffset) {
				x = 0;
				y = 0;
			}
			return NyaVec3(x+(pos.x*fFO2MapSize*GetAspectRatioInv()), y-pos.z*fFO2MapSize, 0);
		}
		else {
			auto startX = fWorldTopLeft[0];
			auto startY = fWorldBottomRight[1];
			auto endX = fWorldBottomRight[0];
			auto endY = fWorldTopLeft[1];

			float left, right, top, bottom;
			GetMapExtents(&left, &right, &top, &bottom);

			pos.x -= startX;
			pos.x /= startX - endX;
			pos.z -= startY;
			pos.z /= startY - endY;
			float spritePosX = std::lerp(left, right, -pos.x);
			float spritePosY = std::lerp(bottom, top, -pos.z);
			return {spritePosX, spritePosY, 0};
		}
	}

	static float GetDirection(UMath::Matrix4 plyMatrix) {
		return std::atan2(plyMatrix.z.x, plyMatrix.z.z);
	}
	static UMath::Vector3 GetFOPosition(UMath::Vector3 position) {
		return {position.x, position.y, position.z};
	}

	static void DrawPlayerOnMap(IVehicle* ply, bool isPlayer) {
		static auto arrow = LoadTexture("CwoeeChaos/data/global/overlay/map_playerarrow.tga");
		static auto arrowPlayer = LoadTexture("CwoeeChaos/data/global/overlay/map_playerarrow_local.tga");

		UMath::Matrix4 plyMatrix;
		ply->mCOMObject->Find<IRigidBody>()->GetMatrix4(&plyMatrix);
		plyMatrix.p = GetFOPosition(*ply->GetPosition());

		NyaVec3 invalidGhostPos = {500,-25,500};
		if ((plyMatrix.p - invalidGhostPos).length() < 1) return;

		auto plyPos = GetPositionOnMap(plyMatrix.p);
		auto plyDir = GetDirection(plyMatrix) - fLocalPlayerHeading;
		DrawRectangle(plyPos.x - (fArrowSize * GetAspectRatioInv()), plyPos.x + (fArrowSize * GetAspectRatioInv()), plyPos.y - fArrowSize, plyPos.y + fArrowSize, GetPlayerColor(isPlayer), 0, isPlayer ? arrowPlayer : arrow, plyDir);
	}

	NyaVec3 gArcadeCheckpoint;
	static void DrawCheckpointOnMap(NyaVec3 pos) {
		static auto texture = LoadTexture("CwoeeChaos/data/global/overlay/map_checkpoint.tga");

		auto spritePos = GetPositionOnMap(pos);
		DrawRectangle(spritePos.x - (fArrowSize * GetAspectRatioInv()), spritePos.x + (fArrowSize * GetAspectRatioInv()), spritePos.y - fArrowSize, spritePos.y + fArrowSize, {8,200,8,255}, 0, texture);
	}

	static inline float fMapOffset = -50;
	void Process() override {
		if (!IsRaceHUDUp()) return;

		if (!pMapTexture) {
			pMapTexture = LoadTexture("CwoeeChaos/data/textures/gamemap.png");
		}

		bool bIsDerby = false;
		fFO2MapSize = bIsDerby ? fFO2MapSizeDerby : fFO2MapSizeRace;

		UMath::Matrix4 plyMatrix;
		GetLocalPlayerInterface<IRigidBody>()->GetMatrix4(&plyMatrix);
		plyMatrix.p = GetFOPosition(*GetLocalPlayerVehicle()->GetPosition());
		fLocalPlayerHeading = bFO2Minimap ? GetDirection(plyMatrix) : 0;
		vLocalPlayerPosition = bFO2Minimap ? plyMatrix.p : NyaVec3(0, 0, 0);

		if (bFO2Minimap && !bIsDerby) {
			DrawCallback([](const ImDrawList* parent_list, const ImDrawCmd* cmd) {
				auto pos = GetPositionOnMap(vLocalPlayerPosition);
				pos.x *= nResX;
				pos.y *= nResY;
				float size = fFO2MapClipSize;
				size /= 1080.0;
				size *= nResY;
				ImGui::GetForegroundDrawList()->PushClipRect(ImVec2(pos.x-size, pos.y-size), ImVec2(pos.x+size, pos.y+size), false);
			}, true);
		}

		if (pMapTexture) {
			if (bFO2Minimap) {
				auto texture = pMapTexture;

				auto startX = fWorldTopLeft[0];
				auto startY = fWorldBottomRight[1];
				auto endX = fWorldBottomRight[0];
				auto endY = fWorldTopLeft[1];
				auto midX = std::lerp(startX, endX, 0.5);
				auto midY = std::lerp(startY, endY, 0.5);
				auto sizeX = (endX - startX) * fFO2MapSize;
				auto sizeY = (endY - startY) * fFO2MapSize;

				auto plyPos = GetPositionOnMap(NyaVec3(midX, 0, midY));
				DrawRectangle(plyPos.x - (sizeX * 0.5 * GetAspectRatioInv()), plyPos.x + (sizeX * 0.5 * GetAspectRatioInv()), plyPos.y - sizeY * 0.5, plyPos.y + sizeY * 0.5, {255,255,255,255}, 0, texture, -fLocalPlayerHeading);
			}
			else {
				float left, right, top, bottom;
				GetMapExtents(&left, &right, &top, &bottom);
				DrawRectangle(left, right, top, bottom, {255,255,255,255}, 0, pMapTexture);
			}
		}

		// draw wrecked players first, opponents after, then arcade checkpoint, local player on top
		for (int i = 0; i < GRaceStatus::fObj->mRacerCount; i++) {
			auto ply = &GRaceStatus::fObj->mRacerInfo[i];
			if (ply->mIndex == 0) continue;
			if (!IsRacerKOd(ply)) continue;
			DrawPlayerOnMap(GetRacerFromHandle(ply->mhSimable), false);
		}
		for (int i = 0; i < GRaceStatus::fObj->mRacerCount; i++) {
			auto ply = &GRaceStatus::fObj->mRacerInfo[i];
			if (ply->mIndex == 0) continue;
			if (IsRacerKOd(ply)) continue;
			DrawPlayerOnMap(GetRacerFromHandle(ply->mhSimable), false);
		}

		// todo
		//if (bIsCarnageRace) {
		//	DrawCheckpointOnMap(gArcadeCheckpoint);
		//}

		DrawPlayerOnMap(GetLocalPlayerVehicle(), true);

		if (bFO2Minimap && !bIsDerby) {
			DrawCallback([](const ImDrawList* parent_list, const ImDrawCmd* cmd) { ImGui::GetForegroundDrawList()->PopClipRect(); }, true);
		}
	}
} HUD_Minimap;