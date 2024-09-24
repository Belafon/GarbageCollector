#ifndef GCMEMORYBASIC_H_

#define GCMEMORYBASIC_H_

namespace gc{

    class GCMemoryBasic : public GCMemory {
        /**
         * @brief Stores pointers to all GCContext objects,
         *      could be decresed only from inside
         * 
         */
        std::unordered_multiset<GCContext *> allContexts;
        void removeContext(GCContext * context){
            gc_print.print("(GCMemory) GCContext id=", context->get_gc_id()," memory released");
            delete context;
        }
        public:
        virtual size_t getCount() override{
            return allContexts.size();
        }
        
        virtual void updateMemory() override {
            gc_print.print("(Invoker) lets go update of memory...");
            memory_mutex.lock();
            
            // průchod grafu referencí do šířky
            std::queue<GCContext *> q;
            for (auto &&i : context_static.get_references())
                q.push(i);
            
            while (!q.empty()){
                GCContext * context = q.front();
                q.pop();
                for (auto &&i : context->get_references())
                    q.push(i);
                context->visited = true;
            }


            std::unordered_multiset<GCContext *> allContextsUpdated;
            for(auto i = allContexts.begin(); i != allContexts.end(); ++i)
                if((*i)->visited)
                    allContextsUpdated.emplace(*i);
                else removeContext(*i); // odstranění nedosažitelných GCContextů v paměti
            
            allContexts = std::move(allContextsUpdated);

            for(auto&& i : allContexts)
                i->visited = false;

            memory_mutex.unlock();
            gc_print.print("(Invoker) memory updated");
        }
        virtual void newContextCreated(GCContext * context, size_t size) override {
            memory_mutex.lock();
            allContexts.emplace(context);
            memory_mutex.unlock();
        }

        virtual void clearAll() override {
            memory_mutex.lock();
            for(auto&& i : allContexts)
                removeContext(i);
            allContexts.clear();
            memory_mutex.unlock();
        }

    };
}


#endif