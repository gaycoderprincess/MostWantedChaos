namespace StatTracker {
	int nTimesBusted = 0;
	double fTimePlayed = 0;

	void OnTick() {
		static CNyaTimer gTimer;
		gTimer.Process();

		if (bTimerEnabled && !IsInSplashScreenOrIntros() && !IsInLoadingScreen()) {
			fTimePlayed += gTimer.fDeltaTime;
		}
	}

	ChloeHook Init([]{
		aDrawingLoopFunctions.push_back(OnTick);
	});
}