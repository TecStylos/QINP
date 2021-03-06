#pragma once

#include <set>

#include "Program.h"

ProgramRef generateProgram(const TokenListRef tokens, const std::set<std::string>& importDirs, const std::string& platform, const std::string& progPath);