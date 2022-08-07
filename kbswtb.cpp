#include <cstring>
#include <vector>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include <X11/extensions/XKBrules.h>

using namespace std;

int main(){
	
	struct stat buffer;
	int postojanje=stat("/tmp/i3statusP",&buffer);
	if(postojanje!=0)
		mkfifo("/tmp/i3statusP",0666);
	int fdw=open("/tmp/i3statusP",O_RDWR|O_NONBLOCK);
	const char *nru="U\n";
	
	char displayName[2048]={0};
	int eventCode;
	int errorReturn;
	int major = XkbMajorVersion;
	int minor = XkbMinorVersion;
	int reasonReturn;
	Display* _display=
		XkbOpenDisplay(displayName, &eventCode, &errorReturn,
			&major, &minor, &reasonReturn);
	
	XkbSelectEventDetails(_display, XkbUseCoreKbd, XkbStateNotify,
		XkbAllStateComponentsMask, XkbGroupStateMask);
	
	while(true){
		XEvent event;
		XNextEvent(_display, &event);
		
		write(fdw,nru,2);
	}
	
	return 0;
}
