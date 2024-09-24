
#ifndef GCBETTERINVOKER_H_

#define GCBETTERINVOKER_H_

#include <iostream>
#include <thread>
#include <chrono>
#include "GCInvoker.h"
#include "GCMemory.h"
#include "GarbageCollector.h"

namespace gc{

    class GCBetterInvoker : public GCInvoker{
        public:
        GCBetterInvoker(GCMemory * memory) : GCInvoker(memory){}
        void run() override {
            gc_print.print("(Invoker) the invoker is running!");
            /* Invoker sees the amount of new memory allocated during the Invokers sleep */
            size_t lastContextsSizeOfAllEver = memory->size_of_all_contexts_ever_used;
            size_t changesOfContextsSize = lastContextsSizeOfAllEver;

            while (!is_stop_invoker.load()) {

                memory->updateMemory();

                size_t actualSize = memory->size_of_all_contexts_ever_used;
                changesOfContextsSize = (changesOfContextsSize + (actualSize - lastContextsSizeOfAllEver)) / 2;
                lastContextsSizeOfAllEver = actualSize;
                
                size_t change = changesOfContextsSize >= 1 ? changesOfContextsSize : 1; 
                gc_print.print("(Invoker) sleep for ", 10000 / change, " ... changes in number of context ", changesOfContextsSize);
                std::this_thread::sleep_for(std::chrono::milliseconds(10000 / change));
            
            }
            is_thread_inactive.store(true);
            gc_print.print("(Invoker) the Invoker is ending");
        }

    };
}

#endif