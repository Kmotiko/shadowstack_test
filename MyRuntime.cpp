#include<algorithm>
#include<cstdint>
#include<iostream>
#include<vector>
#include "ShadowStack.hpp"


#define MAX_OBJECT 10

// objects
std::vector<int*> objects;

/**
 * visitor to check GC-Roots
 */
static void visitor(void **Root, const void *Meta) {
  for (auto &elem : objects) {
    if (elem == *Root){
      intptr_t intptr = (intptr_t)elem;
      elem = (int *)++intptr;
    }
  }
}


/**
 * collect
 */
static void collect(){
  // visit Roots
  visitGCRoots(visitor);
  
  // delete unreferenced object
  for(auto &elem : objects){
    intptr_t intptr = (intptr_t)elem;
    int flag = intptr & 0x01;
    if(flag == 0){
      std::cout << "[DELETE PTR] " << elem <<  " [VALUE]" << *elem << std::endl ;
      delete elem;
    }
  }

  // delete object from vector
  objects.erase(std::remove_if(objects.begin(), objects.end(),
                               [](int *obj) { return ( ((intptr_t)obj & 0x01) == 0); }),
                objects.end());

  // fix address
  for(auto &elem : objects){
    intptr_t intptr = (intptr_t)elem;
    elem = (int *)--intptr;
  }
  return;
}


/**
 * main allocator
 */
int *gc_alloc(){
  // can allocate?
  if(objects.size() >= MAX_OBJECT){
    // print objects before collect
    std::cout << "Print objects before collect" << std::endl ;
    for(auto &elem : objects){
      std::cout << *elem << std::endl ;
    }
    std::cout << std::endl ;

    // exec collect
    collect();

    // print objects after collect
    std::cout << std::endl ;
    std::cout << "Print objects after collect" << std::endl ;
    for(auto &elem : objects){
      std::cout << *elem << std::endl ;
    }
    std::cout << std::endl ;
  }

  // new
  int *p = NULL;
  if(objects.size() < MAX_OBJECT){
    p = new int;
    objects.push_back(p);
  }

  return p;
}


extern "C" {
  /**
   * my allocator
   */
  int *my_alloc() {
    return gc_alloc();
  }
}
