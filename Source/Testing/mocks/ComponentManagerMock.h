#ifndef L_ENGINE_TEST_COMP_MAN_MOCK
#define L_ENGINE_TEST_COMP_MAN_MOCK

#include "../../Engine/BaseComponentManager.h"

class ComponentTestManager;
class ComponentTest : public BaseComponent{
	public:
		ComponentTest(EID id, ComponentTestManager* manager);
		~ComponentTest();

		void Update();
		unsigned int GetUpdateCount();

        void    HandleEvent(const Event* event);

	private:
		///Number of times that 'Update' has been called
		unsigned int updateCount;
};

class ComponentTestManager : public BaseComponentManager_Impl<ComponentTest>{
	public:
        ComponentTestManager(EventDispatcher* e);
		std::unique_ptr<ComponentTest> ConstructComponent(EID id, ComponentTest* parent);
};


#endif
