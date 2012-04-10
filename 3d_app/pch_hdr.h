#pragma once

#include <sys/types.h>
#include <sys/stat.h>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>

#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#ifndef NTDDI_VERSION
#define NTDDI_VERSION   NTDDI_WIN7
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <DXGI.h>
#include <D3D11.h>
#include <D3DX11.h>
#include <D3Dcompiler.h>
#include <DxErr.h>
#include <Windows.h>
