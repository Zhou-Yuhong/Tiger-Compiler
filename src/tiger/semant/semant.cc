#include "tiger/absyn/absyn.h"
#include "tiger/semant/semant.h"
#include <iostream>
#include <stack>
#include <vector>
std::stack<bool> inloop;
namespace absyn {

void AbsynTree::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                           err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  venv->BeginScope();
  tenv->BeginScope();
  this->root_->SemAnalyze(venv,tenv,0,errormsg);
  venv->EndScope();
  tenv->EndScope();
}

type::Ty *SimpleVar::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                int labelcount, err::ErrorMsg *errormsg) const {
  env::EnvEntry* result=venv->Look(this->sym_);
  
  if(result&&(typeid(*result)==typeid(env::VarEntry))){
    return ((env::VarEntry*)result)->ty_->ActualTy();
  }else{
    errormsg->Error(this->pos_,"undefined variable %s",this->sym_->Name().c_str());
    return type::VoidTy::Instance();
  }
}

type::Ty *FieldVar::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  type::Ty* varTy=this->var_->SemAnalyze(venv,tenv,labelcount,errormsg)->ActualTy();
  if(typeid(*varTy)!=typeid(type::RecordTy)){
     errormsg->Error(this->pos_,"not a record type");
     return type::VoidTy::Instance();
  } 
  type::FieldList* fileds=((type::RecordTy*)varTy)->fields_;
  for(const auto& it:fileds->GetList()){
     if(it->name_->Name()==this->sym_->Name()){
        return it->ty_;
     }
  }
  errormsg->Error(this->pos_,"field %s doesn't exist",this->sym_->Name().c_str());
  return type::VoidTy::Instance();
}

type::Ty *SubscriptVar::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                   int labelcount,
                                   err::ErrorMsg *errormsg) const {
  type::Ty* varTy=this->var_->SemAnalyze(venv,tenv,labelcount,errormsg)->ActualTy();
  type::Ty* expTy=this->subscript_->SemAnalyze(venv,tenv,labelcount,errormsg)->ActualTy();
  if(typeid(*varTy)!=typeid(type::ArrayTy)){
    errormsg->Error(this->pos_,"array type required");
    return type::VoidTy::Instance();
  }
  if(typeid(*expTy)!=typeid(type::IntTy)){
    errormsg->Error(this->pos_,"array index must be integer");
    return type::VoidTy::Instance();
  }
  return ((type::ArrayTy*)varTy)->ty_;

}

type::Ty *VarExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {

  return this->var_->SemAnalyze(venv,tenv,labelcount,errormsg);
}

type::Ty *NilExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  return type::NilTy::Instance();
}

type::Ty *IntExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  return type::IntTy::Instance();
}

type::Ty *StringExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                int labelcount, err::ErrorMsg *errormsg) const {
  return type::StringTy::Instance();
}

type::Ty *CallExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                              int labelcount, err::ErrorMsg *errormsg) const {
  env::EnvEntry* func = venv->Look(this->func_);
  if(!func||typeid(*func)!=typeid(env::FunEntry)){
    errormsg->Error(this->pos_,"undefined function %s",this->func_->Name().c_str());
    return type::VoidTy::Instance();
  }
  type::TyList *formals=((env::FunEntry *)func)->formals_;
  type::Ty *result = ((env::FunEntry *)func)->result_;
  std::list<type::Ty *> ty_list= formals->GetList(); //needed args
  std::list<Exp *> exp_list = this->args_->GetList(); //real args
  // if(ty_list.size()>exp_list.size()){
  //   errormsg->Error(this->pos_,"too many params in function %s",this->func_->Name().c_str());
  //   return type::VoidTy::Instance();
  // } 
  // if(ty_list.size()<exp_list.size()){
  //   errormsg->Error(this->pos_,"too little params in function %s",this->func_->Name().c_str());
  //   return type::VoidTy::Instance();
  // } 
  auto it=ty_list.begin();
  auto that=exp_list.begin();
  while(it!=ty_list.end()){
    if(that==exp_list.end()){
      errormsg->Error(this->pos_,"too little params in function %s",this->func_->Name().c_str());
      return type::VoidTy::Instance();
    }
    type::Ty*exp_ty=(*that)->SemAnalyze(venv,tenv,labelcount,errormsg);
    // if(!(*it)->IsSameType(exp_ty)){
      if(typeid(**it)!=typeid(*exp_ty)){
      errormsg->Error((*that)->pos_,"para type mismatch");
      return type::VoidTy::Instance();
    }
    it++;
    that++;
  }
  if(that!=exp_list.end()){
    errormsg->Error(this->pos_,"too many params in function %s",this->func_->Name().c_str());
    return type::VoidTy::Instance();
  }
  return ((env::FunEntry*)func)->result_->ActualTy();
  // for(int i=0;i<ty_list.size();i++){
  //   type::Ty *ty=ty_list[i];
  //   Exp * exp = exp_list[i];
  //   type::Ty *exp_ty=exp->SemAnalyze(venv,tenv,labelcount,errormsg);
  //   if(!ty->IsSameType(exp_ty)){
  //     errormsg->Error(this->pos_,"para type mismatch");
  //     return NULL;
  //   }
  // }
}

type::Ty *OpExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                            int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  type::Ty *left_ty=this->left_->SemAnalyze(venv,tenv,labelcount,errormsg);
  type::Ty *right_ty=this->right_->SemAnalyze(venv,tenv,labelcount,errormsg);
  switch (this->oper_)
  {
  case absyn::PLUS_OP:
  case absyn::MINUS_OP:
  case absyn::TIMES_OP:
  case absyn::DIVIDE_OP:
  if(typeid(*left_ty)!=typeid(type::IntTy)){
    errormsg->Error(this->left_->pos_,"integer required");
    return type::IntTy::Instance();
  }
  if(typeid(*right_ty)!=typeid(type::IntTy)){
    errormsg->Error(this->right_->pos_,"integer required");
    return type::IntTy::Instance();
  }
  break;
  case absyn::EQ_OP:
  case absyn::NEQ_OP:
  case absyn::GE_OP:
  case absyn::GT_OP:
  case absyn::LE_OP:
  case absyn::LT_OP:
  // if(typeid(*left_ty)!=typeid(type::IntTy)&&typeid(*left_ty)!=typeid(type::StringTy)){
  //   errormsg->Error(this->left_->pos_,"integer or string required");
  //   return type::VoidTy::Instance();
  // }
  // if(typeid(*right_ty)!=typeid(type::IntTy)&&typeid(*right_ty)!=typeid(type::StringTy)){
  //   errormsg->Error(this->right_->pos_,"integer or string required");
  //   return type::VoidTy::Instance();
  // }
  if(!right_ty->IsSameType(left_ty)){
    errormsg->Error(this->pos_,"same type required");
    return type::IntTy::Instance();
  }
  default:
    break;
  }
  return type::IntTy::Instance();
}

type::Ty *RecordExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                int labelcount, err::ErrorMsg *errormsg) const {
  type::Ty* standard_ty=tenv->Look(this->typ_);
  if(!standard_ty){
    errormsg->Error(this->pos_,"undefined type %s",this->typ_->Name().c_str());
    return type::VoidTy::Instance();
  }
  standard_ty=standard_ty->ActualTy();
  if(typeid(*standard_ty)!=typeid(type::RecordTy)){
    errormsg->Error(this->pos_,"not a record type");
    return type::VoidTy::Instance();
  }

  std::list<type::Field *> standard_fields = ((type::RecordTy*)standard_ty)->fields_->GetList();
  std::list<absyn::EField *> exp_fields = this->fields_->GetList();
  if(standard_fields.size()!=exp_fields.size()){
    errormsg->Error(this->pos_,"filed num mismatch");
    return type::VoidTy::Instance();
  }
  auto it = standard_fields.begin();
  auto that=exp_fields.begin();
  while (it!=standard_fields.end())
  {
     type::Field* standard_field=*it;
     absyn::EField* exp_field=*that;
     if(standard_field->name_->Name()!=exp_field->name_->Name()){
       errormsg->Error(this->pos_,"field %s doesn't exist",exp_field->name_->Name().c_str());
       return type::VoidTy::Instance();
     }
     if(!standard_field->ty_->IsSameType(exp_field->exp_->SemAnalyze(venv,tenv,labelcount,errormsg))){
       errormsg->Error(this->pos_,"type mismatch");
       return type::VoidTy::Instance();
     }
     it++;
     that++;
  }
  
  // for(int i=0;i<standard_fields.size();i++){
  //    type::Field* standard_field=standard_fields[i];
  //    absyn::EField* exp_field=exp_fields[i];
  //    if(standard_field->name_->Name()!=exp_field->name_->Name()){
  //      errormsg->Error(this->pos_,"field %s doesn't exist",exp_field->name_->Name().c_str());
  //      return NULL;
  //    }
  //    if(!standard_field->ty_->IsSameType(exp_field->exp_->SemAnalyze(venv,tenv,labelcount,errormsg))){
  //      errormsg->Error(this->pos_,"type mismatch");
  //      return NULL;
  //    }
  // }
  return standard_ty;
}

type::Ty *SeqExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {

   std::list<absyn::Exp *> exp_list=this->seq_->GetList();
   type::Ty* ty;
   for(const auto& it:exp_list){
     ty=it->SemAnalyze(venv,tenv,labelcount,errormsg);
   }
   return ty;
}

type::Ty *AssignExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                                int labelcount, err::ErrorMsg *errormsg) const {
   if(typeid(*(this->var_))==typeid(absyn::SimpleVar)){
     env::EnvEntry* entry = venv->Look(((absyn::SimpleVar*)this->var_)->sym_);
     if(entry){
       if(entry->readonly_){
       errormsg->Error(this->pos_,"loop variable can't be assigned");
       return type::VoidTy::Instance();
       }
     }
   }
   absyn::Var* var=this->var_;
   absyn::Exp* exp=this->exp_;
   if(!var->SemAnalyze(venv,tenv,labelcount,errormsg)->IsSameType(exp->SemAnalyze(venv,tenv,labelcount,errormsg))){
     errormsg->Error(this->pos_,"unmatched assign exp");
     return type::VoidTy::Instance();
   }
   return var->SemAnalyze(venv,tenv,labelcount,errormsg);
}

type::Ty *IfExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                            int labelcount, err::ErrorMsg *errormsg) const {
                            
  if(!this->test_->SemAnalyze(venv,tenv,labelcount,errormsg)->IsSameType(type::IntTy::Instance())){
    errormsg->Error(this->pos_,"integer required");
    return type::VoidTy::Instance();
  }

  if(!this->elsee_){
    if(!this->then_->SemAnalyze(venv,tenv,labelcount,errormsg)->IsSameType(type::VoidTy::Instance())){
      errormsg->Error(this->pos_,"if-then exp's body must produce no value");
      return type::VoidTy::Instance();
    }
    return type::VoidTy::Instance();
  }
  
  // if(!this->then_->SemAnalyze(venv,tenv,labelcount,errormsg)->IsSameType(this->elsee_->SemAnalyze(venv,tenv,labelcount,errormsg))){
  //   errormsg->Error(this->pos_,"then exp and else exp type mismatch");
  //   return type::VoidTy::Instance();
  // }
      type::Ty * then_ty=this->then_->SemAnalyze(venv,tenv,labelcount,errormsg)->ActualTy();
  
      type::Ty * else_ty=this->elsee_->SemAnalyze(venv,tenv,labelcount,errormsg)->ActualTy();

      if(typeid(*then_ty)!=typeid(*else_ty)){
          errormsg->Error(this->pos_,"then exp and else exp type mismatch");
          return type::VoidTy::Instance();
      }
     
  return then_ty;
}

type::Ty *WhileExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  if(!this->test_->SemAnalyze(venv,tenv,labelcount,errormsg)->IsSameType(type::IntTy::Instance())){
    errormsg->Error(this->pos_,"integer required");
  }
  bool flag=true;
  inloop.push(flag);                               
  if(!this->body_->SemAnalyze(venv,tenv,labelcount,errormsg)->IsSameType(type::VoidTy::Instance())){
    errormsg->Error(this->body_->pos_,"while body must produce no value");
  }
  inloop.pop();
  return type::VoidTy::Instance();
}

type::Ty *ForExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  type::Ty* lotype=this->lo_->SemAnalyze(venv,tenv,labelcount,errormsg);
  type::Ty* hitype=this->hi_->SemAnalyze(venv,tenv,labelcount,errormsg);
  if(!lotype->IsSameType(type::IntTy::Instance())){
    errormsg->Error(this->pos_,"for exp's range type is not integer");
  }
  if(!hitype->IsSameType(type::IntTy::Instance())){
    errormsg->Error(this->pos_,"for exp's range type is not integer");
  }
  venv->BeginScope();
  tenv->BeginScope();
  //use the readonly to diff loop variable and dec variable
  venv->Enter(this->var_,new env::VarEntry(type::IntTy::Instance(),true));
  //mark loop
  bool flag=true;
  inloop.push(flag);
  this->body_->SemAnalyze(venv,tenv,labelcount,errormsg);
  inloop.pop();
  venv->EndScope();
  tenv->EndScope();
  return type::VoidTy::Instance();
}

type::Ty *BreakExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  if(inloop.empty()){
    errormsg->Error(this->pos_,"break is not inside any loop");
  }                               
  return type::VoidTy::Instance();
}

type::Ty *LetExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
  /* TODO: Put your lab4 code here */
  venv->BeginScope();
  tenv->BeginScope();
  std::list<absyn::Dec *> dec_list = this->decs_->GetList();
  for(const auto& it:dec_list){
    it->SemAnalyze(venv,tenv,labelcount,errormsg);
  }
  type::Ty* result=this->body_->SemAnalyze(venv,tenv,labelcount,errormsg);
  venv->EndScope();
  tenv->EndScope();
  return result;


}

type::Ty *ArrayExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                               int labelcount, err::ErrorMsg *errormsg) const {
  type::Ty* name_ty=tenv->Look(this->typ_);
  if(!name_ty){
    errormsg->Error(this->pos_,"undefined type %s",this->typ_->Name().c_str());
    return type::VoidTy::Instance();
  }
  name_ty=name_ty->ActualTy();
  if(!this->size_->SemAnalyze(venv,tenv,labelcount,errormsg)->IsSameType(type::IntTy::Instance())){
    errormsg->Error(this->size_->pos_,"integer required");
    return type::VoidTy::Instance();
  }
  if(typeid(*name_ty)!=typeid(type::ArrayTy)){
    errormsg->Error(this->pos_,"array type required");
    return type::VoidTy::Instance();
  }
  if(!((type::ArrayTy*)name_ty)->ty_->IsSameType(this->init_->SemAnalyze(venv,tenv,labelcount,errormsg))){
    errormsg->Error(this->pos_,"type mismatch");
    return type::VoidTy::Instance();
  }
  return name_ty;
}

type::Ty *VoidExp::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                              int labelcount, err::ErrorMsg *errormsg) const {
  return type::VoidTy::Instance();
}

void FunctionDec::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv,
                             int labelcount, err::ErrorMsg *errormsg) const {
   //may have bug                            
  //first add all the function header into venv
  std::list<absyn::FunDec*> funlist=this->functions_->GetList();
  for(const auto& it:funlist){
    if(venv->Look(it->name_)){
      errormsg->Error(it->pos_,"two functions have the same name");
    }
    type::Ty *returnty;
    if(it->result_){
      returnty = tenv->Look(it->result_);
      if(!returnty){
        errormsg->Error(it->pos_,"undefined type %s",it->name_->Name().c_str());
      }
    }else{
      returnty=type::VoidTy::Instance();
    }
    //type checking of params
    //type::TyList* tylist=make_tylist(tenv,it->params_,errormsg);
    type::TyList* tylist;
    if(!it->params_) tylist=NULL;
    else{
      std::list<absyn::Field *> fields=it->params_->GetList();
      type::Ty* ty;
      std::list<type::Ty*> result;
      for(const auto& it:fields){
      ty=tenv->Look(it->typ_);
      if(!ty){
       errormsg->Error(it->pos_,"undefined type %s",it->typ_->Name().c_str());
      }
      result.push_front(ty);
      }
      std::reverse_iterator<std::list<type::Ty*>::iterator> reiter(result.end());
      tylist = new type::TyList(*reiter);
      reiter++;
      while (reiter!=result.rend())
      {
        tylist->Append(*reiter);
        reiter++;
      }
      
      // tylist = new type::TyList(result);
    }
    venv->Enter(it->name_,new env::FunEntry(tylist,returnty));
  }
  //deal with the body,should add the params into venv
  for(const auto& it:funlist){
    venv->BeginScope();
    std::list<absyn::Field *> fields=it->params_->GetList();
    for(const auto& that:fields){
      type::Ty* fieldty=tenv->Look(that->typ_);
      venv->Enter(that->name_,new env::VarEntry(fieldty));
    }
    //check if the body return type same with the funentry->result
    type::Ty* body_ty=it->body_->SemAnalyze(venv,tenv,labelcount,errormsg);
    type::Ty* result_ty=((env::FunEntry*)venv->Look(it->name_))->result_;
    if(!body_ty->IsSameType(result_ty)){
      if(result_ty->IsSameType(type::VoidTy::Instance())){
        errormsg->Error(it->pos_,"procedure returns value");
      }else{
        errormsg->Error(it->pos_,"type mismatch");
      }
    }
    venv->EndScope();
  }

}

void VarDec::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                        err::ErrorMsg *errormsg) const {
  if(this->typ_){
    //have tell type_id
    type::Ty *ideal_ty=tenv->Look(this->typ_);
    type::Ty *real_ty=this->init_->SemAnalyze(venv,tenv,labelcount,errormsg);
    if(!ideal_ty){
      errormsg->Error(this->pos_,"undefined type %s",this->typ_->Name().c_str());
    }else if(!ideal_ty->IsSameType(real_ty)){
      errormsg->Error(this->pos_,"type mismatch");
    }
    venv->Enter(this->var_,new env::VarEntry(real_ty));
  }else{
    // if(this->var_->Name()=="a"){
    //   std::cout<<"debug!"<<std::endl;
    //   Globalflag=true;
    // }
    type::Ty *real_ty=this->init_->SemAnalyze(venv,tenv,labelcount,errormsg);
    if(typeid(*real_ty)==typeid(type::NilTy)){
      errormsg->Error(this->pos_,"init should not be nil without type specified");
    }
    venv->Enter(this->var_,new env::VarEntry(real_ty));
  }
}

void TypeDec::SemAnalyze(env::VEnvPtr venv, env::TEnvPtr tenv, int labelcount,
                         err::ErrorMsg *errormsg) const {
  //handle like fundec
  //first add all to tenv
   std::list<absyn::NameAndTy *> typelist=this->types_->GetList();
   for(const auto& it:typelist){
     
     if(tenv->Look(it->name_)){
       errormsg->Error(this->pos_,"two types have the same name");
     }
     tenv->Enter(it->name_,new type::NameTy(it->name_,NULL));
   }
   //one by one check the ty(it->ty_) part
  for(const auto& it:typelist){
    //check and fill the tenv
    type::NameTy* name_ty=(type::NameTy*)tenv->Look(it->name_);
    name_ty->ty_=it->ty_->SemAnalyze(tenv,errormsg);
  }
  for(const auto& it:typelist){
     type::NameTy* name_ty=(type::NameTy*)tenv->Look(it->name_);
  }
  //check if there is circle
  bool has_circle=false;
  for(const auto& it:typelist){
    type::Ty* ty=tenv->Look(it->name_);
    if(typeid(*ty)==typeid(type::NameTy)){
      ty=((type::NameTy*)ty)->ty_;
    }
    std::vector<std::string> tyname;
    while(typeid(*ty)==typeid(type::NameTy)){
        for(int i=0;i<tyname.size();i++){
          if(tyname[i]==((type::NameTy*)ty)->sym_->Name())
          {
          has_circle=true;
          errormsg->Error(this->pos_,"illegal type cycle");
          break;
          }
        }
        if(has_circle) break;
        if(((type::NameTy*)ty)->sym_->Name()==it->name_->Name()){
          has_circle=true;
          errormsg->Error(this->pos_,"illegal type cycle");
          break;
        }
        tyname.push_back(((type::NameTy*)ty)->sym_->Name());
        ty=((type::NameTy*)ty)->ty_;
    }
    //type::Ty* name_ty=tenv->Look(it->name_);
    //*name_ty=*ty;
    type::NameTy* name_ty=(type::NameTy*)tenv->Look(it->name_);
    name_ty->ty_=ty;
    if(has_circle) break;
  }
}

type::Ty *NameTy::SemAnalyze(env::TEnvPtr tenv, err::ErrorMsg *errormsg) const {
  type::Ty* ty = tenv->Look(this->name_);
  if(!ty){
    errormsg->Error(this->pos_,"undefined type %s",this->name_->Name().c_str());
    return type::VoidTy::Instance();
  }
  return new type::NameTy(this->name_,ty);
}

type::Ty *RecordTy::SemAnalyze(env::TEnvPtr tenv,
                               err::ErrorMsg *errormsg) const {
  if(!this->record_){
    return new type::RecordTy(NULL);
  }                               
  std::list<absyn::Field *> field_list=this->record_->GetList();
  std::list<type::Field*> t_fields;
  for(const auto& it:field_list){
    type::Ty *ty=tenv->Look(it->typ_);
    if(!ty){
      errormsg->Error(it->pos_,"undefined type %s",it->typ_->Name().c_str());
      return type::VoidTy::Instance();
    }
    t_fields.push_back(new type::Field(it->name_,ty));
  }
  bool flag=true;
  type::FieldList* result;
  for(const auto& it:t_fields){
    if(flag){
      flag=false;
      result=new type::FieldList(it);
    }
    else result->Append(it);
  } 
  return new type::RecordTy(result);                        
}

type::Ty *ArrayTy::SemAnalyze(env::TEnvPtr tenv,
                              err::ErrorMsg *errormsg) const {
  type::Ty* ty=tenv->Look(this->array_);
  if(!ty){
    errormsg->Error(this->pos_,"undefined type %s",this->array_->Name().c_str());
    return type::VoidTy::Instance();
  }
  return new type::ArrayTy(ty);
}

} // namespace absyn

namespace sem {

void ProgSem::SemAnalyze() {
  FillBaseVEnv();
  FillBaseTEnv();
  absyn_tree_->SemAnalyze(venv_.get(), tenv_.get(), errormsg_.get());
}

} // namespace tr
