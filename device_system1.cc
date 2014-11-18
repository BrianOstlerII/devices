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
class Device;
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
	Device* driver;
	string* bytes;
};

vector<Inode> ilist;
vector<Device*> drivers;

class Device: public Monitor {
	public:
	Condition ok2read;
	Condition ok2write;
	Inode* inode;
	bool readable;
	bool writeable;
	int deviceNumber;
	string driverName;
	Device(string driverName)
	:Monitor(),ok2read(this),
	ok2write(this),deviceNumber(drivers.size()),
	driverName( driverName )
	{
		//Added this line, since inode was never initialized.
		inode = new Inode;
		inode->driver = this;
		drivers.push_back(this);
		++inode->openCount;
	}
	~Device() {
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

class iostreamDevice : Device {
	public:
	int inodeCount = 0;
	int openCount = 0;
	iostream* bytes;
	iostreamDevice( iostream* io )
	: bytes(io), Device("iostreamDevice")
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

class stringstreamDevice : Device {
	public:
	int inodeCount = 0;
	int openCount = 0;
	stringstream* bytes;
	stringstreamDevice( stringstream* ss )
	: bytes(ss), Device("stringstreamDevice")
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
		++inodeCount;
		++openCount;
		return deviceNumber;
	}
	int close( int fd) {
		cout << "Closing device " << driverName << endl;
		--openCount;
		cout << "Device " << driverName << " closed\n";
		return 0;
	}
	int read( int fd, void* buf, size_t count) {
		Device* ssd = drivers[fd];
		cerr << "Beginning read from device " << ssd->driverName << endl;
		char* s = (char*) buf;
		int i = 0;
		for( ; i < count; i++ )	
		{
			*(s+i) = bytes->get();
		}
		return i;
	}
	int write( int fd, void* buf, size_t count) {
		Device* ssd = drivers[fd];
		cerr << "Beginning write to device " << ssd->driverName << endl;
		char* s = (char*) buf;
		int i = 0;
		for( ; i < count; i++ )	
		{
			bytes->put(*s);
			s++;
		}
		return i;
	}
	int seek( int fd, off_t offset, int whence) {
		if( whence == ios_base::SEEK_SET )
		{
			bytes->seekg(offset,beg);
			bytes->seekp(offset,beg);
		}
		else if( whence == ios_base::SEEK_CUR )
		{
			bytes->seekg(offset,cur);
			bytes->seekp(offset,cur);
		}
		else if( whence == ios_base::SEEK_END )
		{
			bytes->seekg(offset,end)
			bytes->seekp(offset,end)
		}
		return offset;
	}
	int rewind( int pos ) {
		seek( deviceNumber, 0, SEEK_CUR);
	}
	/*
	int ioctl( ) {
	}
	//*/
};

int main() {
	stringstream* ss = new stringstream;
	stringstreamDevice* ssDevice = new stringstreamDevice(ss);
	int ssDeviceFd = -1;
	char writeBuf[14] = "Hello, world!";
	char* readBuf = new char[14];
	
	// Open a file to test our input and output.
	ssDeviceFd = ssDevice->open("writeTest.txt", 2);

	assert( ssDeviceFd != -1 );
	cerr << "Write test stream opened, fd = " << ssDeviceFd << "\n";
	
	// Read the following data into a Device.
	ssDevice->write(ssDeviceFd, writeBuf, 14);
	cerr << "Buffer contents written to write-test stream\n"
		 << "Bytes: " << (ssDevice->bytes)->str() << endl;

	// Close the file when finished.
	ssDeviceFd =  (!ssDevice->close(ssDeviceFd))? -1 : ssDeviceFd;
	assert( ssDeviceFd == -1 );
	cout << "Write test stream closed\n";
	
	// If the file was successfully written to, we can use it to test read.
	ssDeviceFd = ssDevice->open("writeTest.txt", 2);
	*ss << "Hello, world!";
	ssDevice->write(ssDeviceFd, writeBuf, 14);
	cout << "Stream contents written to read-test stream\n";
	ssDevice->read(ssDeviceFd, readBuf, 14);
	cout << "Contents stored in buffer: " << readBuf << endl;
	
}
