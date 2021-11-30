#include "tiger/escape/escape.h"
#include "tiger/absyn/absyn.h"

namespace esc {
void EscFinder::FindEscape() { absyn_tree_->Traverse(env_.get()); }
} // namespace esc

namespace absyn {

void AbsynTree::Traverse(esc::EscEnvPtr env) {
  /* TODO: Put your lab5 code here */
  this->root_->Traverse(env,1);
}

void SimpleVar::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  esc::EscapeEntry* valEntry = env->Look(this->sym_);
  if(valEntry&&depth>valEntry->depth_){
    *(valEntry->escape_)=true;
  }
}

void FieldVar::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
    this->var_->Traverse(env,depth);
}

void SubscriptVar::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  this->var_->Traverse(env,depth);
  this->subscript_->Traverse(env,depth);
}

void VarExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  this->var_->Traverse(env,depth);
}

void NilExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  //DO NOTHING
}

void IntExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  //DO NOTHING
}

void StringExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  //DO NOTHING
}

void CallExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  // ExpList * explist=this->args_;
  std::list<Exp*> explist=this->args_->GetList();
  for(auto it:explist){
    it->Traverse(env,depth);
  }
}

void OpExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  this->left_->Traverse(env,depth);
  this->right_->Traverse(env,depth);
}

void RecordExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  std::list<EField *> efildList=this->fields_->GetList();
  for(auto it:efildList){
    it->exp_->Traverse(env,depth);
  }
}

void SeqExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  std::list<Exp*> expList=this->seq_->GetList();
  for(auto it:expList){
    it->Traverse(env,depth);
  }
}

void AssignExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  this->var_->Traverse(env,depth);
  this->exp_->Traverse(env,depth);
}

void IfExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  this->test_->Traverse(env,depth);
  this->then_->Traverse(env,depth);
  if(this->elsee_){
    this->elsee_->Traverse(env,depth);
  }
}

void WhileExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  this->test_->Traverse(env,depth);
  this->body_->Traverse(env,depth);
}

void ForExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  // bool flag=false;
  this->escape_=false;
  esc::EscapeEntry* escape=new esc::EscapeEntry(depth,&(this->escape_));
  env->Enter(this->var_,escape);
  this->lo_->Traverse(env,depth);
  this->hi_->Traverse(env,depth);
  this->body_->Traverse(env,depth);
}

void BreakExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  //DO NOTHING
}

void LetExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  std::list<Dec *> decList=this->decs_->GetList();
  for(auto it:decList){
    it->Traverse(env,depth);
  }
  this->body_->Traverse(env,depth);
}

void ArrayExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  this->size_->Traverse(env,depth);
  this->init_->Traverse(env,depth);
}

void VoidExp::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  //DO NOTHING
}

void FunctionDec::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  std::list<FunDec *> funDecList=this->functions_->GetList();
  for(auto it:funDecList){
    env->BeginScope();
    std::list<Field *> fieldList=it->params_->GetList();
    for(auto that:fieldList){
      that->escape_=false;
       esc::EscapeEntry *e=new esc::EscapeEntry(depth+1,&(that->escape_));
      env->Enter(that->name_,e);
    }
    it->body_->Traverse(env,depth+1);
    env->EndScope();
  }

}

void VarDec::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  this->escape_=false;
  esc::EscapeEntry *e=new esc::EscapeEntry(depth,&(this->escape_));
  env->Enter(this->var_,e);
  this->init_->Traverse(env,depth);
}

void TypeDec::Traverse(esc::EscEnvPtr env, int depth) {
  /* TODO: Put your lab5 code here */
  //DO NOTHING
}

} // namespace absyn
