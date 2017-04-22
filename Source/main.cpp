#include "Kernel.h"
#include "CommandLineArgs.h"

int main(int argc, char *argv[]){
    Kernel* k=Kernel::Instance();
    k->Inst(argc, argv);

    while(k->Run()){}
    k->Close();

    return 0;
}
