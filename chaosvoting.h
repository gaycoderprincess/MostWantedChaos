// todo!!

namespace ChaosVoting {
	bool bEnabled = false;
	int nNumVoteOptions = 4;

	std::vector<ChaosEffectInstance> aOldVotes;
	std::vector<ChaosEffectInstance> aNewVotes;

	void GenerateNewVotes() {
		aOldVotes = aNewVotes;
		aNewVotes.clear();
		for (int i = 0; i < nNumVoteOptions; i++) {
			aNewVotes.push_back(ChaosEffectInstance(GetRandomEffect(false)));
		}
	}

	void TriggerHighestVotedEffect() {
		auto votes = aNewVotes;
		if (votes.empty()) return;

		std::sort(votes.begin(), votes.end(), [](const ChaosEffectInstance& a, const ChaosEffectInstance& b) {
			return a.fVotingDummyVotePercentage > b.fVotingDummyVotePercentage;
		});

		if (votes[0].nVotingDummyVoteCount <= 0) return;

		ChaosEffectInstance* highestVoted = nullptr;
		int highestVoteCount = 0;

		// activate highest voted activatable effect
		for (auto& vote : votes) {
			if (!CanEffectActivate(vote.pEffect)) continue;

			// also trigger any tied votes
			if (!highestVoted || highestVoteCount == vote.nVotingDummyVoteCount) {
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

	void DrawUI() {
		// first frame
		if (aNewVotes.empty()) {
			GenerateNewVotes();
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