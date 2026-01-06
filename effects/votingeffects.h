#define EFFECT_CATEGORY_TEMP "Voting"

class Effect_VotingRigged : public ChaosEffect {
public:
	Effect_VotingRigged() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Rigged Votes";
		sFriendlyName = "Rigged Votes (Lowest Voted Wins)";
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
	bool IsAvailable() override { return ChaosVoting::IsEnabled(); }
	bool AbortOnConditionFailed() override { return true; }
	bool CanQuickTrigger() override { return false; }
	void OnTimerRefill() override { InitFunction(); }
} E_VotingRigged;

class Effect_VotingAll : public ChaosEffect {
public:
	Effect_VotingAll() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Activate All Votes";
	}

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
	bool IsAvailable() override { return ChaosVoting::IsEnabled(); }
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

	static inline int max = 6;

	void InitFunction() override {
		for (auto& effect : aEffects) {
			effect->bTriggeredThisCycleSmart = false;
		}
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

class Effect_VotingAdd : public ChaosEffect {
public:
	Effect_VotingAdd() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Add Extra Voting Option";
	}

	static inline int max = 4;

	void InitFunction() override {
		ChaosVoting::nAddVotingOption = max;
	}
	void TickFunctionMain(double delta) override {
		fTimerLength = max * 5;
		EffectInstance->fTimer = ChaosVoting::nAddVotingOption * 5;
	}
	bool ShouldAbort() override { return !ChaosVoting::nAddVotingOption; }
	bool HasTimer() override { return true; }
	bool IsAvailable() override { return ChaosVoting::IsEnabled() && ChaosVoting::nNumVoteOptions < 9; }
	bool AbortOnConditionFailed() override { return true; }
	void OnTimerRefill() override { InitFunction(); }
	bool InitImmediately() override { return true; }
} E_VotingAdd;

class Effect_VotingCheat : public ChaosEffect {
public:
	Effect_VotingCheat() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Full Chat Chaos Control";
		fTimerLength = 30;
	}

	void InitFunction() override {
		ChaosVoting::ActivateChatCheat();

		CwoeeHints::AddHint("Type an effect's name without spaces & symbols into chat!");
		CwoeeHints::AddHint("Any valid effect will be activated immediately.");

		ChaosEffect* effect = GetRandomEffect(true);
		if (!effect) effect = &E_Teddie;
		CwoeeHints::AddHint(std::format("Example: {}", effect->GetCheatCode(PercentageChanceCheck(50))));
	}
	void DeinitFunction() override {
		ChaosVoting::DeactivateChatCheat();
	}
	bool HasTimer() override { return true; }
	bool IsAvailable() override { return ChaosVoting::IsEnabled(); }
	bool AbortOnConditionFailed() override { return true; }
	bool CanQuickTrigger() override { return false; }
} E_VotingCheat;