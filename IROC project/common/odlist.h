/******************************************************************************
***N***N**Y***Y**X***X*********************************************************
***NN**N***Y*Y****X*X**********************************************************
***N*N*N****Y******X***********************************************************
***N**NN****Y*****X*X************************ Make Date  : Jun 29, 2007 *******
***N***N****Y****X***X*********************** Last Update: Jun 29, 2007 *******
******************************************************************************/
#ifndef N_STL_ODLIST
#define N_STL_ODLIST
#include <memory>

template <class _Type, int _MinLen = 32>//, _Type _DefValue = 0>
class odlist {
public:
	_Type * items;	
	int length;

public:

	odlist(){
		length = 0;
		items = 0;
	}

	~odlist(){
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
	void removei(int i){
		if (i >= 0 && i < length) {
			int l = length-1;
			if (l!=i) {
				memcpy(&items[i], &items[i+1], (l-i)* sizeof(_Type));
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
				removei(i);
				break;
			}
		}
	}

	//set 	sets an element value in the list
	void set(_Type v, int i){
		if (i >=0 && i < length) {
			items[i] = v;
		}
	}

	//get 	gets an element value in the list
	_Type get(int i){
		return (i >= 0 && i < length)? items[i] : 0;//_DefValue;
	}

		_Type operator[] (const int i) {
		return items[i];
	}

	//clear 	removes all elements from the list	
	void clear(){
		length = 0;
		if (items != 0){
			items = (_Type*)realloc(items, _MinLen * sizeof(_Type));
		}
	}

	//size 	returns the number of items in the list
	int size(){
		return length;
	}
};
#endif
