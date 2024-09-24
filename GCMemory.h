#ifndef GCMEMORY_H_

#define GCMEMORY_H_

#include <mutex>

namespace gc{
    
    class GCMemory{
        std::mutex _Object_mutex;
        protected:
        std::recursive_mutex memory_mutex;
        public:
        size_t current_object_count = 0; 
        size_t object_released_count = 0;

        size_t size_of_all_contexts_ever_used = 0; 

        virtual void updateMemory() = 0;
        virtual void clearAll() = 0;
        virtual void newContextCreated(GCContext * context, size_t size) = 0;
        virtual size_t getCount() = 0;
        virtual ~GCMemory(){};


        void _Object_created(){
            _Object_mutex.lock();
            ++current_object_count;
            _Object_mutex.unlock();
        }

        void _Object_delted(){
            _Object_mutex.lock();
            --current_object_count;
            ++object_released_count;
            _Object_mutex.unlock();
        }

        void lockMemory(){
            memory_mutex.lock();
        }
        void unlockMemory(){
            memory_mutex.unlock();
        }

    };
}



#endif