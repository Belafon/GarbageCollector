#ifndef OBJECT_H_

#define OBJECT_H_


namespace gc{

    template<typename T, typename ... Args>
    requires std::is_base_of_v<GCContext, T>
    std::pair<T *, GCContext *> GCContext::new_obj(Args&& ... args){
        
        T * obj;
        try {
            obj = new T ((std::forward <Args> (args))...);
            gctor.newContextCreated(obj, sizeof(obj));
            gc_print.print("(Context) new GCContext id=", obj->get_gc_id()); 

        } catch(const std::exception& e) {
            std::cerr, e.what(), '\n';
        }

        return std::make_pair(obj, this);
    }
    
    template <typename T>
    //requires std::is_base_of_v<GCContext, T> // bohužel omezení ničí možnost vzájemné odkazování dvou Objektů na sebe
    class GCObject {
        T * context = nullptr;
        GCContext * home = nullptr;
        public:

        struct invalid_try_to_assign_gcobject : public std::exception {
            const char * what () const throw () {
                return "There was an attemption to change an object of GCObject from different place instead from the home GCObject.";
            }
        };

        explicit GCObject(GCContext * home) : home(home) {
            gctor.object_created();
        }
        
        explicit GCObject(std::pair<T *, GCContext *> && obj) : context(obj.first), home(obj.second) {
            gctor.object_created();
            home->addReference(obj.first);
        }

        GCObject(const GCObject & o) = delete;
        GCObject(GCObject && o) = delete;

        // we don't want to have another copy/move constructors, or assignments 
        void operator=(std::pair<T *, GCContext *> && obj){
            gc_print.print("(Object) ", "move assignment of GCObject called");

            if(obj.second->get_gc_id() != home->get_gc_id())
                throw invalid_try_to_assign_gcobject();
            
            // odstraníme a pak přidáme odkaz, mezi tím, nesmí dojít k čištění paměti
            gctor.lockMemory();

            if(context != nullptr) removeReference();
            home->addReference(obj.first);
            
            this->context = obj.first;
            gctor.unlockMemory();
        }

        const T & view(){
            return *this->context;
        }
        
        T * get(){
            return this->context;
        }

        void removeReference(){
            if(home != nullptr){
                home->gc_remove_reference(context);
                context = nullptr;
            }
        }

        ~GCObject(){
            if(context != nullptr && home != nullptr)gc_print.print("(Object) Object destructor called, context home id=", home->get_gc_id(), ", context id=", context->get_gc_id()); 
            else gc_print.print("(Object) Object destructor called, null context, or home");
            if(home != nullptr)home->gc_remove_reference(context) ;
            gctor.object_deleted();
        }
    };
}

#endif