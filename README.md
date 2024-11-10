## Pure C Tools

### 0. About

> there are some useful tools for c, u can require `tools.h` file and enjoy the tools ...

### 1. Usage

```c
// define helpers
#define H_PCT_OBJECT_CALLBACKS
void Object_initByType(char type, void *object){}
void Object_freeByType(char type, void *object){}
void Object_printByType(char type, void *object){
    printf("<MyObject t:%c p:%p>\n", type, object);
}
// incluede tools
#include "./pure-c-tools/tools.h"
// use string
String *str = String_format("test...");
printf("length: %i\n", String_length(str));
printf("content: %s\n", String_get(str));
Object_print(str);
```

### 2. Libs

* string
* hashmap
* queue
* stack
* array
* ...

### 3. More
* ...
  
