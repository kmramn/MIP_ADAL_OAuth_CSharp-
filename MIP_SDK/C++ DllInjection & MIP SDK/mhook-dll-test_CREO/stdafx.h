#pragma once
#define NOMINMAX

#include <Windows.h>
#include <winerror.h>
#include <shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include <stdio.h>

#include <iostream>
#include <fstream>
using namespace std;

#include "mhook-lib/mhook.h"

#include "mhook-hook.h"


#include <mip/mip_configuration.h>
#include <mip/mip_context.h>
#include <mip/file/file_profile.h>
#include <mip/file/file_engine.h>
#include <mip/stream_utils.h>
#include <mip/stream.h>
using namespace mip;

#include "consent/consent_delegate_impl.h"
using namespace sample::consent;

#include "file/profile_observer.h"
#include "file/file_handler_observer.h"
#include "file/file_execution_state_impl.h"
#include "common/auth_delegate_impl.h"
#include "common/cxxopts.hpp"
using namespace sample::auth;

#pragma comment(lib, "mip_file_sdk.lib")
#pragma comment(lib, "mip_protection_sdk.lib")
#pragma comment(lib, "mip_upe_sdk.lib")

#include "mhook-hook.h"
using namespace MYMIP;