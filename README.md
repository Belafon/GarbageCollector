

## Garbage collector


for memory management.


> example.cpp shows how to use the program.

## Instalation

**Requirement: C++20**

The simplest way to use it:

1. Copy the files to your project and include `"GarbageCollector.h"`.
2. Then, use the code as desired.

The project contains the file `example.cpp`, which demonstrates the program's capabilities.

**CMake**

The Makefile compiles and runs the tests, and then the example.

**Execute Examples**

```bash
g++ -std=c++20 example.cpp -o example
./example
```

**Execute Tests**

```bash
g++ -std=c++20 tests.cpp -o tests
./tests
```

## Documentation

See ./documentation/domcumentation_cz.pdf
## Example of use

GCObject is an object through which you can look at memory stored in 
the garbage collector.
The new_obj method creates a new object A with parameter 4 and stores it in
the garbage collector (wraps the object in a helper class 'Context').
The correct new_obj() method must be called from the same object where
the corresponding GCObject was created. Otherwise, the program will crash and print
an error message. (This is important for sufficient control over memory
held by the garbage collector)

```cpp

struct A : GCContext
{
    int i;
    A(int i) : i(i) {
    }
};

void example_1() {    
    {
        GCObject<A> o {&context_static}; 
        o = new_obj<A>(4);
        o.get()->i = 9; // working with the value in the garbage collector
        o.get()->print(); // ability to print info about references of the respective object to the console
    }
    /* A breadth-first traversal of references of all objects in the garbage collector is performed, 
       those that are not reachable from the special Context object context_static are discarded */
    gctor.updateMemory(); 
}
```


free to use and modify



~ Created by Ondřej Tichavský

~ 28.5. 2022
