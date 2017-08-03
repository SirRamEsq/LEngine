#include "Kernel.h"
#include "CommandLineArgs.h"
#include "GameStates/GameRunning.h"

int main(int argc, char *argv[]){
//	Kernel* k=Kernel::Instance();
//	k->Inst(argc, argv);

	Kernel::Inst(argc, argv);

	while(Kernel::Run()){}
	Kernel::Close();

	return 0;
}
