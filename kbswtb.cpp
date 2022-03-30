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

namespace kb{
	typedef std::vector<std::string> string_vector;
	typedef std::pair<std::string,std::string> layout_variant_strings;
	class XKeyboard{
		public:
			Display* _display;
			int _deviceId;
			XkbDescRec* _kbdDescPtr;
			size_t _verbose;

			XKeyboard(size_t verbose)
				: _display(0), _deviceId(XkbUseCoreKbd), _kbdDescPtr(0), _verbose(verbose){
			}
			~XKeyboard();

			// Opens display (or throw std::runtime_error)
			int open_display(void);

			// Gets the current layout
			int get_group() const;

			// Sets the layout
			void set_group(int num);

			// Return layout/variant strings
			layout_variant_strings get_layout_variant();

			// Returns keyboard layout string
			void build_layout_from(string_vector& vec, const layout_variant_strings& lv);
			void build_layout(string_vector& vec);

			// Waits for kb event
			void wait_event();
	};

	int XKeyboard::open_display(){
		XkbIgnoreExtension(False);

		char* displayName = strdup(""); // allocates memory for string!
		int eventCode;
		int errorReturn;
		int major = XkbMajorVersion;
		int minor = XkbMinorVersion;
		int reasonReturn;

		_display = XkbOpenDisplay(displayName, &eventCode, &errorReturn, &major,
									&minor, &reasonReturn);
		free(displayName);
		
		if (reasonReturn!=XkbOD_Success)
			goto vrati;
		
		_kbdDescPtr = XkbAllocKeyboard();

		_kbdDescPtr->dpy = _display;
		if (_deviceId != XkbUseCoreKbd) {
			_kbdDescPtr->device_spec = _deviceId;
		}
		
		vrati:
		return reasonReturn;
	}

	XKeyboard::~XKeyboard(){
		if(_kbdDescPtr!=NULL)
			XkbFreeKeyboard(_kbdDescPtr, 0, True);

		if (_display!=NULL) {
			XCloseDisplay(_display);
		}
	}

	// XkbRF_VarDefsRec contains heap-allocated C strings, but doesn't provide a
	// direct cleanup method. This wrapper privides a workaround.
	// See also https://gitlab.freedesktop.org/xorg/lib/libxkbfile/issues/6
	struct XkbRF_VarDefsRec_wrapper{
		XkbRF_VarDefsRec _it;

		XkbRF_VarDefsRec_wrapper() {
			std::memset(&_it,0,sizeof(_it));
		}

		~XkbRF_VarDefsRec_wrapper(){
			if(_it.model) std::free(_it.model);
			if(_it.layout) std::free(_it.layout);
			if(_it.variant) std::free(_it.variant);
			if(_it.options) std::free(_it.options);
		}
	};

	layout_variant_strings XKeyboard::get_layout_variant(){
		XkbRF_VarDefsRec_wrapper vdr;
		char* tmp = NULL;
		//Bool bret =

		XkbRF_GetNamesProp(_display, &tmp, &vdr._it);
		free(tmp);  // return memory allocated by XkbRF_GetNamesProp

		return make_pair(string(vdr._it.layout ? vdr._it.layout : "us"),
							string(vdr._it.variant ? vdr._it.variant : ""));
	}

	void XKeyboard::build_layout_from(string_vector& out, const layout_variant_strings& lv){
		std::istringstream layout(lv.first);
		std::istringstream variant(lv.second);
		while(true){
			string l,v;
			getline(layout, l, ',');
			getline(variant, v, ',');
			if(!layout && !variant)
				break;
			if(v!="")
				v = "(" + v + ")";
			if(l!="")
				out.push_back(l+v);
		}
	}

	void XKeyboard::build_layout(string_vector& out){
		layout_variant_strings lv=this->get_layout_variant();
		build_layout_from(out, lv);
	}

	void XKeyboard::wait_event(){
		//Bool bret =
		XkbSelectEventDetails(_display, XkbUseCoreKbd,
		XkbStateNotify, XkbAllStateComponentsMask, XkbGroupStateMask);

		XEvent event;
		//int iret =
		XNextEvent(_display, &event);
	}

	void XKeyboard::set_group(int groupNum){
		//Bool result =
		XkbLockGroup(_display, _deviceId, groupNum);
		XFlush(_display);
	}

	int XKeyboard::get_group() const {
		XkbStateRec xkbState;
		XkbGetState(_display, _deviceId, &xkbState);
		return static_cast<int>(xkbState.group);
	}

	// returns true if symbol is ok
	bool filter(const string_vector& nonsyms, const std::string& symbol){
		if(symbol.empty())
			return false;

		// Filter out all prohibited words
		string_vector::const_iterator r = find(nonsyms.begin(), nonsyms.end(), symbol);
		if(r != nonsyms.end())
			return false;

		// Filter out all numbers groups started with number
		if(isdigit(symbol[0]))
			return false;

		return true;
	}
}

int main(){
	kb::string_vector syms;
	kb::XKeyboard xkb(1);
	xkb.open_display();
	struct stat buffer;
	int postojanje=stat("/tmp/i3statusP",&buffer);
	printf("%d\n",postojanje);
	if(postojanje!=0)
		mkfifo("/tmp/i3statusP",0666);
	int fdw=open("/tmp/i3statusP",O_RDWR|O_NONBLOCK);
	const char *nru="U\n";
	while(true){
		xkb.wait_event();
		//xkb.build_layout(syms);
		//cout << syms.at(xkb.get_group()) << endl;
		write(fdw,nru,2);
	}
	return 0;
}
