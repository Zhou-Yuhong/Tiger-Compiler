#include "tiger/liveness/liveness.h"
#include <stack>
extern frame::RegManager *reg_manager;

namespace live {

bool Contain(temp::TempList* list,temp::Temp* temp){
  if(!list) return false;
  if(list->GetList().empty()) return false;
  std::list<temp::Temp*> tempList=list->GetList();
  //Note that Temp has only one address space, that is,
  //num and address space are unique because they are assigned by TempFactory  
  for(auto it:tempList){
    if(it==temp) return true;
  }
  return false;
}
bool IsMove(fg::FNodePtr ptr){
  assem::Instr* instr=ptr->NodeInfo();
  if(typeid(*instr)==typeid(assem::MoveInstr)){
    return true;
  }
  return false;
}
assem::Instr* replace(assem::Instr* target,temp::Temp* oldTemp,temp::Temp* newTemp){
  //build a new Instr, and replace all the old Temp with new Temp,only two case:Oper , Move
  if(typeid(*target)==typeid(assem::OperInstr)){
    assem::OperInstr* oldInstr = (assem::OperInstr*)target;
    temp::TempList* newSrc=nullptr;
    temp::TempList* newDst=nullptr;
    if(oldInstr->src_){
      newSrc=new temp::TempList();
      for(auto it:oldInstr->src_->GetList()){
        if(it==oldTemp){
          newSrc->Append(newTemp);
        }
        else{
          newSrc->Append(it);
        }
      }
    }
    if(oldInstr->dst_){
      newDst=new temp::TempList();
      for(auto it:oldInstr->dst_->GetList()){
        if(it==oldTemp){
          newDst->Append(newTemp);
        }else{
          newDst->Append(it);
        }
      }
    }
    return new assem::OperInstr(
      oldInstr->assem_,newDst,newSrc,oldInstr->jumps_
    );
  }else{
    assem::MoveInstr* oldInstr = (assem::MoveInstr*)target;
    temp::TempList* newSrc=nullptr;
    temp::TempList* newDst=nullptr;
    if(oldInstr->src_){
      newSrc=new temp::TempList();
      for(auto it:oldInstr->src_->GetList()){
        if(it==oldTemp){
          newSrc->Append(newTemp);
        }
        else{
          newSrc->Append(it);
        }
      }
    }
    if(oldInstr->dst_){
      newDst=new temp::TempList();
      for(auto it:oldInstr->dst_->GetList()){
        if(it==oldTemp){
          newDst->Append(newTemp);
        }else{
          newDst->Append(it);
        }
      }
    }
    return new assem::MoveInstr(
      oldInstr->assem_,newDst,newSrc
    );
  }
}
temp::TempList* Union(temp::TempList* list1,temp::TempList* list2){
  temp::TempList *unionList = new temp::TempList();
  std::list<temp::Temp*> tempList1 = list1->GetList();
  std::list<temp::Temp*> tempList2 = list2->GetList();
  for(auto it:tempList1){
    unionList->Append(it);
  }
  for(auto it:tempList2){
    if(!Contain(unionList,it)){
      unionList->Append(it);
    }
  }
  return unionList;
}
live::INodeList* Union(live::INode* node,live::INodeList* list){
  live::INodeList* res = new live::INodeList();
  for(auto it:list->GetList()){
    res->Append(it);
  }
  res->Append(node);
  return res;
}


bool IsSame(temp::TempList* list1,temp::TempList* list2){
  std::list<temp::Temp*> tempList1=list1->GetList();
  std::list<temp::Temp*> tempList2=list2->GetList();
  for(auto it:tempList1){
    if(!Contain(list2,it)) return false;
  }
  for(auto it:tempList2){
    if(!Contain(list1,it)) return false;
  }
  return true;
}
temp::TempList* Difference(temp::TempList* list1,temp::TempList* list2){
  temp::TempList *diff = new temp::TempList();
  std::list<temp::Temp*> tempList1 = list1->GetList();
  for(auto it:tempList1){
    if(!Contain(list2,it)){
      diff->Append(it);
    }
  }
  return diff;
}

bool MoveList::Contain(INodePtr src, INodePtr dst) {
  return std::any_of(move_list_.cbegin(), move_list_.cend(),
                     [src, dst](std::pair<INodePtr, INodePtr> move) {
                       return move.first == src && move.second == dst;
                     });
}

void MoveList::Delete(INodePtr src, INodePtr dst) {
  assert(src && dst);
  auto move_it = move_list_.begin();
  for (; move_it != move_list_.end(); move_it++) {
    if (move_it->first == src && move_it->second == dst) {
      break;
    }
  }
  move_list_.erase(move_it);
}

MoveList *MoveList::Union(MoveList *list) {
  auto *res = new MoveList();
  for (auto move : list->GetList()) {
    if (!Contain(move.first, move.second))
      res->move_list_.push_back(move);
  }
  return res;
}

MoveList *MoveList::Intersect(MoveList *list) {
  auto *res = new MoveList();
  for (auto move : list->GetList()) {
    if (Contain(move.first, move.second))
      res->move_list_.push_back(move);
  }
  return res;
}
MoveList *MoveList::Difference(MoveList *list){
  auto *res = new MoveList();
  for(auto move:this->GetList()){
    if(!list->Contain(move.first,move.second)){
      res->Append(move.first,move.second);
    }
  }
  return res;
}

void LiveGraphFactory::LiveMap() {
  /* TODO: Put your lab6 code here */
  //form the live map
  std::list<fg::FNodePtr> nodeList=this->flowgraph_->Nodes()->GetList();
  //initial the in and out table
  std::stack<fg::FNodePtr> nodeStack;
  std::vector<fg::FNodePtr> reverseNodeVector;
  for(auto it:nodeList){
    this->in_.get()->Enter(
      it,new temp::TempList()
    );
    this->out_.get()->Enter(
      it,new temp::TempList()
    );
    nodeStack.push(it);
  }
  while(!nodeStack.empty()){
    reverseNodeVector.push_back(nodeStack.top());
    nodeStack.pop();
  }
  bool solved = false;
  while(!solved){
    solved = true;
    for(auto it:reverseNodeVector){
       temp::TempList *oldIn = in_.get()->Look(it);
       temp::TempList *oldOut = out_.get()->Look(it);
       //update the out set of this node
       std::list<fg::FNodePtr> succList=it->Succ()->GetList();
       for(auto succ:succList){
         out_.get()->Set(
           it,Union(out_.get()->Look(it),in_.get()->Look(succ))
         );
       }
       //update the in set of this node
        in_.get()->Set(
          it,Union(
            it->NodeInfo()->Use(),Difference(out_.get()->Look(it),it->NodeInfo()->Def())
          )
        );
        //test
        if(!IsSame(oldIn,in_.get()->Look(it)) || !IsSame(oldOut,out_.get()->Look(it)))
        solved=false;
    }
  }

}

void LiveGraphFactory::InterfGraph() {
  /* TODO: Put your lab6 code here */
  //first add the basic regs
  std::list<temp::Temp*> origin=reg_manager->RegsWithoutRsp()->GetList();
  for(auto it:origin){
    live::INodePtr ptr=this->live_graph_.interf_graph->NewNode(it);
    this->temp_node_map_->Enter(
      it,ptr
    );
  }
  //form the origin edges
  for(auto from:origin){
    for(auto to:origin){
      if(to == from) continue;
      this->live_graph_.interf_graph->AddEdge(
        this->temp_node_map_->Look(from),
        this->temp_node_map_->Look(to)
      );
    }
  }
  std::list<fg::FNodePtr> Nodelist=this->flowgraph_->Nodes()->GetList();
  //form the edge between the def and out,first should add nodes 
  for(auto it:Nodelist){
    std::list<temp::Temp*> templist = Union(out_.get()->Look(it),it->NodeInfo()->Def())->GetList();
    for(auto that:templist){
      if(that==reg_manager->StackPointer()) continue;
      if(!this->temp_node_map_->Look(that)){
        live::INodePtr ptr = this->live_graph_.interf_graph->NewNode(that);
        this->temp_node_map_->Enter(that,ptr);
      }
    } 
  }
  //add edge and handle the move instr
  for(auto it:Nodelist){
    if(!IsMove(it)){
      std::list<temp::Temp*> defList=it->NodeInfo()->Def()->GetList();
      for(auto def:defList){
        std::list<temp::Temp*> outList=out_.get()->Look(it)->GetList();
        for(auto out:outList){
          if(def == reg_manager->StackPointer()||out == reg_manager->StackPointer())
          continue;

          this->live_graph_.interf_graph->AddEdge(
            this->temp_node_map_->Look(def),
            this->temp_node_map_->Look(out)
          );
          this->live_graph_.interf_graph->AddEdge(
            this->temp_node_map_->Look(out),
            this->temp_node_map_->Look(def)
          );
        }
      }
    }
    else{
      std::list<temp::Temp*> defList=it->NodeInfo()->Def()->GetList();
      for(auto def:defList){
        temp::TempList* outList=Difference(out_.get()->Look(it),it->NodeInfo()->Use());
        for(auto out:outList->GetList()){
          if(def==reg_manager->StackPointer()||out == reg_manager->StackPointer())
          continue;
          this->live_graph_.interf_graph->AddEdge(
            this->temp_node_map_->Look(def),
            this->temp_node_map_->Look(out)
          );
          this->live_graph_.interf_graph->AddEdge(
            this->temp_node_map_->Look(out),
            this->temp_node_map_->Look(def)
          );
        }
        //add to the moves
        std::list<temp::Temp*> srcList = it->NodeInfo()->Use()->GetList();
        for(auto src:srcList){
          if(def == reg_manager->StackPointer()||src == reg_manager->StackPointer())
          continue;
          if(!this->live_graph_.moves->Contain(this->temp_node_map_->Look(src),
            this->temp_node_map_->Look(def)))
              this->live_graph_.moves->Append(
                this->temp_node_map_->Look(src),
                this->temp_node_map_->Look(def)
          );
        }
      }
    }
  }
}

void LiveGraphFactory::Liveness() {
  LiveMap();
  InterfGraph();
}

} // namespace live