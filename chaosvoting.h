#include "IRCClient/src/IRCClient.h"

namespace ChaosVoting {
	bool bAutoReconnect = false;
	char sChannelName[64] = "";
	int nNumVoteOptions = 4;

	IRCClient gIRCClient;
	std::mutex mVotingMutex;

	class VotingPopup {
	public:
		ChaosUIPopup Popup;
		ChaosEffect* pEffect = nullptr;

		std::vector<std::string> aVotes = {};
		bool bVotedByStreamer = false;
		float fVotePercentage = 0;

		VotingPopup(ChaosEffect* effect) : pEffect(effect) {}

		int GetVoteCount() {
			return aVotes.size() + (bVotedByStreamer * 9999);
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
	bool bSelectingEffectsForVote = false;

	void GenerateNewVotes() {
		aOldVotes = aNewVotes;
		aNewVotes.clear();
		bSelectingEffectsForVote = true;
		for (int i = 0; i < nNumVoteOptions; i++) {
			aNewVotes.push_back(VotingPopup(GetRandomEffect(false)));
		}
		bSelectingEffectsForVote = false;
		if (nLowestWins > 0) nLowestWins--;
		if (nStreamerVotes > 0) nStreamerVotes--;
	}

	int GetTotalVoteCount() {
		int totalVotes = 0;
		for (auto& vote : aNewVotes) {
			totalVotes += vote.aVotes.size() + vote.bVotedByStreamer;
		}
		return totalVotes;
	}

	bool AnyEffectGotVotes() {
		return GetTotalVoteCount() > 0;
	}

	void TriggerHighestVotedEffectNoLock() {
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

	void TriggerHighestVotedEffect() {
		mVotingMutex.lock();
		TriggerHighestVotedEffectNoLock();
		mVotingMutex.unlock();
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

	void OnVoteCast(const std::string& username, int i) {
		if (i < 0 || i >= aNewVotes.size()) return;
		for (auto& effect : aNewVotes) {
			int pos = 0;
			for (auto& vote : effect.aVotes) {
				if (vote == username) {
					effect.aVotes.erase(effect.aVotes.begin() + pos);
					break;
				}
				pos++;
			}
		}
		aNewVotes[i].aVotes.push_back(username);
		UpdateVotePercentages();
	}

	void OnStreamerVoteCast(int i) {
		if (i < 0 || i >= aNewVotes.size()) return;
		for (auto& vote : aNewVotes) {
			vote.bVotedByStreamer = false;
		}
		aNewVotes[i].bVotedByStreamer = true;
		UpdateVotePercentages();
	}

	void DrawUI() {
		mVotingMutex.lock();

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

		int y = 0;

		// clear old votes
		if (!aOldVotes.empty()) {
			for (auto& vote : aOldVotes) {
				vote.Popup.Update(gTimer.fDeltaTime, false);
				vote.Draw(y++);
			}
			if (aOldVotes[0].Popup.fTextTimer <= 0) aOldVotes.clear();
		}
		else {
			for (auto& vote : aNewVotes) {
				vote.Popup.Update(gTimer.fDeltaTime, true);
				vote.Draw(y++);
			}
		}

		static ChaosUIPopup VoteCountPopup;
		VoteCountPopup.bIsVotingDummy = true;
		VoteCountPopup.Update(gTimer.fDeltaTime, true);
		VoteCountPopup.Draw(std::format("Vote Count: {}", GetTotalVoteCount()), y++, false);

		static ChaosUIPopup VoteDisabledPopup;
		VoteDisabledPopup.bIsVotingDummy = true;
		VoteDisabledPopup.Update(gTimer.fDeltaTime, nSmartRNG > 0);
		VoteDisabledPopup.Draw("Voting is disabled!", y++, false);

		mVotingMutex.unlock();
	}

	void OnMessageReceived(IRCMessage message, IRCClient* client) {
		//message.parameters:
		//#channel
		//bark woof meow

		if (message.parameters.size() < 2) return;

		auto messageStr = message.parameters[1];

		bool isVoteMessage = false;
		if (messageStr[0] > '0' && messageStr[0] <= '9') {
			if (messageStr.length() == 1) isVoteMessage = true;

			// special case for 7tv antispam
			if (messageStr.length() == 4 && (uint8_t)messageStr[1] == 0x20 && (uint8_t)messageStr[2] == 0xCD && (uint8_t)messageStr[3] == 0x8F) isVoteMessage = true;
		}

		if (isVoteMessage) {
			mVotingMutex.lock();
			OnVoteCast(message.prefix.user, (messageStr[0] - '1'));
			mVotingMutex.unlock();
		}
	}

	void IRCThread() {
		while (gIRCClient.Connected()) {
			gIRCClient.ReceiveData();
		}
		WriteLog("IRC thread quitting");
	}

	void Connect() {
		if (gIRCClient.Connected()) return;

		static bool bOnce = true;
		if (bOnce) {
			if (!gIRCClient.InitSocket()) {
				MessageBoxA(0, "InitSocket failed", "nya?!~", MB_ICONERROR);
				return;
			}
			gIRCClient.HookIRCCommand("PRIVMSG", &OnMessageReceived);
			bOnce = false;
		}

		if (!gIRCClient.Connect("irc.chat.twitch.tv", 6667)) {
			MessageBoxA(0, "Connect failed", "nya?!~", MB_ICONERROR);
			return;
		}
		auto login = std::format("justinfan{}", GetRandomNumber(1024, 32767));
		if (!gIRCClient.Login(login, login)) {
			if (gIRCClient.Connected()) {
				gIRCClient.Disconnect();
			}
			MessageBoxA(0, "Login failed", "nya?!~", MB_ICONERROR);
			return;
		}

		gIRCClient.SendIRC(std::format("JOIN #{}", sChannelName));
		std::thread(IRCThread).detach();
	}

	void Disconnect() {
		bAutoReconnect = false;
		gIRCClient.Disconnect();
	}

	bool IsEnabled() {
		return gIRCClient.Connected();
	}
}