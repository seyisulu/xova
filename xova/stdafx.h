// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#include <string>
#include <vector>
#include <cassert>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <fstream>
#include <random>

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#else
# include <sys/time.h>
#endif

#include "cpprest/json.h"
#include "cpprest/http_listener.h"
#include "cpprest/uri.h"
#include "cpprest/asyncrt_utils.h"

#include "sqlite/sqlite3.h"
#include "sha1/sha1.h"