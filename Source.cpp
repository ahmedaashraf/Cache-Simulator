#include <iostream>
#include <iomanip>
#include <fstream>
#include<algorithm>
using namespace std;

#define		DBG				1
#define		DRAM_SIZE		(64*1024*1024)
#define		CACHE_SIZE		(64*1024)


float miss, hit;
int **cache;
int cachesize, linesize, lineNum, col;
enum cacheResType { MISS = 0, HIT = 1 };

unsigned int m_w = 0xABABAB55;    /* must not be zero, nor 0x464fffff */
unsigned int m_z = 0x05080902;    /* must not be zero, nor 0x9068ffff */

unsigned int rand_()
{
	m_z = 36969 * (m_z & 65535) + (m_z >> 16);
	m_w = 18000 * (m_w & 65535) + (m_w >> 16);
	return (m_z << 16) + m_w;  /* 32-bit result */
}

unsigned int memGen1()
{
	//fully assciative fifo
	static unsigned int addr = 0;
	return (addr++) % (DRAM_SIZE); //beygeeb address gowa ram
}

unsigned int memGen2()
{
	//direct mapped
	static unsigned int addr = 0;
	return  rand_() % (128 * 1024);
}

unsigned int memGen3()
{
	//fully ass
	return rand_() % (DRAM_SIZE);
}

unsigned int memGen4()
{
	//fully ass gowa set ass
	static unsigned int addr = 0;
	return (addr++) % (1024);
}

unsigned int memGen5()
{
	//
	static unsigned int addr = 0;
	return (addr++) % (1024 * 64);
}

unsigned int memGen6()
{
	static unsigned int addr = 0;
	return (addr += 256) % (DRAM_SIZE);
}

void build()
{
	cache = new int *[col];
	lineNum = cachesize / linesize;
	for (int i = 0; i < col; i++)
		cache[i] = new int[lineNum];
	for (int k = 0; k < col; k++)
		for (int i = 0; i < lineNum; i++)
			cache[k][i] = 0;
}

// Cache Simulator
cacheResType DirectMapped(unsigned int addr)
{
	// This function accepts the memory address for the read and 
	// returns whether it caused a cache miss or a cache hit
	// The current implementation assumes there is no cache; so, every transaction is a miss
	int t = addr / linesize;
	int tag = t / lineNum;
	int index = t % lineNum;
	if (cache[0][index] == 1 && cache[1][index] == tag)
	{
		hit++;
		return HIT;
	}
	else
	{
		cache[0][index] = 1;
		cache[1][index] = tag;
		miss++;
		return MISS;
	}
}
cacheResType FullyAss(unsigned int addr, int r)
{
	int x;
	int tag = addr / linesize;
	for (int i = 0; i < lineNum; i++)
	{
		if (cache[0][i] == 1 && cache[1][i] == tag)
		{
			if (r == 2)
				cache[2][i]++;
			hit++;
			if (r == 1)
				cache[2][i] = hit + miss;
			return HIT;
		}
		else if (cache[0][i] == 0)
		{
			miss++;
			cache[0][i] = 1;
			cache[1][i] = tag;
			if (r == 1)
				cache[2][i] = hit + miss;
			return MISS;
		}
	}


	//Capacity misses
	if (r == 4)//random
	{
		x = rand() % lineNum;
	}
	else if (r == 3)//FIFO
	{
		x = (int)miss % lineNum;
	}
	else if (r == 2 || r == 1)//LFU & LRU
	{
		x = 0;
		int min = cache[2][0];
		for (int i = 1; i < lineNum; i++)
		{
			if (min > cache[2][i])
			{
				x = i;
				min = cache[2][i];
			}
		}
	}
	cache[0][x] = 1;
	cache[1][x] = tag;
	miss++;
	if (r == 1)
		cache[2][x] = hit + miss;
	if (r == 2)
		cache[2][x] = 0;
	return MISS;

}

cacheResType SetAss(unsigned int addr, int ways)
{
	int sets, tag, index, rep = -1;
	sets = cachesize / (linesize * ways);
	index = addr % sets;
	tag = addr / cachesize;
	bool empty = false;
	for (int i = index*ways; i < index*ways + ways; i++)
	{
		if (cache[0][i] == 1 && cache[1][i] == tag)
		{
			hit++;
			return HIT;
		}
		else if (cache[0][i] == 0 && !empty)
		{
			rep = i;
			empty = true;
		}
	}
	if (!empty)
	{
		rep = rand() % ways;
		rep = index*ways + rep;

	}
	cache[0][rep] = 1;
	cache[1][rep] = tag;
	miss++;
	return MISS;


}

char *msg[2] = { "Miss","Hit" };

int main()
{
	float missrate, hitrate;
	int inst = 0;
	cacheResType r;
	unsigned int addr;
	int type;
	string filename = "out.txt";
	ofstream file;
	file.open(filename.c_str());
	if (!file.is_open())
		cout << "Failed to open file\n";
	cout << "Cache Simulator\n";
	cout << "Please enter cache size (please note that sizes are 1K, 2K, 4K, 8K, 16K, 32K, 64K)\n";
	cin >> cachesize;
	cachesize *= 1024;

	cout << "Please enter line size (please note that sizes are 4, 8, 16, 32, 64 or 128 bytes)\n";
	cin >> linesize;
	cout << "Please enter cache type to be simulated (Dired mapped: 0, Fully Associative: 1, Set Associative: 2\n ";
	cin >> type;
	cout << "Please enter number of transactions (1000000-10000000)\n";
	cin >> inst;

	if (type == 0)
	{
		col = 2;
		// change the number of iterations into 10,000,000
		build();
		for (int Z = 0; Z < inst; Z++)
		{
			 addr = memGen1();
			//addr = memGen2();
			//addr = memGen3();
			//addr = memGen4();
			//addr = memGen5();
			//addr = memGen6();
			r = DirectMapped(addr);
			//file << "0x" << setfill('0') << setw(8) << hex << addr << "\t" << msg[r] << "\n";
		}

	}
	if (type == 1)
	{
		int replacement;
		cout << "Please enter replacement policy (LRU:1, LFU:2, FIFO:3, Random:4)\n";
		cin >> replacement;
		col = 3;
		build();
		for (int Z = 0; Z < inst; Z++)
		{
			 
			//addr = memGen1();
			//addr = memGen2();
			//addr = memGen3();
			//addr = memGen4();
			//addr = memGen5();
		     addr = memGen6();
			r = FullyAss(addr, replacement);
			//file << "0x" << setfill('0') << setw(8) << hex << addr << "\t" << msg[r] << endl;
		}
	}
	if (type == 2)
	{
		int ways;
		cout << "PLease enter number of ways (2, 4, 6, 8) \n";
		cin >> ways;
		col = 2;
		build();
		for (int Z = 0; Z < inst; Z++) {
			
			//addr = memGen1();
			//addr = memGen2();
			//addr = memGen3();
			//addr = memGen4();
			//addr = memGen5();
			addr = memGen6();
			r = SetAss(addr, ways);
			//file << "0x" << setfill('0') << setw(8) << hex << addr << "\t" << msg[r] << "\n";
		}

	}
	missrate = miss / (miss + hit);
	hitrate = hit / (miss + hit);
	file << "Number of hits: " << hit << endl;
	file << "Number of Misses: " << miss << endl;
	file << "Hit rate: " << hitrate << endl;
	file << "Miss rate: " << missrate << endl << endl;
	file.close();

	delete[]cache;
}