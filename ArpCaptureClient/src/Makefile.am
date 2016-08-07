bin_PROGRAMS=ArpCaptureClient
AM_CPPFLAGS=
ArpCaptureClient_DEPENDENCIES=
ArpCaptureClient_SOURCES=ArpCaptureEngine.cpp CapturePacketThread.cpp    DaemonProcess.cpp\
                    Definitions.cpp   LoadConfigThread.cpp  main.cpp\
                    RegisterArpCaptureThread.cpp  Util.cpp Session.cpp
ArpCaptureClient_LDFLAGS=-I../include \
						-I/usr/local/include  \
						-DLDAP_DEPRECATED=1 \
						-lpthread \
						-lev \
						-lpcap \ 
						-ljsoncpp \
						-lcpp_common \
						-L/usr/local/lib/

