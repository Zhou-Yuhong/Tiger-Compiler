#ifndef TIGER_TRANSLATE_TRANSLATE_H_
#define TIGER_TRANSLATE_TRANSLATE_H_

#include <list>
#include <memory>

#include "tiger/absyn/absyn.h"
#include "tiger/env/env.h"
#include "tiger/errormsg/errormsg.h"
#include "tiger/frame/frame.h"
#include "tiger/semant/types.h"
#include "tiger/frame/x64frame.h"
namespace tr {
class Exp;
class ExpAndTy;
class Level;
class Access {
public:
  Level *level_;
  frame::Access *access_;

  Access(Level *level, frame::Access *access)
      : level_(level), access_(access) {}
  static Access *AllocLocal(Level *level, bool escape);
};

class Level {
public:
  frame::Frame *frame_;
  Level *parent_;
  static tr::Level* newLevel(tr::Level* parent_,temp::Label* name,std::list<bool> formals,frame::RegManager* regmanager){
      formals.push_front(true);
      //the first argument is static link,strange?
      frame::Frame* frame = frame::X64Frame::newFrame(name,formals,regmanager);
      tr::Level *level = new tr::Level(frame,parent_);
      return level;
  }
  std::list<Access*> getFormalAccessList(){
    std::list<Access*> result;
    std::list<frame::Access*> fAccessList=this->frame_->getFormals();
    for(auto it:fAccessList){
      tr::Access* item=new tr::Access(this,it);
      result.push_back(item);
    }
    return result;
  }
  Level(frame::Frame *frame_,Level *parent_){
    this->frame_=frame_;
    this->parent_=parent_;
  }
  /* TODO: Put your lab5 code here */

};

class ProgTr {
public:
  /* TODO: Put your lab5 code here */ 
  /**
   * Translate IR tree
   */
  void Translate();

  //temp 
  ProgTr( std::unique_ptr<absyn::AbsynTree> absyn_tree,  std::unique_ptr<err::ErrorMsg> errmsg){
      this->absyn_tree_=std::move(absyn_tree);
      this->errormsg_=std::move(errmsg);
  }

  /**
   * Transfer the ownership of errormsg to outer scope
   * @return unique pointer to errormsg
   */
  std::unique_ptr<err::ErrorMsg> TransferErrormsg() {
    return std::move(errormsg_);
  }



private:
  std::unique_ptr<absyn::AbsynTree> absyn_tree_;
  std::unique_ptr<err::ErrorMsg> errormsg_;
  std::unique_ptr<Level> main_level_;
  std::unique_ptr<env::TEnv> tenv_;
  std::unique_ptr<env::VEnv> venv_;

  // Fill base symbol for var env and type env
  void FillBaseVEnv();
  void FillBaseTEnv();
};
tree::MemExp* formMemInstruction(tree::Exp* base,int offset);
Exp* emptyExp();

Exp *simpleVar(Access* access,Level *level);
tr::Exp* ifExp(tr::Exp *test,tr::Exp* then,tr::Exp* elsee);
//helpful function in CallExp's translate
tr::Exp *funCall(temp::Label *label,std::list<tr::Exp *> args,tr::Level* caller,tr::Level* callee);
tr::Exp* translateSeq(tr::Exp* left,tr::Exp* right);
} // namespace tr

#endif
