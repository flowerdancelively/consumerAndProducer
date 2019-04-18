#include<iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <exception>
#include <condition_variable>

using namespace std;

static const int kItemRespositorySize = 10;
static const int kItemProduce = 1000;

struct ItemRepository {
	int itemBuffer[kItemRespositorySize];
	size_t readPos;
	size_t writePos;
	size_t conCnt;
	size_t proCnt;

	std::mutex mtx;
	std::mutex mtxconcnt;
	std::mutex mtxprocnt;

	std::condition_variable repoNotFull;
	std::condition_variable repoNotEmpty;
} gItemRepository;

typedef struct ItemRepository ItemRepository;


void ProducerItem(ItemRepository*ir, int item)
{
	std::unique_lock<mutex> lock(ir->mtx);
	while (((ir->writePos + 1) % kItemRespositorySize) == ir->readPos)
	{
		cout << "Producer is waiting \n";
		ir->repoNotFull.wait(lock);
	}
	cout << "produce "<<ir->writePos<<" pos and " << item << "^th item .. \n";
	ir->itemBuffer[ir->writePos] = item;
	++ ir->writePos;
	++ ir->proCnt;
	if (ir->writePos == kItemRespositorySize)
		ir->writePos = 0;

	ir->repoNotEmpty.notify_all();
	lock.unlock();
}
void  Consumer(ItemRepository*ir)
{
	int data;
	std::unique_lock<mutex> lock(ir->mtx);
	while ( ir->readPos  == ir->writePos)
	{
		cout << "the buffer is empty" << endl;
		ir->repoNotEmpty.wait(lock);
	}
	data = ir->itemBuffer[ir->readPos];
	std::cout << "thread "<<this_thread::get_id()<< " Consume the "<<ir->readPos <<" pos and " << data << "^th item\n";
	ir->readPos ++;
	ir->conCnt++;
	if (ir->readPos == kItemRespositorySize)
		ir->readPos = 0;

	ir->repoNotFull.notify_all();
	lock.unlock();
}


void produceTask()
{
	
		bool readyExit = false;
		while (1)
		{
			this_thread::sleep_for(chrono::microseconds(10));
			unique_lock<mutex> lock(gItemRepository.mtxprocnt);
			if (gItemRepository.proCnt < kItemProduce)
			{
				ProducerItem(&gItemRepository, gItemRepository.proCnt);
			}
			else
			{
				readyExit = true;
			}
			lock.unlock();
			if (readyExit)
			{
				cout << "product readyExit " << this_thread::get_id() << endl;
				break;
			}
		}
		
}

void consumeTask()
{
	bool readyExit = false;
	while (1)
	{
		this_thread::sleep_for(chrono::microseconds(10));
		unique_lock<mutex> lock(gItemRepository.mtxconcnt);
		if (gItemRepository.conCnt < kItemProduce)
		{
			Consumer(&gItemRepository);
			//out << "consumeTask" << std::this_thread::get_id() << endl;

		}
		else
		{
			readyExit = true;
		}
		lock.unlock();
		if (readyExit)
		{
			cout << "consumeTask readyExit" << std::this_thread::get_id() << endl;
			break;
		}
			
	}


}

void iniReposity(ItemRepository*ir)
{
	ir->writePos = 0;
	ir->readPos = 0;
	ir->conCnt = 0;
	ir->proCnt = 0;

}

int main()
{
	
	try
	{
		iniReposity(&gItemRepository);

		std::thread threadPro(produceTask);
		std::thread threadPro2(produceTask);
		std::thread threadPro3(produceTask);
		std::thread threadPro4(produceTask);

		thread threadCons(consumeTask);
		thread threadCons2(consumeTask);
		thread threadCons3(consumeTask);
		thread threadCons4(consumeTask);

		threadPro.join();
		threadPro2.join();
		threadPro3.join();
		threadPro4.join();

		threadCons.join();
		threadCons2.join();
		threadCons3.join();
		threadCons4.join();


	}
	catch (const exception&e)
	{
		e.what();
	}

	system("pause");

	quick_exit(0);
}