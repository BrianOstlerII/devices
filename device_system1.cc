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

//Define flags according to the linux standard.
#define O_RDONLY	0
#define O_WRONLY	1
#define O_RDWR		2
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2


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
		//Added this line, since inode was never initialized.
		inode = new Inode;
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
	int ioctl( ) {
	}
	//*/
};

class stringstreamDevice : DeviceDriver {
	public:
	int inodeCount = 0;
	int openCount = 0;
	stringstream* bytes;
	stringstreamDevice( stringstream* ss )
	: bytes(ss), DeviceDriver("stringstreamDevice")
	{
		readable = true;
		writeable = true;
	}
	~stringstreamDevice() {
	}
	int open( const char* pathname, int flags) {
		//Unable to get rid of the error for specifying the inode kind.
		//inode->kind = device;
		inode->driver = this;
		if( flags == O_RDONLY )
		{
			readable = true;
			writeable = false;
		}	
		else if( flags == O_WRONLY )
		{	
			readable = false;
			writeable = true;
		}
		else if( flags == O_RDWR )
		{
			readable = true;
			writeable = true;
		}
		return ++openCount;
	}
	int close( int fd) {
		--openCount;
		return 0;
	}
	int read( int fd, void* buf, size_t count) {
		stringstream* data;
		*data << *(ilist[fd].bytes);
		string* s;
		int i = 0;
		for( ; i < count && !bytes->eof(); i++)
		{
			*data >> *s;
		}
		buf = s;
		return i;
	}
	int write( int fd, void* buf, size_t count) {
		stringstream* ss;
		int i = 0;
		for( ; i < count && !ss->eof(); i++ )
		{
			//Can't access the data pointed to by the void*
		}
		*(ilist[fd].bytes) += ss->str();
		return i;
	}
	int seek( int fd, off_t offset, int whence) {
		//This thing is lying to me.  It says beg, cur, and end aren't
		//declared when they clearly are in ios_base.
		//Will finish when my will returns.
/*		if( whence == SEEK_SET )
		{
			bytes->seekg(offset,beg);
		}
		else if( whence == SEEK_CUR )
		{
			bytes->seekg(offset,cur);
		}
		else if( whence == SEEK_END )
		{
			bytes->seekg(offset,end)
		}
		return bytes->tellg();*/
	}
	int rewind( int pos ) {
//		seekg( deviceNumber, pos, SEEK_SET);
	}
	/*
	int ioctl( ) {
	}
	//*/
};

int main() {
	stringstream* ssp = new stringstream;
	*ssp << "hello world" << endl;
	stringstreamDevice ssd(ssp);
	
}
