//! \file       segfault_handler.h
//! \brief      Misc functions for catching and handling segfaults

#pragma once

#include <memory>
#include <string>

namespace olio {
namespace utils {

void InstallSegfaultHandler();
std::string Backtrace(int skip);

}  // namespace utils
}  // namespace olio
