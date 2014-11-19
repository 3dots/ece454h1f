
#ifndef HASH_H
#define HASH_H

#include <stdio.h>
#include <pthread.h>
#include "list.h"

#define HASH_INDEX(_addr,_size_mask) (((_addr) >> 2) & (_size_mask))

template<class Ele, class Keytype> class hash;

template<class Ele, class Keytype> class hash {
 private:
  unsigned my_size_log;
  unsigned my_size;
  unsigned my_size_mask;
  list<Ele,Keytype> *entries;
  list<Ele,Keytype> *get_list(unsigned the_idx);
  pthread_rwlock_t *rwlock_list;

 public:
  void setup(unsigned the_size_log=5);
  void insert(Ele *e);
  Ele *lookup(Keytype the_key);
  void print(FILE *f=stdout);
  void reset();
  void cleanup();

  void lookup_and_insert_if_absent(Keytype the_key);
};

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::setup(unsigned the_size_log){
  my_size_log = the_size_log;
  my_size = 1 << my_size_log;
  my_size_mask = (1 << my_size_log) - 1;
  entries = new list<Ele,Keytype>[my_size];

  rwlock_list = new pthread_rwlock_t[my_size];
  unsigned i;
  for(i = 0; i < my_size; i++){
	  pthread_rwlock_init(&rwlock_list[i], NULL);
  }
}

template<class Ele, class Keytype> 
list<Ele,Keytype> *
hash<Ele,Keytype>::get_list(unsigned the_idx){
  if (the_idx >= my_size){
    fprintf(stderr,"hash<Ele,Keytype>::list() public idx out of range!\n");
    exit (1);
  }
  return &entries[the_idx];
}

template<class Ele, class Keytype> 
Ele *
hash<Ele,Keytype>::lookup(Keytype the_key){
  list<Ele,Keytype> *l;

  l = &entries[HASH_INDEX(the_key,my_size_mask)];
  return l->lookup(the_key);
}  

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::print(FILE *f){
  unsigned i;

  for (i=0;i<my_size;i++){
    entries[i].print(f);
  }
}

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::reset(){
  unsigned i;
  for (i=0;i<my_size;i++){
    entries[i].cleanup();
  }
}

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::cleanup(){
  unsigned i;
  reset();
  delete [] entries;

  for(i = 0; i < my_size; i++){
	  pthread_rwlock_destroy(&rwlock_list[i]);
  }
  delete [] rwlock_list;
}

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::insert(Ele *e){
  entries[HASH_INDEX(e->key(),my_size_mask)].push(e);
}


template<class Ele, class Keytype>
void
hash<Ele,Keytype>::lookup_and_insert_if_absent(Keytype the_key){
  list<Ele,Keytype> *l;

  unsigned i = HASH_INDEX(the_key,my_size_mask);

  pthread_rwlock_rdlock(&rwlock_list[i]);

  l = &entries[i];
  Ele *e = l->lookup(the_key);

  if(!e){
	  pthread_rwlock_unlock(&rwlock_list[i]);

	  pthread_rwlock_wrlock(&rwlock_list[i]);
	  //Have to make sure noone else added this. While we waited.
	  e = l->lookup(the_key);
	  if(!e){ //We are the one adding. No-one has access to this pointer yet.
		  //we can increment and leave immediately, without having to lock then unlock the entry.
		  e = new Ele(the_key);
		  l->push(e);
		  e->count++;
		  pthread_rwlock_unlock(&rwlock_list[i]);
		  return;
	  }
	  //No need for writer lock anymore, or any lock really, we found the needed entry.
  }
  //Entry Found.
  pthread_rwlock_unlock(&rwlock_list[i]);
  //Locking entry
  e->lock();
  e->count++;
  e->unlock();

}

#endif
