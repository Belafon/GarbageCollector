#ifndef CONTEXT_H_

#define CONTEXT_H_


namespace gc{

    class GCContext{
        std::unordered_multiset<GCContext *> gc_references;
        int gc_id = 0;
        public:
        /* if somebody sets this to true, then the next update of memory doesn't remove this gccontext,
            but some gccontexts of his references could be!!! */
        bool visited = false;

        GCContext(){
            static int next_id = 0;
            
            gc_id = ++next_id;
        } 
        
        std::unordered_multiset<GCContext *> & get_references(){
            return gc_references;
        }
    

        template<typename T>
        requires std::is_base_of_v<GCContext, T>
        std::pair<T *, GCContext *> make_ptr(GCObject<T> & obj){
            if(obj.get() != nullptr)
                gc_print.print("(Context) GCObject created, reffers to GCContext id=", obj.get()->get_gc_id(), ", located in id=", get_gc_id());
            return std::make_pair(obj.get(), this);
        }

        template<typename T>
        requires std::is_base_of_v<GCContext, T>
        void addReference(T * obj){
            gc_references.emplace(obj);
        }

        template<typename T, typename ... Args>
        requires std::is_base_of_v<GCContext, T>
        std::pair<T *, GCContext *> new_obj(Args&& ... args);



        void print(){
            gc_print.print();
            gc_print.print("(Context) References of Context with id ", gc_id,": ");
            for (auto&& i : gc_references)
                gc_print.print("    Context ref ", i->get_gc_id());  
        }

        int get_gc_id(){
            return gc_id;
        }

        void gc_remove_reference(GCContext * context){
            auto itr = gc_references.find(context);
            if(itr!=gc_references.end())
                gc_references.erase(itr);
        }
        virtual ~GCContext() = default;
    };




    // If the gc clears the lost memory, the searching through graph starts here 
    GCContext context_static = GCContext();

    template<typename T>
    requires std::is_base_of_v<GCContext, T>
    inline std::pair<T *, GCContext *> make_ptr(GCObject<T> & obj){
        return context_static.make_ptr<T>(obj);
    }

    template<typename T, typename ... Args>
    requires std::is_base_of_v<GCContext, T>
    inline std::pair<T *, GCContext *> new_obj(Args&& ... args){
        return context_static.new_obj<T>((std::forward <Args> (args))...);
    }
}

#endif