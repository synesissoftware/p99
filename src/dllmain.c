/**
 * @file dllmain.c
 * @brief Windows DLL entry points for p99.
 *
 * @copyright Copyright (c) 2026, Matthew Wilson and Synesis Information
 *   Systems
 * @license BSD-3-Clause
 */

#include <p99/p99.h>

#include <windows.h>

/* --- DllGetVersion ---------------------------------------------------- */

struct p99_DllVersionInfo
{
    DWORD cbSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformID;
};

#define P99_DLLVER_PLATFORM_WINDOWS                         0x00000001

HRESULT WINAPI
DllGetVersion(struct p99_DllVersionInfo* pdvi)
{
    if (NULL == pdvi)
    {
        return E_INVALIDARG;
    }

    if (pdvi->cbSize != sizeof(struct p99_DllVersionInfo))
    {
        return E_INVALIDARG;
    }

    pdvi->dwMajorVersion = P99_VER_MAJOR;
    pdvi->dwMinorVersion = P99_VER_MINOR;
    pdvi->dwBuildNumber  = P99_VER;
    pdvi->dwPlatformID   = P99_DLLVER_PLATFORM_WINDOWS;

    return S_OK;
}
