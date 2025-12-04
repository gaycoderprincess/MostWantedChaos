namespace Render3DObjects {
	float CollisionStrength = 10;

	class Object {
	public:
		std::vector<Render3D::tModel*> aModels;
		NyaMat4x4 mMatrix = UMath::Matrix4::kIdentity;
		NyaVec3 vColPosition = UMath::Vector3::kZero;
		float fColSize = 1;

		Object(std::vector<Render3D::tModel*> models, NyaMat4x4 matrix, NyaVec3 colPosition, float collisionSize) : aModels(models), mMatrix(matrix), vColPosition(colPosition), fColSize(collisionSize) {}

		void CheckCollision(IRigidBody* other, double delta) {
			// todo while this works it's very janky and id prefer to actually add collisions to the map somehow later

			UMath::Vector3 dim;
			other->GetDimension(&dim);
			UMath::Vector3 fwd;
			other->GetForwardVector(&fwd);
			fwd.y = 0;
			fwd.Normalize();
			auto otherPos = *other->GetPosition();
			auto velocity = *other->GetLinearVelocity();

			auto colSize1 = fColSize + dim.x;
			auto colSize2 = fColSize + dim.z;
			auto dir = (otherPos - vColPosition);
			auto dirNorm = dir;
			dirNorm.Normalize();
			auto dirNormXZ = dirNorm;
			dirNormXZ.y = 0;
			dirNormXZ.Normalize();

			auto colSize = std::lerp(colSize1, colSize2, abs(dirNormXZ.Dot(fwd)));
			if ((otherPos - vColPosition).length() < colSize) {
				velocity += dir * CollisionStrength * delta * velocity.length();
				other->SetLinearVelocity(&velocity);
				auto newPos = vColPosition + (dirNorm * colSize);
				other->SetPosition((UMath::Vector3*)&newPos);
			}
		}
	};
	std::vector<Object> aObjects;

	void OnTick() {
		static CNyaTimer gTimer;
		gTimer.Process();

		auto cars = GetActiveVehicles();
		for (auto& obj : aObjects) {
			if (obj.aModels.empty()) continue;
			if (obj.aModels[0]->bInvalidated) continue;
			for (auto& car: cars) {
				obj.CheckCollision(car->mCOMObject->Find<IRigidBody>(), gTimer.fDeltaTime);
			}
		}
	}

	void OnTick3D() {
		for (auto& obj : aObjects) {
			for (auto& model : obj.aModels) {
				model->RenderAt(WorldToRenderMatrix(obj.mMatrix));
			}
		}
	}

	ChloeHook Init([](){
		aMainLoopFunctions.push_back(OnTick);
		aDrawing3DLoopFunctions.push_back(OnTick3D);
	});
}