#pragma once

// 라이브러리 추가 종속성 추가하기
#pragma comment(lib, "Engine.lib")


// pre-compiled header
// 이 파일을 한 번 컴파일해서 모든 파일에 include 시키는 것
// 여기서 자주 사용하는 라이브러리, stl 등을 포함시켜서 범용적으로 사용한다
#include "EnginePch.h"

#include <vector>
#include <memory>