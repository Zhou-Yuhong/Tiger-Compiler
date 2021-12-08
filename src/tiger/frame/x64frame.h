//
// Created by wzl on 2021/10/12.
//

#ifndef TIGER_COMPILER_X64FRAME_H
#define TIGER_COMPILER_X64FRAME_H

#include "tiger/frame/frame.h"

namespace frame {
class InFrameAccess : public Access {
public:
  int offset;

  explicit InFrameAccess(int offset) : Access(INFRAME),offset(offset) {}
  /* TODO: Put your lab5 code here */
};
tree::Exp* externalCall(std::string s,tree::ExpList* explist);
tree::Stm* F_procEntryExit1(frame::Frame *frame,tree::Stm* stm);
assem::Proc* ProcEntryExit3(frame::Frame *frame_,assem::InstrList *body);
class InRegAccess : public Access {
public:
  temp::Temp *reg;

  explicit InRegAccess(temp::Temp *reg) : Access(INREG),reg(reg) {}
  /* TODO: Put your lab5 code here */
};  
class X64Frame : public Frame {
  /* TODO: Put your lab5 code here */  
public:
  
  static const int wordSize=8;
  static Frame* newFrame(temp::Label *name,std::list<bool> list,frame::RegManager* regmanager){
    frame::Frame* frame_=new X64Frame();
    frame_->name_=name;
    frame_->offset=-wordSize;
    //form the formals
    for(auto it:list){
      frame::Access *access;
      if(it){
        access = new InFrameAccess(frame_->offset);
        frame_->offset -= X64Frame::wordSize;
      }else{
        access = new InRegAccess(temp::TempFactory::NewTemp());
      }
      frame_->formals.push_back(access);
    }
    //form the view shift
    frame_->viewShift=new tree::StmList();
    // std::list<tree::Stm *> stmList;
    int count = 0;
    int posOffset=0;
    for(auto it:frame_->formals){
      tree::Stm *stm;
      tree::Exp *dst;
      //get the destination
      if(it->kind == frame::Access::INFRAME){
        frame::InFrameAccess* Faccess=(frame::InFrameAccess*)it;
        dst = new tree::MemExp(new tree::BinopExp(tree::BinOp::PLUS_OP,new tree::TempExp(regmanager->FramePointer()),new tree::ConstExp(Faccess->offset)));
      }else{
        dst = new tree::TempExp(((frame::InRegAccess*)it)->reg);
      }
      //form the move stm
      switch (count)
      {
        //0-5 the arguments passed through reg
      case 0:
        stm = new tree::MoveStm(dst,new tree::TempExp(regmanager->RDI()));
        break;
      case 1:
        stm = new tree::MoveStm(dst,new tree::TempExp(regmanager->RSI()));
        break;
      case 2:
        stm = new tree::MoveStm(dst,new tree::TempExp(regmanager->RDX()));
        break;
      case 3:
        stm = new tree::MoveStm(dst,new tree::TempExp(regmanager->RCX()));
        break;
      case 4:
        stm = new tree::MoveStm(dst,new tree::TempExp(regmanager->R8()));
        break;
      case 5:
        stm = new tree::MoveStm(dst,new tree::TempExp(regmanager->R9()));
        break; 
      case 6:
        stm = new tree::MoveStm(dst,new tree::TempExp(regmanager->R10()));
        break; 
      case 7:
        stm = new tree::MoveStm(dst,new tree::TempExp(regmanager->R11()));
        break; 
      case 8:
        stm = new tree::MoveStm(dst,new tree::TempExp(regmanager->R12()));
        break;             
      default:
      //the argument passed in frame
        posOffset = (count-5)*frame::X64Frame::wordSize;
        //should add the size of frame in the pro3
        stm = new tree::MoveStm(dst,new tree::MemExp(
          new tree::BinopExp(tree::BinOp::PLUS_OP,new tree::TempExp(regmanager->FramePointer()),new tree::ConstExp(posOffset))
          )
        );
        break;
      }
      // stmList.push_back(stm);
      frame_->viewShift->Linear(stm);
      count++;
    }
    return frame_;
    // frame_->viewShift->GetList()

  }
  frame::Access* allocLocal(bool escape){
    frame::Access *access;
    if(escape){
      access = new InFrameAccess(this->offset);
      offset = offset - wordSize;
    }else{
      access = new InRegAccess(temp::TempFactory::NewTemp());
    }
    std::list<frame::Access*> localList=this->locals;
    locals.push_back(access);
    return access;
  }
};
class X64RegManager : public RegManager {
  /* TODO: Put your lab5 code here */
public:
  X64RegManager();
  temp::Temp *RAX();
  temp::Temp *RBX();
  temp::Temp *RCX();
  temp::Temp *RDX();
  temp::Temp *RSI();
  temp::Temp *RDI();
  temp::Temp *RBP();
  temp::Temp *RSP();
  temp::Temp *R8();
  temp::Temp *R9();
  temp::Temp *R10();
  temp::Temp *R11();
  temp::Temp *R12();
  temp::Temp *R13();
  temp::Temp *R14();
  temp::Temp *R15();
  temp::TempList *Registers();
  temp::TempList *ArgRegs();
  temp::TempList *CallerSaves();  
  temp::TempList *CalleeSaves();
  temp::TempList *ReturnSink(){}
  int WordSize(){}
  temp::Temp *FramePointer();
  temp::Temp *StackPointer();
  temp::Temp *ReturnValue();
  temp::Temp *GetArgByNum(int num);
private:
  temp::Temp *rax=nullptr;

  temp::Temp *rbx=nullptr;

  temp::Temp *rcx=nullptr;  

  temp::Temp *rdx=nullptr;  

  temp::Temp *rsi=nullptr;  
 
  temp::Temp *rdi=nullptr;  

  temp::Temp *rbp=nullptr;  

  temp::Temp *rsp=nullptr;  

  temp::Temp *r8=nullptr;  

  temp::Temp *r9=nullptr;  

  temp::Temp *r10=nullptr;  

  temp::Temp *r11=nullptr;  

  temp::Temp *r12=nullptr;  

  temp::Temp *r13=nullptr;  

  temp::Temp *r14=nullptr;  
 
  temp::Temp *r15=nullptr;    
  void inputMap(temp::Temp* key,std::string val);
};
  //register
} // namespace frame
#endif // TIGER_COMPILER_X64FRAME_H
