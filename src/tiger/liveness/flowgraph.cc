#include "tiger/liveness/flowgraph.h"

namespace fg {

void FlowGraphFactory::AssemFlowGraph() {
  /* TODO: Put your lab6 code here */
  std::list<assem::Instr*> instrList=this->instr_list_->GetList();
  FNode *prev=nullptr;
  FNode *next=nullptr;
  for(auto it:instrList){ 
    next = this->flowgraph_->NewNode(it);
    if(prev && !isJmp(prev)){
      this->flowgraph_->AddEdge(prev,next);
    }
    if(typeid(*it)==typeid(assem::LabelInstr)){
      this->label_map_.get()->Enter(
        ((assem::LabelInstr*)it)->label_,
        next
      );
    }
    prev = next;
  }
  std::list<FNodePtr> nodeList = this->flowgraph_->Nodes()->GetList();
  for(auto it:nodeList){
    if(typeid(*(it->NodeInfo()))!=typeid(assem::OperInstr)||!((assem::OperInstr*)it->NodeInfo())->jumps_)
    continue;
    //add edge
    std::vector<temp::Label*> jumpList=*(((assem::OperInstr*)(it->NodeInfo()))->jumps_->labels_);
    for(auto that:jumpList){
      this->flowgraph_->AddEdge(
        it,this->label_map_->Look(that)
      );
    }
  }

}

bool isJmp(FNodePtr ptr){
  assem::Instr *instr = ptr->NodeInfo();
  bool result = false;
  if(typeid(*instr)==typeid(assem::OperInstr)){
    if(
      ((assem::OperInstr*)instr)->assem_.find("jmp")!= std::string::npos
    )
    result = true;
  }
  return result;
}
} // namespace fg

namespace assem {

temp::TempList *LabelInstr::Def() const {
  /* TODO: Put your lab6 code here */
  return nullptr;
}

temp::TempList *MoveInstr::Def() const {
  /* TODO: Put your lab6 code here */
  return this->dst_;
}

temp::TempList *OperInstr::Def() const {
  /* TODO: Put your lab6 code here */
  return this->dst_;
}

temp::TempList *LabelInstr::Use() const {
  /* TODO: Put your lab6 code here */
  return nullptr;
}

temp::TempList *MoveInstr::Use() const {
  /* TODO: Put your lab6 code here */
  return this->src_;
}

temp::TempList *OperInstr::Use() const {
  /* TODO: Put your lab6 code here */
  return this->src_;
}
} // namespace assem
