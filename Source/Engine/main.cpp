#include "Kernel.h"
#include "CommandLineArgs.h"

int main(int argc, char *argv[]){
    Kernel* k=Kernel::Instance();
    k->Inst(argc, argv);

    Kernel.stateMan.PushState(std::move( std::unique_ptr<GameStartState> (new GameStartState(&stateMan)) ));
    Kernel.stateMan.UpdateCurrentState();

    while(k->Run()){}
    k->Close();

    return 0;
}
