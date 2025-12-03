class CHUD_TimeTrial : public CIngameHUDElement {
public:

	void Process() override {
		if (!IsRaceHUDUp()) return;
		if (!IsInRace()) return;
		if (GRaceParameters::GetRaceType(GRaceStatus::fObj->mRaceParms) != 4) return; // tollbooth

		DrawElement(1, "TIME LEFT", FormatGameTime(GRaceStatus::GetRaceTimeRemaining(GRaceStatus::fObj) * 1000));
	}
} HUD_TimeTrial;
