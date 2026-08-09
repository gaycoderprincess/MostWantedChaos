namespace CustomPhysics {
	b3WorldId m_worldId;

	bool bLowQualityPhysics = false;

	b3MeshData* CreateMesh(b3Vec3* vertices, int numVertices, int* indices, int numIndices) {
		b3MeshDef def = {};
		def.vertices = vertices;
		def.vertexCount = numVertices;
		def.indices = indices;
		def.triangleCount = numIndices / 3;
		def.materialIndices = nullptr;
		def.useMedianSplit = false;
		def.identifyEdges = true;
		def.weldVertices = true;
		def.weldTolerance = 0.002f;

		b3MeshData* meshData = b3CreateMesh( &def, nullptr, 0 );
		return meshData;
	}

	struct CustomArticleInstance {
		CollisionCache::CachedInstance* pInstance = nullptr;
		b3MeshData* pB3Mesh = nullptr;
		b3BodyId nB3Body;
		bool bB3MeshEnabled;
	};

	struct CustomArticle {
		std::vector<CustomArticleInstance> aInstances;
	};
	CustomArticle aCollisionArticles[2701];

	const int COLLISIONARTICLE_CUSTOM = 2700;
	int nNumTrisCustom = 0;

	struct CustomObjectInstance {
		b3BodyId nB3Body;
		IRigidBody* pGameBody;
		bool bReturnChangesToGame = false;

		void CollectFromGameBody() {
			auto rb = pGameBody;

			auto p = *rb->GetPosition();

			UMath::Matrix4 m;
			rb->GetMatrix4(&m);

			b3Matrix3 m3;
			m3.cx.x = m.x.x;
			m3.cx.y = m.x.y;
			m3.cx.z = m.x.z;
			m3.cy.x = m.y.x;
			m3.cy.y = m.y.y;
			m3.cy.z = m.y.z;
			m3.cz.x = m.z.x;
			m3.cz.y = m.z.y;
			m3.cz.z = m.z.z;

			b3Body_SetTransform(nB3Body, {p.x,p.y,p.z}, b3MakeQuatFromMatrix(&m3));

			auto vel = *rb->GetLinearVelocity();
			auto avel = *rb->GetAngularVelocity();
			b3Body_SetLinearVelocity(nB3Body, {vel.x,vel.y,vel.z});
			b3Body_SetAngularVelocity(nB3Body, {avel.x,avel.y,avel.z});
		}

		void ApplyToGameBody() {
			auto m = b3MakeMatrixFromQuat(b3Body_GetRotation(nB3Body));

			UMath::Matrix4 mat;
			mat.x.x = m.cx.x;
			mat.x.y = m.cx.y;
			mat.x.z = m.cx.z;
			mat.y.x = m.cy.x;
			mat.y.y = m.cy.y;
			mat.y.z = m.cy.z;
			mat.z.x = m.cz.x;
			mat.z.y = m.cz.y;
			mat.z.z = m.cz.z;
			pGameBody->SetOrientation(&mat);

			auto p = b3Body_GetPosition(nB3Body);
			auto v = b3Body_GetLinearVelocity(nB3Body);
			auto av = b3Body_GetAngularVelocity(nB3Body);

			UMath::Vector3 pos;
			pos.x = p.x;
			pos.y = p.y;
			pos.z = p.z;
			pGameBody->SetPosition(&pos);

			UMath::Vector3 vel;
			vel.x = v.x;
			vel.y = v.y;
			vel.z = v.z;
			pGameBody->SetLinearVelocity(&vel);

			UMath::Vector3 avel;
			avel.x = av.x;
			avel.y = av.y;
			avel.z = av.z;
			pGameBody->SetAngularVelocity(&avel);
		}
	};
	std::vector<CustomObjectInstance> aB3Objects;

	CustomObjectInstance* GetGameObjectInstanceForB3Body(b3BodyId body) {
		for (auto& obj : aB3Objects) {
			if (B3_ID_EQUALS(obj.nB3Body, body)) {
				return &obj;
			}
		}
		return nullptr;
	}

	CustomObjectInstance* GetGameObjectInstanceForGameBody(IRigidBody* body) {
		for (auto& obj : aB3Objects) {
			if (obj.pGameBody == body) {
				return &obj;
			}
		}
		return nullptr;
	}

	IRigidBody* GetGameBodyForB3Body(b3BodyId body) {
		if (auto obj = GetGameObjectInstanceForB3Body(body)) {
			return obj->pGameBody;
		}
		return nullptr;
	}

	IVehicle* GetVehicleForB3Body(b3BodyId body) {
		if (auto game = GetGameBodyForB3Body(body)) return game->mCOMObject->Find<IVehicle>();
		return nullptr;
	}

	void ConvertCollisionArticle(CustomArticleInstance& article, bool makeDoubleSided) {
		if (article.pB3Mesh) return;

		std::vector<b3Vec3> vertices;
		std::vector<int> indices;
		for (auto& tri : article.pInstance->aTriStrips) {
			indices.push_back(vertices.size());
			vertices.push_back({tri.fPt0.x, tri.fPt0.y, tri.fPt0.z});
			indices.push_back(vertices.size());
			vertices.push_back({tri.fPt1.x, tri.fPt1.y, tri.fPt1.z});
			indices.push_back(vertices.size());
			vertices.push_back({tri.fPt2.x, tri.fPt2.y, tri.fPt2.z});

			if (makeDoubleSided) {
				indices.push_back(vertices.size());
				vertices.push_back({tri.fPt2.x, tri.fPt2.y, tri.fPt2.z});
				indices.push_back(vertices.size());
				vertices.push_back({tri.fPt1.x, tri.fPt1.y, tri.fPt1.z});
				indices.push_back(vertices.size());
				vertices.push_back({tri.fPt0.x, tri.fPt0.y, tri.fPt0.z});
			}
		}

		for (auto& tri : article.pInstance->aBarriers) {
			indices.push_back(vertices.size());
			vertices.push_back({tri.fPt0.x, tri.fPt0.y, tri.fPt0.z});
			indices.push_back(vertices.size());
			vertices.push_back({tri.fPt1.x, tri.fPt1.y, tri.fPt1.z});
			indices.push_back(vertices.size());
			vertices.push_back({tri.fPt2.x, tri.fPt2.y, tri.fPt2.z});

			if (makeDoubleSided) {
				indices.push_back(vertices.size());
				vertices.push_back({tri.fPt2.x, tri.fPt2.y, tri.fPt2.z});
				indices.push_back(vertices.size());
				vertices.push_back({tri.fPt1.x, tri.fPt1.y, tri.fPt1.z});
				indices.push_back(vertices.size());
				vertices.push_back({tri.fPt0.x, tri.fPt0.y, tri.fPt0.z});
			}
		}

		if (!vertices.empty()) {
			article.pB3Mesh = CreateMesh(&vertices[0], vertices.size(), &indices[0], indices.size());

			b3BodyDef def = b3DefaultBodyDef();
			def.type = b3_staticBody;
			def.position = {0,0,0};
			article.nB3Body = b3CreateBody(m_worldId, &def);

			b3ShapeDef shapeDef = b3DefaultShapeDef();
			//shapeDef.materials = materials;
			//shapeDef.materialCount = 7;
			b3CreateMeshShape(article.nB3Body, &shapeDef, article.pB3Mesh, b3Vec3_one);

			article.bB3MeshEnabled = true;
		}
	}

	bool bCollectLocalPlayerCar = true;
	float fWorldObjectMassScale = 100.0;
	float fWorldObjectMassMinimum = 400.0;

	bool CanCollectWorldObject(IRigidBody* rb) {
		if (!bCollectLocalPlayerCar && rb == GetLocalPlayerInterface<IRigidBody>()) return false;
		if (rb->GetMass() < fWorldObjectMassMinimum) return false;

		if (auto veh = rb->mCOMObject->Find<IVehicle>()) {
			if (auto rb = veh->mCOMObject->Find<IRBVehicle>()) {
				if (rb->GetInvulnerability() != INVULNERABLE_NONE) return false;
			}
		}
		return true;
	}

	bool bLowFPSMode = false;
	void PurgeWorldObjects() {
		std::vector<CustomObjectInstance> objectsToKeep;

		for (auto& obj : aB3Objects) {
			if (IsRigidBodyValidAndActive(obj.pGameBody)) {
				if (!CanCollectWorldObject(obj.pGameBody)) {
					obj.pGameBody = nullptr;
				}
			}
			else {
				obj.pGameBody = nullptr;
			}

			// either apply new state to game, or read new state from game and apply to b3
			if (obj.pGameBody) {
				if (obj.bReturnChangesToGame) {
					obj.ApplyToGameBody();
					obj.bReturnChangesToGame = false;
				}
				else {
					obj.CollectFromGameBody();
				}
			}

			if (!bLowFPSMode && obj.pGameBody) {
				objectsToKeep.push_back(obj);
			}
			else {
				b3DestroyBody(obj.nB3Body);
			}
		}
		aB3Objects.clear();
		aB3Objects = objectsToKeep;
	}

	void CollectWorldObjects() {
		auto objs = GetActiveRigidBodies();
		bLowFPSMode = objs.size() > 100;

		PurgeWorldObjects();

		for (auto& rb : objs) {
			if (bLowFPSMode && rb != GetLocalPlayerInterface<IRigidBody>()) continue;
			if (!CanCollectWorldObject(rb)) continue;
			if (GetGameObjectInstanceForGameBody(rb)) continue;

			auto objInst = CustomObjectInstance();

			UMath::Vector3 dim;
			rb->GetDimension(&dim);

			b3BodyDef def = b3DefaultBodyDef();
			def.type = b3_dynamicBody;
			def.position = {0,0,0};
			objInst.nB3Body = b3CreateBody(m_worldId, &def);

			b3ShapeDef shapeDef = b3DefaultShapeDef();
			auto hull = b3MakeBoxHull(dim.x, dim.y, dim.z);
			b3CreateHullShape(objInst.nB3Body, &shapeDef, &hull.base);

			b3MassData massData = b3Body_GetMassData(objInst.nB3Body);
			massData.mass = rb->GetMass() * fWorldObjectMassScale;
			//massData.inertia = {mass*dim.x,mass*dim.y,mass*dim.z};
			massData.center = {0,0,0};
			b3Body_SetMassData(objInst.nB3Body, massData);

			objInst.pGameBody = rb;
			objInst.CollectFromGameBody();
			aB3Objects.push_back(objInst);
		}
	}

	bool bEnabled = false;
	void OnWorldTick() {
		PerformanceBenchmarker _perf("CustomPhysics::OnWorldTick");

		if (!bEnabled) return;
		if (TheGameFlowManager.CurrentGameFlowState != GAMEFLOW_STATE_RACING) return;
		if (IsInLoadingScreen() || IsInMovie()) return;

		static CNyaTimer gTimer;
		gTimer.Process();

		for (int i = 0; i < 2700; i++) {
			auto& pack = CollisionCache::aCachedCollisions[i];
			if (pack.aInstances.empty()) continue;

			if (!aCollisionArticles[i].aInstances.empty()) continue;

			for (auto& inst : pack.aInstances) {
				CustomArticleInstance tmp;
				tmp.pInstance = &inst;
				aCollisionArticles[i].aInstances.push_back(tmp);
			}

			for (auto& inst : aCollisionArticles[i].aInstances) {
				ConvertCollisionArticle(inst, true);
			}
		}

		for (int i = 0; i < 2700; i++) {
			auto pack = WCollisionAssets::mCollisionPackList[i];
			if (!pack) continue;

			for (auto& inst : aCollisionArticles[i].aInstances) {
				auto enabled = !inst.pInstance->nSceneryGroupId || SceneryGroupEnabledTable[inst.pInstance->nSceneryGroupId];
				if (enabled != inst.bB3MeshEnabled) {
					if (enabled) {
						b3Body_Enable(inst.nB3Body);
					}
					else {
						b3Body_Disable(inst.nB3Body);
					}
					inst.bB3MeshEnabled = enabled;
				}
			}
		}

		auto customTris = Render3DObjects::GetFullTriList(true);
		if (nNumTrisCustom != customTris.size()) {
			WriteLog(std::format("nNumTrisCustom {}", nNumTrisCustom));
			WriteLog(std::format("customTris.size() {}", customTris.size()));

			// delete all old custom col instances first, keeping them makes no sense in case something despawns
			for (auto& oldInst : aCollisionArticles[COLLISIONARTICLE_CUSTOM].aInstances) {
				if (oldInst.pB3Mesh) {
					b3DestroyMesh(oldInst.pB3Mesh);
				}
				if (b3Body_IsValid(oldInst.nB3Body)) {
					b3DestroyBody(oldInst.nB3Body);
				}
			}
			aCollisionArticles[COLLISIONARTICLE_CUSTOM].aInstances.clear();
			CollisionCache::ClearTempArticle();

			for (auto& inst : customTris) {
				CollisionCache::ProcessCollisionArticle(COLLISIONARTICLE_CUSTOM, inst);
			}

			// combine everything into one huge collision article
			static auto bigArticleInstance = CollisionCache::CachedInstance();
			bigArticleInstance.aTriStrips.clear();
			bigArticleInstance.aBarriers.clear();

			auto bigArticle = CustomArticleInstance();
			bigArticle.pInstance = &bigArticleInstance;
			for (auto& inst : CollisionCache::gTempArticle.aInstances) {
				for (auto& tri : inst.aTriStrips) {
					bigArticle.pInstance->aTriStrips.push_back(tri);
				}
				inst.aTriStrips.clear();

				for (auto& tri : inst.aBarriers) {
					bigArticle.pInstance->aBarriers.push_back(tri);
				}
				inst.aBarriers.clear();
			}
			aCollisionArticles[COLLISIONARTICLE_CUSTOM].aInstances = {bigArticle};
			CollisionCache::ClearTempArticle();

			for (auto& inst : aCollisionArticles[COLLISIONARTICLE_CUSTOM].aInstances) {
				ConvertCollisionArticle(inst, false);
			}
		}
		nNumTrisCustom = customTris.size();

		CollectWorldObjects();

		if (!FEManager::mPauseRequest) {
			b3World_Step(m_worldId, gTimer.fDeltaTime * Sim::Internal::mSystem->mSpeed, bLowQualityPhysics ? 1 : 4);
		}
	}

	ChloeHook Init([]{
		aMainLoopFunctions.push_back(OnWorldTick);

		b3WorldDef worldDef = b3DefaultWorldDef();
		worldDef.workerCount = 8;
		m_worldId = b3CreateWorld(&worldDef);
	});
}