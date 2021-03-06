#include <string>
#include <iostream>
#include <vector>
#include <list>
#include <cassert>

#include <sstream>
#include <errno.h>

#include <pthread.h>
#include <semaphore.h>
#include <map>
#include <queue>
#include <iomanip>
#include <signal.h>
#include <sys/time.h>
#include <climits>
#include <mutex>
#include <condition_variable>

using namespace std;

#define EXCLUSION Sentry exclusion(this); exclusion.touch();

class DeviceDriver;

class Monitor {
  // ...
};

class Condition {
public:
  Condition( Monitor* m ) {}
  // ...
};

class Inode: Monitor {
public: 
	int linkCount = 0;
	int openCount = 0;
	bool readable = false;
	bool writable = false;
	enum Kind { regular, directory, symlink, pipe, socket, device } kind;

	time_t ctime, mtime, atime;

	~Inode() {
	}

	int unlink() {
		assert( linkCount > 0);
		--linkCount;
		cleanup();	
	}
	
	void cleanup() {
		if (! openCount && !linkCount ) {
			//throwaway stuff
		}
	}
	DeviceDriver* driver;
	string* bytes;
};

vector<Inode> ilist;
vector<DeviceDriver*> drivers;

class DeviceDriver: public Monitor {
	public:
		Condition ok2read;
		Condition ok2write;
		Inode* inode;
		bool readable;
		bool writeable;
		int deviceNumber;
		string driverName;

	DeviceDriver(string driverName)
		:Monitor(),ok2read(this),
		ok2write(this),deviceNumber(drivers.size()),
		driverName( driverName )
		{
			drivers.push_back(this);
			++inode->openCount;
		}

	~DeviceDriver() {
		--inode->openCount;
	}

	virtual int read() {}
	virtual int write() {}
	virtual int seek() {}
	virtual int rewind() {}
	virtual int ioctl() {}
	virtual void online() {}
	virtual void offline() {}
	virtual void fireup() {}
	virtual void suspend() {}

};



class iostreamDevice : DeviceDriver {

	public: 
		int inodeCount = 0;
		int openCount = 0;
		
		iostream* bytes;

		iostreamDevice( iostream* io )
			: bytes(io), DeviceDriver("iostreamDevice")
		{
			readable = true;
			writeable = true;
		}

		~iostreamDevice() {
		}

		int open( const char* pathname, int flags) {
			
		}

		int close( int fd) {

		}

		int read( int fd, void* buf, size_t count) {

		}

		int write( int fd, void* buf, size_t count) {

		}

		int seek( int fd, off_t offset, int whence) {

		}

		int rewind( int pos ) {

		}

		/*
		int ioctl(  ) {

		}
		//*/
};

int main() {}
