class CHUD_TotalTime : public CIngameHUDElement {
public:
	static constexpr float fPosX = 0.07;
	static constexpr float fPosXFragDerby = 0.14;
	static constexpr float fPosY = 0.3;
	static constexpr float fSize = 0.038;
	static constexpr float fSpacing = 0.03;
	static constexpr float fSpacingX = 0.006;

	void Process() override {
		if (!IsRaceHUDUp()) return;
		if (IsInRace()) {
			auto racer = GRaceStatus::fObj->GetRacerInfo(GetLocalPlayerSimable());

			int numLaps = 1;
			if (auto laps = GetRaceNumLaps()) {
				numLaps = *laps;
			}

			std::string posNth = "th";
			int pos = racer->mRanking;
			if (pos < 10 || pos > 20) {
				if ((pos % 10) == 1) posNth = "st";
				if ((pos % 10) == 2) posNth = "nd";
				if ((pos % 10) == 3) posNth = "rd";
			}

			auto time = (Sim::GetTime() - racer->mRaceTimer.mStartTime) * 1000;
			if (time < 0) time = 0;
			DrawElement(0, TextHook::GetStringWithAllModifiers("TOTAL TIME"), FormatGameTime(time));
			DrawElementRight(0, TextHook::GetStringWithAllModifiers("POSITION"), std::format("{}{}", racer->mRanking, posNth));
			if (numLaps <= 1) {
				DrawElementRight(1, TextHook::GetStringWithAllModifiers("COMPLETE"), std::format("{:.0f}%", racer->mPctRaceComplete));
			}
			else {
				DrawElementRight(1, TextHook::GetStringWithAllModifiers("LAP"), std::format("{}/{}", racer->mLapsCompleted+1, numLaps));
			}
		}
		else if (IsInPursuit()) {
			auto pursuit = GetLocalPlayerInterface<IVehicleAI>()->GetPursuit();
			auto perp = GetLocalPlayerInterface<IPerpetrator>();
			DrawElement(0, TextHook::GetStringWithAllModifiers("TOTAL TIME"), FormatGameTime(pursuit->GetPursuitDuration() * 1000));
			switch (pursuit->GetPursuitStatus()) {
				case PS_BACKUP_REQUESTED:
					DrawElement(1, TextHook::GetStringWithAllModifiers("BACKUP"), FormatGameTime(pursuit->GetBackupETA() * 1000));
					break;
				case PS_COOL_DOWN:
					DrawElement(1, TextHook::GetStringWithAllModifiers("COOLDOWN"), FormatGameTime(pursuit->GetCoolDownTimeRemaining() * 1000));
					break;
			}
			DrawElementRight(0, TextHook::GetStringWithAllModifiers("BOUNTY"), std::to_string(perp->GetPendingRepPointsNormal() + perp->GetPendingRepPointsFromCopDestruction()));
			DrawElementRight(1, TextHook::GetStringWithAllModifiers("COST TO STATE"), std::to_string(pursuit->CalcTotalCostToState()));
			DrawElementRight(2, TextHook::GetStringWithAllModifiers("TRADE PAINT"), std::to_string(pursuit->GetNumCopsDamaged()));
			DrawElementRight(3, TextHook::GetStringWithAllModifiers("COPS DESTROYED"), std::to_string(pursuit->GetNumCopsDestroyed()));
		}
	}
} HUD_TotalTime;