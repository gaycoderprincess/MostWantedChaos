UMath::Vector3 SceneryScale = {1,1,1};
UMath::Vector3 SceneryMove = {0,0,0};

auto WorldRenderOrig = (void(__thiscall*)(eViewPlatInterface*, eModel*, bMatrix4*, eLightContext*, unsigned int, bMatrix4*))nullptr;
void __thiscall WorldRenderHooked(eViewPlatInterface* pThis, eModel* model, bMatrix4* local_world, eLightContext* light_context, unsigned int flags, bMatrix4* blending_matricies) {
	// these pointers are saved for later, so they all need to be unique
	static UMath::Matrix4 tmp[4096];
	static int counter = 0;
	auto mat = &tmp[counter++];
	if (counter >= 4096) counter = 0;
	*mat = local_world ? *local_world : UMath::Matrix4::kIdentity;

	mat->p.x += SceneryMove.x;
	mat->p.y += SceneryMove.y;
	mat->p.z += SceneryMove.z;

	/*if (auto rb = GetLocalPlayerInterface<IRigidBody>()) {
		UMath::Matrix4 ply;
		rb->GetMatrix4(&ply);
		ply = (UMath::Matrix4)WorldToRenderMatrix(ply);
		*mat *= ply.Invert();
		mat->x *= SceneryScale.x;
		mat->y *= SceneryScale.y;
		mat->z *= SceneryScale.z;
		mat->p.x *= SceneryScale.x;
		mat->p.y *= SceneryScale.y;
		mat->p.z *= SceneryScale.z;
		mat->p.x += SceneryMove.x;
		mat->p.y += SceneryMove.y;
		mat->p.z += SceneryMove.z;
		*mat *= ply;
	}*/

	return WorldRenderOrig(pThis, model, mat, light_context, flags, blending_matricies);
}

ChloeHook Hook_WorldRender([]() {
	WorldRenderOrig = (void(__thiscall*)(eViewPlatInterface*, eModel*, bMatrix4*, eLightContext*, unsigned int, bMatrix4*))NyaHookLib::PatchRelative(NyaHookLib::CALL, 0x7241FE, &WorldRenderHooked);
});