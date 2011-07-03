/******************************************************************************
***N***N**Y***Y**X***X*********************************************************
***NN**N***Y*Y****X*X**********************************************************
***N*N*N****Y******X***********************************************************
***N**NN****Y*****X*X************************ Make Date  : Jun 29, 2007 *******
***N***N****Y****X***X*********************** Last Update: Jun 29, 2007 *******
******************************************************************************/
#ifndef N_STL_DLIST
#define N_STL_DLIST
#include <memory>

template <class _Type, int _MinLen = 32>//, _Type _DefValue = 0>
class dlist {
public:
	_Type * items;	
	int length;

public:

	dlist(){
		length = 0;
		items = 0;
	}

	~dlist(){
		if (items != 0) {
			free(items);
			items = 0;
			length = 0;
		}
	}

	//add 	adds an element to the list
	void add(_Type element){
		if (items == 0) {
			items = (_Type*)malloc(_MinLen * sizeof(_Type));
		} else {
			if (length > _MinLen)
				items = (_Type*)realloc(items, (length + 1) * sizeof(_Type));
		}
		items[length++] = element;
	}

	//remove 	removes an element from the list
	void remove(int i){
		if (i >= 0 && i < length) {
			int l = length-1;
			if (l!=i) {
				items[i] = items[l];
			}
			length = l;
			if (l >= _MinLen) {
				items = (_Type*)realloc(items, l * sizeof(_Type));
			}
		}
	}

	void remove(_Type t){
		int m = length;
		for (int i = 0; i < m; i++) {
			if (items[i] == t) {
				remove(i);
				break;
			}
		}
	}

	//set 	sets an element value in the list
	void set(_Type v, int i){
		if (i >=0 & i < length) {
			items[i] = v;
		}
	}

	//get 	gets an element value in the list
	_Type get(int i){
		return (i >= 0 && i < length)? items[i] : 0;//_DefValue;
	}

	//clear 	removes all elements from the list	
	void clear(){
		length = 0;
		if (items != 0){
			items = realloc(items, _MinLen * sizeof(_Type));
		}
	}

	//size 	returns the number of items in the list
	int size(){
		return length;
	}
};
#endif
