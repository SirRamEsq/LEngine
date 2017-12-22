#ifndef L_ENGINE_EXCEPTIONS
#define L_ENGINE_EXCEPTIONS

#include <exception>
#include <string>

// Ideally, whatever class wants to use this should inheret it's own exception
// from it using
// class Exception : public LEngineException{using
// LEngineException::LEngineException;};
// This way exception types can be specified with SomeClass::Exception
// and LEngineException can be used to refer to any engine generated exception
class LEngineException : public std::exception {
 public:
  LEngineException(const std::string &w) : wot(w) {}

  virtual const char *what() const throw() { return wot.c_str(); }

 private:
  const std::string wot;
};

class LEngineFileException : public LEngineException {
 public:
  LEngineFileException(const std::string &w, const std::string &fileName);

  const std::string fileName;
};

class ExitException : public std::exception {
 public:
  ExitException() {}
  virtual const char *what() const throw() { return "Exit"; }
};

#endif  // L_ENGINE_EXCEPTIONS
