class CHUD_Wrecked : public CIngameHUDElement {
public:
	struct tNotif {
		std::string string1;
		std::string string2;
		bool fadeSize;
	};
	std::vector<tNotif> aNotifs;
	double fNotifTimer = 0;

	float fNotifX = 0.5;
	float fNotifY = 0.1;
	float fNotifSize = 0.05;
	float fNotifSize2 = 0.1;

	static constexpr float fNotifFadeinStart = 3;
	static constexpr float fNotifFadeinEnd = 2.5;
	static constexpr float fNotifFadeinSpeed = 2;
	static constexpr float fNotifFadeoutStart = 0.5;
	static constexpr float fNotifFadeoutSpeed = 2;

	void AddWreckedNotif(GRacerInfo* pPlayer) {
		if (pPlayer->mIndex == 0) {
			AddNotif("YOU ARE WRECKED!", "", true);
		}
		else {
			AddNotif(pPlayer->mName, "IS WRECKED", true);
		}
	}

	void AddKONotif(GRacerInfo* pPlayer) {
		if (pPlayer->mIndex == 0) {
			AddNotif("YOU ARE KNOCKED OUT!", "", true);
		}
		else {
			AddNotif(pPlayer->mName, "IS KNOCKED OUT", true);
		}
	}

	void AddTimeoutNotif(GRacerInfo* pPlayer) {
		if (pPlayer->mIndex == 0) {
			AddNotif("OUT OF TIME!", "", true);
		}
		else {
			AddNotif(pPlayer->mName, "RAN OUT OF TIME", true);
		}
	}

	void DrawWreckedNotif() {
		tNyaStringData data;
		data.x = fNotifX;
		data.y = fNotifY;
		data.size = fNotifSize;
		data.XCenterAlign = true;
		int a = 255;
		if (fNotifTimer >= fNotifFadeinEnd) {
			a = (fNotifFadeinStart - fNotifTimer) * fNotifFadeinSpeed * 255;
			if (aNotifs[0].fadeSize) data.size = std::lerp(fNotifSize2, fNotifSize, a / 255.0);
		}
		if (fNotifTimer <= fNotifFadeoutStart) {
			a = fNotifTimer * fNotifFadeoutSpeed * 255;
		}
		data.SetColor(255,255,255,a);
		DrawStringFO2_Condensed12(data, aNotifs[0].string1);
		data.y += data.size * 0.9;
		DrawStringFO2_Condensed12(data, aNotifs[0].string2);
	}

	void Process() override {
		if (!IsRaceHUDUp()) return;
		if (!IsInRace()) return;

		static bool bWrecked[16] = {};
		for (int i = 0; i < 16; i++) {
			if (i >= GRaceStatus::fObj->mRacerCount) {
				bWrecked[i] = false;
				continue;
			}
			auto ply = &GRaceStatus::fObj->mRacerInfo[i];
			bool wrecked = ply->mDNF || ply->mTotalled || ply->mEngineBlown || ply->mKnockedOut || ply->mBusted;
			if (wrecked && !bWrecked[i]) {
				if (ply->mKnockedOut) AddKONotif(ply);
				else AddWreckedNotif(ply);
			}
			bWrecked[i] = wrecked;
		}

		static CNyaTimer gTimer;
		gTimer.Process();
		if (fNotifTimer > 0) {
			fNotifTimer -= gTimer.fDeltaTime;
			// increase speed if there's more than one
			if (aNotifs.size() > 1) {
				//fNotifTimer -= gTimer.fDeltaTime;
				if (fNotifTimer < fNotifFadeinEnd && fNotifTimer > fNotifFadeoutStart) {
					fNotifTimer = fNotifFadeoutStart;
				}
			}
			// play next in queue
			if (fNotifTimer <= 0) {
				if (!aNotifs.empty()) aNotifs.erase(aNotifs.begin());
				if (!aNotifs.empty()) fNotifTimer = fNotifFadeinStart;
			}
		}
		else if (!aNotifs.empty()) {
			fNotifTimer = fNotifFadeinStart;
		}

		//if (pPlayerHost->nRaceTime <= 0) {
		//	fNotifTimer = 0;
		//	aNotifs.clear();
		//}

		if (!aNotifs.empty()) {
			DrawWreckedNotif();
		}
	}

	void AddNotif(const std::string& notif, const std::string& notif2, bool fadeSize) {
		if (!aNotifs.empty() && aNotifs[aNotifs.size()-1].string1 == notif && aNotifs[aNotifs.size()-1].string2 == notif2) return; // ignore immediate duplicates
		aNotifs.push_back({notif, notif2, fadeSize});
	}
} HUD_Wrecked;