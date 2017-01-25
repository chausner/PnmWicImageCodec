#pragma once

// MEMORY LEAK DETECTION
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
#ifndef DBG_NEW
//#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
//#define new DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new(nt) DBG_NEW // does not support nothrow modifier
#endif
#endif  // _DEBUG
// MEMORY LEAK DETECTION

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN // exclude rarely-used stuff from Windows headers

#include <windows.h>
#include <wincodec.h>
#include <Shlobj.h>
#include <vector>