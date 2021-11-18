#pragma once
#include "Object.h"

class Animator;

enum class COMPONENT_TYPE : uint8
{
	// MonoBehavior를 제외한 것들은 각자 슬롯 번호를 가진다
	// GameObject는 각 슬롯당 하나만을 가질 수 있다
	// 하지만 Mono_behavior의 경우 여러개를 만들어 꽂아줄 수 있다
	TRANSFORM,
	MESH_RENDERER,
	CAMERA,
	LIGHT,
	PARTICLE_SYSTEM,
	TERRAIN,
	COLLIDER,
	ANIMATOR,

	// ...
	MONO_BEHAVIOUR,
	END,
};

enum
{
	FIXED_COMPONENT_COUNT = static_cast<uint8>(COMPONENT_TYPE::END) - 1
};

class GameObject;
class Transform;
class MeshRenderer;

class Component : public Object
{
public:
	Component(COMPONENT_TYPE type);
	virtual ~Component();

public:
	virtual void Awake() { }		// 초기화
	virtual void Start() { }
	virtual void Update() { }		// 매 틱마다 업데이트
	virtual void LateUpdate() { }	// 업데이트 끝난 후 마지막 실행
	virtual void FinalUpdate() { }	// 엔진에서 계산에서 활용하는 찐막 업데이트

public:
	COMPONENT_TYPE GetType() { return _type; }
	bool IsValid() { return _gameObject.expired() == false; }

	shared_ptr<GameObject> GetGameObject();
	shared_ptr<Transform> GetTransform();
	shared_ptr<MeshRenderer> GetMeshRenderer();
	shared_ptr<Animator> GetAnimator();

private:
	// 오직 GameObject만 실행할 수 있도록 하고 싶다!
	friend class GameObject;
	void SetGameObject(shared_ptr<GameObject> gameObject) { _gameObject = gameObject; }


protected:
	COMPONENT_TYPE _type;
	// component 입장에서 어느 gameObject에 있는지 알아야 하고
	// 동시에 gameObject 입장에서 어떤 component를 가지고 있는지 알아야 함
	// 즉, 양쪽을 모두 shared_ptr를 만들어버리면 loop 관계가 되므로 weak_ptr로 만듦
	weak_ptr<GameObject> _gameObject;
};

