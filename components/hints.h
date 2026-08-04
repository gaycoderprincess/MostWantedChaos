namespace CwoeeHints {
	struct tHint {
		std::string text;
		double timer;
		ChaosUIPopup popup;
	};
	std::vector<tHint> aHints;

	std::mutex mHintMutex;

	void AddHint(const std::string& text, float timer = 10) {
		mHintMutex.lock();
		aHints.push_back({text, timer});
		mHintMutex.unlock();
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

		mHintMutex.lock();

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

		mHintMutex.unlock();
	}

	ChloeHook Init([]() {
		aDrawingLoopFunctions.push_back(OnTick);
	});
}