#pragma once
#include "Component.h"

enum class LIGHT_TYPE : uint8
{
	DIRECTIONAL_LIGHT,
	POINT_LIGHT,
	SPOT_LIGHT,
};

struct LightColor
{
	// hlsl�� 16����Ʈ�� �������� �����͸� �ܾ�´�
	// 16����Ʈ�� �ѱ�� ���� ���ο� array�� ���۵Ǿ����
	Vec4	diffuse;	// �׷��ϱ� Vec3�� ����ص� �Ǵ� �� �׳� 16����Ʈ�� ����
	Vec4	ambient;
	Vec4	specular;
};

struct LightInfo
{
	LightColor	color;		// 16 16 16
	Vec4		position;	// 16
	Vec4		direction;	// 16
	int32		lightType;	// 4
	float		range;		// 4
	float		angle;		// 4
	int32		padding;	// 4	�� ������� 16����Ʈ! 
};

struct LightParams
{
	uint32		lightCount;
	Vec3		padding;
	LightInfo	lights[50];
};

class Light : public Component
{
public:
	Light();
	virtual ~Light();

	virtual void FinalUpdate() override;
	void Render();
	void RenderShadow();

public:
	LIGHT_TYPE GetLightType() { return static_cast<LIGHT_TYPE>(_lightInfo.lightType); }

	const LightInfo& GetLightInfo() { return _lightInfo; }

	void SetLightDirection(Vec3 direction);

	void SetDiffuse(const Vec3& diffuse) { _lightInfo.color.diffuse = diffuse; }
	void SetAmbient(const Vec3& ambient) { _lightInfo.color.ambient = ambient; }
	void SetSpecular(const Vec3& specular) { _lightInfo.color.specular = specular; }

	void SetLightType(LIGHT_TYPE type);
	void SetLightRange(float range) { _lightInfo.range = range; }
	void SetLightAngle(float angle) { _lightInfo.angle = angle; }
	void SetLightIndex(int8 index) { _lightIndex = index; }

private:
	LightInfo _lightInfo = {};

	int8 _lightIndex = -1;
	shared_ptr<class Mesh> _volumeMesh;
	shared_ptr<class Material> _lightMaterial;
	
	// �׸��ڸ� �ľ��ϱ� ���� ���� ��ġ���� �Կ��ϴ� ī�޶�
	shared_ptr<GameObject> _shadowCamera;
};


