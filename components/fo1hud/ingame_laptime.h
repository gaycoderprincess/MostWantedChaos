class CHUD_LapTime : public CIngameHUDElement {
public:
	float fAlpha = 0;

	bool bCentered = true;
	int nNonCenterX = 851;
	int nNonCenterX_1 = 858;

	void Process() override {
		static CNyaTimer gTimer;
		gTimer.Process();

		static int lapTimes[10];

		if (!IsRaceHUDUp()) return;
		if (!IsInRace()) return;
		if (IsLocalPlayerStaging() || !HUD_Wrecked.aNotifs.empty() || GRaceParameters::GetNumLaps(GRaceStatus::fObj->mRaceParms) < 2) {
			fAlpha = 0;
			memset(lapTimes, 0, sizeof(lapTimes));
			return;
		}

		bool showingLastLap = false;
		auto ply = GRaceStatus::fObj->GetRacerInfo(GetLocalPlayerSimable());
		int lapTime = (Sim::GetTime() - ply->mLapTimer.mStartTime) * 1000;
		lapTimes[ply->mLapsCompleted] = lapTime;
		if (ply->mLapsCompleted > 0 && lapTime < 3000) {
			if (lapTime < 2000) {
				fAlpha += gTimer.fDeltaTime * 2;
			}
			else {
				fAlpha -= gTimer.fDeltaTime * 2;
			}
			lapTime = lapTimes[ply->mLapsCompleted-1];
			showingLastLap = true;
		}
		else if (ply->mPctLapComplete > 90) {
			fAlpha += gTimer.fDeltaTime * 2;
		}
		else {
			fAlpha -= gTimer.fDeltaTime * 2;
		}
		if (fAlpha < 0) fAlpha = 0;
		if (fAlpha > 1) fAlpha = 1;
		if (fAlpha <= 0) return;

		NyaDrawing::CNyaRGBA32 rgb = {255,255,255,255};
		rgb.a = fAlpha * 255;
		if (bCentered) {
			DrawElementCenter(0.5, "LAP TIME", FormatGameTime(lapTime), rgb);
		}
		else {
			DrawElementCenter(0.5, "LAP TIME", "", rgb);
			if (lapTime >= 60 * 1000 && lapTime <= 120 * 1000) {
				DrawElementCustomX(JUSTIFY_CENTER, nNonCenterX_1, 0.5, "", FormatGameTime(lapTime), rgb);
			}
			else {
				DrawElementCustomX(JUSTIFY_CENTER, nNonCenterX, 0.5, "", FormatGameTime(lapTime), rgb);
			}
		}
	}
} HUD_LapTime;