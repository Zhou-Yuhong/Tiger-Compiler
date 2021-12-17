#ifndef TIGER_COMPILER_COLOR_H
#define TIGER_COMPILER_COLOR_H

#include "tiger/codegen/assem.h"
#include "tiger/frame/temp.h"
#include "tiger/liveness/liveness.h"
#include "tiger/util/graph.h"
#include <stack>
namespace col {
#define K 15  

struct Result {
  Result() : coloring(nullptr), spills(nullptr) {}
  Result(temp::Map *coloring, live::INodeListPtr spills)
      : coloring(coloring), spills(spills) {}
  temp::Map *coloring;
  live::INodeListPtr spills;
};

class Color {
  /* TODO: Put your lab6 code here */
public:
  Color(frame::Frame* frame_,assem::InstrList* list){
    this->frame_=frame_;
    this->InsList_=list;
  }
  void run();
  void initLiveGraph();
  void init();
  void Simplify();
  void DecrementDegree(live::INode* node);
  live::MoveList* NodeMoves(live::INode* node);
  bool MoveRelated(live::INode* node);
  void EnableMoves(live::INodeList* nodelist);
  void Coalesce();
  void Freeze();
  void SelectSpill();
  void FreezeMoves(live::INode* u);
  live::INode* GetAlias(live::INode* node);
  void AddWorkList(live::INode* node);
  bool OK(live::INode *t,live::INode *r);
  bool Conservative(live::INodeList* nodelist);
  void Combine(live::INode *u,live::INode *v);
  void AddEdge(live::INode *u,live::INode *v);
  assem::InstrList* getInstrList();
  void RewriteProgram();
  bool isEmpty(live::INodeListPtr ptr);
  bool isEmpty(live::MoveList* movelist);
  void AssignColors();
  temp::Map* AssignRegisters();
private:
  frame::Frame* frame_;
  assem::InstrList* InsList_;  
  live::INodeListPtr simplifyWorklist;
  live::INodeListPtr freezeWorklist;
  live::INodeListPtr spillWorklist;
  live::INodeListPtr spilledNodes;
  live::INodeListPtr coalescedNodes;
  live::INodeListPtr coloredNodes;
  std::stack<live::INode*> selectStack;
  live::MoveList *coalescedMoves;
  live::MoveList *constrainedMoves;
  live::MoveList *frozenMoves;
  live::MoveList *worklistMoves;
  live::MoveList *activeMoves;
  live::LiveGraph livegraph;
  graph::Table<temp::Temp,int>* degreeTable;
  graph::Table<temp::Temp,live::MoveList>* moveListTable;
  graph::Table<temp::Temp,live::INode>* aliasTable;
  graph::Table<temp::Temp,std::string>* colorTable; 
  temp::TempList *NotSpill=nullptr;
  
};
} // namespace col

#endif // TIGER_COMPILER_COLOR_H
