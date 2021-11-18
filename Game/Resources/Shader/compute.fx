#ifndef _COMPUTE_FX_
#define _COMPUTE_FX_

#include "params.fx"

// u : unordered register : compute shader 전용 레지스터
// 보통은 레지스터로 받은 것들은 read only이지만
// g_'rw'tex 이므로 read write가 모두 가능한 texture임
RWTexture2D<float4> g_rwtex_0 : register(u0);

// 쓰레드 그룹당 쓰레드 개수
// max : 1024 (CS_5.0) ( X * Y * Z <= 1024)
// - 하나의 쓰레드 그룹은 하나의 다중처리기에서 실행
[numthreads(1024, 1, 1)] // 우리의 화면 크기는 1024 768. 

/*
* < Dispatch(5, 3, 2) : Thread Group 모음의 Size >
SV_GroupID = (2, 1, 0)
000 100 200 300 400         001 101 201 301 401
010    '210'                011 ...
020                         ...

* < numthreads(10, 8, 3) : 각 group 내부의 size
SV_GroupThreadID = (7, 5, 0) (for SV_GroupID 210)
000 100 200 300 400 500 600 700 800 900     001 101 201 ...     002 102 202 ...
...
...
...
...
...
...
                           '750'
...

SV_GroupThreadID = (7, 5, 0)
SV_GroupID = (2, 1, 0)
SV_DispatchThreadID = SV_GroupID * numthreads + SV_GroupThreadID
                    = [(2, 1, 0) * (10, 8, 3)] + (7, 5, 0) = (27, 13, 0)
SV_GroupIndex = 0*10*8 + 5*10 + 7 = 57
*/

void CS_Main(int3 threadIndex : SV_DispatchThreadID)
{
    if (threadIndex.y % 2 == 0)
        g_rwtex_0[threadIndex.xy] = float4(1.f, 0.f, 0.f, 1.f);
    else
        g_rwtex_0[threadIndex.xy] = float4(0.f, 1.f, 0.f, 1.f);
}

#endif