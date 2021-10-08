#include "straightline/slp.h"

#include <iostream>

namespace A {
int A::CompoundStm::MaxArgs() const {
  // TODO: put your code here (lab1).
  int leftArgs=this->stm1->MaxArgs();
  int rightArgs=this->stm2->MaxArgs();
  return leftArgs>rightArgs ? leftArgs:rightArgs;
}

Table *A::CompoundStm::Interp(Table *t) const {
  // TODO: put your code here (lab1).
  return this->stm2->Interp(this->stm1->Interp(t));
}

int A::AssignStm::MaxArgs() const {
  // TODO: put your code here (lab1).
  return this->exp->MaxArgs();
}

Table *A::AssignStm::Interp(Table *t) const {
  // TODO: put your code here (lab1).
  IntAndTable* result=this->exp->Interp(t);
  return t->Update(this->id,result->i);
}

int A::PrintStm::MaxArgs() const {
  // TODO: put your code here (lab1).
  //get the num of it args
  ExpList* node=this->exps;
  int argsCount=1;
  int innerCount=node->MaxArgs();
  while(node->IsLast()!=1){
   argsCount++;
   node=((PairExpList *)node)->getTail();
  }
  return argsCount>innerCount ? argsCount:innerCount;
}

Table *A::PrintStm::Interp(Table *t) const {
  // TODO: put your code here (lab1).
  ExpList* node=this->exps;
  IntAndTable *it;
  while(node->IsLast()!=1){
    it=((PairExpList*)node)->Interp(t);
    std::cout<<it->i<<" ";
    node=((PairExpList*)node)->getTail();
  }
  it=((LastExpList*)node)->Interp(t);
  std::cout<<it->i<<std::endl;
  return t;
}
int A::IdExp::MaxArgs() const{
  return 0;
}
IntAndTable *A::IdExp::Interp(Table * t) const{
  int value=t->Lookup(this->id);
  return new IntAndTable(value,t);
}
int A::NumExp::MaxArgs() const{
  return 0;
}
IntAndTable *A::NumExp::Interp(Table *t) const{
  return new IntAndTable(this->num,t);
}
int A::OpExp::MaxArgs() const{
  int leftVal=this->left->MaxArgs();
  int rightVal=this->right->MaxArgs();
  return leftVal>rightVal? leftVal:rightVal;
}
IntAndTable *A::OpExp::Interp(Table* t)const{
  IntAndTable* leftVal=this->left->Interp(t);
  IntAndTable* rightVal=this->right->Interp(t);
  int value;
  switch(this->oper){
    case PLUS:
      value=leftVal->i+rightVal->i;
      break;
    case MINUS:
      value=leftVal->i-rightVal->i;
      break;
    case TIMES:
      value=leftVal->i*rightVal->i;
      break;
    case DIV:
      value=leftVal->i/rightVal->i;
      break;      
  }
  return new IntAndTable(value,t);
}
int A::EseqExp::MaxArgs()const{
  int leftArgs=this->stm->MaxArgs();
  int rightArgs=this->exp->MaxArgs();
  return leftArgs>rightArgs ? leftArgs:rightArgs;
}
IntAndTable *A::EseqExp::Interp(Table *t)const{
  this->stm->Interp(t);
  return this->exp->Interp(t);
}
int A::PairExpList::IsLast()const{
  return 0;
}
int A::PairExpList::MaxArgs()const{
  int leftArgs=this->exp->MaxArgs();
  int rightArgs=this->tail->MaxArgs();
  return leftArgs>rightArgs ? leftArgs:rightArgs;
}
IntAndTable * A::PairExpList::Interp(Table *t)const{
  return this->exp->Interp(t);
  
}

int A::LastExpList::IsLast()const{
  return 1;
}
int A::LastExpList::MaxArgs()const{
  return this->exp->MaxArgs();
}
IntAndTable * A::LastExpList::Interp(Table *t)const{
  return this->exp->Interp(t);
}
int Table::Lookup(const std::string &key) const {
  if (id == key) {
    return value;
  } else if (tail != nullptr) {
    return tail->Lookup(key);
  } else {
    assert(false);
  }
}

Table *Table::Update(const std::string &key, int val) const {
  return new Table(key, val, this);
}
}  // namespace A
