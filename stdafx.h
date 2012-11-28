// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

/*
#pragma once

#ifndef _WIN32_WINNT		// Windows XP 以降のバージョンに固有の機能の使用を許可します。                   
#define _WIN32_WINNT 0x0501	// これを Windows の他のバージョン向けに適切な値に変更してください。
#endif						

#include <windows.h>
//#include <stdio.h>
//#include <tchar.h>
*/

// TODO: プログラムに必要な追加ヘッダーをここで参照してください。
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
typedef int BOOL;
typedef __uint32_t ui32;
#include <gpac/isomedia.h>
#include <gpac/internal/isomedia_dev.h>
#include "common.h"

#define ZeroMemory(ptr,size) memset(ptr,0,size)
#define _stricmp(str1,str2) strcasecmp(str1,str2)
#define GetFileAttributes(path) ((__uint32_t)access(path,F_OK))
#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif
