#ifndef GCINVOKER_H_

#define GCINVOKER_H_

namespace gc{

    class GCInvoker{    
        protected:
            std::atomic<bool> is_thread_inactive {true};
            std::atomic<bool> is_stop_invoker {true};
            GCInvoker(){}
            GCMemory * memory;
        public:
            std::atomic<bool> closeInvoker {false};
            virtual void run() = 0;
            void start(){
                if(!is_stop_invoker.load()){
                    gc_print.print("(Invoker) Error, attempt to start new invoker even invoker was running");
                    return;
                }

                while(!is_thread_inactive.load()){
                    std::this_thread::sleep_for(std::chrono::milliseconds(40));
                    std::this_thread::yield();
                }
                
                is_stop_invoker.store(false);
                std::thread t(&GCInvoker::run, this);
                t.detach();

                is_thread_inactive.store(false);
            }
            GCInvoker(GCMemory * memory) : memory(memory){}
            void stopInvoker(){
                is_stop_invoker.store(true);
            }
            virtual ~GCInvoker(){
                stopInvoker();
                while(!is_thread_inactive.load()){
                    std::this_thread::sleep_for(std::chrono::milliseconds(40));
                    std::this_thread::yield();
                }
            };
    };
}

#endif