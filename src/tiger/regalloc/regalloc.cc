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
    assem::InstrList* final = this->RemoveMoveInstr(this->colorManager->getInstrList(),this->colorManager->AssignRegisters());
    return std::make_unique<ra::Result>(this->colorManager->AssignRegisters(),final);
}
assem::InstrList* RegAllocator::RemoveMoveInstr(assem::InstrList* instrlist,temp::Map *color){
    assem::InstrList* newlist=new assem::InstrList();
    for(auto it:instrlist->GetList()){
        if(typeid(*it)==typeid(assem::MoveInstr)){
            assem::MoveInstr* moveInstr = (assem::MoveInstr*)it;
            if(color->Look(moveInstr->src_->NthTemp(0))== color->Look(moveInstr->dst_->NthTemp(0))){
                continue;
            }
        }
        newlist->Append(it);
    }
    return newlist;
}
} // namespace ra