// ============================================================================
//  File: Resource.h
//  Module: Obsidian.Runtime
//  Purpose:
//      Defines resource identifiers used by the runtime executable.
//      These IDs map to items in Obsidian.Runtime.rc such as dialogs,
//      icons, menus, and string resources.
//
//  Why we KEEP this file:
//      - It is generated automatically by Visual Studio.
//      - The resource compiler (RC.EXE) depends on these IDs.
//      - The .rc file references these constants directly.
//      - Removing or renaming anything here WILL break the build.
//
//  Notes:
//      - We do NOT modify these IDs unless we modify the .rc file.
//      - We do NOT delete the APSTUDIO blocks — they are used by the
//        Visual Studio resource editor.
// ============================================================================

//{{NO_DEPENDENCIES}}
// Microsoft Visual C++ generated include file.
// Used by Obsidian.Runtime.rc

// ---------------------------------------------------------------------------
//  String table identifiers
// ---------------------------------------------------------------------------
#define IDS_APP_TITLE            103   // Application title string

// ---------------------------------------------------------------------------
//  Main frame and dialog identifiers
// ---------------------------------------------------------------------------
#define IDR_MAINFRAME            128   // Main application frame resource
#define IDD_OBSIDIANRUNTIME_DIALOG 102 // Default dialog template
#define IDD_ABOUTBOX             103   // "About" dialog template

// ---------------------------------------------------------------------------
//  Menu command identifiers
// ---------------------------------------------------------------------------
#define IDM_ABOUT                104   // Menu: Help → About
#define IDM_EXIT                 105   // Menu: File → Exit

// ---------------------------------------------------------------------------
//  Icon resource identifiers
// ---------------------------------------------------------------------------
#define IDI_OBSIDIANRUNTIME      107   // Main application icon
#define IDI_SMALL                108   // Small icon (taskbar, alt‑tab)

// ---------------------------------------------------------------------------
//  Cursor and control identifiers
// ---------------------------------------------------------------------------
#define IDC_OBSIDIANRUNTIME      109   // Main window cursor
#define IDC_MYICON               2     // Custom icon placeholder

#ifndef IDC_STATIC
#define IDC_STATIC               -1    // Static control placeholder
#endif

// ---------------------------------------------------------------------------
//  APSTUDIO (Visual Studio Resource Editor) generated defaults
//  DO NOT MODIFY unless you know exactly what you're doing.
// ---------------------------------------------------------------------------
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS

#define _APS_NO_MFC              130
#define _APS_NEXT_RESOURCE_VALUE 129
#define _APS_NEXT_COMMAND_VALUE  32771
#define _APS_NEXT_CONTROL_VALUE  1000
#define _APS_NEXT_SYMED_VALUE    110

#endif
#endif
