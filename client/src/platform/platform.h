#ifndef PLATFORM_H
#define PLATFORM_H

// Include the appropriate platform implementation based on compiler definitions

#if defined(_WIN32)
#include "windows.h"
#elif defined(__linux__) || defined(__unix__)
#include "linux.h"
#else
#error "Unsupported platform"
#endif

#endif // PLATFORM_H