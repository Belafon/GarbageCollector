
#ifndef GCDUMBINVOKER_H_

#define GCDUMBINVOKER_H_

#include <iostream>
#include <thread>
#include <chrono>
#include "GCInvoker.h"
#include "GCMemory.h"

namespace gc{

    class GCDumbInvoker : public GCInvoker{
        public:
        GCDumbInvoker() : GCInvoker(){}
        GCDumbInvoker(GCMemory * memory) : GCInvoker(memory){

        }

        void run() override {
            gc_print.print("(Invoker) the invoker is running!");
            
            while (!is_stop_invoker.load()) {
                memory->updateMemory();
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            is_thread_inactive.store(true);
            gc_print.print("(Invoker) the Invoker is ending");
        }

    };
}

#endif