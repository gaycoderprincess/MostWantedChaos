enum class eHUDLayer {
	WORLD, // world UI, e.g. 3d player icons
	BASE, // normal UI
	FADE, // car reset fade
	OVERLAY, // pause menu, tutorial screen, etc.
	NUM_LAYERS
};

float FontSize_Condensed12 = 0.8;
float FontOffset_Condensed12 = 0;
float FontOutline_Condensed12 = 0.04;
void DrawStringFO2_Condensed12(const tNyaStringData& data, const std::string& str) {
	auto tmp = data;
	tmp.size *= FontSize_Condensed12;
	tmp.outlinea = tmp.a;
	tmp.outlinedist = FontOutline_Condensed12;
	if (tmp.YCenterAlign) tmp.y += tmp.size * FontOffset_Condensed12;
	DrawString(tmp, str);
}

float FontSize_Italic24 = 1.1;
float FontOffset_Italic24 = 0;
float FontOutline_Italic24 = 0.04;
void DrawStringFO2_Italic24(const tNyaStringData& data, const std::string& str) {
	auto tmp = data;
	tmp.size *= FontSize_Italic24;
	tmp.outlinea = tmp.a;
	tmp.outlinedist = FontOutline_Italic24;
	if (tmp.YCenterAlign) tmp.y += tmp.size * FontOffset_Italic24;
	DrawString(tmp, str);
}

float FontSize_Regular18 = 1.4;
float FontOffset_Regular18 = 0.15;
float FontOutline_Regular18 = 0.04;
void DrawStringFO2_Regular18(const tNyaStringData& data, const std::string& str) {
	auto tmp = data;
	tmp.size *= FontSize_Regular18;
	tmp.outlinea = tmp.a;
	tmp.outlinedist = FontOutline_Regular18;
	if (tmp.YCenterAlign) tmp.y += tmp.size * FontOffset_Regular18;
	DrawString(tmp, str);
}

bool IsInRace() {
	if (!GRaceStatus::fObj) return false;
	if (!GRaceStatus::fObj->mRaceParms) return false;
	return true;
}

bool IsInPursuit() {
	if (auto ply = GetLocalPlayerInterface<IPerpetrator>()) {
		return ply->IsBeingPursued();
	}
	return false;
}

std::vector<GRacerInfo*> GetSortedPlayerScores() {
	std::vector<GRacerInfo*> racers;
	if (!GRaceStatus::fObj) return racers;
	for (int i = 0; i < GRaceStatus::fObj->mRacerCount; i++) {
		racers.push_back(&GRaceStatus::fObj->mRacerInfo[i]);
	}
	std::sort(racers.begin(), racers.end(), [](GRacerInfo *a, GRacerInfo *b) {
		return a->mRanking < b->mRanking;
	});
	return racers;
}

std::string FormatGameTime(int ms, bool leadingZero = true) {
	if (ms < 0) ms = 0;
	std::string str = GetTimeFromMilliseconds(ms, true);
	str.pop_back(); // remove trailing zero
	// add leading zero
	if (leadingZero && ms < 60 * 10 * 1000) {
		str = "0" + str;
	}
	return str;
}

enum eFlatOutPaletteColor {
	COLOR_INGAME_YELLOW = 2,
	COLOR_INGAMEMENU_WARNING = 12,
	COLOR_INGAMEMENU_TITLE = 13,
	COLOR_MENU_WHITE = 17,
	COLOR_MENU_YELLOW = 18,
	COLOR_MENU_RED = 22,
};

NyaDrawing::CNyaRGBA32 GetPaletteColor(eFlatOutPaletteColor color) {
	switch (color) {
		case COLOR_INGAME_YELLOW: return {241,192,88,255};
		case COLOR_INGAMEMENU_WARNING: return {225,165,0,255};
		case COLOR_INGAMEMENU_TITLE: return {225,200,255};
		case COLOR_MENU_WHITE: return {225,225,225,255};
		case COLOR_MENU_YELLOW: return {227,183,63,255};
		case COLOR_MENU_RED: return {95,0,0,255};
		default: return {0,0,0,255};
	}
}

class CIngameHUDElement : public CHUDElement {
public:
	static inline std::vector<CIngameHUDElement*> aGameHUD;

	eHUDLayer nHUDLayer = eHUDLayer::BASE;

	CIngameHUDElement() {
		aGameHUD.push_back(this);
	}

	static bool IsRaceHUDUp() {
		//if (GetScoreManager()->nHideRaceHUD) return false;
		return true;
	}

	static inline tDrawPositions gElementBase = {0.008, 0.029, 0.042, 0, 0.034};
	static inline float fElementTotalSpacing = 0.092;

	static void DrawElement(float y, const std::string& title, const std::string& value, const NyaDrawing::CNyaRGBA32 rgb = {255,255,255,255}) {
		tNyaStringData data;
		data.x = gElementBase.fPosX * GetAspectRatioInv();
		data.y = gElementBase.fPosY + y * fElementTotalSpacing;
		data.size = gElementBase.fSize;
		data.SetColor(GetPaletteColor(COLOR_INGAME_YELLOW));
		data.a = rgb.a;
		DrawStringFO2_Condensed12(data, title);
		data.y += gElementBase.fSpacingY;
		data.SetColor(rgb);
		DrawStringFO2_Italic24(data, value);
	}

	static void DrawElementCustomX(eJustify justify, int x, float y, const std::string& title, const std::string& value, const NyaDrawing::CNyaRGBA32 rgb = {255,255,255,255}) {
		float x2 = x, tmp;
		DoJustify(justify, x2, tmp);

		tNyaStringData data;
		data.x = x2;
		data.y = gElementBase.fPosY + y * fElementTotalSpacing;
		data.size = gElementBase.fSize;
		data.SetColor(GetPaletteColor(COLOR_INGAME_YELLOW));
		data.a = rgb.a;
		DrawStringFO2_Condensed12(data, title);
		data.y += gElementBase.fSpacingY;
		data.SetColor(rgb);
		DrawStringFO2_Italic24(data, value);
	}

	static void DrawElementCenter(float y, const std::string& title, const std::string& value, const NyaDrawing::CNyaRGBA32 rgb = {255,255,255,255}) {
		tNyaStringData data;
		data.x = 0.5;
		data.y = gElementBase.fPosY + y * fElementTotalSpacing;
		data.size = gElementBase.fSize;
		data.XCenterAlign = true;
		data.SetColor(GetPaletteColor(COLOR_INGAME_YELLOW));
		data.a = rgb.a;
		DrawStringFO2_Condensed12(data, title);
		data.y += gElementBase.fSpacingY;
		data.SetColor(rgb);
		DrawStringFO2_Italic24(data, value);
	}

	static void DrawElementRight(float y, const std::string& title, const std::string& value, const NyaDrawing::CNyaRGBA32 rgb = {255,255,255,255}) {
		tNyaStringData data;
		data.x = 1.0 - (gElementBase.fPosX * GetAspectRatioInv());
		data.y = gElementBase.fPosY + y * fElementTotalSpacing;
		data.size = gElementBase.fSize;
		data.XRightAlign = true;
		data.SetColor(GetPaletteColor(COLOR_INGAME_YELLOW));
		data.a = rgb.a;
		DrawStringFO2_Condensed12(data, title);
		data.y += gElementBase.fSpacingY;
		data.SetColor(rgb);
		DrawStringFO2_Italic24(data, value);
	}
};

namespace NewGameHud {
	void OnReset() {
		for (auto& hud : CIngameHUDElement::aGameHUD) {
			hud->Reset();
		}
	}

	void OnHUDTick() {
		if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_RACING || IsInLoadingScreen() || IsInNIS()) {
			OnReset();
			return;
		}
		if (FEManager::mPauseRequest) return;

		for (int i = 0; i < (int)eHUDLayer::NUM_LAYERS; i++) {
			for (auto& hud : CIngameHUDElement::aGameHUD) {
				if (hud->nHUDLayer != (eHUDLayer)i) continue;
				hud->Process();
			}
		}
	}

	ChloeHook Init([]() {
		for (auto& hud : CIngameHUDElement::aGameHUD) {
			hud->Init();
		}
	});
}

#include "ingame_playerlist.h"
#include "ingame_damagemeter.h"
#include "ingame_wrecked.h"
#include "ingame_laptime.h"
//#include "ingame_minimap.h"
//#include "ingame_derby_overhead.h"
//#include "ingame_multiplayer_overhead.h" // todo after i get 3d to 2d in mw
#include "ingame_timetrial.h"
#include "ingame_totaltime.h"