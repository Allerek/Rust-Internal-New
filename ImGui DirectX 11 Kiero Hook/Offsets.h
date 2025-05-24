#pragma once
#include <cstdint>
#include "Memory.h"
#include "globals.h"
namespace Offsets {
	constexpr uint64_t il2cppGCHandleBase = 0xC057BA0;
	constexpr uint64_t BaseNetworkableOffset = 0xBDB0D38;
	constexpr uint64_t staticBaseNetOffset = 0xB8;
	constexpr uint64_t wrapperPtrOffset = 0x10;
}

uint64_t decryptIl2cppHandle(int32_t ObjectHandleID)
{
    uint64_t rdi_1 = ((uint64_t)(ObjectHandleID >> 3));
    uint64_t rcx_1 = ((uint64_t)((ObjectHandleID & 7) - 1));
    
    // Calculate base address for GC handle
    uintptr_t gcHandleBase = Globals::BaseAddr + Offsets::il2cppGCHandleBase;
    
    // Read ObjectArray pointer
    uintptr_t ObjectArray = *(uintptr_t*)((rcx_1 * 0x28) + gcHandleBase + 0x8) + (rdi_1 << 3);
    
    // Read flag byte
    uint8_t flag = *(uint8_t*)((rcx_1 * 0x28) + gcHandleBase + 0x14);
    
    if (flag > 1)
    {
        return *(uintptr_t*)ObjectArray;
    }
    else
    {
        uint32_t eax = *(uint32_t*)ObjectArray;
        eax = ~eax;
        return eax;
    }
}

inline uint64_t BaseNetworkable(uint64_t wrapper)
{
    uint64_t rax = *(uint64_t*)(wrapper + 0x18);
    uint64_t* rdx = &rax;
    uint32_t r8d = 0x2;
    uint32_t eax, ecx, edx;

    do {
        eax = *(uint32_t*)(rdx);
        rdx = (uint64_t*)((uint8_t*)rdx + 0x4);
        ecx = eax - 36061522;
        ecx = ecx ^ 0x73999BB1u;
        ecx = ecx + 726187055;
        edx = (ecx * 2) | (ecx >> 31);
        *((uint32_t*)rdx - 1) = edx;
        --r8d;
    } while (r8d);

    return decryptIl2cppHandle(rax);
}

inline uintptr_t BaseNetworkable2(uintptr_t pointer)
{
    uint64_t rax = *(uint64_t*)(pointer + 0x18);
    uint64_t* rdx = &rax;
    uint32_t r8d = 2;
    uint32_t eax, ecx, cond;

    do {
        eax = *(uint32_t*)(rdx);
        rdx = (uint64_t*)((uint8_t*)rdx + 4);
        ecx = (eax - 784061614u) ^ 0xA406ACCau;
        ecx = ecx + 907435896u;
        ecx = ecx ^ 0xB6679C03u;
        *((uint32_t*)rdx - 1) = ecx;
        cond = (r8d == 1);
        --r8d;
    } while (!cond);

    return decryptIl2cppHandle(rax);
}
