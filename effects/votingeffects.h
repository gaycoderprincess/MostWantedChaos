class Effect_VotingRigged : public ChaosEffect {
public:
	Effect_VotingRigged() : ChaosEffect("Uncategorized") {
		sName = "Rigged Votes";
	}

	void InitFunction() override {
		ChaosVoting::nLowestWins = 2;
	}
	void DeinitFunction() override {
		ChaosVoting::nLowestWins = 2;
	}
	void TickFunctionMain(double delta) override {
		EffectInstance->fTimer = fTimerLength * ChaosVoting::nLowestWins;
	}
	bool ShouldAbort() override { return !ChaosVoting::nLowestWins; }
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
	bool RunWhenBlocked() override { return true; }
	bool IsAvailable() override { return ChaosVoting::bEnabled; }
	bool AbortOnConditionFailed() override { return true; }
	bool CanQuickTrigger() override { return false; }
} E_VotingRigged;

class Effect_VotingAll : public ChaosEffect {
public:
	Effect_VotingAll() : ChaosEffect("Uncategorized") {
		sName = "Activate All Votes";
	}

	bool RunInMenus() override { return true; }
	bool RunWhenBlocked() override { return true; }
	bool IsAvailable() override { return ChaosVoting::bEnabled; }
	bool AbortOnConditionFailed() override { return true; }
	bool CanQuickTrigger() override { return false; }
} E_VotingAll;

class Effect_VotingStreamer : public ChaosEffect {
public:
	Effect_VotingStreamer() : ChaosEffect("Uncategorized") {
		sName = "Streamer Votes (Press 1/2/3/4)";
		sFriendlyName = "Streamer Votes";
	}

	void InitFunction() override {
		ChaosVoting::nStreamerVotes = 3;
	}
	void TickFunctionMain(double delta) override {
		EffectInstance->fTimer = fTimerLength * ChaosVoting::nStreamerVotes;
	}
	bool ShouldAbort() override { return !ChaosVoting::nStreamerVotes; }
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
	bool RunWhenBlocked() override { return true; }
	bool IsAvailable() override { return ChaosVoting::bEnabled; }
	bool AbortOnConditionFailed() override { return true; }
	bool CanQuickTrigger() override { return false; }
} E_VotingStreamer;