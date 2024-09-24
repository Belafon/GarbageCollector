#ifndef GARBAGECOLLECTOR_H_

#define GARBAGECOLLECTOR_H_

#include <iostream>
#include <memory>
#include <unordered_set>
#include <vector>
#include <type_traits>
#include <mutex>
#include <optional>
#include <exception>
#include <thread>
#include <queue>
#include <atomic>
#include <utility>

namespace gc{
        
    class GCContext;

    /*
        this cannot be with the concept, because there would be a problem to self reference two GCContexts with two GCObjects to each other
    */    
    template <typename T>
    //requires std::is_base_of_v<GCContext, T>
    class GCObject;

    struct Print{
        bool print_messages = false;
        std::mutex print_mutex;
        template<typename ... Messages>
        void print(Messages&& ... messages){
            if(!print_messages) return;
            
            print_mutex.lock();
            (std::cout << ... << messages) << std::endl;
            print_mutex.unlock();
        }
        void print(){
            if(!print_messages) return;

            print_mutex.lock();
            std::cout << std::endl;
            print_mutex.unlock();
        }
    };

    Print gc_print;
}

#include "GCMemory.h"
#include "GCInvoker.h"

namespace gc{

    template<typename Memory>
    requires std::is_base_of_v<GCMemory, Memory> 
    class GarbageCollector {
        Memory memory;
        using Invoker = std::optional<std::unique_ptr<GCInvoker>>;
        Invoker invoker;

        struct invalid_try_stop_gc_invoker : public std::exception {
            const char * what () const throw () {
                return "There was an attemption to stop the invoker, which wasn't running.";
            }
        };
        struct invalid_try_start_gc_invoker : public std::exception {
            const char * what () const throw () {
                return "There was an attemption to start another invoker even the first was running.";
            }
        };

        std::mutex edit_invoker_mtx;
        public:

        void newContextCreated(GCContext * context, size_t size){
            memory.newContextCreated(context, size);
        }

        /* removes unreachable memory from object GCContext context_static */
        void updateMemory(){
            memory.updateMemory();
        }

        template<typename I>
        requires std::is_base_of_v<GCInvoker, I>
        void startNewinvoker(){
            edit_invoker_mtx.lock();
            if(invoker.has_value()) {
                edit_invoker_mtx.unlock();
                throw invalid_try_start_gc_invoker(); 
            }
            invoker = std::make_unique<I>(&memory);

            invoker->get()->start();
            edit_invoker_mtx.unlock();
        }

        void restartInvoker(){
            edit_invoker_mtx.lock();
            if(!invoker.has_value()) {
                edit_invoker_mtx.unlock();
                gc_print.print("There was an attemption to restart the invoker, which wasn't created.");
                throw invalid_try_start_gc_invoker();
            }
            if(invoker->get()->closeInvoker.load()){
                edit_invoker_mtx.unlock();
                gc_print.print("There was an attemption to restart the invoker, which has been closed.");
                throw invalid_try_start_gc_invoker();
            }
            invoker->get()->start();
            edit_invoker_mtx.unlock();
        }

        void stopInvoker(){
            edit_invoker_mtx.lock();
            if(!invoker.has_value()) {
                gc_print.print("There was an attemption to stop the invoker, which wasn't created.");
                throw invalid_try_stop_gc_invoker();
            }
            invoker->get()->stopInvoker();
            edit_invoker_mtx.unlock();
        }
        void closeInvoker(){
            edit_invoker_mtx.lock();
            if(!invoker.has_value()) {
                edit_invoker_mtx.unlock();
                gc_print.print("There was an attemption to close the invoker, which wasn't created.");
                throw invalid_try_stop_gc_invoker();
            }
            invoker->get()->closeInvoker.store(true);        
            invoker->get()->stopInvoker();
            edit_invoker_mtx.unlock();
        }

        // informs gc, how many GCObjects there are
        void object_created(){
            memory._Object_created();
        }
        void object_deleted(){
            memory._Object_delted();
        }

        void lockMemory(){
            memory.lockMemory();
        }
        void unlockMemory(){
            memory.unlockMemory();
        }
        ~GarbageCollector(){
            memory.clearAll();
        }

        // PRINT INFO
        size_t get_Object_released_count(){
            return memory.object_released_count; 
        }
        size_t get_current_object_count(){
            return memory.current_object_count; 
        }
    };
}



#include "Context.h"

#include "GCMemoryBasic.h"
#include "GCDumbInvoker.h"
#include "GCBetterInvoker.h"


namespace gc {
    GarbageCollector<GCMemoryBasic> gctor = GarbageCollector<GCMemoryBasic>();
}

#include "Object.h"

#endif