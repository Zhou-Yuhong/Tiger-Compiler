#include "tiger/translate/translate.h"

#include <tiger/absyn/absyn.h>

#include "tiger/env/env.h"
#include "tiger/errormsg/errormsg.h"
#include "tiger/frame/x64frame.h"
#include "tiger/frame/temp.h"
#include "tiger/frame/frame.h"

extern frame::Frags *frags;
extern frame::RegManager *reg_manager;

namespace tr {


class Cx {
public:
  temp::Label **trues_;
  temp::Label **falses_;
  tree::Stm *stm_;

  Cx(temp::Label **trues, temp::Label **falses, tree::Stm *stm)
      : trues_(trues), falses_(falses), stm_(stm) {}
};

class Exp {
public:
  [[nodiscard]] virtual tree::Exp *UnEx() const = 0;
  [[nodiscard]] virtual tree::Stm *UnNx() const = 0;
  [[nodiscard]] virtual Cx UnCx(err::ErrorMsg *errormsg) const = 0;
};

class ExpAndTy {
public:
  tr::Exp *exp_;
  type::Ty *ty_;

  ExpAndTy(tr::Exp *exp, type::Ty *ty) : exp_(exp), ty_(ty) {}
};

class ExExp : public Exp {
public:
  tree::Exp *exp_;

  explicit ExExp(tree::Exp *exp) : exp_(exp) {}

  [[nodiscard]] tree::Exp *UnEx() const override { 
    /* TODO: Put your lab5 code here */
    return this->exp_;
  }
  [[nodiscard]] tree::Stm *UnNx() const override {
    /* TODO: Put your lab5 code here */
    return new tree::ExpStm(this->exp_);
  }
  [[nodiscard]] Cx UnCx(err::ErrorMsg *errormsg) const override {
    /* TODO: Put your lab5 code here */
    tree::CjumpStm *stm = new tree::CjumpStm(tree::RelOp::NE_OP,this->exp_,new tree::ConstExp(0),NULL,NULL);
    Cx cx(&(stm->true_label_),&(stm->false_label_),stm);
    return cx;
    // temp::Label *t = temp::LabelFactory::NewLabel();
    // temp::Label *f = temp::LabelFactory::NewLabel();
    // temp::Label **trues=&t;
    // temp::Label **falses=&f;
    // tree::CjumpStm *stm = new tree::CjumpStm(tree::NE_OP,this->exp_,new tree::ConstExp(0),t,f);
    // Cx cx(trues,falses,stm);
    // return cx;
  }
};

class NxExp : public Exp {
public:
  tree::Stm *stm_;

  explicit NxExp(tree::Stm *stm) : stm_(stm) {}

  [[nodiscard]] tree::Exp *UnEx() const override {
    /* TODO: Put your lab5 code here */
    //just return 0
    return new tree::EseqExp(this->stm_,new tree::ConstExp(0));

  }
  [[nodiscard]] tree::Stm *UnNx() const override { 
    /* TODO: Put your lab5 code here */
    return this->stm_;
  }
  [[nodiscard]] Cx UnCx(err::ErrorMsg *errormsg) const override {
    /* TODO: Put your lab5 code here */
    errormsg->Error(0,"ERROR: nx can not turn to cx");
  }
};

class CxExp : public Exp {
public:
  Cx cx_;

  CxExp(temp::Label** trues, temp::Label** falses, tree::Stm *stm)
      : cx_(trues, falses, stm) {}
  
  [[nodiscard]] tree::Exp *UnEx() const override {
    /* TODO: Put your lab5 code here */
    temp::Temp* r=temp::TempFactory::NewTemp();
    if(*(this->cx_.trues_)==NULL){
      *(this->cx_.trues_)=temp::LabelFactory::NewLabel();
    }
    if(*(this->cx_.falses_)==NULL){
      *(this->cx_.falses_)=temp::LabelFactory::NewLabel();
    }
    temp::Label* t=*(this->cx_.trues_);
    temp::Label* f=*(this->cx_.falses_);
    return new tree::EseqExp(
      new tree::MoveStm(new tree::TempExp(r),new tree::ConstExp(1)),
      new tree::EseqExp(
        this->cx_.stm_,
        new tree::EseqExp(
          new tree::LabelStm(f),
          new tree::EseqExp(new tree::MoveStm(new tree::TempExp(r),new tree::ConstExp(0)),
          new tree::EseqExp(new tree::LabelStm(t),new tree::TempExp(r))
          )
        )
      )
    );
  }
  [[nodiscard]] tree::Stm *UnNx() const override {
    /* TODO: Put your lab5 code here */
    temp::Label *label = temp::LabelFactory::NewLabel();
    *(this->cx_.trues_)=label;
    *(this->cx_.falses_)=label;
    return new tree::SeqStm(
      cx_.stm_,new tree::LabelStm(label)
    );
  }
  [[nodiscard]] Cx UnCx(err::ErrorMsg *errormsg) const override { 
    /* TODO: Put your lab5 code here */
    return this->cx_;
  }
};

void ProgTr::Translate() {
  /* TODO: Put your lab5 code here */
  std::list<bool> escape;
  temp::Label* mainLabel = temp::LabelFactory::NamedLabel("tigermain");
  frame::Frame *frame = frame::X64Frame::newFrame(mainLabel,escape,reg_manager);
  main_level_ = std::make_unique<Level>(frame,nullptr);
  this->tenv_ = std::make_unique<env::TEnv>();
  this->venv_ = std::make_unique<env::VEnv>();
  this->FillBaseTEnv();
  this->FillBaseVEnv();
  tr::ExpAndTy* expAndTy=absyn_tree_->Translate(venv_.get(),tenv_.get(),main_level_.get(),mainLabel,errormsg_.get());
  frags->PushBack(new frame::ProcFrag(expAndTy->exp_->UnNx(),frame));
}
tree::MemExp* formMemInstruction(tree::Exp* base,int offset){
  return new tree::MemExp(new tree::BinopExp(tree::BinOp::PLUS_OP,base,new tree::ConstExp(offset)));
}
tr::Exp* emptyExp(){
  return new tr::ExExp(new tree::ConstExp(0));
}
tr::Exp* translateSeq(tr::Exp* left,tr::Exp* right){
  if(right){
    return new tr::ExExp(new tree::EseqExp(left->UnNx(),right->UnEx()));
  }
  else{
    return new tr::ExExp(new tree::EseqExp(left->UnNx(),new tree::ConstExp(0)));
  }
}
tr::Access *Access::AllocLocal(Level *level, bool escape) {
  /* TODO: Put your lab5 code here */
  frame::Access* frameAccess = level->frame_->allocLocal(escape);
  return new tr::Access(level,frameAccess);
}
//TODO DEBUG HERE, IGNORE THE SITUATION THAT IN THE SANME LEVEL AND IN REG
tr::Exp *simpleVar(tr::Access* access,tr::Level *level){
  if(access->access_->kind==frame::Access::INREG){
    tree::Exp* result= new tree::TempExp(
      ((frame::InRegAccess*)(access->access_))->reg
    );
    return new tr::ExExp(result);
  }
    tree::Exp *staticLink = new tree::TempExp(reg_manager->FramePointer());
    while(access->level_!=level){
      staticLink = new tree::MemExp(new tree::BinopExp(tree::BinOp::PLUS_OP,staticLink,new tree::ConstExp(-frame::X64Frame::wordSize)));
      level = level->parent_;
    }
    staticLink = new tree::MemExp(new tree::BinopExp(tree::BinOp::PLUS_OP,staticLink,new tree::ConstExp(((frame::InFrameAccess*)(access->access_))->offset)));
    return new tr::ExExp(staticLink);

}
tr::Exp* ifExp(tr::Exp *test,tr::Exp* then,tr::Exp* elsee,err::ErrorMsg *errormsg){
  tr::Cx cx = test->UnCx(errormsg);
  //alloc three labels and fill in
  temp::Label *trueLabel = temp::LabelFactory::NewLabel();
  temp::Label *falseLabel = temp::LabelFactory::NewLabel();
  temp::Label *finalLabel = temp::LabelFactory::NewLabel();
  *(cx.trues_) = trueLabel;
  *(cx.falses_)= falseLabel;
  if(elsee){
    temp::Temp* reg = temp::TempFactory::NewTemp();
    std::vector<temp::Label*>* jumps = new std::vector<temp::Label*>;
    jumps->push_back(finalLabel);
    return new tr::ExExp(
      new tree::EseqExp(cx.stm_,
        new tree::EseqExp(new tree::LabelStm(trueLabel),
          new tree::EseqExp(new tree::MoveStm(new tree::TempExp(reg),then->UnEx()),
            new tree::EseqExp(new tree::JumpStm(new tree::NameExp(finalLabel),jumps),
              new tree::EseqExp(new tree::LabelStm(falseLabel),
                new tree::EseqExp(new tree::MoveStm(new tree::TempExp(reg),elsee->UnEx()),
                  new tree::EseqExp(new tree::JumpStm(new tree::NameExp(finalLabel),jumps),
                    new tree::EseqExp(new tree::LabelStm(finalLabel),
                      new tree::TempExp(reg)
                    )
                  )
                )
              )
            )
          )
        )
      )
    );

  }else{
    return new tr::NxExp(
      new tree::SeqStm(
        cx.stm_,new tree::SeqStm(
          new tree::LabelStm(trueLabel),
          new tree::SeqStm(
            then->UnNx(),
            new tree::LabelStm(falseLabel)
          )
        )
      )
    );
  }
}
tr::Exp *funCall(temp::Label *label,std::list<tr::Exp *> args,tr::Level* caller,tr::Level* callee){
  //form the staticLink
  tree::Exp *staticLink = new tree::TempExp(reg_manager->FramePointer());
  while(caller != callee->parent_){
    staticLink = new tree::MemExp(new tree::BinopExp(tree::BinOp::PLUS_OP,staticLink,new tree::ConstExp(-frame::X64Frame::wordSize)));
    caller = caller->parent_;
  }
  tree::ExpList* explist = new tree::ExpList();
  for(auto it:args){
    explist->Append(it->UnEx());
  }
  if(callee->parent_){
    explist->Insert(staticLink);
    return new tr::ExExp(new tree::CallExp(new tree::NameExp(label),explist));
  }
  else{
    //external call
    return new tr::ExExp(new tree::CallExp(
      new tree::NameExp(temp::LabelFactory::NamedLabel(label->Name())),explist)
    );
  }
}

} // namespace tr

namespace absyn {

tr::ExpAndTy *AbsynTree::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return this->root_->Translate(venv,tenv,level,label,errormsg);
}

tr::ExpAndTy *SimpleVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  env::VarEntry* varEnv=((env::VarEntry*)venv->Look(this->sym_));
  tr::Access *access = varEnv->access_;
  return new tr::ExpAndTy(tr::simpleVar(access,level),varEnv->ty_->ActualTy());
}

tr::ExpAndTy *FieldVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  tr::ExpAndTy *base = this->var_->Translate(venv,tenv,level,label,errormsg);
  type::Ty* varTy=this->var_->SemAnalyze(venv,tenv,1,errormsg)->ActualTy();
  std::list<type::Field*> flist = ((type::RecordTy*)varTy)->fields_->GetList();
  int count=0;
  for(auto it:flist){
    if(it->name_->Name()==this->sym_->Name()){
      tr::Exp* exp=new tr::ExExp(
      new tree::MemExp(new tree::BinopExp(tree::BinOp::PLUS_OP,base->exp_->UnEx(),new tree::ConstExp(count*frame::X64Frame::wordSize)))
      );
      return new tr::ExpAndTy(
        exp,
        it->ty_
      );
    }
    count++;
  }
  return new tr::ExpAndTy(nullptr,type::VoidTy::Instance());
}

tr::ExpAndTy *SubscriptVar::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                      tr::Level *level, temp::Label *label,
                                      err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  type::Ty* varTy=this->var_->SemAnalyze(venv,tenv,0,errormsg)->ActualTy();
  tr::ExpAndTy *base = this->var_->Translate(venv,tenv,level,label,errormsg);
  tr::ExpAndTy *offset = this->subscript_->Translate(venv,tenv,level,label,errormsg);
  tr::Exp* exp = new tr::ExExp(new tree::MemExp(
    new tree::BinopExp(
      tree::BinOp::PLUS_OP,
      base->exp_->UnEx(),
      new tree::BinopExp(tree::BinOp::MUL_OP,offset->exp_->UnEx(),new tree::ConstExp(frame::X64Frame::wordSize))
    )
  ));
  return new tr::ExpAndTy(exp,((type::ArrayTy*)varTy)->ty_->ActualTy());
}

tr::ExpAndTy *VarExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return this->var_->Translate(venv,tenv,level,label,errormsg);

}

tr::ExpAndTy *NilExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return new tr::ExpAndTy(
    new tr::ExExp(new tree::ConstExp(0)),
    type::NilTy::Instance()
  );
}

tr::ExpAndTy *IntExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return new tr::ExpAndTy(
    new tr::ExExp(new tree::ConstExp(this->val_)),
      type::IntTy::Instance()
    );
}

tr::ExpAndTy *StringExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  //form the stringFrag
  temp::Label* stringLabel=temp::LabelFactory::NewLabel();
  frame::Frag* sFrag=new frame::StringFrag(stringLabel,this->str_);
  frags->PushBack(sFrag);
  return new tr::ExpAndTy(
    new tr::ExExp(
      new tree::NameExp(stringLabel)
    ),      
    type::StringTy::Instance()
  );
}

tr::ExpAndTy *CallExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                 tr::Level *level, temp::Label *label,
                                 err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::list<absyn::Exp*> args=this->args_->GetList();
  std::list<tr::Exp*> expArgs;
  for(auto it:args){
    tr::ExpAndTy* arg=it->Translate(venv,tenv,level,label,errormsg);
    expArgs.push_back(arg->exp_);
  }
  env::EnvEntry* func=venv->Look(this->func_);
  // if(func!=nullptr){
    tr::Exp* result=tr::funCall(this->func_,expArgs,level,((env::FunEntry*)func)->level_);
    return new tr::ExpAndTy(result,((env::FunEntry*)func)->result_->ActualTy());
  // }else{
  //   //externalCall
  //   tree::ExpList* explist = new tree::ExpList();
  //   for(auto it:expArgs){
  //     explist->Append(it->UnEx());
  //   }
  //   tr::Exp* result=new tree::CallExp(
  //     new tree::NameExp(
  //       new temp::LabelFactory::NamedLabel(this->func_->Name())
  //     ),explist
  //   );
  //   return new tr::ExpAndTy()
  // }  

}

tr::ExpAndTy *OpExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                               tr::Level *level, temp::Label *label,
                               err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  tr::ExpAndTy* transLeft=this->left_->Translate(venv,tenv,level,label,errormsg);
  tr::ExpAndTy* transRight=this->right_->Translate(venv,tenv,level,label,errormsg);
  tree::CjumpStm* stm;
  tr::Exp* exp;
  switch (this->oper_)
  {
  case absyn::Oper::PLUS_OP:
    exp = new tr::ExExp(new tree::BinopExp(
      tree::BinOp::PLUS_OP,transLeft->exp_->UnEx(),transRight->exp_->UnEx()
    ));  
    break;
  case absyn::Oper::MINUS_OP:
    exp = new tr::ExExp(new tree::BinopExp(
      tree::BinOp::MINUS_OP,transLeft->exp_->UnEx(),transRight->exp_->UnEx()
    ));
    break;
  case absyn::Oper::TIMES_OP:
    exp = new tr::ExExp(new tree::BinopExp(
      tree::BinOp::MUL_OP,transLeft->exp_->UnEx(),transRight->exp_->UnEx()
    ));
    break;
  case absyn::Oper::DIVIDE_OP:
    exp = new tr::ExExp(new tree::BinopExp(
      tree::BinOp::DIV_OP,transLeft->exp_->UnEx(),transRight->exp_->UnEx()
    ));
    break;
  case absyn::Oper::LT_OP:
  case absyn::Oper::LE_OP:
  case absyn::Oper::GT_OP:
  case absyn::Oper::GE_OP:
  case absyn::Oper::EQ_OP:
  case absyn::Oper::NEQ_OP:
  {
    switch (this->oper_)
    {
    case absyn::Oper::LT_OP:
      stm = new tree::CjumpStm(tree::LT_OP,transLeft->exp_->UnEx(),transRight->exp_->UnEx(),NULL,NULL);
      break;
    case absyn::Oper::LE_OP:
      stm = new tree::CjumpStm(tree::LE_OP,transLeft->exp_->UnEx(),transRight->exp_->UnEx(),NULL,NULL);
      break;
    case absyn::Oper::GT_OP:
      stm = new tree::CjumpStm(tree::GT_OP,transLeft->exp_->UnEx(),transRight->exp_->UnEx(),NULL,NULL);
      break;
    case absyn::Oper::GE_OP:
      stm = new tree::CjumpStm(tree::GE_OP,transLeft->exp_->UnEx(),transRight->exp_->UnEx(),NULL,NULL);
      break;  
    case absyn::Oper::EQ_OP:
      stm = new tree::CjumpStm(tree::EQ_OP,transLeft->exp_->UnEx(),transRight->exp_->UnEx(),NULL,NULL);
      break;
    case absyn::Oper::NEQ_OP:
      stm = new tree::CjumpStm(tree::NE_OP,transLeft->exp_->UnEx(),transRight->exp_->UnEx(),NULL,NULL);
      break;     
    }
    exp = new tr::CxExp(&(stm->true_label_),&(stm->false_label_),stm);
    tr::CxExp* cxdebug = (tr::CxExp*)exp;
    break;
  }        
  default:
    break;
  }
  return new tr::ExpAndTy(exp,type::IntTy::Instance());
}

tr::ExpAndTy *RecordExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,      
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  type::Ty* ty = tenv->Look(this->typ_)->ActualTy();
  std::list<absyn::EField *> efields=this->fields_->GetList();
  std::list<tree::Exp*> explist;
  for(auto it:efields){
    tr::ExpAndTy *item=it->exp_->Translate(venv,tenv,level,label,errormsg);
    explist.push_back(item->exp_->UnEx());
  }
  temp::Temp *reg = temp::TempFactory::NewTemp();
  tree::ExpList* args = new tree::ExpList();
  args->Append(new tree::ConstExp(efields.size()*frame::X64Frame::wordSize));
  tree::Stm *stm = new tree::MoveStm(new tree::TempExp(reg),frame::externalCall("alloc_record",args));
  int count=0;
  for(auto it:explist){
    stm = new tree::SeqStm(stm,new tree::MoveStm(tr::formMemInstruction(new tree::TempExp(reg),count*frame::X64Frame::wordSize),it));
    count++;
  }
  tr::ExExp* result=new tr::ExExp(new tree::EseqExp(stm,new tree::TempExp(reg)));
  return new tr::ExpAndTy(result,ty);
}

tr::ExpAndTy *SeqExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::list<absyn::Exp*> explist=this->seq_->GetList();
  if(explist.empty()){
    return new tr::ExpAndTy(tr::emptyExp(),type::VoidTy::Instance());
  }
  tr::Exp* exp=tr::emptyExp();
  tr::ExpAndTy* item;
  for(auto it:explist){
    item=it->Translate(venv,tenv,level,label,errormsg);
    exp = tr::translateSeq(exp,item->exp_);
  }
  return new tr::ExpAndTy(exp,item->ty_);
}

tr::ExpAndTy *AssignExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   tr::Level *level, temp::Label *label,                       
                                   err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  tr::ExpAndTy *transVal=this->var_->Translate(venv,tenv,level,label,errormsg);
  tr::ExpAndTy *transExp=this->exp_->Translate(venv,tenv,level,label,errormsg);
  tr::Exp* result=new tr::NxExp(new tree::MoveStm(transVal->exp_->UnEx(),transExp->exp_->UnEx()));
  return new tr::ExpAndTy(result,type::VoidTy::Instance());
}

tr::ExpAndTy *IfExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                               tr::Level *level, temp::Label *label,
                               err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  // tr::ExpAndTy* transTest = this->test_->Translate(venv,tenv,level,label,errormsg);
  // tr::ExpAndTy* transThen = this->then_->Translate(venv,tenv,level,label,errormsg);
  // if(elsee_){
  //   tr::ExpAndTy *transelse = this->elsee_->Translate(venv,tenv,level,label,errormsg);
  //   return new tr::ExpAndTy(
  //     tr::ifExp(transTest->exp_,transThen->exp_,transelse->exp_,errormsg),transThen->ty_
  //   );
  // }
  // else{
  //   return new tr::ExpAndTy(
  //     tr::ifExp(transTest->exp_,transThen->exp_,nullptr,errormsg),transThen->ty_
  //   );
  // }
  
  tr::ExpAndTy* transTest = this->test_->Translate(venv,tenv,level,label,errormsg);
  tr::ExpAndTy* transThen = this->then_->Translate(venv,tenv,level,label,errormsg);
  tr::Exp* result;
  tr::Cx test=transTest->exp_->UnCx(errormsg);
  temp::Temp *reg = temp::TempFactory::NewTemp();
  temp::Label *trueLabel = temp::LabelFactory::NewLabel();
  temp::Label *falseLabel = temp::LabelFactory::NewLabel();
  temp::Label *finalLabel = temp::LabelFactory::NewLabel();
  *(test.trues_)=trueLabel;
  *(test.falses_)=falseLabel;
  if(this->elsee_){
    tr::ExpAndTy* transElse = this->elsee_->Translate(venv,tenv,level,label,errormsg);
    // std::vector<temp::Label *> jumps;
    std::vector<sym::Symbol*>* jumps=new std::vector<temp::Label*>;
    (*jumps).push_back(finalLabel);
    result = new tr::ExExp(
      new tree::EseqExp(test.stm_,
        new tree::EseqExp(new tree::LabelStm(trueLabel),
          new tree::EseqExp(new tree::MoveStm(new tree::TempExp(reg),transThen->exp_->UnEx()),
            new tree::EseqExp(new tree::JumpStm(new tree::NameExp(finalLabel),jumps),
              new tree::EseqExp(new tree::LabelStm(falseLabel),
                new tree::EseqExp(new tree::MoveStm(new tree::TempExp(reg),transElse->exp_->UnEx()),
                  new tree::EseqExp(new tree::JumpStm(new tree::NameExp(finalLabel),jumps),
                    new tree::EseqExp(new tree::LabelStm(finalLabel),
                      new tree::TempExp(reg)
                    )
                  )
                )
              )
            )
          )
        )
      )
    );
  }else{
    result = new tr::NxExp(
      new tree::SeqStm(test.stm_,
        new tree::SeqStm(new tree::LabelStm(trueLabel),
          new tree::SeqStm(transThen->exp_->UnNx(),
            new tree::LabelStm(falseLabel)
          )
        )
      )
    );
  }
  return new tr::ExpAndTy(result,transThen->ty_);
}

tr::ExpAndTy *WhileExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,            
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  tr::Exp* result=nullptr;
  temp::Label* testLabel=temp::LabelFactory::NewLabel();
  temp::Label* bodyLabel=temp::LabelFactory::NewLabel();
  temp::Label* doneLabel=temp::LabelFactory::NewLabel();
  tr::ExpAndTy* transTest=this->test_->Translate(venv,tenv,level,label,errormsg);
  tr::ExpAndTy* transBody=this->body_->Translate(venv,tenv,level,doneLabel,errormsg);
  tr::Cx cx = transTest->exp_->UnCx(errormsg);
  *(cx.trues_)=bodyLabel;
  *(cx.falses_)=doneLabel;
  //std::vector<temp::Label*> jumpList;
  std::vector<temp::Label*>* jumpList=new std::vector<temp::Label*>;
  (*jumpList).push_back(testLabel);
  // jumpList.push_back(testLabel);
  result = new tr::NxExp(
    new tree::SeqStm(new tree::LabelStm(testLabel),
      new tree::SeqStm(cx.stm_,
        new tree::SeqStm(new tree::LabelStm(bodyLabel),
          new tree::SeqStm(transBody->exp_->UnNx(),
            new tree::SeqStm(new tree::JumpStm(new tree::NameExp(testLabel),jumpList),
              new tree::LabelStm(doneLabel)
            )
          )
        )
      )
    )
  );
  return new tr::ExpAndTy(result,transBody->ty_);
}

tr::ExpAndTy *ForExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  // tr::ExpAndTy checkLo = this->lo_->Translate(venv,tenv,level,label,errormsg);
  // tr::ExpAndTy checkHi = this->hi_->Translate(venv,tenv,level,label,errormsg);

  //form the devlist
  absyn::DecList *declist = new absyn::DecList();
  absyn::VarDec *i = new absyn::VarDec(0,this->var_,NULL,this->lo_);
  absyn::VarDec *end=new absyn::VarDec(0,sym::Symbol::UniqueSymbol("limit"),NULL,this->hi_);
  i->escape_=this->escape_;
  declist->Prepend(end);
  declist->Prepend(i);
  //form the body
  absyn::Exp* test=new absyn::OpExp(0,absyn::LE_OP,new absyn::VarExp(0,new absyn::SimpleVar(0,this->var_)),new absyn::VarExp(
    0,new absyn::SimpleVar(0,sym::Symbol::UniqueSymbol("limit"))
  ));
  absyn::ExpList* explist=new ExpList();
  explist->Prepend(new absyn::AssignExp(
    0,new absyn::SimpleVar(0,this->var_),
    new absyn::OpExp(0,absyn::PLUS_OP,new absyn::VarExp(0,new absyn::SimpleVar(0,this->var_)),
    new absyn::IntExp(0,1)
    )
  ));  
  explist->Prepend(new absyn::IfExp(0,
    new absyn::OpExp(0,absyn::EQ_OP,
    new absyn::VarExp(0,new absyn::SimpleVar(0,this->var_)),
    new absyn::VarExp(0,new absyn::SimpleVar(0,sym::Symbol::UniqueSymbol("limit")))),
    new absyn::BreakExp(0),nullptr
  )); 
  explist->Prepend(this->body_);
  absyn::SeqExp *seq=new absyn::SeqExp(0,explist);
  absyn::WhileExp* whileExp=new absyn::WhileExp(0,test,seq);

  absyn::LetExp* letexp=new absyn::LetExp(0,declist,whileExp);
  return letexp->Translate(venv,tenv,level,label,errormsg);
}

tr::ExpAndTy *BreakExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  //jump to the label
  // std::vector<temp::Label*> jumplist;
  // jumplist.push_back(label);
  std::vector<temp::Label*> *jumplist = new std::vector<temp::Label*>;
  (*jumplist).push_back(label);
  tree::Stm* stm = new tree::JumpStm(new tree::NameExp(label),jumplist);
  return new tr::ExpAndTy(new tr::NxExp(stm),type::VoidTy::Instance());
}

tr::ExpAndTy *LetExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::list<Dec*> decList=this->decs_->GetList();
  tr::Exp* seq=nullptr;
  venv->BeginScope();
  tenv->BeginScope();
  for(auto it:decList){
    if(seq){
      seq = tr::translateSeq(seq,it->Translate(venv,tenv,level,label,errormsg));
    }
    else{
      seq = it->Translate(venv,tenv,level,label,errormsg);
    }
  }
  tr::ExpAndTy* body = this->body_->Translate(venv,tenv,level,label,errormsg);
  if(seq){
    seq = tr::translateSeq(seq,body->exp_);
  }else{
    seq = body->exp_;
  }
  venv->EndScope();
  tenv->EndScope();
  return new tr::ExpAndTy(seq,body->ty_);

}

tr::ExpAndTy *ArrayExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                  tr::Level *level, temp::Label *label,                    
                                  err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  tr::ExpAndTy *transSize=this->size_->Translate(venv,tenv,level,label,errormsg);
  tr::ExpAndTy *transInit=this->init_->Translate(venv,tenv,level,label,errormsg);
  tree::ExpList* explist = new tree::ExpList();
  explist->Append(transSize->exp_->UnEx());
  explist->Append(transInit->exp_->UnEx());
  tr::Exp* result = new tr::ExExp(frame::externalCall("init_array",explist));
  return new tr::ExpAndTy(result,tenv->Look(this->typ_)->ActualTy());
}

tr::ExpAndTy *VoidExp::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                 tr::Level *level, temp::Label *label,
                                 err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return new tr::ExpAndTy(nullptr,type::VoidTy::Instance());
}

tr::Exp *FunctionDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                                tr::Level *level, temp::Label *label,
                                err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::list<FunDec*> funList=this->functions_->GetList();
  //form the level and env
  for(auto it:funList){
    type::TyList * formals=new type::TyList();
    std::list<Field *> fieldList=it->params_->GetList();
    std::list<bool> escapeList;
    for(auto that:fieldList){
      formals->Append(
        tenv->Look(that->typ_)
      );
      escapeList.push_back(that->escape_);
    }
    //alloc new level
    temp::Label *name=temp::LabelFactory::NamedLabel(it->name_->Name());
    tr::Level* downLevel=tr::Level::newLevel(level,name,escapeList,reg_manager);
    if(!it->result_){
      venv->Enter(it->name_,new env::FunEntry(
        downLevel,it->name_,formals,type::VoidTy::Instance()
      ));
    }else{
      venv->Enter(it->name_,new env::FunEntry(
        downLevel,it->name_,formals,tenv->Look(it->result_)
      ));
    }
  }
  for(auto it:funList){
    env::FunEntry *funentry = (env::FunEntry*)venv->Look(it->name_);
    std::list<absyn::Field*>fieldList=it->params_->GetList();
    std::list<absyn::Field*>::iterator fieldIter=fieldList.begin();
    type::TyList* formals = funentry->formals_;
    std::list<type::Ty*> formalList = formals->GetList();
    std::list<type::Ty*>::iterator tyIter = formalList.begin();
    venv->BeginScope();
    std::list<tr::Access*> accessList=funentry->level_->getFormalAccessList();
    //pay attention 
    for(std::list<tr::Access*>::iterator iter = ++accessList.begin();iter != accessList.end();iter++,tyIter++,fieldIter++){
      //the loop will escape the staticlink
      venv->Enter((*fieldIter)->name_,new env::VarEntry(*iter,(*tyIter)));
    }
    tr::ExpAndTy* body = it->body_->Translate(venv,tenv,funentry->level_,label,errormsg);
    venv->EndScope();
    //form the procFrag
    tree::Stm* stm = new tree::MoveStm(
      new tree::TempExp(reg_manager->ReturnValue()),
      body->exp_->UnEx()
    );
    stm = frame::F_procEntryExit1(funentry->level_->frame_,stm);
    frags->PushBack(new frame::ProcFrag(stm,funentry->level_->frame_));
  }
  return tr::emptyExp();
}

tr::Exp *VarDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                           tr::Level *level, temp::Label *label,
                           err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  tr::ExpAndTy* init=this->init_->Translate(venv,tenv,level,label,errormsg);
  tr::Access* access = tr::Access::AllocLocal(level,this->escape_);
  venv->Enter(this->var_,new env::VarEntry(access,init->ty_));
  return new tr::NxExp(
    new tree::MoveStm(
      tr::simpleVar(access,level)->UnEx(),
      init->exp_->UnEx()
    )
  );
}

tr::Exp *TypeDec::Translate(env::VEnvPtr venv, env::TEnvPtr tenv,
                            tr::Level *level, temp::Label *label,
                            err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  std::list<absyn::NameAndTy*> nameAndTyList=this->types_->GetList();
  //first put nameTy
  for(auto it:nameAndTyList){
    tenv->Enter(it->name_,new type::NameTy(it->name_,nullptr));
  }
  for(auto it:nameAndTyList){
    type::NameTy* namety=(type::NameTy*)tenv->Look(it->name_);
    namety->ty_=it->ty_->Translate(tenv,errormsg);
  }
  return tr::emptyExp();
}

type::Ty *NameTy::Translate(env::TEnvPtr tenv, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return new type::NameTy(this->name_,tenv->Look(this->name_));
}

type::Ty *RecordTy::Translate(env::TEnvPtr tenv,
                              err::ErrorMsg *errormsg) const {
  std::list<absyn::Field*> fieldList = this->record_->GetList();
  type::FieldList* tfieldList = new type::FieldList();
  for(auto it:fieldList){
    type::Ty* ty = tenv->Look(it->typ_);
    tfieldList->Append(
      new type::Field(it->name_,ty)
    );
  }
  return new type::RecordTy(tfieldList);
}

type::Ty *ArrayTy::Translate(env::TEnvPtr tenv, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab5 code here */
  return new type::ArrayTy(tenv->Look(this->array_));
}

} // namespace absyn
