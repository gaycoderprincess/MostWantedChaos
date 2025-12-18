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

	int GetTotalVoteCount() {
		int totalVotes = 0;
		for (auto& vote : aNewVotes) {
			totalVotes += vote.nVotingDummyVoteCount + vote.nVotingDummyVoteCountStreamer;
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
			if (allOfTheAbove || !highestVoted || highestVoteCount == vote.GetVotingDummyVoteCount()) {
				AddRunningEffect(vote.pEffect);
				highestVoted = &vote;
				highestVoteCount = vote.GetVotingDummyVoteCount();
			}
		}

		GenerateNewVotes();
	}

	void UpdateVotePercentages() {
		int totalVotes = 0;
		for (auto& vote : aNewVotes) {
			totalVotes += vote.GetVotingDummyVoteCount();
		}
		for (auto& vote : aNewVotes) {
			vote.fVotingDummyVotePercentage = (vote.GetVotingDummyVoteCount() / (double)totalVotes) * 100.0;
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
			vote.nVotingDummyVoteCountStreamer = 0;
		}
		aNewVotes[i].nVotingDummyVoteCountStreamer++;
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

		static ChaosEffect VoteCountDummyEffect("DUMMY", true);
		static char tmp[256];
		strcpy_s(tmp, 256, std::format("Vote Count: {}", GetTotalVoteCount()).c_str());
		VoteCountDummyEffect.sName = tmp;
		static ChaosEffectInstance VoteCountDummy(&VoteCountDummyEffect);
		VoteCountDummy.bIsVotingDummy = true;
		VoteCountDummy.fVotingDummyVotePercentage = -1;
		VoteCountDummy.UpdatePopup(gTimer.fDeltaTime, true);
		VoteCountDummy.Draw(aNewVotes.size(), false);
	}
}