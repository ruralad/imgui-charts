//-----------------------------------------------------------------------------
// IMGUI CONFIGURATION FILE
//-----------------------------------------------------------------------------
// This file is for Emscripten/WebAssembly builds
//-----------------------------------------------------------------------------

#pragma once

//---- Define assertion handler. Defaults to calling assert().
// If your macro uses multiple statements, make sure is enclosed in a 'do { .. } while (0)' block so it can be used as a single statement.
//#define IM_ASSERT(_EXPR)  OurAssert(_EXPR)
//#define IM_ASSERT(_EXPR)  ((void)(_EXPR))     // Disable asserts

//---- Define attributes of all API symbols declarations, e.g. for DLL under Windows
// Using Dear ImGui via a shared library is not recommended, so no need to use these.
//#define IMGUI_API __declspec( dllimport )

//---- Don't define obsolete functions/enums/behaviors. Consider enabling from time to time after updating to avoid using soon-to-be obsolete function/names.
//#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS

//---- Disable all of Dear ImGui code
//#define IMGUI_DISABLE

//---- Disable all of Dear ImGui or don't implement default handlers.
//#define IMGUI_DISABLE_DEFAULT_IO_HANDLERS

//---- Disable demo windows: ShowDemoWindow()/ShowStyleEditor() will be empty. Not recommended.
//#define IMGUI_DISABLE_DEMO_WINDOWS

//---- Disable metrics/debugger window: ShowMetricsWindow() will be empty.
//#define IMGUI_DISABLE_METRICS_WINDOW

//---- Disable io.ConfigDpiScaleFonts setting and font rendering using font size in pixels.
//#define IMGUI_DISABLE_DPI_SCALE_FONTS

//---- Disable automatic ini saving/loading functionality (io.IniFilename == nullptr)
//#define IMGUI_DISABLE_INI_SETTINGS_FILE_HANDLER

//---- Disable automatic .ini loading (io.IniFilename != nullptr) but keep automatic saving.
//#define IMGUI_DISABLE_INI_SETTINGS_FILE_LOADING

//---- Disable automatic .ini saving (io.IniFilename != nullptr) but keep automatic loading.
//#define IMGUI_DISABLE_INI_SETTINGS_FILE_SAVING

//---- Disable default font
//#define IMGUI_DISABLE_DEFAULT_FONT

//---- Disable default font + specify alternative font loaded from file
//#define IMGUI_DISABLE_DEFAULT_FONT
//#define IMGUI_ENABLE_FREETYPE
//#define IMGUI_DEFAULT_FONT  "c:\\Windows\\Fonts\\segoeui.ttf"

//---- Include imgui_user.h at the end of imgui.h
//#define IMGUI_INCLUDE_IMGUI_USER_H

//---- Use 32-bit for ImWchar (default is 16-bit) to support unicode planes 1-16. (e.g. point beyond 0xFFFF like emoticons, dingbats, symbols, shapes, ancient languages, etc.)
//#define IMGUI_USE_WCHAR32

//---- Disable obsolete functions that are marked Obsolete and will be removed.
//#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS

//---- Disable all warnings emitted by Dear ImGui functions.
//#define IMGUI_DISABLE_DEBUG_TOOLS

//---- Define implicit cast operators to convert back-and-forth between your math types and ImVec2/ImVec4.
// This is only required if you need to use Dear ImGui with custom math types (e.g. DirectX). Dear ImGui doesn't use this by default.
//#define IMGUI_DEFINE_MATH_OPERATORS

//---- Disable anti-aliasing on lines/borders. Disable if you are really tight on CPU/GPU.
//#define IMGUI_DISABLE_LINE_RECTANGLES_OPTIMIZATION

//---- We don't use this for Emscripten/WebAssembly
//#define IMGUI_DISABLE_FILE_FUNCTIONS

//---- Tip: You can add extra functions within the ImGui:: namespace, here or in your own headers files.
/*
namespace ImGui
{
    void MyFunction(const char* name, const MyMatrix44& v);
}
*/
