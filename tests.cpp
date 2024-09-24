// SCROLL DOLU K MAIN METODE
#include "GarbageCollector.h"
#include <cassert>

using namespace std;
using namespace gc;

atomic<bool> controlContextCount {false};
atomic<int> countContext {0};
atomic<bool> stopInv {false};
atomic<bool> testContinue {true};

class TestInvoker : public GCInvoker{
    public:
    TestInvoker() : GCInvoker(){}

    TestInvoker(GCMemory * memory) : GCInvoker(memory){
        assert(countContext == memory->getCount()); 
    }

    void run() override {
        while(!stopInv.load()){
            while(!controlContextCount.load() && !stopInv.load()){
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
            if(!stopInv.load())
                assert(countContext == memory->getCount());
            controlContextCount.store(false);
            testContinue.store(true);
        }
        is_thread_inactive.store(true);
    }

};

struct A : GCContext
{
    int i;
    A(int i) : i(i){
    }
};

void checkNumberContext(int count){
    countContext.store(count);
    testContinue.store(false);
    controlContextCount.store(true);
    while(!testContinue.load()){
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
void test_1(){    
    {
        checkNumberContext(0);

        GCObject<A> o {&context_static};
        o = new_obj<A>(4);

        checkNumberContext(1);

        o.get()->i = 9; // práce s hodnotou v garbage collectoru
        assert(gctor.get_current_object_count() == 1);
        assert(o.get()->get_gc_id() == 2);
        assert(o.get()->i == 9);
    }
    assert(gctor.get_current_object_count() == 0);
    gctor.updateMemory(); 
    checkNumberContext(0);

}

void test_3(){
    {
        assert(gctor.get_current_object_count() == 0);
        // způsob, jak vytvořit objekt bez odkazu, při přístupu k datům vrací nullptr
        // potřebuje alespoň odkaz na místo vytvoření
        GCObject<A> o0 {&context_static};
        
        /*
            pokud je mimo třídu, která by byla potomkem GCObject,
            pak se zavolá automaticky statická proměnná, která nastaví
            GCObjectu jeho polohu, jako v context_static objektu. */
        
        o0 = new_obj<A, int>(5);
        checkNumberContext(1);

        assert(gctor.get_current_object_count() == 1);

        // this prints all references of Context context_static
        GCObject<A> o1 {new_obj<A, int>(4)};
        checkNumberContext(2);
        
        assert(gctor.get_current_object_count() == 2);
        assert(o1.get()->i == 4);

        GCObject<A> o2 {make_ptr<A>(o1)};

        assert(gctor.get_current_object_count() == 3);
        assert(o2.get()->i == 4);

        
        assert(o0.get()->i == 5);

        o0 = make_ptr<A>(o1);

        assert(o0.get()->i == 4);

        assert(gctor.get_current_object_count() == 3);

        o0 = make_ptr<A>(o2);
        
        assert(o0.get()->i == 4);
        assert(gctor.get_current_object_count() == 3);

        

        // all memory is removed at the end (in GarbageCollector destructor), 
        // this removes just the objects with lost references
        // měl by odstranit 1 context objekty
        gctor.updateMemory();
        checkNumberContext(1);


        o0.removeReference();
        assert(o0.get() == nullptr);
        assert(gctor.get_current_object_count() == 3);
    }
    gctor.updateMemory();

    assert(gctor.get_current_object_count() == 0);
    checkNumberContext(0);

}

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
void test_4(){
    {
        GCObject<C> o2 {new_obj<C>()}; // Object -> C, obsahující Object -> D
        assert(gctor.get_current_object_count() == 2);
        checkNumberContext(1);
        GCObject<D> o1 {new_obj<D>(o2)};
        assert(gctor.get_current_object_count() == 4);
        checkNumberContext(2);
        o2.get()->o2 = o2.get()->make_ptr(o1);
        assert(gctor.get_current_object_count() == 4);
        checkNumberContext(2);
        //o2.get()->o2 = make_ptr(o1); // this makes an ERROR

    }
    checkNumberContext(2);
    gctor.updateMemory();
    checkNumberContext(0);

}


void b(GCObject<A> obj){
    obj.get()->print();
}
GCObject<A> & e(GCObject<A> & obj){
    obj = new_obj<A>(obj.get()->i + 2);  
    return obj;
}
void test_5(){
    {
        GCObject<A> o {new_obj<A>(4)}; // pokud se nenachází v jiném 
        //b(o); //ERROR
        
        assert(o.get()->i == 4);
        o = make_ptr(e(o));
        assert(o.get()->i == 6);

    }
    // při destructoru 
}


int main(int argc, char const *argv[])
{
    gctor.startNewinvoker<TestInvoker>();
    test_1(); // základ
    test_3(); // sledování context_static referencí v průběhu
    test_4(); // odstranění cyklu v paměti
    test_5(); // předávání parametrem, vracení funkcí
    stopInv.store(true);

    return 0;
}

