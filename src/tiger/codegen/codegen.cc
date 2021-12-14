#include "tiger/codegen/codegen.h"

#include <cassert>
#include <sstream>
#include <stack>
int orderCount=0;
extern frame::RegManager *reg_manager;

namespace {

constexpr int maxlen = 1024;


} // namespace

namespace cg {

temp::TempList* makeTempList(std::vector<temp::Temp*> list){
  temp::TempList* result = new temp::TempList();
  for(auto it:list){
    result->Append(it);
  }
  return result;
}
void CodeGen::Codegen() {
  /* TODO: Put your lab5 code here */
  fs_=frame_->name_->Name()+"_framesize";
  assem::InstrList* inslist = new assem::InstrList();
  std::list<tree::Stm *> stms=traces_->GetStmList()->GetList();
  for(auto it:stms){
    it->Munch(*inslist,fs_);
  }
  this->assem_instr_=std::make_unique<cg::AssemInstr>(inslist);
}

void AssemInstr::Print(FILE *out, temp::Map *map) const {
  for (auto instr : instr_list_->GetList()){
    instr->Print(out, map);
    orderCount++;
    }
  fprintf(out, "\n");
}
} // namespace cg

namespace tree {
/* TODO: Put your lab5 code here */


void SeqStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  this->left_->Munch(instr_list,fs);
  this->right_->Munch(instr_list,fs);
}

void LabelStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  assem::LabelInstr* instr = new assem::LabelInstr(this->label_->Name(),this->label_);
  instr_list.Append(instr);
}

void JumpStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  assem::OperInstr* instr = new assem::OperInstr(
    "jmp `j0",nullptr,nullptr,new assem::Targets(this->jumps_)
  );
  instr_list.Append(instr);
}

void CjumpStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  temp::Temp *leftT=this->left_->Munch(instr_list,fs);
  temp::Temp *rightT=this->right_->Munch(instr_list,fs);
  //form the cmp instr,right-s0;left-s1
  temp::TempList* srcList=new temp::TempList();
  srcList->Append(rightT);
  srcList->Append(leftT);
  assem::OperInstr *cmp= new assem::OperInstr("cmpq `s0,`s1",nullptr,srcList,nullptr);
  instr_list.Append(cmp);
  //cjump instr
  std::string str;
  switch (this->op_)
  {
  case tree::RelOp::EQ_OP:
    str = "je ";
    break;
  case tree::RelOp::NE_OP:
    str = "jne ";
    break;
  case tree::RelOp::GE_OP:
    str = "jge ";
    break;
  case tree::RelOp::GT_OP:      
    str = "jg ";
    break;
  case tree::RelOp::LT_OP:
    str = "jl ";
    break;
  case tree::RelOp::LE_OP:
    str = "jle";
    break;    
  default:
    break;
  }
  std::vector<temp::Label*> *labels=new std::vector<temp::Label *>();
  labels->push_back(this->true_label_);
  labels->push_back(this->false_label_);
  assem::Targets* targets=new assem::Targets(labels);
  assem::OperInstr *cjmp=new assem::OperInstr(
    str+" `j0",nullptr,nullptr,targets
  );
  instr_list.Append(cjmp);

}

void MoveStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  if(typeid(*(this->dst_))==typeid(tree::TempExp)){   
    temp::Temp* src = this->src_->Munch(instr_list,fs);
    std::vector<temp::Temp*> srcs;
    std::vector<temp::Temp*> dsts;
    srcs.push_back(src);
    dsts.push_back(((tree::TempExp*)this->dst_)->temp_);
    assem::MoveInstr* movestr=new assem::MoveInstr(
    "movq `s0, `d0",cg::makeTempList(dsts),cg::makeTempList(srcs)
    );  
    instr_list.Append(movestr);
    return;
  }
  else{
    //the dst is in mem
    temp::Temp* src = this->src_->Munch(instr_list,fs);
    temp::Temp* src2 = ((tree::MemExp*)this->dst_)->exp_->Munch(instr_list,fs);
    std::vector<temp::Temp*> srcs;
    srcs.push_back(src);
    srcs.push_back(src2);
    assem::OperInstr* oper=new assem::OperInstr("movq `s0, (`s1)",nullptr,cg::makeTempList(srcs),nullptr);
    instr_list.Append(oper);
    return;
  }
}

void ExpStm::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  this->exp_->Munch(instr_list,fs);
}

temp::Temp *BinopExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  temp::Temp* left;
  temp::Temp* right;
  left = this->left_->Munch(instr_list,fs);
  right = this->right_->Munch(instr_list,fs);
  temp::Temp* reg = temp::TempFactory::NewTemp();
  std::vector<temp::Temp*> srcs;
  std::vector<temp::Temp*> dsts;
  assem::MoveInstr* moveInstr;
  assem::OperInstr* operInstr;
  switch (this->op_)
  {
  case tree::PLUS_OP:
    dsts.push_back(reg);
    srcs.push_back(left);
    moveInstr = new assem::MoveInstr(
      "movq `s0, `d0",
      cg::makeTempList(dsts),
      cg::makeTempList(srcs)
    );
    instr_list.Append(moveInstr);
    dsts.clear();
    srcs.clear();
    dsts.push_back(reg);
    srcs.push_back(right);
    srcs.push_back(reg);
    operInstr = new assem::OperInstr(
      "addq `s0, `d0",
      cg::makeTempList(dsts),
      cg::makeTempList(srcs),nullptr
    );
    instr_list.Append(operInstr);
    break;
  case tree::MINUS_OP:
    dsts.push_back(reg);
    srcs.push_back(left);
    moveInstr = new assem::MoveInstr(
      "movq `s0, `d0",
      cg::makeTempList(dsts),
      cg::makeTempList(srcs)
    );
    instr_list.Append(moveInstr);
    dsts.clear();
    srcs.clear();
    dsts.push_back(reg);
    srcs.push_back(right);
    srcs.push_back(reg);
    operInstr = new assem::OperInstr(
      "subq `s0, `d0",
      cg::makeTempList(dsts),
      cg::makeTempList(srcs),nullptr
    );
    instr_list.Append(operInstr);
    break;
  case tree::MUL_OP:
   dsts.push_back(reg_manager->RAX());
    srcs.push_back(left);
    instr_list.Append(
      new assem::MoveInstr(
        "movq `s0, `d0",cg::makeTempList(dsts),cg::makeTempList(srcs)
      )
    );
    dsts.clear();
    srcs.clear();
    instr_list.Append(
      new assem::OperInstr(
        "cqto",nullptr,nullptr,nullptr
      )
    );
    dsts.push_back(reg_manager->RAX());
    srcs.push_back(right);
    instr_list.Append(
      new assem::OperInstr(
        "imulq `s0",cg::makeTempList(dsts),cg::makeTempList(srcs),nullptr
      )
    );
    dsts.clear();
    srcs.clear();
    //store the result
    dsts.push_back(reg);
    srcs.push_back(reg_manager->RAX());
    instr_list.Append(
      new assem::MoveInstr(
        "movq `s0, `d0",cg::makeTempList(dsts),cg::makeTempList(srcs)
      )
    );
    break;
  case tree::DIV_OP:
    //special 
    //first move the dividend into rax
    dsts.push_back(reg_manager->RAX());
    srcs.push_back(left);
    instr_list.Append(
      new assem::MoveInstr(
        "movq `s0, `d0",cg::makeTempList(dsts),cg::makeTempList(srcs)
      )
    );
    dsts.clear();
    srcs.clear();
    instr_list.Append(
      new assem::OperInstr(
        "cqto",nullptr,nullptr,nullptr
      )
    );
    dsts.push_back(reg_manager->RAX());
    srcs.push_back(right);
    instr_list.Append(
      new assem::OperInstr(
        "idivq `s0",cg::makeTempList(dsts),cg::makeTempList(srcs),nullptr
      )
    );
    dsts.clear();
    srcs.clear();
    //store the result
    dsts.push_back(reg);
    srcs.push_back(reg_manager->RAX());
    instr_list.Append(
      new assem::MoveInstr(
        "movq `s0, `d0",cg::makeTempList(dsts),cg::makeTempList(srcs)
      )
    );
    break;
  default:
    break;
  }
  return reg;
}

temp::Temp *MemExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  temp::Temp *src = this->exp_->Munch(instr_list,fs);
  std::vector<temp::Temp*> srcs;
  srcs.push_back(src);
  std::vector<temp::Temp*> dsts;
  temp::Temp *dst = temp::TempFactory::NewTemp();
  dsts.push_back(dst);
  instr_list.Append(
    new assem::OperInstr(
      "movq (`s0), `d0",cg::makeTempList(dsts),cg::makeTempList(srcs),nullptr
    )
  );
  return dst;
}

temp::Temp *TempExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  //if temp is frame pointer,should translate
  if(this->temp_!=reg_manager->FramePointer()){
    return this->temp_;
  }
  else{
    //translate
    temp::Temp* reg=temp::TempFactory::NewTemp();
    std::string s = {fs.begin(),fs.end()};
    std::string assemStr="leaq "+s+"(`s0), `d0";
    std::vector<temp::Temp*> srcs;
    std::vector<temp::Temp*> dsts;
    srcs.push_back(reg_manager->StackPointer());
    dsts.push_back(reg);
    instr_list.Append(
      new assem::OperInstr(
        assemStr,cg::makeTempList(dsts),cg::makeTempList(srcs),nullptr
      )
    );
    return reg;
  }
}

temp::Temp *EseqExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  this->stm_->Munch(instr_list,fs);
  return this->exp_->Munch(instr_list,fs);
}

temp::Temp *NameExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  //use %rip
  std::string assemStr = "leaq "+this->name_->Name()+"(%rip), `d0";
  std::vector<temp::Temp*> dsts;
  temp::Temp* reg=temp::TempFactory::NewTemp();
  dsts.push_back(reg);
  instr_list.Append(
    new assem::OperInstr(
      assemStr,cg::makeTempList(dsts),nullptr,nullptr
    )
  );
  return reg;
}

temp::Temp *ConstExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  temp::Temp* reg=temp::TempFactory::NewTemp();
  std::string assemStr="movq $"+std::to_string(this->consti_)+", `d0";
  std::vector<temp::Temp*> dsts;
  dsts.push_back(reg);
  instr_list.Append(
    new assem::OperInstr(
      assemStr,cg::makeTempList(dsts),nullptr,nullptr
    )
  );
  return reg;
}

temp::Temp *CallExp::Munch(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  temp::Temp* reg=temp::TempFactory::NewTemp();
  temp::TempList* arglist=this->args_->MunchArgs(instr_list,fs);
  std::string assemStr = "callq "+((tree::NameExp*)this->fun_)->name_->Name();
  assem::OperInstr *oper=new assem::OperInstr(
      assemStr,nullptr,arglist,nullptr
    );
  instr_list.Append(oper);
  std::vector<temp::Temp*> srcs;
  std::vector<temp::Temp*> dsts;
  //collect the space that alloc to the args
  int size = arglist->GetList().size();
  if(size>6){
    int offset = (size-6)*8;
    dsts.push_back(reg_manager->StackPointer());
    instr_list.Append(new assem::OperInstr(
      "addq $"+std::to_string(offset)+", `d0",cg::makeTempList(dsts),
      nullptr,nullptr
    ));
    dsts.clear();
  }
  dsts.push_back(reg);
  srcs.push_back(reg_manager->ReturnValue());
  assem::OperInstr *oper2=new assem::OperInstr(
    "movq `s0, `d0",cg::makeTempList(dsts),cg::makeTempList(srcs),nullptr
  );
  instr_list.Append(oper2);
  return reg;
}

temp::TempList *ExpList::MunchArgs(assem::InstrList &instr_list, std::string_view fs) {
  /* TODO: Put your lab5 code here */
  // std::list<temp::Temp*> templist=this->GetList();
  std::list<tree::Exp*> explist=this->GetList();
  int count=0;
  std::vector<temp::Temp*> srcs;
  std::vector<temp::Temp*> dsts;
  std::stack<temp::Temp*> argsInStack;
  temp::TempList* result=new temp::TempList();
  for(auto it:explist){
    temp::Temp* item=it->Munch(instr_list,fs);
    switch (count)
    {
    case 0:
      srcs.push_back(item);
      dsts.push_back(reg_manager->RDI());
      instr_list.Append(
        new assem::MoveInstr(
          "movq `s0, `d0",cg::makeTempList(dsts),cg::makeTempList(srcs)
        )
      );
      srcs.clear();
      dsts.clear();
      break;
    case 1:
      srcs.push_back(item);
      dsts.push_back(reg_manager->RSI());
      instr_list.Append(
        new assem::MoveInstr(
          "movq `s0, `d0",cg::makeTempList(dsts),cg::makeTempList(srcs)
        )
      );
      srcs.clear();
      dsts.clear();
      break;
    case 2:
      srcs.push_back(item);
      dsts.push_back(reg_manager->RDX());
      instr_list.Append(
        new assem::MoveInstr(
          "movq `s0, `d0",cg::makeTempList(dsts),cg::makeTempList(srcs)
        )
      );
      srcs.clear();
      dsts.clear();
      break;
    case 3:
      srcs.push_back(item);
      dsts.push_back(reg_manager->RCX());
      instr_list.Append(
        new assem::MoveInstr(
          "movq `s0, `d0",cg::makeTempList(dsts),cg::makeTempList(srcs)
        )
      );
      srcs.clear();
      dsts.clear();
      break;  
    case 4:
      srcs.push_back(item);
      dsts.push_back(reg_manager->R8());
      instr_list.Append(
        new assem::MoveInstr(
          "movq `s0, `d0",cg::makeTempList(dsts),cg::makeTempList(srcs)
        )
      );
      srcs.clear();
      dsts.clear();
      break; 
    case 5:
      srcs.push_back(item);
      dsts.push_back(reg_manager->R9());
      instr_list.Append(
        new assem::MoveInstr(
          "movq `s0, `d0",cg::makeTempList(dsts),cg::makeTempList(srcs)
        )
      );
      srcs.clear();
      dsts.clear();
      break;   
    //TODO change to stack args             
    default:
    argsInStack.push(item);  
      break;
    }
     count++;
     result->Append(item);
  }
  //put the argInstack Into stack
  while (!argsInStack.empty())
  {
    temp::Temp* arg = argsInStack.top();
    dsts.push_back(reg_manager->StackPointer());
    instr_list.Append(new assem::OperInstr(
      "subq $8, `d0",cg::makeTempList(dsts),nullptr,nullptr
    ));
    dsts.clear();
    srcs.push_back(arg);
    srcs.push_back(reg_manager->StackPointer());
    instr_list.Append(new assem::OperInstr(
      "movq `s0, (`s1)",nullptr,cg::makeTempList(srcs),nullptr
    ));
    srcs.clear();
    argsInStack.pop();
  }
  
  return result;
}

} // namespace tree
