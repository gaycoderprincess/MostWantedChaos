#define EFFECT_CATEGORY_TEMP "Voting"

class Effect_VotingRigged : public ChaosEffect {
public:
	Effect_VotingRigged() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Rigged Votes";
	}

	static inline int max = 2;

	void InitFunction() override {
		ChaosVoting::nLowestWins = max;
	}
	void TickFunctionMain(double delta) override {
		fTimerLength = max * 5;
		EffectInstance->fTimer = ChaosVoting::nLowestWins * 5;
	}
	bool ShouldAbort() override { return !ChaosVoting::nLowestWins; }
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
	bool RunWhenBlocked() override { return true; }
	bool IsAvailable() override { return ChaosVoting::bEnabled; }
	bool AbortOnConditionFailed() override { return true; }
	bool CanQuickTrigger() override { return false; }
	void OnTimerRefill() override { InitFunction(); }
} E_VotingRigged;

class Effect_VotingAll : public ChaosEffect {
public:
	Effect_VotingAll() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Activate All Votes";
	}

	bool RunInMenus() override { return true; }
	bool RunWhenBlocked() override { return true; }
	bool IsAvailable() override { return ChaosVoting::bSelectingEffectsForVote; }
	bool AbortOnConditionFailed() override { return true; }
	bool CanQuickTrigger() override { return false; }
} E_VotingAll;

class Effect_VotingStreamer : public ChaosEffect {
public:
	Effect_VotingStreamer() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Streamer Votes (Press 1/2/3/4)";
		sFriendlyName = "Streamer Votes";
	}

	static inline int max = 3;

	void InitFunction() override {
		ChaosVoting::nStreamerVotes = max;
	}
	void TickFunctionMain(double delta) override {
		fTimerLength = max * 5;
		EffectInstance->fTimer = ChaosVoting::nStreamerVotes * 5;
	}
	bool ShouldAbort() override { return !ChaosVoting::nStreamerVotes; }
	bool HasTimer() override { return true; }
	bool RunInMenus() override { return true; }
	bool RunWhenBlocked() override { return true; }
	bool IsAvailable() override { return ChaosVoting::bEnabled; }
	bool AbortOnConditionFailed() override { return true; }
	bool CanQuickTrigger() override { return false; }
	void OnTimerRefill() override { InitFunction(); }
} E_VotingStreamer;

class Effect_VotingSmart : public ChaosEffect {
public:
	Effect_VotingSmart() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Insanely Unfair RNG";
		nFrequency *= 2;
	}

	static inline int max = 5;

	void InitFunction() override {
		nSmartRNG = max;
	}
	void TickFunctionMain(double delta) override {
		fTimerLength = max * 5;
		EffectInstance->fTimer = nSmartRNG * 5;
	}
	bool ShouldAbort() override { return !nSmartRNG; }
	bool HasTimer() override { return true; }
	bool CanQuickTrigger() override { return false; }
	void OnTimerRefill() override { InitFunction(); }
} E_VotingSmart;