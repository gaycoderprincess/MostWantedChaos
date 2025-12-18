namespace ChaosVoting {
	bool bEnabled = false;
	int nNumVoteOptions = 4;

	class VotingPopup {
	public:
		ChaosUIPopup Popup;
		ChaosEffect* pEffect = nullptr;

		int nVoteCount = 0;
		int nVoteCountStreamer = 0;
		float fVotePercentage = 0;

		VotingPopup(ChaosEffect* effect) : pEffect(effect) {}

		int GetVoteCount() {
			return nVoteCount + (nVoteCountStreamer * 9999);
		}

		void Draw(float y) {
			Popup.bIsVotingDummy = true;
			Popup.Draw(std::format("{} ({}%)", pEffect->GetFriendlyName(), (int)fVotePercentage), y, false);
		}
	};

	std::vector<VotingPopup> aOldVotes;
	std::vector<VotingPopup> aNewVotes;

	// voting tweaks
	int nLowestWins = 0;
	int nStreamerVotes = 0;
	ChaosEffect* pAllOfTheAbove = nullptr;

	void GenerateNewVotes() {
		aOldVotes = aNewVotes;
		aNewVotes.clear();
		for (int i = 0; i < nNumVoteOptions; i++) {
			aNewVotes.push_back(VotingPopup(GetRandomEffect(false)));
		}
		if (nLowestWins > 0) nLowestWins--;
		if (nStreamerVotes > 0) nStreamerVotes--;
	}

	int GetTotalVoteCount() {
		int totalVotes = 0;
		for (auto& vote : aNewVotes) {
			totalVotes += vote.nVoteCount + vote.nVoteCountStreamer;
		}
		return totalVotes;
	}

	bool AnyEffectGotVotes() {
		return GetTotalVoteCount() > 0;
	}

	void TriggerHighestVotedEffect() {
		auto votes = aNewVotes;
		if (votes.empty()) return;

		if (!AnyEffectGotVotes()) {
			AddRunningEffect(GetRandomEffect());
			return;
		}

		std::sort(votes.begin(), votes.end(), [](const VotingPopup& a, const VotingPopup& b) {
			if (nLowestWins > 0) return a.fVotePercentage < b.fVotePercentage;
			return a.fVotePercentage > b.fVotePercentage;
		});
		bool allOfTheAbove = votes[0].pEffect == pAllOfTheAbove;

		VotingPopup* highestVoted = nullptr;
		int highestVoteCount = 0;

		// activate highest voted activatable effect
		for (auto& vote : votes) {
			if (!CanEffectActivate(vote.pEffect)) continue;

			// also trigger any tied votes
			if (allOfTheAbove || !highestVoted || highestVoteCount == vote.GetVoteCount()) {
				AddRunningEffect(vote.pEffect);
				highestVoted = &vote;
				highestVoteCount = vote.GetVoteCount();
			}
		}

		GenerateNewVotes();
	}

	void UpdateVotePercentages() {
		int totalVotes = 0;
		for (auto& vote : aNewVotes) {
			totalVotes += vote.GetVoteCount();
		}
		for (auto& vote : aNewVotes) {
			vote.fVotePercentage = (vote.GetVoteCount() / (double)totalVotes) * 100.0;
		}
	}

	void OnVoteCast(int i) {
		if (i < 0 || i >= aNewVotes.size()) return;
		aNewVotes[i].nVoteCount++;
		UpdateVotePercentages();
	}

	void OnStreamerVoteCast(int i) {
		if (i < 0 || i >= aNewVotes.size()) return;
		for (auto& vote : aNewVotes) {
			vote.nVoteCountStreamer = 0;
		}
		aNewVotes[i].nVoteCountStreamer++;
		UpdateVotePercentages();
	}

	void DrawUI() {
		// first frame
		if (aNewVotes.empty()) {
			GenerateNewVotes();
		}

		if (nStreamerVotes > 0) {
			for (int i = 0; i < aNewVotes.size(); i++) {
				if (IsKeyJustPressed('1' + i)) {
					OnStreamerVoteCast(i);
				}
			}
		}

		static CNyaTimer gTimer;
		gTimer.Process();

		// clear old votes
		if (!aOldVotes.empty()) {
			int y = 0;
			for (auto& vote : aOldVotes) {
				vote.Popup.Update(gTimer.fDeltaTime, false);
				vote.Draw(y++);
			}
			if (aOldVotes[0].Popup.fTextTimer <= 0) aOldVotes.clear();
		}
		else {
			int y = 0;
			for (auto& vote : aNewVotes) {
				vote.Popup.Update(gTimer.fDeltaTime, true);
				vote.Draw(y++);
			}
		}

		static ChaosUIPopup VoteCountPopup;
		VoteCountPopup.bIsVotingDummy = true;
		VoteCountPopup.Update(gTimer.fDeltaTime, true);
		VoteCountPopup.Draw(std::format("Vote Count: {}", GetTotalVoteCount()), aNewVotes.size(), false);
	}
}