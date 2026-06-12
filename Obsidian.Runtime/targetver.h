// ============================================================================
//  File: targetver.h
//  Module: Obsidian.Runtime
//  Purpose:
//      Defines the minimum Windows platform version that the application
//      targets. This file is generated automatically by Visual Studio.
//
//  Why this file matters:
//      - It ensures the project uses the correct Windows SDK version.
//      - It controls which Win32 APIs are available at compile time.
//      - It prevents accidental use of APIs not supported by the chosen SDK.
//      - It is required for stable Win32 builds.
//
//  How it works:
//      - Including <SDKDDKVer.h> sets the target to the highest available
//        Windows version installed on the system.
//      - If you want to target an OLDER Windows version, you would:
//            1) include <WinSDKVer.h>
//            2) define _WIN32_WINNT to the desired version
//            3) then include <SDKDDKVer.h>
//        But we are NOT doing that here.
//
//  Notes:
//      - We KEEP this file exactly as-is.
//      - We do NOT modify _WIN32_WINNT unless we intentionally downgrade support.
//      - This file is required by framework.h and must remain present.
// ============================================================================

#pragma once

// Including SDKDDKVer.h defines the highest available Windows platform.
// If you wish to build your application for a previous Windows platform,
// include WinSDKVer.h and set the _WIN32_WINNT macro to the platform you
// wish to support before including SDKDDKVer.h.
#include <SDKDDKVer.h>
