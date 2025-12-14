// todo!!

namespace ChaosVoting {
	bool bEnabled = false;
	int nNumVoteOptions = 4;

	std::vector<ChaosEffectInstance> aOldVotes;
	std::vector<ChaosEffectInstance> aNewVotes;

	// voting tweaks
	int nLowestWins = 0;
	int nStreamerVotes = 0;
	ChaosEffect* pAllOfTheAbove = nullptr;

	void GenerateNewVotes() {
		aOldVotes = aNewVotes;
		aNewVotes.clear();
		for (int i = 0; i < nNumVoteOptions; i++) {
			aNewVotes.push_back(ChaosEffectInstance(GetRandomEffect(false)));
		}
		if (nLowestWins > 0) nLowestWins--;
		if (nStreamerVotes > 0) nStreamerVotes--;
	}

	bool AnyEffectGotVotes() {
		int totalVotes = 0;
		for (auto& vote : aNewVotes) {
			totalVotes += vote.nVotingDummyVoteCount;
		}
		return totalVotes > 0;
	}

	void TriggerHighestVotedEffect() {
		auto votes = aNewVotes;
		if (votes.empty()) return;
		if (!AnyEffectGotVotes()) return;

		std::sort(votes.begin(), votes.end(), [](const ChaosEffectInstance& a, const ChaosEffectInstance& b) {
			if (nLowestWins > 0) return a.fVotingDummyVotePercentage < b.fVotingDummyVotePercentage;
			return a.fVotingDummyVotePercentage > b.fVotingDummyVotePercentage;
		});
		bool allOfTheAbove = votes[0].pEffect == pAllOfTheAbove;

		ChaosEffectInstance* highestVoted = nullptr;
		int highestVoteCount = 0;

		// activate highest voted activatable effect
		for (auto& vote : votes) {
			if (!CanEffectActivate(vote.pEffect)) continue;

			// also trigger any tied votes
			if (allOfTheAbove || !highestVoted || highestVoteCount == vote.nVotingDummyVoteCount) {
				AddRunningEffect(vote.pEffect);
				highestVoted = &vote;
				highestVoteCount = vote.nVotingDummyVoteCount;
			}
		}

		GenerateNewVotes();
	}

	void UpdateVotePercentages() {
		int totalVotes = 0;
		for (auto& vote : aNewVotes) {
			totalVotes += vote.nVotingDummyVoteCount;
		}
		for (auto& vote : aNewVotes) {
			vote.fVotingDummyVotePercentage = (vote.nVotingDummyVoteCount / (double)totalVotes) * 100.0;
		}
	}

	void OnVoteCast(int i) {
		if (i < 0 || i >= aNewVotes.size()) return;
		aNewVotes[i].nVotingDummyVoteCount++;
		UpdateVotePercentages();
	}

	void OnStreamerVoteCast(int i) {
		if (i < 0 || i >= aNewVotes.size()) return;
		for (auto& vote : aNewVotes) {
			vote.nVotingDummyVoteCount = 0;
		}
		aNewVotes[i].nVotingDummyVoteCount += 9999;
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
				vote.bIsVotingDummy = true;
				vote.UpdatePopup(gTimer.fDeltaTime, false);
				vote.Draw(y++, false);
			}
			if (aOldVotes[0].fTextTimer <= 0) aOldVotes.clear();
		}
		else {
			int y = 0;
			for (auto& vote : aNewVotes) {
				vote.bIsVotingDummy = true;
				vote.UpdatePopup(gTimer.fDeltaTime, true);
				vote.Draw(y++, false);
			}
		}
	}
}