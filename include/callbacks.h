#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <functional>
#include <string>

using LogCallback = std::function<void(const std::string&)>;
using ProgressCallback = std::function<void(float)>; // 0.0 to 100.0

#endif // CALLBACKS_H
