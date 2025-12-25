#include "IRCClient/src/IRCClient.h"

namespace ChaosVoting {
	bool bAutoReconnect = false;
	char sChannelName[64] = "";
	int nNumVoteOptions = 4;

	IRCClient* pIRCClient = nullptr;
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
			Popup.bIsVotingOption = true;
			Popup.bLeftSide = true;
			Popup.Draw(std::format("{} ({}%)", pEffect->GetFriendlyName(), (int)fVotePercentage), y, false);
		}
	};

	std::vector<VotingPopup> aOldVotes;
	std::vector<VotingPopup> aNewVotes;
	std::vector<ChaosEffect*> aNextVotes;

	// voting tweaks
	int nLowestWins = 0;
	int nStreamerVotes = 0;
	int nAddVotingOption = 0;
	ChaosEffect* pAllOfTheAbove = nullptr;
	bool bSelectingEffectsForVote = false;

	bool bRecordChatCheat = false;
	bool bRecordChatCheatDuplicates = false;
	int nNumVotingUsers = 0;
	std::vector<ChaosEffect*> aChatCheatEffects;
	std::vector<std::string> aChatCheatUsers;

	bool IsEffectInNextVotes(ChaosEffect* effect) {
		for (auto& vote : aNextVotes) {
			if (vote == effect) return true;
		}
		return false;
	}

	void GenerateNewVotes() {
		aNextVotes.clear();

		mVotingMutex.lock();
		aOldVotes = aNewVotes;
		aNewVotes.clear();
		mVotingMutex.unlock();

		// this shouldn't be in the mutex portion as some effects can check for voting being on
		bSelectingEffectsForVote = true;
		auto newVotesTemp = aNewVotes;
		int numVoteOptions = nNumVoteOptions;
		if (nAddVotingOption > 0) numVoteOptions++;
		for (int i = 0; i < numVoteOptions; i++) {
			ChaosEffect* effect = nullptr;
			do {
				effect = GetRandomEffect(false);
			} while (GetNumEffectsAvailableForRandom() > numVoteOptions && IsEffectInNextVotes(effect));
			aNextVotes.push_back(effect);
			newVotesTemp.push_back(VotingPopup(effect));
		}
		bSelectingEffectsForVote = false;

		mVotingMutex.lock();
		aNewVotes = newVotesTemp;
		if (nLowestWins > 0) nLowestWins--;
		if (nStreamerVotes > 0) nStreamerVotes--;
		if (nAddVotingOption > 0) nAddVotingOption--;
		mVotingMutex.unlock();
	}

	int GetTotalVoteCount() {
		int totalVotes = 0;
		for (auto& vote : aNewVotes) {
			totalVotes += vote.aVotes.size() + vote.bVotedByStreamer;
		}
		return totalVotes;
	}

	void TriggerHighestVotedEffect() {
		auto votes = aNewVotes;
		if (votes.empty()) return;

		auto voteCount = GetTotalVoteCount();
		nNumVotingUsers = voteCount;

		if (voteCount <= 0) {
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

	void OnVoteCast(const std::string& username, int i) {
		if (i < 0 || i >= aNewVotes.size()) return;
		for (auto& effect : aNewVotes) {
			for (auto& vote : effect.aVotes) {
				if (vote == username) {
					effect.aVotes.erase(effect.aVotes.begin() + (&vote - &effect.aVotes[0]));
					break;
				}
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

	void DrawUI(bool on) {
		mVotingMutex.lock();

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
				vote.Popup.Update(gTimer.fDeltaTime, on);
				vote.Draw(y++);
			}
		}

		static ChaosUIPopup VoteCountPopup;
		VoteCountPopup.bIsVotingOption = true;
		VoteCountPopup.bLeftSide = true;
		VoteCountPopup.Update(gTimer.fDeltaTime, on);
		VoteCountPopup.Draw(std::format("Vote Count: {}", GetTotalVoteCount()), y++, false);

		static ChaosUIPopup VoteDisabledPopup;
		VoteDisabledPopup.bIsVotingOption = true;
		VoteDisabledPopup.bLeftSide = true;
		VoteDisabledPopup.Update(gTimer.fDeltaTime, on && nSmartRNG > 0);
		VoteDisabledPopup.Draw("Voting is disabled!", y++, false);

		mVotingMutex.unlock();
	}

	void Update() {
		if (nNumVoteOptions < 2) nNumVoteOptions = 2;
		if (nNumVoteOptions > 9) nNumVoteOptions = 9;

		// first frame
		if (aNewVotes.empty()) {
			GenerateNewVotes();
		}

		mVotingMutex.lock();
		if (nStreamerVotes > 0) {
			for (int i = 0; i < aNewVotes.size(); i++) {
				if (IsKeyJustPressed('1' + i)) {
					OnStreamerVoteCast(i);
				}
			}
		}
		mVotingMutex.unlock();
	}

	void ActivateChatCheat() {
		ChaosVoting::bRecordChatCheat = true;
		ChaosVoting::bRecordChatCheatDuplicates = ChaosVoting::nNumVotingUsers <= 3; // allow duplicate effects per user if the chat is small
	}

	void DeactivateChatCheat() {
		ChaosVoting::bRecordChatCheat = false;
		ChaosVoting::aChatCheatUsers.clear();
		ChaosVoting::aChatCheatEffects.clear();
	}

	void ProcessChatCheatRequest(const std::string& username, const std::string& message) {
		DLLDirSetter _setdir;

		if (!bRecordChatCheatDuplicates) {
			for (auto& user : aChatCheatUsers) {
				if (user == username) return;
			}
		}

		ChaosEffect* pEffect = nullptr;
		for (auto& effect : ChaosEffect::aEffects) {
			if (!effect->MatchesCheatCode(message)) continue;
			if (!effect->CanQuickTrigger()) continue;
			if (!CanEffectActivate(effect)) continue;
			pEffect = effect;
		}
		if (!pEffect) return;

		if (!pEffect->CanMultiTrigger()) {
			for (auto& effect : aChatCheatEffects) {
				if (pEffect == effect) return;
			}
		}

		aChatCheatUsers.push_back(username);
		aChatCheatEffects.push_back(pEffect);

		if (auto effect = AddRunningEffect(pEffect)) {
			effect->sUsername = username;
		}
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
		else if (bRecordChatCheat) {
			ProcessChatCheatRequest(message.prefix.user, message.parameters[1]);
		}
	}

	void IRCThread() {
		while (pIRCClient->Connected()) {
			pIRCClient->ReceiveData();
		}
		mVotingMutex.lock();
		if (pIRCClient) {
			auto ptr = pIRCClient;
			pIRCClient = nullptr;
			delete ptr;
		}
		mVotingMutex.unlock();
		WriteLog("IRC thread quitting");
	}

	void Connect() {
		static std::string url = "irc.chat.twitch.tv";
		static int port = 6667;
		static std::string username = "";
		static std::string password = "";

		static bool bOnce = true;
		if (bOnce) {
			if (std::filesystem::exists("CwoeeChaos/irc.toml")) {
				auto toml = toml::parse_file("CwoeeChaos/irc.toml");
				url = toml["url"].value_or(url);
				port = toml["port"].value_or(port);
				username = toml["username"].value_or(username);
				password = toml["password"].value_or(password);
			}
			bOnce = false;

			WriteLog("IRC config initialized");
			WriteLog(std::format("URL: {}", url));
			WriteLog(std::format("Port: {}", port));
		}

		if (pIRCClient && pIRCClient->Connected()) return;
		pIRCClient = new IRCClient;
		pIRCClient->HookIRCCommand("PRIVMSG", &OnMessageReceived);

		if (!pIRCClient->InitSocket()) {
			MessageBoxA(0, "InitSocket failed", "nya?!~", MB_ICONERROR);
			delete pIRCClient;
			pIRCClient = nullptr;
			bAutoReconnect = false;
			return;
		}

		// this is stupid, the library takes non-const char* but doesn't ever modify it
		if (!pIRCClient->Connect((char*)url.c_str(), port)) {
			MessageBoxA(0, "Connect failed", "nya?!~", MB_ICONERROR);
			delete pIRCClient;
			pIRCClient = nullptr;
			bAutoReconnect = false;
			return;
		}

		auto login = std::format("justinfan{}", GetRandomNumber(1024, 32767));
		if (!username.empty()) login = username;
		if (!pIRCClient->Login(login, login, password)) {
			MessageBoxA(0, "Login failed", "nya?!~", MB_ICONERROR);
			if (pIRCClient->Connected()) {
				pIRCClient->Disconnect();
			}
			delete pIRCClient;
			pIRCClient = nullptr;
			bAutoReconnect = false;
			return;
		}

		pIRCClient->SendIRC(std::format("JOIN #{}", sChannelName));
		std::thread(IRCThread).detach();
	}

	void Reconnect() {
		mVotingMutex.lock();
		Connect();
		mVotingMutex.unlock();
	}

	void Disconnect() {
		bAutoReconnect = false;
		if (pIRCClient) pIRCClient->Disconnect();
	}

	bool IsEnabled() {
		mVotingMutex.lock();
		auto b = pIRCClient && pIRCClient->Connected();
		mVotingMutex.unlock();
		return b;
	}
}