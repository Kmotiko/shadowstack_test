#include "llvm/PassManager.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/FileSystem.h"
using namespace llvm;

int main(int argc, char **argv) {
  //std::string file_name = "test.ll";
  std::string file_name = argv[argc-1];
  std::string error;
  llvm::raw_fd_ostream raw_stream(file_name.c_str(), error,
                                  llvm::sys::fs::OpenFlags::F_RW);
  llvm::IRBuilder<> *builder = new llvm::IRBuilder<>(llvm::getGlobalContext());

  // Create Module
  llvm::Module *module = new llvm::Module("test", llvm::getGlobalContext());

  // Add Allocator PorototypeDeclaration
  std::vector<llvm::Type *> arg_types;
  llvm::FunctionType *func_type = llvm::FunctionType::get(
      llvm::Type::getInt32PtrTy(llvm::getGlobalContext()), arg_types, false);
  llvm::Function *func = llvm::Function::Create(
      func_type, llvm::Function::ExternalLinkage, "my_alloc", module);

  // add intrinsic @gcroot
  arg_types.clear();
  arg_types.push_back(llvm::PointerType::get(
      llvm::Type::getInt8PtrTy(llvm::getGlobalContext()), 0));
  arg_types.push_back(llvm::Type::getInt8PtrTy(llvm::getGlobalContext()));
  func_type = llvm::FunctionType::get(
      llvm::Type::getVoidTy(llvm::getGlobalContext()), arg_types, false);
  func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage,
                                "llvm.gcroot", module);

  // add main Function
  arg_types.clear();
  func_type = llvm::FunctionType::get(
      llvm::Type::getInt32Ty(llvm::getGlobalContext()), arg_types, false);
  func = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage,
                                "main", module);
  func->setGC("shadow-stack");

  // Create BasicBlock(entry)
  llvm::BasicBlock *bb =
      llvm::BasicBlock::Create(llvm::getGlobalContext(), "entry", func);
  builder->SetInsertPoint(bb);
  llvm::Value *alloc_v = builder->CreateAlloca(llvm::PointerType::get(
      llvm::Type::getInt32Ty(llvm::getGlobalContext()), 0));
  llvm::Value *v = builder->CreatePointerCast(
      alloc_v, llvm::PointerType::get(
                   llvm::PointerType::get(
                       llvm::Type::getInt8Ty(llvm::getGlobalContext()), 0),
                   0));
  std::vector<llvm::Value *> arg_vec;
  arg_vec.push_back(v);
  arg_vec.push_back(llvm::Constant::getNullValue(llvm::PointerType::get(
      llvm::Type::getInt8Ty(llvm::getGlobalContext()), 0)));
  builder->CreateCall(module->getFunction("llvm.gcroot"), arg_vec);

  // Add For Stmt Create BasicBlock(loop cond)
  llvm::BasicBlock *cond_bb =
      llvm::BasicBlock::Create(llvm::getGlobalContext(), "loop.cond", func);
  builder->CreateBr(cond_bb);
  builder->SetInsertPoint(cond_bb);
  llvm::PHINode *phi = llvm::PHINode::Create(
      llvm::Type::getInt32Ty(llvm::getGlobalContext()), 2, "condv", cond_bb );

  // Create BasicBlock(loop body)
  llvm::BasicBlock *body_bb =
      llvm::BasicBlock::Create(llvm::getGlobalContext(), "loop.body", func);
  builder->SetInsertPoint(body_bb);
  arg_vec.clear();
  // create call
  llvm::Value *ptrv = builder->CreateCall(module->getFunction("my_alloc"), arg_vec);
  builder->CreateStore(ptrv, alloc_v);
  builder->CreateStore(phi, ptrv);

  // Create BasicBlock(loop latch)
  llvm::BasicBlock *latch_bb =
      llvm::BasicBlock::Create(llvm::getGlobalContext(), "loop.latch", func);
  builder->CreateBr(latch_bb);
  builder->SetInsertPoint(latch_bb);
  // increment cond value
  llvm::Value *condv = builder->CreateAdd(
      phi, llvm::ConstantInt::get(
               llvm::Type::getInt32Ty(llvm::getGlobalContext()), 1));
  builder->CreateBr(cond_bb);

  // Create BasicBlock(exit)
  llvm::BasicBlock *exit_bb =
      llvm::BasicBlock::Create(llvm::getGlobalContext(), "exit", func);
  builder->SetInsertPoint(exit_bb);
  builder->CreateRet(llvm::ConstantInt::get(
      llvm::Type::getInt32Ty(llvm::getGlobalContext()), 0));

  // add loop branch instruction
  builder->SetInsertPoint(cond_bb);
  phi->addIncoming(llvm::ConstantInt::get(
                       llvm::Type::getInt32Ty(llvm::getGlobalContext()), 0),
                   bb);
  phi->addIncoming(condv, latch_bb);
  condv = builder->CreateICmpSLT(
      phi, llvm::ConstantInt::get(
               llvm::Type::getInt32Ty(llvm::getGlobalContext()), 11));
  builder->CreateCondBr(condv, body_bb, exit_bb);

  // Dump
  llvm::PassManager pm;
  pm.add(createPrintModulePass(raw_stream));
  pm.run(*module);
  raw_stream.close();

  delete builder;
  return 0;
}
