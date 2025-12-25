namespace CwoeeHints {
	struct tHint {
		std::string text;
		double timer;
		ChaosUIPopup popup;
	};
	std::vector<tHint> aHints;

	void AddHint(const std::string& text, float timer = 10) {
		aHints.push_back({text, timer});
	}

	bool HintsCleanup() {
		for (auto& hint : aHints) {
			if (hint.timer <= 0.0 && hint.popup.fTextTimer <= 0.0) {
				aHints.erase(aHints.begin() + (&hint - &aHints[0]));
				return true;
			}
		}
		return false;
	}

	void OnTick() {
		static CNyaTimer gTimer;
		gTimer.Process();

		float y = 0;
		for (auto& hint : aHints) {
			hint.timer -= gTimer.fDeltaTime;
			hint.popup.bIsHint = true;
			hint.popup.bLeftSide = true;
			hint.popup.Update(gTimer.fDeltaTime, hint.timer > 0.0);
			hint.popup.Draw(hint.text, y, false);
			y += 1 - hint.popup.GetOffscreenPercentage();
		}

		while (HintsCleanup()) {}
	}

	ChloeHook Init([]() {
		aDrawingLoopFunctions.push_back(OnTick);
	});
}