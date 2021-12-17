#include "tiger/regalloc/color.h"
#include <sstream>
#include "tiger/codegen/codegen.h"
extern frame::RegManager *reg_manager;

namespace col {
/* TODO: Put your lab6 code here */

void Color::initLiveGraph(){
    //first form a flowGraphFactory
    fg::FlowGraphFactory* flow=new fg::FlowGraphFactory(this->InsList_);
    flow->AssemFlowGraph();
    live::LiveGraphFactory* live=new live::LiveGraphFactory(flow->GetFlowGraph());
    live->Liveness();
    this->livegraph=live->GetLiveGraph();
}
live::MoveList* Color::NodeMoves(live::INode* node){
    return moveListTable->Look(node)->Intersect(activeMoves->Union(worklistMoves));
}
bool Color::MoveRelated(live::INode* node){
    live::MoveList* moveList = this->NodeMoves(node);
    return !(moveList->GetList().empty());
}
bool Color::isEmpty(live::INodeListPtr ptr){
    if(!ptr){
        return true;
    }
    if(ptr->GetList().empty()) return true;
    return false;
}
bool Color::isEmpty(live::MoveList* movelist){
    if(!movelist) return true;
    if(movelist->GetList().empty()) return true;
    return false;
}
temp::Map* Color::AssignRegisters(){
    temp::Map *result = temp::Map::Empty();
    // result->Enter(reg_manager->StackPointer(),new std::string("%rsp"));
    for(auto it:this->livegraph.interf_graph->Nodes()->GetList()){
        result->Enter(it->NodeInfo(),colorTable->Look(it));
    }
    return result;
    // return temp::Map::LayerMap(result,reg_manager->temp_map_);
}
assem::InstrList* Color::getInstrList(){
    return this->InsList_;
}
void Color::run(){
    bool finished = false;
    while(!finished){
        finished = true;
        initLiveGraph();
        init();
        while (!isEmpty(simplifyWorklist) || !isEmpty(worklistMoves) || !isEmpty(freezeWorklist) || !isEmpty(spillWorklist)){
            if(!isEmpty(simplifyWorklist)) Simplify();
            else if(!isEmpty(worklistMoves)) Coalesce();
            else if(!isEmpty(freezeWorklist)) Freeze();
            else if(!isEmpty(spillWorklist)) SelectSpill(); 
        }
        AssignColors();
        if(!isEmpty(spilledNodes)){
            RewriteProgram();
            finished=false;
        }    
    }
}
void Color::init(){
    this->simplifyWorklist=new live::INodeList();
    this->freezeWorklist=new live::INodeList();
    this->spillWorklist=new live::INodeList();
    this->spilledNodes=new live::INodeList();
    this->coalescedNodes=new live::INodeList();
    this->coloredNodes=new live::INodeList();
    //this->selectStack;
    this->coalescedMoves=new live::MoveList();
    this->constrainedMoves=new live::MoveList();
    this->frozenMoves=new live::MoveList();
    this->worklistMoves=this->livegraph.moves;
    this->activeMoves=new live::MoveList();
    this->degreeTable=new graph::Table<temp::Temp,int>();
    this->moveListTable=new graph::Table<temp::Temp,live::MoveList>();
    this->aliasTable=new graph::Table<temp::Temp,live::INode>();
    this->colorTable=new graph::Table<temp::Temp,std::string>();
    std::list<live::INode*> tempList=this->livegraph.interf_graph->Nodes()->GetList();
    for(auto it:tempList){
        degreeTable->Enter(it,new int(it->OutDegree()));
        // std::string* str=temp::Map::Name()->Look(it->NodeInfo());
        // if(!str){
        //     if(str->find("t")!=std::string::npos){
        //         str=nullptr;
        //     }
        // }
        colorTable->Enter(it,reg_manager->temp_map_->Look(it->NodeInfo()));
        aliasTable->Enter(it,it);
        live::MoveList* movelist = new live::MoveList();
        for(auto sigleMove:this->worklistMoves->GetList()){
            if(sigleMove.first == it||sigleMove.second == it){
                movelist->Append(sigleMove.first,sigleMove.second);
            }
        }
        moveListTable->Enter(it,movelist);
    }
    //init workList
    for(auto it:tempList){
        if(colorTable->Look(it)) continue;
        if(*degreeTable->Look(it)>=K){
            spillWorklist->Append(it);
        }
        else if(MoveRelated(it)){
            freezeWorklist->Append(it);
        }
        else{
            simplifyWorklist->Append(it);
        } 
    }
}
void Color::EnableMoves(live::INodeList* nodelist){
    for(auto it:nodelist->GetList()){
        live::MoveList* relateList = this->NodeMoves(it);
        for(auto that:relateList->GetList()){
            if(activeMoves->Contain(that.first,that.second)){
                live::MoveList* cut = new live::MoveList();
                cut->Append(that.first,that.second);
                activeMoves = activeMoves->Difference(cut);
                worklistMoves->Append(that.first,that.second);
            }
        }
    }
}
void Color::DecrementDegree(live::INode* node){
    (*degreeTable->Look(node))--;
    if(*(degreeTable->Look(node)) == K-1 && !colorTable->Look(node)){
        EnableMoves(live::Union(node,node->Succ()));
        spillWorklist->DeleteNode(node);
        if(MoveRelated(node)){
            freezeWorklist->Append(node);
        }else{
            simplifyWorklist->Append(node);
        }
    }
}
void Color::Simplify(){
    std::list<live::INodePtr> nodelist = this->simplifyWorklist->GetList();
    if(nodelist.empty()) return;
    live::INode* node=nodelist.front();
    selectStack.push(node);
    for(auto it:node->Succ()->GetList()){
        DecrementDegree(it);
    }
}
live::INode* Color::GetAlias(live::INode* node){
    if(coalescedNodes->Contain(node)){
        return GetAlias(aliasTable->Look(node));
    }
    else return node;
}
void Color::AddWorkList(live::INode* node){
    if(!colorTable->Look(node)&&!MoveRelated(node)&&*degreeTable->Look(node) < K){
        freezeWorklist->DeleteNode(node);
        simplifyWorklist->Append(node);
    }
}
bool Color::OK(live::INode* t,live::INode* r){
    bool result = true;
    for(auto it:t->Succ()->GetList()){
        if(!(*degreeTable->Look(it) < K || colorTable->Look(it) || r->Succ()->Contain(it))){
            result = false;
            break;
        }
    }
    return result;
}
bool Color::Conservative(live::INodeList* nodelist){
    int k=0;
    for(auto it:nodelist->GetList()){
        if(*degreeTable->Look(it) >= K) k++;
    }
    return (k < K);
}
void Color::AddEdge(live::INode *u,live::INode *v){
    if(!u->Succ()->Contain(v)&& u!=v){
        if(!colorTable->Look(u)){
            livegraph.interf_graph->AddEdge(u,v);
            (*(degreeTable->Look(u)))++;
        }
        if(!colorTable->Look(v)){
            livegraph.interf_graph->AddEdge(v,u);
            (*degreeTable->Look(v))++;
        }
    }
}
void Color::Combine(live::INode *u,live::INode *v){
    if(freezeWorklist->Contain(v)){
        freezeWorklist->DeleteNode(v);
    }else{
        spillWorklist->DeleteNode(v);
    }
    coalescedNodes->Append(v);
    aliasTable->Set(v,u);
    moveListTable->Set(u,moveListTable->Look(u)->Union(moveListTable->Look(v)));
    live::INodeListPtr ptr = new live::INodeList();
    ptr->Append(v);
    EnableMoves(ptr);
    for(auto it:v->Succ()->GetList()){
        AddEdge(it,u);
        DecrementDegree(it);
    }
    if(*degreeTable->Look(u) >= K && freezeWorklist->Contain(u)){
        freezeWorklist->DeleteNode(u);
        spillWorklist->Append(u);
    }
}
void Color::Coalesce(){
    live::INodePtr x,y,u,v;
    //get a move
    auto movelist = this->worklistMoves->GetList();
    x = movelist.front().first;
    y = movelist.front().second;
    worklistMoves->Delete(x,y);
    if(colorTable->Look(GetAlias(y))){
        u = GetAlias(y);
        v = GetAlias(x);
    }else{
        u = GetAlias(x);
        v = GetAlias(y);
    }
    if(u == v){
        coalescedMoves->Append(x,y);
        AddWorkList(u);
    }else if(colorTable->Look(v) || v->Succ()->Contain(u)){
        constrainedMoves->Append(x,y);
        AddWorkList(u);
        AddWorkList(v);
    }else if((colorTable->Look(u)&&OK(v,u)) || 
    (!colorTable->Look(u) && Conservative(u->Succ()->Union(v->Succ())))){
        coalescedMoves->Append(x,y);
        Combine(u,v);
        AddWorkList(u);
    }else{
        activeMoves->Append(x,y);
    }
}
void Color::Freeze(){
    if(freezeWorklist->GetList().empty()) return;
    live::INode *u = freezeWorklist->GetList().front();
    freezeWorklist->DeleteNode(u);
    simplifyWorklist->Append(u);
    FreezeMoves(u);
}
void Color::FreezeMoves(live::INode* u){
    for(auto it:NodeMoves(u)->GetList()){
        live::INode *x = it.first;
        live::INode *y = it.second;
        live::INode *v;
        if(GetAlias(y) == GetAlias(u))
            v = GetAlias(x);
        else
            v = GetAlias(y);
        activeMoves->Delete(x,y);
        frozenMoves->Append(x,y);
        if(!MoveRelated(v) && *degreeTable->Look(v)<K){
            freezeWorklist->DeleteNode(v);
            simplifyWorklist->Append(v);
        }
    }
}
void Color::SelectSpill(){
    if(spillWorklist->GetList().empty()) return;
    live::INode* m;
    int maxweight=0;
    for(auto it:spillWorklist->GetList()){
        if(live::Contain(NotSpill,it->NodeInfo())) continue;
        if(*degreeTable->Look(it)>maxweight){
            m = it;
            maxweight = *degreeTable->Look(it);
        }
    }
    if(!m) m =spillWorklist->GetList().front();
    spillWorklist->DeleteNode(m);
    simplifyWorklist->Append(m);
    FreezeMoves(m);
}
void Color::AssignColors(){
    //true means can use, false means not
    tab::Table<std::string,bool> okColors;
    temp::TempList* regsWithoutRsp=reg_manager->RegsWithoutRsp();
    for(auto it:regsWithoutRsp->GetList()){
        okColors.Enter(temp::Map::Name()->Look(it),new bool(true));
    }
    while(!selectStack.empty()){
        live::INode* n = this->selectStack.top();
        this->selectStack.pop();
        for(auto it:regsWithoutRsp->GetList()){
            okColors.Set(temp::Map::Name()->Look(it),new bool(true));
        }
        for(auto it:n->Succ()->GetList()){
            if(colorTable->Look(GetAlias(it))){
                okColors.Set(
                    colorTable->Look(GetAlias(it)),new bool(false)
                );
            }
        }
        //check if have color to choose
        bool remainColor = false;
        temp::Temp* color;
        for(auto it:regsWithoutRsp->GetList()){
            if(*okColors.Look(reg_manager->temp_map_->Look(it))){
                color = it;
                remainColor=true;
                break;
            }
        }
        if(remainColor){
            coloredNodes->Append(n);
            colorTable->Set(n,temp::Map::Name()->Look(color));
        }else{
            spilledNodes->Append(n);
        }
    }
    for(auto it:coalescedNodes->GetList()){
        colorTable->Set(it,colorTable->Look(GetAlias(it)));
    } 
}
void Color::RewriteProgram(){
    NotSpill = new temp::TempList();
    for(auto& it:spilledNodes->GetList()){
      assem::InstrList* newInslist = new assem::InstrList();  
      temp::Temp* spilledTemp = it->NodeInfo();  
      this->frame_->offset-=frame::X64Frame::wordSize;
      for(auto ins:this->InsList_->GetList()){
          temp::TempList* oldSrc = nullptr;
          temp::TempList* oldDst = nullptr;
          if(typeid(*ins)==typeid(assem::OperInstr)){
              oldSrc = ((assem::OperInstr*)ins)->src_;
              oldDst = ((assem::OperInstr*)ins)->dst_;
          }else if(typeid(*ins)==typeid(assem::MoveInstr)){
              oldSrc = ((assem::MoveInstr*)ins)->src_;
              oldDst = ((assem::MoveInstr*)ins)->dst_;
          }
          temp::Temp* newtemp;
          std::vector<temp::Temp*> srcvec;
          std::vector<temp::Temp*> dstvec;
          if(oldSrc&&live::Contain(oldSrc,spilledTemp)&&oldDst&&live::Contain(oldDst,spilledTemp)){
              //the spilledTemp appear in both src and dst
              newtemp = temp::TempFactory::NewTemp();
              NotSpill->Append(newtemp);
              //add the fetch instr
              std::stringstream stream;
              stream << "movq ("<<this->frame_->name_->Name()<<"_framesize"<<this->frame_->offset<<")(`s0), `d0";
              std::string str = stream.str();
              stream.str(0);
              dstvec.push_back(newtemp);
              srcvec.push_back(reg_manager->StackPointer());
              newInslist->Append(new assem::OperInstr(
                  str,cg::makeTempList(dstvec),cg::makeTempList(srcvec),nullptr
              ));
              dstvec.clear();
              srcvec.clear();
              //add the replace instr
              newInslist->Append(live::replace(
                  ins,spilledTemp,newtemp
              ));
              //add the store instr
              stream << "movq `s0, ("<<this->frame_->name_->Name()<<"_framesize"<<this->frame_->offset<<")(`s1)";
              str = stream.str();
              stream.str(0);
              srcvec.push_back(newtemp);
              srcvec.push_back(reg_manager->StackPointer());
              newInslist->Append(new assem::OperInstr(
                str,nullptr,cg::makeTempList(srcvec),nullptr
              )); 
              srcvec.clear(); 
          }else if(oldSrc && live::Contain(oldSrc,spilledTemp)){
              newtemp = temp::TempFactory::NewTemp();
              NotSpill->Append(newtemp);
              //add the fetch instr
              std::stringstream stream;
              stream << "movq ("<<this->frame_->name_->Name()<<"_framesize"<<this->frame_->offset<<")(`s0), `d0";
              std::string str = stream.str();
              stream.str(0);
              dstvec.push_back(newtemp);
              srcvec.push_back(reg_manager->StackPointer());
              newInslist->Append(new assem::OperInstr(
                  str,cg::makeTempList(dstvec),cg::makeTempList(srcvec),nullptr
              ));
              dstvec.clear();
              srcvec.clear();
              //add the replace instr
              newInslist->Append(live::replace(
                  ins,spilledTemp,newtemp
              ));
          }else if(oldDst&&live::Contain(oldDst,spilledTemp)){
              newtemp = temp::TempFactory::NewTemp();
              NotSpill->Append(newtemp);
              //add the fetch instr
              std::stringstream stream;
              //add the replace instr
              newInslist->Append(live::replace(
                  ins,spilledTemp,newtemp
              ));
              //add the store instr
              stream << "movq `s0, ("<<this->frame_->name_->Name()<<"_framesize"<<this->frame_->offset<<")(`s1)";
              std::string str = stream.str();
              stream.str(0);
              srcvec.push_back(newtemp);
              srcvec.push_back(reg_manager->StackPointer());
              newInslist->Append(new assem::OperInstr(
                str,nullptr,cg::makeTempList(srcvec),nullptr
              )); 
              srcvec.clear(); 
          }else{
              newInslist->Append(ins);
          }
      }
      this->InsList_=newInslist;

    }
}
} // namespace col
