#include "tiger/regalloc/regalloc.h"

#include "tiger/output/logger.h"

extern frame::RegManager *reg_manager;

namespace ra {
/* TODO: Put your lab6 code here */
RegAllocator::RegAllocator(frame::Frame* frame_,std::unique_ptr<cg::AssemInstr> ptr){
    this->frame_=frame_;
    this->instr_=ptr.get()->GetInstrList();
    this->colorManager=new col::Color(this->frame_,this->instr_);
}
void RegAllocator::RegAlloc(){
    this->colorManager->run();
    // this->result.coloring_=this->colorManager->AssignRegisters();
    // this->result.il_=this->colorManager->getInstrList();
}
std::unique_ptr<ra::Result> RegAllocator::TransferResult(){
    //return Result(this->colorManager->AssignRegisters(),this->colorManager->getInstrList());
    return std::make_unique<ra::Result>(this->colorManager->AssignRegisters(),this->colorManager->getInstrList());
}
} // namespace ra