#ifndef L_ENGINE_TEST_COMP_MAN_MOCK
#define L_ENGINE_TEST_COMP_MAN_MOCK

#include "../../Engine/BaseComponent.h"

class ComponentTest : public BaseComponent{
	public:
		ComponentTest(EID id, const std::string& logName);
		~ComponentTest();

		void Update();
		unsigned int GetUpdateCount();

        void    HandleEvent(const Event* event);

	private:
		///Number of times that 'Update' has been called
		unsigned int updateCount;
};

class ComponentTestManager : public BaseComponentManager{
	public:
        ComponentTestManager(EventDispatcher* e);
        void AddComponent(EID id, EID parent = 0);
};


#endif
