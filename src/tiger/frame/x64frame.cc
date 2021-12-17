#include "tiger/frame/x64frame.h"
#include <sstream>
extern frame::RegManager *reg_manager;

namespace frame {
/* TODO: Put your lab5 code here */
  temp::Temp* X64RegManager::RAX(){
    if(rax==nullptr){
      rax = temp::TempFactory::NewTemp();
    }
    return rax;
  }
  temp::Temp* X64RegManager::RBX(){
    if(rbx==nullptr){
      rbx = temp::TempFactory::NewTemp();
    }
    return rbx;
  }
  temp::Temp* X64RegManager::RCX(){
    if(rcx==nullptr){
      rcx = temp::TempFactory::NewTemp();
    }
    return rcx;
  }
  temp::Temp* X64RegManager::RDX(){
    if(rdx==nullptr){
      rdx = temp::TempFactory::NewTemp();
    }
    return rdx;
  }
  temp::Temp* X64RegManager::RSI(){
    if(rsi==nullptr){
      rsi = temp::TempFactory::NewTemp();
    }
    return rsi;
  }
  temp::Temp* X64RegManager::RDI(){
    if(rdi==nullptr){
      rdi = temp::TempFactory::NewTemp();
    }
    return rdi;
  }
  temp::Temp* X64RegManager::RBP(){
    if(rbp==nullptr){
      rbp = temp::TempFactory::NewTemp();
    }
    return rbp;
  }
  temp::Temp* X64RegManager::RSP(){
    if(rsp==nullptr){
      rsp = temp::TempFactory::NewTemp();
    }
    return rsp;
  }
  temp::Temp* X64RegManager::R8(){
    if(r8==nullptr){
      r8 = temp::TempFactory::NewTemp();
    }
    return r8;
  }
  temp::Temp* X64RegManager::R9(){
    if(r9==nullptr){
      r9 = temp::TempFactory::NewTemp();
    }
    return r9;
  }
  temp::Temp* X64RegManager::R10(){
    if(r10==nullptr){
      r10 = temp::TempFactory::NewTemp();
    }
    return r10;
  }
  temp::Temp* X64RegManager::R11(){
    if(r11==nullptr){
      r11 = temp::TempFactory::NewTemp();
    }
    return r11;
  }
  temp::Temp* X64RegManager::R12(){
    if(r12==nullptr){
      r12 = temp::TempFactory::NewTemp();
    }
    return r12;
  }
  temp::Temp* X64RegManager::R13(){
    if(r13==nullptr){
      r13 = temp::TempFactory::NewTemp();
    }
    return r13;
  }
  temp::Temp* X64RegManager::R14(){
    if(r14==nullptr){
      r14 = temp::TempFactory::NewTemp();
    }
    return r14;
  }
  temp::Temp* X64RegManager::R15(){
    if(r15==nullptr){
      r15 = temp::TempFactory::NewTemp();
    }
    return r15;
  }
  X64RegManager::X64RegManager(){
    this->inputMap(RAX(),"%rax");
    this->inputMap(RBX(),"%rbx");
    this->inputMap(RCX(),"%rcx");
    this->inputMap(RDX(),"%rdx");
    this->inputMap(RSI(),"%rsi");
    this->inputMap(RDI(),"%rdi");
    this->inputMap(RBP(),"%rbp");
    this->inputMap(RSP(),"%rsp");
    this->inputMap(R8(),"%r8");
    this->inputMap(R9(),"%r9");
    this->inputMap(R10(),"%r10");
    this->inputMap(R11(),"%r11");
    this->inputMap(R12(),"%r12");
    this->inputMap(R13(),"%r13");
    this->inputMap(R14(),"%r14");
    this->inputMap(R15(),"%r15");

    // std::string sRax="rax";
    // std::string sRbx="rbx";
    // std::string sRcx="rcx";
    // std::string sRdx="rdx";
    // std::string sRsi="rsi";
    // std::string sRdi="rdi";
    // std::string sRbp="rbp";
    // std::string sRsp="rsp";
    // std::string sR8="r8";
    // std::string sR9="r9";
    // std::string sR10="r10";
    // std::string sR11="r11";
    // std::string sR12="r12";
    // std::string sR13="r13";
    // std::string sR14="r14";
    // std::string sR15="r15";
    // this->temp_map_->Enter(RAX(),&sRax);
    // this->temp_map_->Enter(RBX(),&sRbx);
    // this->temp_map_->Enter(RCX(),&sRcx);
    // this->temp_map_->Enter(RDX(),&sRdx);
    // this->temp_map_->Enter(RSI(),&sRsi);
    // this->temp_map_->Enter(RDI(),&sRdi);
    // this->temp_map_->Enter(RBP(),&sRbp);
    // this->temp_map_->Enter(RSP(),&sRsp);
    // this->temp_map_->Enter(R8(),&sR8);
    // this->temp_map_->Enter(R9(),&sR9);
    // this->temp_map_->Enter(R10(),&sR10);
    // this->temp_map_->Enter(R11(),&sR11);
    // this->temp_map_->Enter(R12(),&sR12);
    // this->temp_map_->Enter(R13(),&sR13);
    // this->temp_map_->Enter(R14(),&sR14);
    // this->temp_map_->Enter(R15(),&sR15);
  }
   void X64RegManager::inputMap(temp::Temp* key,std::string val){
     std::stringstream stream;
     stream << val;
     this->temp_map_->Enter(key,new std::string(stream.str()));
   }
  temp::TempList* X64RegManager::Registers(){
    temp::TempList* templist = new temp::TempList(RAX());
    templist->Append(RBX());
    templist->Append(RCX());
    templist->Append(RDX());
    templist->Append(RSI());
    templist->Append(RDI());
    templist->Append(RBP());
    templist->Append(RSP());
    templist->Append(R8());
    templist->Append(R9());
    templist->Append(R10());
    templist->Append(R11());
    templist->Append(R12());
    templist->Append(R13());
    templist->Append(R14());
    templist->Append(R15());
    return templist;
  }
  temp::TempList* X64RegManager::ArgRegs(){
    temp::TempList* templist = new temp::TempList(RDI());
    templist->Append(RSI());
    templist->Append(RDX());
    templist->Append(RCX());
    templist->Append(R8());
    templist->Append(R9());
    return templist;
  }
  temp::TempList* X64RegManager::CallerSaves(){
    temp::TempList* templist = new temp::TempList(RAX());
    templist->Append(RCX());
    templist->Append(RDX());
    templist->Append(RSI());
    templist->Append(RDI());
    templist->Append(R8());
    templist->Append(R9());
    templist->Append(R10());
    templist->Append(R11());
    return templist;
  }
  temp::TempList* X64RegManager::CalleeSaves(){
    temp::TempList* templist = new temp::TempList(RBX());
    templist->Append(RBP());
    templist->Append(R12());
    templist->Append(R13());
    templist->Append(R14());
    templist->Append(R15());
    return templist;
  }
  temp::TempList* X64RegManager::RegsWithoutRsp(){
    temp::TempList* templist = new temp::TempList(RAX());
    templist->Append(RBX());
    templist->Append(RCX());
    templist->Append(RDX());
    templist->Append(RSI());
    templist->Append(RDI());
    templist->Append(RBP());
    templist->Append(R8());
    templist->Append(R9());
    templist->Append(R10());
    templist->Append(R11());
    templist->Append(R12());
    templist->Append(R13());
    templist->Append(R14());
    templist->Append(R15());
    return templist;

  }
  temp::Temp* X64RegManager::FramePointer(){
    return RBP();
  }
  temp::Temp* X64RegManager::StackPointer(){
    return RSP();
  }
  temp::Temp* X64RegManager::ReturnValue(){
    return RAX();
  }
  temp::Temp* X64RegManager::GetArgByNum(int num){
    switch (num)
    {
    case 1:
      return RDI();
    case 2:
      return RSI();
    case 3:
      return RDX();
    case 4:
      return RCX();
    case 5:
      return R8();
    case 6:
      return R9();        
    default:
      return nullptr;
    }
  }
tree::Exp* externalCall(std::string s,tree::ExpList* explist){
    return new tree::CallExp(
      new tree::NameExp(temp::LabelFactory::NamedLabel(s)),explist
    );
}
tree::Stm* F_procEntryExit1(frame::Frame *frame,tree::Stm* stm){
  std::list<tree::Stm*> stmList=frame->viewShift->GetList();
  tree::Stm* result=nullptr;
  for(auto it:stmList){
    if(!result){
      result=it;
    }
    else{
      result=new tree::SeqStm(result,it);
    }
  }
  if(!result){
    result = stm;
  }
  else{
    result = new tree::SeqStm(result,stm);
  }
  return result;
}
assem::Proc* ProcEntryExit3(frame::Frame *frame_,assem::InstrList *body){
  std::string prologue = frame_->name_->Name()+":\n";
  prologue += ".set "+frame_->name_->Name()+"_framesize, "+std::to_string(-frame_->offset)+"\n";
  prologue += "subq $"+ std::to_string(-frame_->offset)+", %rsp\n";
  std::string epilog = "addq $"+std::to_string(-frame_->offset)+", %rsp\n";
  epilog += "retq\n";
  return new assem::Proc(prologue,body,epilog);
}
/* TODO: Put your lab5 code here */
} // namespace frame