
#ifndef LIST_H
#define LIST_H

#include <stdio.h>

#define DEBUG_REDUCTION 0

template<class Ele, class Keytype> class list;

template<class Ele, class Keytype> class list {
 private:
  Ele *my_head;
  unsigned long long my_num_ele;
 public:
  list(){ 
    my_head = NULL;
    my_num_ele = 0;
  }

  void setup();

  unsigned num_ele(){return my_num_ele;}

  Ele *head(){ return my_head; }
  Ele *lookup(Keytype the_key);

  void push(Ele *e);
  Ele *pop();
  void print(FILE *f=stdout);

  void cleanup();

  void combine_with(class list *l);
};

template<class Ele, class Keytype> 
void 
list<Ele,Keytype>::setup(){
  my_head = NULL;
  my_num_ele = 0;
}

template<class Ele, class Keytype> 
void 
list<Ele,Keytype>::push(Ele *e){
  e->next = my_head;
  my_head = e;
  my_num_ele++;
}

template<class Ele, class Keytype> 
Ele *
list<Ele,Keytype>::pop(){
  Ele *e;
  e = my_head;
  if (e){
    my_head = e->next;
    my_num_ele--;
  }
  return e;
}

template<class Ele, class Keytype> 
void 
list<Ele,Keytype>::print(FILE *f){
  Ele *e_tmp = my_head;

  while (e_tmp){
    e_tmp->print(f);
    e_tmp = e_tmp->next;
  }
}

template<class Ele, class Keytype> 
Ele *
list<Ele,Keytype>::lookup(Keytype the_key){
  Ele *e_tmp = my_head;
  
  while (e_tmp && (e_tmp->key() != the_key)){
    e_tmp = e_tmp->next;
  }
  return e_tmp;
}

template<class Ele, class Keytype> 
void
list<Ele,Keytype>::cleanup(){
  Ele *e_tmp = my_head;
  Ele *e_next;

  while (e_tmp){
    e_next = e_tmp->next;
    delete e_tmp;
    e_tmp = e_next;
  }
  my_head = NULL;
  my_num_ele = 0;
}

template<class Ele, class Keytype>
void
list<Ele,Keytype>::combine_with(class list<Ele,Keytype> *l){
	Ele *new_e_tmp = l->head();
	Ele *my_e;

	while (new_e_tmp){//Transversing the new given list.
		//Looking for the same element in my list.
		if(DEBUG_REDUCTION){
			printf("The new list was not empty.\nAbout to look up key: %u\n", new_e_tmp->key());
		}

		my_e = lookup(new_e_tmp->key());

		if(DEBUG_REDUCTION){
			printf("The lookup result address is %d\n", my_e);
			unsigned int a = new_e_tmp->key();
			unsigned int b;
			if(my_e != NULL)
				 b = my_e->key();
			else
				 b = 0;
			printf("My element's key: %u. The desired key: %u.\n", b, a);
		}

		if(!my_e){//This element wasn't in my list

			Ele *e_copy = new Ele(new_e_tmp->key());
			e_copy->count = new_e_tmp->count;

			push(e_copy);
		}
		else{//Combine counts
			my_e->count += new_e_tmp->count;
		}

		new_e_tmp = new_e_tmp->next;
	}
}

#endif
