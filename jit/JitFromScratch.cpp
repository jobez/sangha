// going through intro llvm jit tutorial https://github.com/weliveindetail/JitFromScratch/blob/e7b2183cd5e92507a2f6c9fe3603569e1132b066/JitFromScratch.cpp

#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/ManagedStatic.h>
#include <llvm/Support/PrettyStackTrace.h>
#include <llvm/Support/Signals.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>

#include "SimpleOrcJit.h"

template <typename T, size_t sizeOfArray>
constexpr unsigned arrayElements (T (&)[sizeOfArray]) {

  return sizeOfArray;

}

int *customIntAllocator(unsigned items) {
  static int memory[100];
  static unsigned allocIdx = 0;

  if (allocIdx + items > arrayElements(memory))
    exit(-1);

  int *block = memory + allocIdx;

  allocIdx += items;

  return block;
}

// this function will be replaced by a runtime compiled version
template <size_t sizeOfArray>
int *integerDistances(const int (&x)[sizeOfArray], int *y) {

  unsigned items = arrayElements(x);
  int *results = customIntAllocator(items);

  for (int i = 0; i < items; i++) {

    results[i] = abs(x[i] - y[i]);

  }

  return results;
}


int main(int argc, char **argv) {
  using namespace llvm;

  sys::PrintStackTraceOnErrorSignal(argv[0]);
  PrettyStackTraceProgram X(argc, argv);

  atexit(llvm_shutdown);

  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();

  int x[]{0, 1, 2};
  int y[]{3, 1, -1};

  auto targetMachine = EngineBuilder().selectTarget();
  auto jit = std::make_unique<SimpleOrcJit>(*targetMachine);

  LLVMContext context;
  auto module = std::make_unique<Module>("Sangha", context);
  module->setDataLayout(targetMachine->createDataLayout());

  jit->submitModule(std::move(module));
  int *z = integerDistances(x, y);

  outs() << "Integer Distance: ";

  outs() << z[0] << ", " << z[1] << ", " << z[2] << "\n\n";
  outs().flush();


  return 0;




}
