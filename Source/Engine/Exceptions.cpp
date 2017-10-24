#include "Exceptions.h"

LEngineFileException::LEngineFileException(const std::string &w,
                                           const std::string &fN)
    : LEngineException(w), fileName(fN) {}
