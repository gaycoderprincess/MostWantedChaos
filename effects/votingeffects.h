#define EFFECT_CATEGORY_TEMP "Voting"

class Effect_VotingRigged : public ChaosEffect {
public:
	Effect_VotingRigged() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Rigged Votes";
		sFriendlyName = "Rigged Votes (Lowest Voted Wins)";
		bAbortOnConditionFailed = true;
		bCanQuickTrigger = false;
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
	void OnTimerRefill() override { InitFunction(); }
} E_VotingRigged;

class Effect_VotingAll : public ChaosEffect {
public:
	Effect_VotingAll() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Activate All Votes";
		bAbortOnConditionFailed = true;
		bRigProportionalChances = true;
		bCanQuickTrigger = false;
		bRigProportionalChances = true;
	}

	bool IsAvailable() override { return ChaosVoting::bSelectingEffectsForVote; }
} E_VotingAll;

class Effect_VotingStreamer : public ChaosEffect {
public:
	Effect_VotingStreamer() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Streamer Votes (Press 1/2/3/4)";
		sFriendlyName = "Streamer Votes";
		bAbortOnConditionFailed = true;
		bCanQuickTrigger = false;
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
	void OnTimerRefill() override { InitFunction(); }
} E_VotingStreamer;

class Effect_VotingSmart : public ChaosEffect {
public:
	Effect_VotingSmart() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Insanely Unfair RNG";
		nFrequency *= 2;
		fCycleTimeMultiplier = 2;
		bCanQuickTrigger = false;
	}

	static inline int max = 5;

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
	void OnTimerRefill() override { InitFunction(); }
} E_VotingSmart;

class Effect_VotingAdd : public ChaosEffect {
public:
	Effect_VotingAdd() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Add Extra Voting Option";
		bAbortOnConditionFailed = true;
		bInitImmediately = true;
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
	void OnTimerRefill() override { InitFunction(); }
} E_VotingAdd;

class Effect_VotingCheat : public ChaosEffect {
public:
	Effect_VotingCheat() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Full Chat Chaos Control";
		fTimerLength = 45;
		bAbortOnConditionFailed = true;
		bCanQuickTrigger = false;
		bRigProportionalChances = true;
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
} E_VotingCheat;

class Effect_VotingMajority : public ChaosEffect {
public:
	Effect_VotingMajority() : ChaosEffect(EFFECT_CATEGORY_TEMP) {
		sName = "Majority Voting";
		bAbortOnConditionFailed = true;
		bInitImmediately = true;
	}

	static inline int max = 4;

	void InitFunction() override {
		ChaosVoting::nForceMajorityVoting = max;
	}
	void TickFunctionMain(double delta) override {
		fTimerLength = max * 5;
		EffectInstance->fTimer = ChaosVoting::nForceMajorityVoting * 5;
	}
	bool ShouldAbort() override { return !ChaosVoting::nForceMajorityVoting; }
	bool HasTimer() override { return true; }
	bool IsAvailable() override { return ChaosVoting::IsEnabled() && ChaosVoting::bProportionalVotes; }
	void OnTimerRefill() override { InitFunction(); }
} E_VotingMajority;