#pragma once

#include <Windows.h>
#include <Psapi.h>
#include <string>
#include <vector>
#include <cstdint>
#include <type_traits>

namespace mem {

    // Check if a memory address is readable
    inline bool is_valid_ptr(uintptr_t addr, size_t size = sizeof(void*)) {
        MEMORY_BASIC_INFORMATION mbi{};
        if (VirtualQuery(reinterpret_cast<void*>(addr), &mbi, sizeof(mbi))) {
            return !(mbi.Protect & PAGE_NOACCESS) && !(mbi.State & MEM_FREE);
        }
        return false;
    }

    // Safely read memory
    template<typename T>
    bool read(uintptr_t address, T& out) {
        if (is_valid_ptr(address, sizeof(T))) {
            out = *reinterpret_cast<T*>(address);
            return true;
        }
        return false;
    }

    // Read a pointer chain (multi-level pointer)
    template<typename T = uintptr_t>
    T read_chain(uintptr_t base, const std::vector<uintptr_t>& offsets) {
        uintptr_t addr = base;
        for (uintptr_t offset : offsets) {
            if (!is_valid_ptr(addr)) return 0;
            addr = *reinterpret_cast<uintptr_t*>(addr);
            addr += offset;
        }
        return reinterpret_cast<T>(addr);
    }

    // Get the base address of a loaded module by name
    inline uintptr_t get_module_base(const char* module_name) {
        return reinterpret_cast<uintptr_t>(GetModuleHandleA(module_name));
    }

    // Get the size of a module
    inline size_t get_module_size(const char* module_name) {
        MODULEINFO modInfo{};
        HMODULE hMod = GetModuleHandleA(module_name);
        if (hMod && GetModuleInformation(GetCurrentProcess(), hMod, &modInfo, sizeof(modInfo))) {
            return static_cast<size_t>(modInfo.SizeOfImage);
        }
        return 0;
    }

    // Pattern scanning inside a module (basic AOB scan)
    inline uintptr_t pattern_scan(const char* module_name, const char* pattern, const char* mask) {
        uintptr_t base = get_module_base(module_name);
        size_t size = get_module_size(module_name);

        for (size_t i = 0; i < size; ++i) {
            bool found = true;
            for (size_t j = 0; mask[j]; ++j) {
                if (mask[j] != '?' && pattern[j] != *reinterpret_cast<char*>(base + i + j)) {
                    found = false;
                    break;
                }
            }
            if (found) {
                return base + i;
            }
        }
        return 0;
    }

}
