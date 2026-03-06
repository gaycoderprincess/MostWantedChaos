class CHUD_TimeTrial : public CIngameHUDElement {
public:

	void Process() override {
		if (!IsRaceHUDUp()) return;
		if (!IsInRace()) return;
		if (GRaceStatus::fObj->mRaceParms->GetRaceType() != GRace::kRaceType_Tollbooth) return;

		DrawElement(1, "TIME LEFT", FormatGameTime(GRaceStatus::fObj->GetRaceTimeRemaining() * 1000));
	}
} HUD_TimeTrial;
