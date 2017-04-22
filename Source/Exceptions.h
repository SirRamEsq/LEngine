#ifndef L_ENGINE_EXCEPTIONS
#define L_ENGINE_EXCEPTIONS

#include <exception>
#include <string>

class LEngineException : public std::exception{
    public:
        LEngineException(const std::string& w) : wot(w){}

        virtual const char* what () const throw () {
            return wot.c_str();
        }

        const std::string wot;
};

class LEngineFileException : public LEngineException {
    public:
        LEngineFileException(const std::string& w, const std::string& fileName);

        const std::string fileName;
};

#endif // L_ENGINE_EXCEPTIONS
