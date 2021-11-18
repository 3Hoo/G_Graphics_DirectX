#pragma once
#include "Component.h"

class MonoBehaviour : public Component
{
public:
	MonoBehaviour();
	virtual ~MonoBehaviour();

private:
	// 이제 MonoBehaviour를 끝으로 FinalUpdate를 사용하지 않을 것이다
	// (이후 상속받은 놈들은 FianlUpdate 호출 불가)
	virtual void FinalUpdate() sealed { };
};
