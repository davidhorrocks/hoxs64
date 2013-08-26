#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <assert.h>
#include <vector>
#include <list>
#include <algorithm>

#include "boost2005.h"
#include "defines.h"
#include "bits.h"
#include "savestate.h"


const char SaveState::SIGNATURE[]= "COMMODORE 64 STATE SNAPSHOT";
const char SaveState::NAME[]= "Hoxs64";