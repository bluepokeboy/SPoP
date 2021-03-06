#include <bits/stdc++.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "v1.cpp"
#include <semaphore.h>
using namespace std;

pthread_mutex_t db_mutex;

string random_key(int stringLength)
{
	string key = "";
	string letters = "";
	for (char i = 'a'; i <= 'z'; i++)
		letters += i;
	for (char i = 'A'; i <= 'Z'; i++)
		letters += i;
	for (int i = 0; i < stringLength; i++)
		key = key + letters[rand() % 52];

	return key;
}

string random_value(int stringLength)
{
	string value = "";
	string letters = "";
	for (int i = 1; i <= 127; i++)
		letters += char(i); // contains all ASCII except null character

	for (int i = 0; i < stringLength; i++)
		value = value + letters[rand() % 127]; // iterates through all ASCII except null character

	return value;
}

long CLOCKS_PER_SECOND = 1000000;
kvStore kv(1000000);
map<string, string> db, db2;
int db_size = 0;
int num = 0;


void *myThreadFun(void *vargp) 
{ 
	int transactions=0;
	clock_t start = clock();
	int time = 10;
	clock_t tt = clock();
	while((float(tt-start)/CLOCKS_PER_SECOND)<=time)
	{

		for(int i=0;i<10000;i++)
		{
			transactions+=1;
			int x = rand()%5;
			if(x==0) // get key
			{
				string k = random_key(10);
				int keyLength = k.length();
				char *keyArray;
				keyArray = (char *)malloc(keyLength + 1);

				strcpy(keyArray, k.c_str());

				Slice keySlice, valueSlice;
				keySlice.size = (uint8_t)keyLength;
				keySlice.data = keyArray;

				bool ans = kv.get(keySlice, valueSlice);

			}
			else if(x == 1) // put key
			{
				int k = rand() % 64 + 1;
				string key = random_key(k);
				int keyLength = key.length();
				char *keyArray;
				keyArray = (char *)malloc(keyLength);

				strcpy(keyArray, key.c_str());

				int v = rand() % 256 + 1;
				string value = random_value(v);
				int valueLength = value.length();
				char *valueArray;
				valueArray = (char *)malloc(valueLength);

				strcpy(valueArray, value.c_str());

				Slice keySlice, valueSlice;
				keySlice.size = (uint8_t)keyLength;
				keySlice.data = keyArray;

				valueSlice.size = (uint8_t)valueLength;
				valueSlice.data = valueArray;

				kv.put(keySlice, valueSlice);	
				pthread_mutex_lock(&(db_mutex));			
				db_size++;
				pthread_mutex_unlock(&(db_mutex));			

			}
			else if(x==2) // delete
			{
				int max_size = db.size();
				if (max_size == 0)
					continue;		
				int rem = rand()%max_size;

				pthread_mutex_lock(&(db_mutex));
				map<string, string>::iterator itr = db.begin();

				for (int i = 0; i < rem; i++)
					itr++;
				
				string key = itr->first;

				int keyLength = key.length();
				char *keyArray;
				keyArray = (char *)malloc(keyLength + 1);
				db_size--;
				db.erase(itr);
				pthread_mutex_unlock(&(db_mutex));

				strcpy(keyArray, key.c_str());

				Slice keySlice;
				keySlice.size = (uint8_t)keyLength;
				keySlice.data = keyArray;

				kv.del(keySlice);				// tada

			}

			else if(x==3) // getN
			{
				pthread_mutex_lock(&(db_mutex));
				int max_size = db.size();
				pthread_mutex_unlock(&(db_mutex));
				int rem = rand() % max_size;
				Slice keySlice, valueSlice;
				bool checkStatus = kv.get(rem + 1, keySlice, valueSlice); // tada
			}
			else if(x==4)
			{
				pthread_mutex_lock(&(db_mutex));
				int max_size = db.size();
				if (!max_size)
					break;
				int rem = rand() % max_size;
				map<string, string>::iterator itr = db.begin();
				for (int i = 0; i < rem; i++)
					itr++;
				assert(max_size <= db.size());
				
				kv.del(rem + 1); //tada
				db.erase(itr);
				db_size--;
				pthread_mutex_unlock(&(db_mutex));
			}
		}
		tt=clock();
	}
	cout<<transactions/time<<endl;
	return NULL;  
} 

int main()
{
	srand(time(NULL));
	for (int i = 0; i < 100000; i++)
	{
		int k = rand() % 64 + 1;
		int v = rand() % 256 + 1;
		string key = random_key(k);
		int keyLength = key.length();
		char *keyArray;
		keyArray = (char *)malloc(keyLength);

		for (int i = 0; i < keyLength; i++)
			keyArray[i] = key[i];

		string value = random_value(v);
		int valueLength = value.length();
		char *valueArray;
		valueArray = (char *)malloc(valueLength);

		for (int i = 0; i < valueLength; i++)
			valueArray[i] = value[i];

		Slice keySlice, valueSlice;
		keySlice.size = (uint8_t)keyLength;
		keySlice.data = keyArray;

		valueSlice.size = (uint8_t)valueLength;
		valueSlice.data = valueArray;

		// if(db[key].size())
		// cout<<"OVERWRITTEN: "<<key<<" "<<value<<endl;

		db.insert(pair<string, string>(key, value));
		db2[key] = value;
		kv.put(keySlice, valueSlice);
		db_size++;
	}

	// bool incorrect = false;

	// int count = 0;
	// for (int i = 0; i < 10000; i++)
	// {
	// 	int x = rand() % 5;
	// 	if (x == 0)
	// 	{
	// 		string k = random_key(10);
	// 		int keyLength = k.length();
	// 		char *keyArray;
	// 		keyArray = (char *)malloc(keyLength + 1);

	// 		strcpy(keyArray, k.c_str());

	// 		Slice keySlice, valueSlice;
	// 		keySlice.size = (uint8_t)keyLength;
	// 		keySlice.data = keyArray;

	// 		bool ans = kv.get(keySlice, valueSlice);
	// 		map<string, string>::iterator itr = db.find(k);
	// 		if ((ans == false && itr != db.end()) || (ans == true && itr == db.end()))
	// 			incorrect = true;
	// 	}

	// 	else if (x == 1)
	// 	{
	// 		int k = rand() % 64 + 1;
	// 		string key = random_key(k);
	// 		int keyLength = key.length();
	// 		char *keyArray;
	// 		keyArray = (char *)malloc(keyLength);

	// 		strcpy(keyArray, key.c_str());

	// 		int v = rand() % 256 + 1;
	// 		string value = random_value(v);
	// 		int valueLength = value.length();
	// 		char *valueArray;
	// 		valueArray = (char *)malloc(valueLength);

	// 		strcpy(valueArray, value.c_str());

	// 		Slice keySlice, valueSlice;
	// 		keySlice.size = (uint8_t)keyLength;
	// 		keySlice.data = keyArray;

	// 		valueSlice.size = (uint8_t)valueLength;
	// 		valueSlice.data = valueArray;

	// 		db.insert(pair<string, string>(key, value));
	// 		db2[key] = value;

	// 		bool check1 = kv.get(keySlice, valueSlice);
	// 		bool ans = kv.put(keySlice, valueSlice);
	// 		bool check2 = kv.get(keySlice, valueSlice);
	// 		db_size++;
	// 		if (check2 == false || check1 != ans)
	// 			incorrect = true;
	// 	}

	// 	else if (x == 2)
	// 	{
	// 		int max_size = db.size();
	// 		int rem = rand() % max_size;
	// 		map<string, string>::iterator itr = db.begin();
	// 		for (int i = 0; i < rem; i++)
	// 			itr++;
	// 		string key = itr->first;

	// 		int keyLength = key.length();
	// 		char *keyArray;
	// 		keyArray = (char *)malloc(keyLength + 1);

	// 		strcpy(keyArray, key.c_str());

	// 		Slice keySlice;
	// 		keySlice.size = (uint8_t)keyLength;
	// 		keySlice.data = keyArray;

	// 		bool check = kv.del(keySlice);
	// 		db_size--;
	// 		db.erase(itr);
	// 		bool check2 = kv.del(keySlice);
	// 		if (check2 == true)
	// 			incorrect = true;
	// 	}

	// 	else if (x == 3)
	// 	{
	// 		count++;
	// 		incorrect = false;
	// 		int max_size = db.size();
	// 		int rem = rand() % max_size;
	// 		Slice keySlice, valueSlice;
	// 		bool checkStatus = kv.get(rem + 1, keySlice, valueSlice);

	// 		pair<string, string> check = make_pair(string(keySlice.data), string(valueSlice.data));

	// 		map<string, string>::iterator itr = db.begin();
	// 		for (int i = 0; i < rem; i++)
	// 			itr++;

	// 		// if (check.first != itr->first || check.second != itr->second)
	// 		if (check.first != itr->first || check.second != db2[itr->first])
	// 		{
	// 			// cout << count << endl;
	// 			// cout << check.first << " " << itr->first << endl;
	// 			// cout << "OUR: " << check.second << " THEIR OLD: " << itr->second << endl;
	// 			// cout << "OUR: " << check.second << " THEIR NEW: " << db2[itr->first] << endl;
	// 			incorrect = true;
	// 		}
	// 	}

	// 	else if (x == 4)
	// 	{
	// 		// count++;
	// 		int max_size = db.size();
	// 		if (!max_size)
	// 			break;
	// 		int rem = rand() % max_size;
	// 		assert(max_size == db.size());
	// 		map<string, string>::iterator itr = db.begin();
	// 		for (int i = 0; i < rem; i++)
	// 			itr++;
	// 		string key = itr->first;
	// 		int keyLength = key.length();
	// 		char *keyArray;
	// 		keyArray = (char *)malloc(keyLength + 1);

	// 		strcpy(keyArray, key.c_str());

	// 		Slice keySlice, valueSlice;
	// 		keySlice.size = (uint8_t)(keyLength);
	// 		keySlice.data = keyArray;

	// 		bool check = (kv.del(rem + 1));
	// 		db.erase(itr);
	// 		db_size--;

	// 		bool check2 = kv.get(keySlice, valueSlice);

	// 		if (check2 == true)
	// 			incorrect = true;
	// 	}
	// }

	// if (incorrect == true)
	// {
	// 	cout << 0 << endl;
	// 	return 0;
	// }

	int threads = 4;

	pthread_mutex_init(&db_mutex, NULL);
	pthread_t tid[threads];
	for (int i = 0; i < threads; i++) 
	{
		tid[i] = i;
        pthread_create(&tid[i], NULL, myThreadFun, (void *)&tid[i]); 
	}

	for(int i=0;i<threads;i++)
		pthread_join(tid[i],NULL);

	pthread_mutex_destroy(&(db_mutex));
	return 0;
}