#pragma once
#include "Object.h"

class Animator;

enum class COMPONENT_TYPE : uint8
{
	// MonoBehavior�� ������ �͵��� ���� ���� ��ȣ�� ������
	// GameObject�� �� ���Դ� �ϳ����� ���� �� �ִ�
	// ������ Mono_behavior�� ��� �������� ����� �Ⱦ��� �� �ִ�
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
	virtual void Awake() { }		// �ʱ�ȭ
	virtual void Start() { }
	virtual void Update() { }		// �� ƽ���� ������Ʈ
	virtual void LateUpdate() { }	// ������Ʈ ���� �� ������ ����
	virtual void FinalUpdate() { }	// �������� ��꿡�� Ȱ���ϴ� �� ������Ʈ

public:
	COMPONENT_TYPE GetType() { return _type; }
	bool IsValid() { return _gameObject.expired() == false; }

	shared_ptr<GameObject> GetGameObject();
	shared_ptr<Transform> GetTransform();
	shared_ptr<MeshRenderer> GetMeshRenderer();
	shared_ptr<Animator> GetAnimator();

private:
	// ���� GameObject�� ������ �� �ֵ��� �ϰ� �ʹ�!
	friend class GameObject;
	void SetGameObject(shared_ptr<GameObject> gameObject) { _gameObject = gameObject; }


protected:
	COMPONENT_TYPE _type;
	// component ���忡�� ��� gameObject�� �ִ��� �˾ƾ� �ϰ�
	// ���ÿ� gameObject ���忡�� � component�� ������ �ִ��� �˾ƾ� ��
	// ��, ������ ��� shared_ptr�� ���������� loop ���谡 �ǹǷ� weak_ptr�� ����
	weak_ptr<GameObject> _gameObject;
};

