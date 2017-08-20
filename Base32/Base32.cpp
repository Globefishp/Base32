// Base32.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "windows.h"
using namespace std;

#define allocERR 2
#define dllERR 0
#define IOERR 3
#define funcERR 4
#define PutIn 10 //10的倍数
#define ReadBytes 1020//流大小的BUG已经修复,指定10的倍数,否则结果不正确
typedef char* (*pEnc)(char*);

char Base32EncL[33] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
char Base32DecL[91] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,26,27,28,29,30,31,0,0,0,0,0,0,0,0,0,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25};

//void GetMemory(char **p, int num) {
//	*p = (char *)malloc(sizeof(char) * num);
//}
//
//void Test(void) {
//	char *str = NULL;
//	GetMemory(&str, 100); // str 仍然为 NULL
//
//	strcpy(str, "hello"); 
//	cout << str << endl;
//
//	free(str);
//}

char* Enc(char* argv, bool binary = false) {
	unsigned int StrCharSize = 0;//转换UNICODE编码后的char字符个数
	wchar_t* sp = NULL;

	if (binary == true) {
		StrCharSize = ReadBytes / sizeof(wchar_t);
		sp = (wchar_t*)argv;
	} else {
		StrCharSize = mbstowcs(NULL, argv, 0);//调用函数获得需要的内存空间
		sp = new(nothrow) wchar_t[StrCharSize + 1]();//分配内存空间,存储UNICODE元数据
		mbstowcs((wchar_t*)sp, argv, 2 * StrCharSize);//在申请的内存空间存入转换后的UNICODE字符
	}

	unsigned char* od = new(nothrow) unsigned char[(8 * StrCharSize * sizeof(wchar_t) / sizeof(char) + 4) / 5 + 1]();//分配内存空间,存储编码后数据
	unsigned char* pProc, *dProc;//以8bit为单位的处理用指针
	void* pSrc, *pOverflow;//存储元数据首地址和溢出地址(最后一位地址+1)
	void* dSrc, *dOverflow;//存储译文首地址和溢出地址(最后一位地址+1)
	if ((sp == NULL) || (od == NULL)) return NULL;//检测空指针

	pProc = (unsigned char*)(sp);
	pSrc = sp;
	pOverflow = sp + StrCharSize;
	dProc = (unsigned char*)(od);
	dSrc = od;
	dOverflow = od + ((8 * StrCharSize * sizeof(wchar_t) / sizeof(char) + 4) / 5 + 1);//向上取整,末尾\0
	//各类指针初始化

	//for (unsigned int i = 0; i < StrCharSize + 1; i++) {
	//	for (int j = 15; j >= 0; j--) {
	//		if ((*sp & (1 << j)) != 0) {
	//			putchar('1');
	//		}
	//		else
	//		{
	//			putchar('0');
	//		}
	//	}//显示二进制
	//	*sp = *sp << 8 | *sp >> 8;
	//	sp++;
	//}
	//sp = (wchar_t*)srcp;
	//printf("\n");
	//由于wchar_t内存数据如此存放:第一个字符低位,第一个字符高位,第二个字符低位,第二个字符高位...
	//读取时wchar_t指针指向16bit数据,逆向读取(变成正常的高位->低位)
	//所以用char指针的时候就会出现(低位->高位)的错误
	//进行高低位交换后即可避免.

	//for (unsigned int i = 0; i < StrCharSize; i++) {
	//	//wcout << pProc;
	//	for (int i = 15; i >= 0; i--) {
	//		if ((*sp & (1 << i)) != 0) {
	//			putchar('1');
	//		}
	//		else
	//		{
	//			putchar('0');
	//		}
	//	}
	//	//显示二进制
	//	sp++;
	//}
	//printf("\n");
	//sp = (wchar_t*)pSrc;
	////输出源

	for (unsigned int i = 0; i <= (StrCharSize * sizeof(wchar_t) / PutIn); i++) {//以8 * #PutIn# bit数据为单位处理
		if (binary == false) {
			for (unsigned int j = 0; j < PutIn / 2; j++) {
				if (sp >= pOverflow) { break; }//溢出检测
				//for (int k = 15; k >= 0; k--) {
				// if ((*sp & (1 << k)) != 0) {
				// putchar('1');
				// }
				// else
				// {
				// putchar('0');
				// }
				//}
				////显示二进制
				*sp = *sp << 8 | *sp >> 8;
				sp++;
			}//对读入数据高低位交换
			 //sp = (wchar_t*)srcp;
			 //printf("\n");
		}

		unsigned char pCode[8 * PutIn / 5] = { 0 };//创建译码后译文存储空间(8 * #PutIn# / 5位)
		int count = 0;
		for (int j = 0; j < PutIn; j++) {
			if (pProc >= pOverflow) { break; }//溢出检测
			//for (int k = 7; k >= 0; k--) {
			// if ((*pProc & (1 << k)) != 0) {
			// putchar('1');
			// }
			// else
			// {
			// putchar('0');
			// }
			//}
			////显示二进制
			for (int k = 0; k < 8; k++) {
				if ((*pProc & (1 << (7 - k))) != 0) { pCode[count / 5] |= (1 << (4 - (count % 5))); }//判断该位是否是1(用1移位再取与可知此位是0还是有值)并按5位二进制的实际值大小相加
				count++;
			}//对每一个char的8bit做操作
			pProc++;
		}//对一次性输入的#PutIN#个char字符操作
		 //printf("\n");
		for (int j = 0; j < 8 * PutIn / 5; j++) {
			if ((dProc + 1) >= dOverflow) { *dProc = 0; break; }//判断是否为最后一位,写入\0标记
			*dProc = Base32EncL[pCode[j]];
			dProc++;
		}//译文译码
	}
	if (binary == false) delete[] (wchar_t*)pSrc;
	return (char*)od;
}

int Dec(char* argv) {
	unsigned int StrCharSize = 0;//待解码字符个数
	unsigned char* pProc = (unsigned char*)argv;//定义处理用指针
	void* pSrc = argv;//定义元数据首地址
	do {
		StrCharSize++;
		pProc++;
	} while (*pProc != 0);//获得字符串长度,从而计算所需内存空间

	wchar_t* od = new(nothrow) wchar_t[StrCharSize * 5 / 16 + 1]();//分配内存空间,存储解码后数据

	void* pOverflow = pProc + 1;//存储元数据溢出地址(最后一位地址+1)
	pProc = (unsigned char*)pSrc;
	unsigned char* dProc;//以8bit为单位的放置数据指针
	void* dSrc, *dOverflow;//存储译文首地址和溢出地址(最后一位地址+1)
	if (od == NULL) return allocERR;//检测空指针

	dProc = (unsigned char*)(od) + 1;//初始置于第一个宽字符的高8位
	dSrc = (unsigned char*)od;
	dOverflow = (unsigned char*)od + (StrCharSize * 5 / 16) * sizeof(wchar_t) / sizeof(char) + 1;
	//各类指针初始化

	for (unsigned int i = 0; i <= (StrCharSize / (PutIn * 8 / 5)); i++) {//实际读入64*PutIn/5bit,处理结束后成为8 * #PutIn# bit数据.
		int count = 0;
		for (int j = 0; j < PutIn * 8 / 5; j++) {
			if (pProc + 1 >= pOverflow) { *dProc = 0; break; }//解码结束检测
			for (int k = 0; k < 5; k++) {
				if (dProc + 1 >= dOverflow) {
					dProc = (unsigned char*)dOverflow - 1;
					*dProc = 0;
					break;
				}//缓存池溢出检测
				if ((Base32DecL[*pProc] & (1 << (4 - k))) != 0) { *dProc |= 1 << (7 - count % 8); }//查表后直接读写缓存池
				count++;
				if (count % 16 == 0) { dProc += 3; }
				else if (count % 8 == 0) { dProc--; }//缓存池指针移动
			}
			pProc++;//元数据指针移动
		}
	}
	wcout << od;
	delete[] od;//释放内存与指针
	//od, dProc, dSrc, dOverflow, pProc, pSrc, pOverflow = NULL;//指针让程序自己回收.
	return 0;
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "chs");
	int Err = 0;//Err部分未完成
	if (argc != 3 && argc != 4) printf("Usage: Base32 [-Enc|-Dec|fEnc|fDec] \"String|File\" [\"output file\"]\n");
	if (argc == 3) {
		if (_stricmp(argv[1], "-Enc") == 0) {
			char* p;
			p = Enc(argv[2]);
			if (p == NULL) { printf("Function ERROR!\n"); delete[] p; return funcERR; }
			else { printf(p); delete[] p; p = NULL; }//保险起见用[](实际上并不需要)
		} 
		else if (_stricmp(argv[1], "-Dec") == 0) {
			Dec(argv[2]);
		}
	}
	else if (argc == 4) {
		if(_stricmp(argv[1], "-fEnc")==0) {
			ifstream inFile(argv[2], ios::binary);
			ofstream otFile(argv[3], ios::binary);
			if (otFile.is_open() == false) { printf("IO ERROR,Can't open %s!\n", argv[3]); return IOERR; }
			char* fp = new(nothrow) char[ReadBytes];
			if (fp==NULL) { printf("Memory ERROR!\n"); return allocERR; }
			char* wp = NULL;
			do {
				inFile.read(fp, ReadBytes);
				wp = Enc(fp, true);
				if (wp == NULL) { printf("Memory ERROR!\n"); return allocERR; }
				otFile.write(wp, strlen(wp));
				delete wp;//重要!释放内存
			} while (!inFile.eof());
			inFile.close();
			otFile.close();
		}
		else if (_stricmp(argv[1], "-fDec") == 0) {

		}
	}
	else {
		printf("%s Parameter ERROR!\n",argv[1]);
	};

	return Err;
}

