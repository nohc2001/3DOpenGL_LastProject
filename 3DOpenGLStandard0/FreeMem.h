#pragma once
#include <iostream>
using namespace std;
typedef unsigned char byte8;

class FM_Model {
public:
	FM_Model(){}
	virtual ~FM_Model(){}

	virtual byte8* _New(unsigned int byteSiz) {
		return nullptr;
	}

	virtual void ClearAll() {
		return;
	}

	virtual bool _Delete(byte8* variable, unsigned int size) {
		return false;
	}

	//해당 주소에 메모리가 할당되었는지.
	virtual bool bAlloc(byte8* variable, unsigned int size) {
		return true;
	}
};

void CheckRemainMemorySize() {
	byte8 end = 10;
	byte8* start = new byte8();
	unsigned int RemainMemSiz = (unsigned int)start - (unsigned int)&end;
	cout << "RemainMemSiz : " << RemainMemSiz << " byte \t(" << (float)RemainMemSiz / 1000.0f << " KB \t(" << (float)RemainMemSiz / 1000000.0f << " MB \t(" << (float)RemainMemSiz / 1000000000.0f << " GB ) ) )" << endl;
	delete start;
}

class FM_Model0 : FM_Model {
public:
	unsigned int siz = 0;
	byte8* Data = nullptr;
	unsigned int Fup = 0;
	bool isHeap = false;

	FM_Model0() {}
	FM_Model0(byte8* data, unsigned int Size) {
		Data = data;
		siz = Size;
	}
	virtual ~FM_Model0() {
		if (isHeap && Data != nullptr) {
			delete[] Data;
		}
	}

	void SetHeapData(byte8* data, unsigned int Size) {
		isHeap = true;
		Data = data;
		siz = Size;
	}

	byte8* _New(unsigned int byteSiz) {
		if (Fup + byteSiz < siz) {
			unsigned int fup = Fup;
			Fup += byteSiz;
			return &Data[fup];
		}
		else {
			ClearAll();
			return _New(byteSiz);
		}
	}

	bool _Delete(byte8* variable, unsigned int size) {
		return false;
	}

	void ClearAll() {
		Fup = 0;
	}

	void PrintState() {
		cout << "FreeMemory Model 0 State -----------------" << endl;
		CheckRemainMemorySize();
		cout << "MAX byte : \t" << siz << " byte \t(" << (float)siz / 1024.0f << " KB \t(" << (float)siz / powf(1024.0f, 2) << " MB \t(" << (float)siz / powf(1024.0f, 3) << " GB ) ) )" << endl;
		cout << "Alloc Number : \t" << Fup << " byte \t(" << (float)Fup / 1024.0f << " KB \t(" << (float)Fup / powf(1024.0f, 2) << " MB \t(" << (float)Fup / powf(1024.0f, 3) << " GB ) ) )" << endl;
		cout << "FreeMemory Model 0 State END -----------------" << endl;
	}
};

bool GetByte8(byte8 dat, int loc) {
	byte8 rb = dat;
	rb = rb / pow(2, loc);
	if (rb % 2 == 1) return true;
	else return false;
}

byte8 SetByte8(byte8 dat, int loc, bool is1) {
	int add = pow(2, loc);
	byte8 dat2 = dat;
	if (GetByte8(dat2, loc) == true) {
		if (is1 == false) {
			dat2 = dat2 - add;
		}
	}
	else {
		if (is1) {
			dat2 = dat2 + add;
		}
	}

	return dat2;
}

class FM_Model1 : FM_Model {
public:
	bool isHeap = false; // true면 heap, false면 stack
	byte8* DataPtr = nullptr;
	unsigned int realDataSiz = 0;
	unsigned int sumDataSiz = 0;

	unsigned int Fup = 0;
	int isvalidNum = 0;

	FM_Model1() {

	}

	FM_Model1(unsigned int RDS, byte8* dataptr) {
		DataPtr = dataptr;
		realDataSiz = RDS;
		sumDataSiz = 9 * realDataSiz / 8;
	}

	virtual ~FM_Model1() {
		if (isHeap && DataPtr != nullptr) {
			delete[] DataPtr;
		}
	}

	void SetHeapData(byte8* dataptr, unsigned int SDS) {
		isHeap = true;
		DataPtr = dataptr;
		sumDataSiz = SDS;
		realDataSiz = 8 * sumDataSiz / 9;
	}

	bool isValid(unsigned int address) {
		int bigloc = address / 8;
		int smallLoc = address % 8;
		if (GetByte8(DataPtr[realDataSiz + bigloc], smallLoc)) {
			return false;
		}
		else return true;
	}

	void SetValid(unsigned int address, bool enable) {
		int bigloc = address / 8;
		int smallLoc = address % 8;
		DataPtr[realDataSiz + bigloc] = SetByte8(DataPtr[realDataSiz + bigloc], smallLoc, enable);
	}

	byte8* _New(unsigned int size) {
		int stack = 0;
		if (Fup + size < realDataSiz) {
			int RAD = Fup;
			for (int i = 0; i < size; ++i) {
				SetValid(RAD + i, true);
			}
			Fup += size;
			return &DataPtr[RAD];
		}
		else {
			for (int ad = 0; ad < realDataSiz; ++ad) {
				if (isValid(ad)) {
					stack += 1;
					if (stack == size) {
						int RAD = ad - size + 1;
						for (int i = 0; i < size; ++i) {
							SetValid(RAD + i, true);
						}
						return &DataPtr[RAD];
					}
				}
				else {
					stack = 0;
				}
			}
		}
		return nullptr;
	}

	bool _Delete(byte8* variable, unsigned int size) {
		unsigned int address = variable - DataPtr;
		//for (int i = 0; i < size; ++i) {
		//	if (isValid(address + i) == false) {
		//		return false;
		//	}
		//}

		for (int i = 0; i < size; ++i) {
			SetValid(address + i, false);
		}

		return true;
	}

	//해당 주소에 메모리가 할당되었는지.
	bool bAlloc(byte8* variable, unsigned int size) {
		unsigned int address = variable - DataPtr;
		for (int i = 0; i < size; ++i) {
			if (isValid(address + i) == false) {
				return false;
			}
		}

		return true;
	}

	void DebugAddresses() {
		int count = 0;
		isvalidNum = 0;
		for (int i = 0; i < realDataSiz; ++i) {
			if (isValid(i) == false) {
				count += 1;
			}
		}

		cout << "Non Release Free Memory(no matter) : " << count << endl;

		ofstream out;
		out.open("DebugFile.txt");
		for (int i = 0; i < Fup; ++i) {
			if (isValid(i)) {
				out << '0';
				isvalidNum += 1;
			}
			else {
				out << '1';
			}
		}
		out.close();
	}

	void ClearAll() {
		for (int i = 0; i < realDataSiz; ++i) {
			SetValid(i, false);
		}
	}
};

template<typename T> class InfiniteArray {
public:
	FM_Model* FM;
	T* Arr;
	size_t maxsize = 0;
	size_t up = 0;

	InfiniteArray() :
		FM(nullptr),
		Arr(nullptr),
		maxsize(0)
	{

	}

	virtual ~InfiniteArray() {

	}

	void SetFM(FM_Model* fm) {
		FM = fm;
	}

	void NULLState() {
		FM = nullptr;
		Arr = nullptr;
		maxsize = 0;
		up = 0;
	}

	void Init(size_t siz) {
		InfiniteArray<T> go;
		int vp = *(int*)&go;
		*((int*)this) = vp;

		
		T* newArr = (T*)FM->_New(sizeof(T) * siz);
		if (Arr != nullptr) {
			for (int i = 0; i < maxsize; ++i) {
				newArr[i] = Arr[i];
			}

			if (FM->bAlloc((byte8*)Arr, sizeof(T) * maxsize)) {
				FM->_Delete((byte8*)Arr, sizeof(T) * maxsize);
			}
		}
		
		Arr = newArr;
		maxsize = siz;
	}

	T& at(size_t i) {
		return Arr[i];
	}

	T& operator[](size_t i) {
		return Arr[i];
	}

	void push_back(const T& value) {
		if (up < maxsize) {
			Arr[up] = value;
			up += 1;
		}
		else {
			Init(maxsize * 2 + 1);
			Arr[up] = value;
			up += 1;
		}
	}

	void erase(size_t i) {
		for (int k = i; k < up; ++k) {
			Arr[k] = Arr[k + 1];
		}
		up -= 1;
	}

	void insert(size_t i, const T& value) {
		push_back(value);
		for (int k = maxsize-1; k > i; k--) {
			Arr[k] = Arr[k - 1];
		}
		Arr[i] = value;
	}

	size_t size() {
		return up;
	}

	void SetVPTR() {
		T go;
		int vp = *(int*)&go;
		for (int i = 0; i < up; ++i) {
			*((int*)&Arr[i]) = vp;
		}
	}

	void clear() {
		if (FM->bAlloc((byte8*)Arr, sizeof(T) * maxsize)) {
			FM->_Delete((byte8*)Arr, sizeof(T) * maxsize);
		}
		Arr = nullptr;
		up = 0;

		Init(2);
	}
};