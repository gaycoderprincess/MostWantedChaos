namespace CustomPhysicsBall {
	bool bEnabled = false;

	bool bDoReset = false;

	float fMoveSpeed = 15.0;
	float fMaxMoveSpeed = 50.0;
	float fBallSize = 2.5;

	b3BodyId BallBody;

	void EnableBall() {
		bEnabled = true;
		CustomPhysics::bEnabled = true;
		CustomPhysics::bCollectLocalPlayerCar = false;
		CustomPhysics::fWorldObjectMassScale = 100.0;
		CustomPhysics::fWorldObjectMassMinimum = 400.0;
		CarRender_DontRenderPlayer = true;
	}

	void DisableBall() {
		bEnabled = false;
		CustomPhysics::bEnabled = false;
		CustomPhysics::bCollectLocalPlayerCar = true;
		CustomPhysics::fWorldObjectMassScale = 1.0;
		CustomPhysics::fWorldObjectMassMinimum = 15.0;
		CarRender_DontRenderPlayer = false;
	}

	void OnTick() {
		static bool bOnce = true;
		if (bOnce) {
			b3BodyDef def = b3DefaultBodyDef();
			def.type = b3_dynamicBody;
			def.position = {0,0,0};
			def.enableSleep = false;
			BallBody = b3CreateBody(CustomPhysics::m_worldId, &def);
			WriteLog(std::format("BallBody {}", BallBody.index1));

			b3ShapeDef shapeDef = b3DefaultShapeDef();
			b3Sphere sphere;
			sphere.center = {0,0,0};
			sphere.radius = fBallSize;
			b3CreateSphereShape(BallBody, &shapeDef, &sphere);

			//auto massData = b3Body_GetMassData(BallBody);
			//massData.mass = 100;
			//massData.inertia = {massData.mass*fBallSize,massData.mass*fBallSize,massData.mass*fBallSize};
			//massData.center = {0,0,0};
			//b3Body_SetMassData(BallBody, massData);

			bOnce = false;
		}

		if (!bEnabled) {
			bDoReset = true;
			return;
		}
		if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_RACING) {
			bDoReset = true;
			return;
		}
		if (IsInLoadingScreen() || IsInMovie()) {
			bDoReset = true;
			return;
		}
		
		static CNyaTimer gTimer;
		gTimer.Process();

		// ball controls and player car teleport
		if (auto ply = GetLocalPlayerInterface<IRigidBody>()) {
			if (bDoReset) {
				auto pos = *ply->GetPosition();
				pos.y += 5;
				auto q = *ply->GetOrientation();
				b3Body_SetTransform(BallBody, {pos.x, pos.y, pos.z}, {q.x, q.y, q.z, q.w});
				b3Body_SetLinearVelocity(BallBody, {0,0,0});
				b3Body_SetAngularVelocity(BallBody, {0,0,0});
				bDoReset = false;
			}

			if (FEManager::mPauseRequest) return;

			auto pos = b3Body_GetPosition(BallBody);
			auto quat = b3Body_GetRotation(BallBody);
			auto vel = b3Body_GetLinearVelocity(BallBody);
			auto avel = b3Body_GetAngularVelocity(BallBody);
			if (auto ply = GetLocalPlayerInterface<IRigidBody>()) {
				UMath::Vector3 v = {pos.x, pos.y, pos.z};
				ply->SetPosition(&v);
				v = {vel.x, vel.y, vel.z};
				ply->SetLinearVelocity(&v);
				v = {avel.x, avel.y, avel.z};
				ply->SetAngularVelocity(&v);

				UMath::Vector4 q = {quat.v.x, quat.v.y, quat.v.z, quat.s};
				ply->SetOrientation(&q);
				//auto m = b3MakeMatrixFromQuat(quat);
				//UMath::Matrix4 mat;
				//mat.x.x = m.cx.x;
				//mat.x.y = m.cx.y;
				//mat.x.z = m.cx.z;
				//mat.y.x = m.cy.x;
				//mat.y.y = m.cy.y;
				//mat.y.z = m.cy.z;
				//mat.z.x = m.cz.x;
				//mat.z.y = m.cz.y;
				//mat.z.z = m.cz.z;
				//ply->SetOrientation(&mat);
			}

			auto mat = PrepareCameraMatrix(GetLocalPlayerCamera());
			auto fwd = RenderToWorldCoords(mat.z);
			auto side = RenderToWorldCoords(mat.x);
			fwd.y = 0;
			side.y = 0;
			fwd.Normalize();
			side.Normalize();

			auto stick = NyaVec3(GetPadKeyState(NYA_PAD_KEY_LSTICK_X) / 32767.0,GetPadKeyState(NYA_PAD_KEY_LSTICK_Y) / -32767.0,0);
			if (TheGameFlowManager.CurrentGameFlowState == GAMEFLOW_STATE_RACING) {
				if (IsKeyPressed(VK_LEFT)) {
					stick.x = -1.0;
				}
				if (IsKeyPressed(VK_RIGHT)) {
					stick.x = 1.0;
				}
				if (IsKeyPressed(VK_UP)) {
					stick.y = -1.0;
				}
				if (IsKeyPressed(VK_DOWN)) {
					stick.y = 1.0;
				}
			}

			if (stick.length() > 1.0) {
				stick.Normalize();
			}

			float y = vel.y;
			vel.y = 0;

			auto oldLen = b3Length(vel);
			vel.x += fwd.x * -stick.y * fMoveSpeed * gTimer.fDeltaTime;
			vel.z += fwd.z * -stick.y * fMoveSpeed * gTimer.fDeltaTime;
			vel.x += side.x * stick.x * fMoveSpeed * gTimer.fDeltaTime;
			vel.z += side.z * stick.x * fMoveSpeed * gTimer.fDeltaTime;
			auto newLen = b3Length(vel);
			if (newLen > fMaxMoveSpeed) {
				vel.x /= newLen;
				vel.z /= newLen;
				vel.x *= oldLen;
				vel.z *= oldLen;
			}
			// double force when braking
			else if (newLen < oldLen) {
				vel.x += fwd.x * -stick.y * fMoveSpeed * gTimer.fDeltaTime;
				vel.z += fwd.z * -stick.y * fMoveSpeed * gTimer.fDeltaTime;
				vel.x += side.x * stick.x * fMoveSpeed * gTimer.fDeltaTime;
				vel.z += side.z * stick.x * fMoveSpeed * gTimer.fDeltaTime;
			}

			b3ContactData contactData;
			b3Body_GetContactData(BallBody, &contactData, 1);
			if (b3Contact_IsValid(contactData.contactId) && (IsKeyJustPressed(VK_SPACE) || IsPadKeyJustPressed(NYA_PAD_KEY_A))) {
				y += 10;
			}
			vel.y = y;

			b3Body_SetLinearVelocity(BallBody, vel);
		}
	}

	void OnTick3D() {
		if (!bEnabled) {
			bDoReset = true;
			return;
		}
		if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_RACING) {
			bDoReset = true;
			return;
		}
		if (IsInLoadingScreen() || IsInMovie()) {
			bDoReset = true;
			return;
		}

		static auto mdl = Render3D::CreateModels("beachball.fbx");
		if (!mdl.empty()) {
			WriteLog("DFGHDFGHL:DFGHKLFDGHLKGF");

			auto rb = GetLocalPlayerInterface<ICollisionBody>();

			UMath::Matrix4 mat = *rb->GetMatrix4();
			mat.p = *rb->GetPosition();

			//auto pos = b3Body_GetPosition(BallBody);
			//mat.p.x = pos.x;
			//mat.p.y = pos.y;
			//mat.p.z = pos.z;

			mat.x *= fBallSize;
			mat.y *= fBallSize;
			mat.z *= fBallSize;

			mdl[0]->RenderAt(WorldToRenderMatrix(mat));
		}
	}

	ChloeHook Init([]{
		aDrawing3DLoopFunctions.push_back(OnTick3D);
		aMainLoopFunctions.push_back(OnTick);
	});
}