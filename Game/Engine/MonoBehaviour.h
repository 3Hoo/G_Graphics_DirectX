#pragma once
#include "Component.h"

class MonoBehaviour : public Component
{
public:
	MonoBehaviour();
	virtual ~MonoBehaviour();

private:
	// ���� MonoBehaviour�� ������ FinalUpdate�� ������� ���� ���̴�
	// (���� ��ӹ��� ����� FianlUpdate ȣ�� �Ұ�)
	virtual void FinalUpdate() sealed { };
};
