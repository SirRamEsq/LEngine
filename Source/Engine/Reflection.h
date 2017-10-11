#ifndef LENGINE_REFLECTION
#define LENGINE_REFLECTION

#include <vector>

// m_inherits must also enable reflection!
#define ENABLE_REFLECTION_INHERIT(m_class, m_inherits)                                                                                  \
/*declare this class and its members to be public*/																						\
public:																																	\
class REFLECTION{																														\
public:                                                                                                                                 \
	static std::string GetClass() {                                                                                  					\
		return std::string(#m_class);                                                                                                   \
	}                                                                                                                                   \
																																		\
	static std::string GetParent() {                                                                           				 			\
		return m_inherits::REFLECTION::GetClass();                                                                                          		\
	}                                                                                                                                   \
																																		\
	static bool IsClass(const std::string &p_class) { return (p_class == (#m_class)) ? true : m_inherits::REFLECTION::IsClass(p_class); }    \
                                                                                                                                        \
	/*Non Recursive interface, calls parent recursive implementation*/																			\
	static void GetParents(std::vector<std::string> *parents) {                                                                     	\
		m_inherits::REFLECTION::_GetParents(parents);                                                                                				\
	}                                                                                                                                   \
																																		\
	/*Recursive implementation DONT CALL DIRECTLY*/																										\
	static void  _GetParents(std::vector<std::string>* parents){																		\
		m_inherits::REFLECTION::_GetParents(parents);                                                                               				\
		parents->push_back(m_class::REFLECTION::GetClass());                                                                              		\
	}																																	\
                                                                                                                                        \
};                                                                                                                                      \
/*Declare class member of type _REFLECTION to be public as well*/																		\
REFLECTION reflection;																													\
private:

#define ENABLE_REFLECTION(m_class)                                                                                                    	\
/*declare this class and its members to be public*/																						\
public:																																	\
class REFLECTION{																														\
public:                                                                                                                                 \
	static std::string GetClass() {                                                                        						        \
		return std::string(#m_class);                                                                                                   \
	}                                                                                                                                   \
																																		\
	static std::string GetParent() {                                                                            \
		return "";																														\
	}                                                                                                                                   \
																																		\
	static bool IsClass(const std::string &p_class) { return (p_class == (#m_class)) ? true : false; }       \
                                                                                                                                        \
	/*Non Recursive interface, calls recursive implementation*/																			\
	static void GetParents(std::vector<std::string> *parents) {                                                                     	\
		/*No Parents, return*/ return;                                                                                				\
	}                                                                                                                                   \
																																		\
	/*Recursive implementation DONT CALL DIRECTLY*/																						\
	static void  _GetParents(std::vector<std::string>* parents){																		\
		parents->push_back(m_class::REFLECTION::GetClass());                                                                           	\
	}																																	\
                                                                                                                                        \
};                                                                                                                                      \
/*Declare class member of type _REFLECTION to be public as well*/																		\
REFLECTION reflection;																													\
private:
#endif
