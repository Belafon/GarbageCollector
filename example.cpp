#include "GarbageCollector.h"

using namespace std;
#include "GarbageCollector.h"
using namespace std;
using namespace gc;

struct A : GCContext
{
    int i;
    A(int i) : i(i) {
    }
};

void example_1() {    
    {
        /*  GCObject is an object through which you can look at memory stored in 
            the garbage collector.
            The new_obj method creates a new object A with parameter 4 and stores it in
            the garbage collector (wraps the object in a helper class 'Context').
            The correct new_obj() method must be called from the same object where
            the corresponding GCObject was created. Otherwise, the program will crash and print
            an error message. (This is important for sufficient control over memory
            held by the garbage collector) */
        GCObject<A> o {&context_static}; 
        o = new_obj<A>(4);
        o.get()->i = 9; // working with the value in the garbage collector
        o.get()->print(); // ability to print info about references of the respective object to the console
    }
    /* A breadth-first traversal of references of all objects in the garbage collector is performed, 
       those that are not reachable from the special Context object context_static are discarded */
    gctor.updateMemory(); 
}

void example_2() {
    /* the invoker will start (invoker handles the calls to updateMemory(), which removes lost memory) */
    // invoker uses a new thread, you can program your own invoker using the GCInvoker interface
    // GCDumbInvoker only calls gctor.updateMemory once every certain period
    gctor.startNewinvoker<GCDumbInvoker>();
    // GCBetterInvoker calls the garbage collector more frequently if there has been
    // a larger increase in memory usage since the last updateMemory
    //gctor.startNewinvoker<GCBetterInvoker>(); // only 1 invoker can run

    {
        // creating a new object must call the constructor with at least information about the creation location
        GCObject<A> o {new_obj<A, int>(4)};
        o.get()->i = 9;
        o.get()->print();
    }
    // the following methods affect the invoker, you can try them
    //gctor.stopInvoker();
    //gctor.restartInvoker();
    //gctor.closeInvoker();
}

void example_3(){
    {
        gc_print.print("1 ---------- ");
        
        // A way to create an object without a reference, 
        // when accessing data, it returns nullptr
        // It needs at least a reference to the creation location
        GCObject o0 {&context_static};
        
        /* 
            If it is outside a class that would be a descendant of GCObject,
            then a static variable is automatically called, which sets 
            the position of the GCObject, as in the context_static object.
        */
        o0 = new_obj(5);
        
        // this prints all references of Context context_static
        context_static.print();
        
        GCObject o1 {new_obj(4)};
        context_static.print();
        
        GCObject o2 {make_ptr(o1)};
        context_static.print();
        
        gc_print.print("2 ---------- ");
        
        o0 = make_ptr(o1);
        context_static.print();
        
        o0 = make_ptr(o2);
        context_static.print();
        
        gc_print.print("3 ---------- ");
        
        // All memory is removed at the end (in the GarbageCollector destructor),
        // this only removes objects with lost references
        // It should remove 1 context object
        gctor.updateMemory();
        
        gc_print.print("4 ---------- ");
        
        // view() returns a const reference for inspection
        gc_print.print("view across o1, i = ", o1.view().i);
        o1.get()->print();
        context_static.print();
        
        gc_print.print("5 ---------- ");
        
        o0.removeReference();
    }
    gc_print.print("6 ---------- ");
    
    gctor.updateMemory();
}

struct D;
struct C : GCContext{
    // When calling the constructor, it is necessary to specify at least the location
    // (the location must be a descendant of GCContext, otherwise it should be
    // a member of the static object context_static)
    GCObject o2 {this};
};


struct D;

struct C : GCContext{
    // při volání kontruktoru je nutné zadat alespoň místo výskytu 
    // (místo výskytu musí být potomkem GCContextu, jinak má být 
    // příslušníkem statického objektu context_static)
    GCObject<D> o2 {this};  
};

struct D : GCContext{
    GCObject<C> o2 {this};
    D(GCObject<C> & o1) {
        o2 = make_ptr(o1);
    }
};

/* this will create two objects C and D referencing to each other
        then, when this block ends, the pointers to these objects are lost,
        and when I call garbage collector, these objects are removed 
        
        there is created circle from references */
void example_4(){
    {
        gc_print.print("1 ---------- "); 
        GCObject<C> o2 {new_obj<C>()}; // Object -> C, obsahující Object -> D
        gc_print.print("2 ---------- ", o2.get()->get_gc_id());
        GCObject<D> o1 {new_obj<D>(o2)};
        gc_print.print("3 ---------- ", o1.get()->get_gc_id(), " ", o1.get()->o2.get()->get_gc_id());
        o2.get()->o2 = o2.get()->make_ptr(o1);
        //o2.get()->o2 = make_ptr(o1); // this makes an ERROR

        gc_print.print("4 ---------- ", gctor.get_current_object_count());
    }
    gc_print.print("5 ---------- ", gctor.get_current_object_count());
    gctor.updateMemory();
    gc_print.print("6 ---------- ");
}


void b(GCObject<A> obj){
    obj.get()->print();
    gc_print.print(obj.get()->get_gc_id());
}

GCObject<A> & e(GCObject<A> & obj){
    obj = new_obj<A>(obj.get()->i + 2);  
    return obj;
}

void example_5(){
    {
        GCObject<A> o {new_obj<A>(4)};
        //b(o); //ERROR
        o = make_ptr(e(o));
    }
}


int main(int argc, char const *argv[])
{
    /* Follow the examples and comments */

    /* For better performance, it's worth deleting all comments
        (it would save many function calls), but for
        easier program inspection, I left them here */
    gc_print.print_messages = true;
    
    /* You can try individual examples */
    //example_1(); // basic
    //example_2(); // Invoker
    //example_3(); // tracking context_static references over time
    //example_4(); // removing memory cycles
    //example_5(); // passing by parameter, returning from function
    
    this_thread::sleep_for(std::chrono::seconds(5));
     
    gc_print.print("the end of main function", "\n");    
    return 0;
}


