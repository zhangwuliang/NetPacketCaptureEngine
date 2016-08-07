#include "CapturePacketThread.h"

#include "Definitions.h"
#include "Util.h"
#include <cpp_common/BaseLock.h>
#include "ArpPacketDefinitions.h"
#include "DaemonProcess.h"

#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <arpa/inet.h>
#include <unistd.h>

namespace ARP_CAPTURE_CLIENT
{

//the length of the packet should be captured
const int MAX_CAPATURE_LENGTH           = 65535;
//set the mode of network card promiscuous
const int PROMISC                       = 1;
//the seconds of pcap timeout
const int PCAP_TIME_OUT                 = 10;

//The ether code of IPV4
const unsigned short IPV4_ETHER_CODE    = 0x0800;
//the ether code  of ARP
const unsigned short ARP_EHTER_CODE     = 0x0806;
//the ether code of 802.1q
const unsigned short VLAN_EHTER_CODE    = 0x8100;

//The len of ether data packet header
const int ETHER_HEADER_LEN              = 14;
//The len of VLAN ether header data
const int VLAN_ETHER_HEADER_LEN         = 18;
//The len of IP data packet header
const int IP_HEADER_LEN                 = 20;
//The len of TCP data packet header
const int TCP_HEADER_LEN                = 20;

//the code in arp packet, indicate that it is a arp request
const int ARP_REQUEST_CODE              = 1;
//the code in arp packet, indicate that it is a arp reply
const int ARP_REPLY_CODE                = 2;

//ip is active or not
const int IP_IS_ACTIVE                  = 1;
const int IP_IS_OFFLINE                 = 0;

const int BUF_SIZE = 4096;

BaseLock                            g_deviceLock;

/**
* store all the devs info
*/
std::map<u_int32_t, DEV_INFO>           g_dev_infos;

extern DaemonProcess* g_DaemonProcess;

static void ArpPacketHandler(unsigned char *argument,const struct pcap_pkthdr* pcap_header,const unsigned char *packet_content);

static int DecodeArpPacket(ARP_HEADER *arp_header, timeval capture_time, u_int16_t vlanID);

static void threadExit(struct ev_loop * loop, ev_async *watcher, int revents)
{
	CapturePacketThread *pThread = (CapturePacketThread*)watcher->data;
	if (pThread != NULL)
	{
		pThread->Stop();
	}
}

static void ArpPacketHandler(unsigned char *argument,const struct pcap_pkthdr* pcap_header,const unsigned char *packet_content)
{
	timeval captureTime = pcap_header->ts;
	ETHER_HEADER* etherHeader = (ETHER_HEADER*)packet_content;
	u_int16_t etherType = ntohs(etherHeader->ether_proto_type);

	 
    g_log.Log(DEBUG, "[%s-%d-%s]: capture packet time %ld:%ld, ether_type=%02x, ether_src_mac=%02x:%02x:%02x:%02x:%02x:%02x, ether_dest_mac=%02x:%02x:%02x:%02x:%02x:%02x ",
    __FILE__, __LINE__, __FUNCTION__, captureTime.tv_sec, captureTime.tv_usec, etherType,
    etherHeader->ether_src_mac[0], etherHeader->ether_src_mac[1], etherHeader->ether_src_mac[2],
    etherHeader->ether_src_mac[3],etherHeader->ether_src_mac[4], etherHeader->ether_src_mac[5],
    etherHeader->ether_dest_mac[0], etherHeader->ether_dest_mac[1], etherHeader->ether_dest_mac[2],
    etherHeader->ether_dest_mac[3],etherHeader->ether_dest_mac[4], etherHeader->ether_dest_mac[5]);

	ARP_HEADER *arpHeader = NULL;
	u_int16_t vlan_id = 0;

	if (VLAN_EHTER_CODE == etherType) //VLAN packeet
	{
		vlan_id = ((VLAN_PACKET*)packet_content)->vlan_id;
		g_log.Log(DEBUG, "[%s-%d-%s]: vlanId=%d", __FILE__, __LINE__, __FUNCTION__, vlan_id);

		arpHeader = (ARP_HEADER*)(packet_content+VLAN_ETHER_HEADER_LEN);
	}
	else if (ARP_EHTER_CODE == etherType) //ARP Packet
	{
		arpHeader = (ARP_HEADER*)(packet_content+ETHER_HEADER_LEN);
	}
	else
	{
		//log warning the packet can not be recogrnitioned
        g_log.Log(WARNING, "[%s-%d-%s]: the packet can not be recogrnitioned, it's code is %d", __FILE__, __LINE__, __FUNCTION__, etherType);
        return;
	}

	//decode Arp Packet
	if (NULL != arpHeader)
	{
		DecodeArpPacket(arpHeader, captureTime, vlan_id);
	}

	sleep(1);
	
}


static int DecodeArpPacket(ARP_HEADER *arp_header, timeval capture_time, u_int16_t vlanID)
{	
	if (NULL == arp_header)	
	{
		g_log.Log(ERROR, "[%s-%d-%s]: the arp header is null", __FILE__, __LINE__, __FUNCTION__);
        return RET_ERROR;
	}

	if (ntohs(arp_header->arp_operation_code) == ARP_REQUEST_CODE)
	{
		u_int8_t* mac = arp_header->arp_source_ethernet_address;

		DEV_INFO devInfo;
		memset(&devInfo, 0, sizeof(DEV_INFO));

		devInfo.active = 1;
		devInfo.find_time = capture_time;
		devInfo.dev_ip = arp_header->arp_source_ip_address;
		devInfo.vlan = vlanID;
		memcpy(&(devInfo.dev_mac), mac, sizeof(u_int8_t)*6);

		{
			BSLock bsLock(g_deviceLock);
			std::map<u_int32_t, DEV_INFO>::iterator iter = g_dev_infos.find(arp_header->arp_source_ip_address);
			if (iter != g_dev_infos.end())
			{
				g_dev_infos.erase(iter);
			}
			g_dev_infos.insert(std::make_pair(arp_header->arp_source_ip_address, devInfo));
		}

		//send to server
		CmdArpCaptureData arpCaptureData;
		Util::SetCmdHead(&arpCaptureData.cmdHead, BS_CMD_ARPCAPTURE_DATA, 0, RST_SUCCESS, CMD_ARPCAPTURE_DATA_LEN);
		arpCaptureData.arpCaptureData.active = 1;
		arpCaptureData.arpCaptureData.vlanID = vlanID;
		sprintf(arpCaptureData.arpCaptureData.ip, NIPQUAD_FMT, NIPQUAD(arp_header->arp_source_ip_address));
		sprintf(arpCaptureData.arpCaptureData.mac, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

		g_DaemonProcess->m_arpCaptureEngine.m_arpSession.pushData2WriteQueue((char*)(&arpCaptureData), CMD_ARPCAPTURE_DATA_LEN);
		
	}

	return RET_SUCCESS;
	
}

CapturePacketThread::CapturePacketThread():m_capture(false), m_PcapHandle(NULL), m_loop(NULL)
{
}


CapturePacketThread::~CapturePacketThread()
{
}


int CapturePacketThread::Init(const std::string& interface)
{
	if (interface.empty())
	{
		return RET_ERROR;
	}

	struct bpf_program pcapProgram;
	char filterBuf[BUF_SIZE] = {0};
	char errBuf[BUF_SIZE]    = {0};

	m_PcapHandle = pcap_open_live(interface.c_str(), MAX_CAPATURE_LENGTH, PROMISC, PCAP_TIME_OUT, errBuf);
	if (NULL == m_PcapHandle)
	{
		g_log.Log(ERROR, "[%s-%d-%s]: Init capture interface: %s failed.", __FILE__, __LINE__, __FUNCTION__, errBuf);
        return RET_ERROR;
	}

	//0x0800 IPv4, 0x0806 ARP,  0x8100 802.1q
	sprintf(filterBuf, "(ether proto 0x0806) or ((ether proto 0x8100) and (ether[16:2] = 0x0806))");
	if ( -1 == pcap_compile(m_PcapHandle, &pcapProgram, filterBuf, 0, 0) )
	{
		g_log.Log(WARNING, "[%s-%d-%s]: interface:%s compile pcap filter failed.", __FILE__, __LINE__, __FUNCTION__, interface.c_str());
        pcap_close(m_PcapHandle);
        return RET_ERROR;
	}

	if (-1 == pcap_setfilter(m_PcapHandle, &pcapProgram))
	{
		g_log.Log(WARNING, "[%s-%d-%s]: interface:%s set pcap filter failed.", __FILE__, __LINE__, __FUNCTION__, interface.c_str());
        pcap_close(m_PcapHandle);
        return RET_ERROR;
	}

	//create event loop
	m_loop = ev_loop_new(EVFLAG_AUTO);
	if (NULL == m_loop)
	{
		return RET_ERROR;
	}

	m_exit_notify.data = this;
	ev_async_init(&m_exit_notify, threadExit);
	ev_async_start(m_loop, &m_exit_notify);
	
}

int CapturePacketThread::UnInit()
{
	if (m_capture)
	{
		if (ev_is_active(&m_exit_notify))
		{
			ev_async_stop(m_loop, &m_exit_notify);
		}

		if (m_loop != NULL)
		{
			ev_loop_destroy(m_loop);
		}

		if (NULL != m_PcapHandle)
		{
			pcap_close(m_PcapHandle);
		}

		m_capture = false;
	}

	return RET_SUCCESS;
}

int CapturePacketThread::ThreadMain(void* pArg)
{	
	m_capture = true;

	do
	{
		int id = 10;
		pcap_loop(m_PcapHandle, -1, ArpPacketHandler, (u_char*)&id);
	}while(0);

	return RET_SUCCESS;
}

int CapturePacketThread::StopThread(void)
{
	if (m_capture)
	{
		ev_async_send(m_loop, &m_exit_notify);

		if (NULL != m_PcapHandle)
		{
			pcap_breakloop(m_PcapHandle);
		}

		void *pReturnValue = 0;
		return pthread_join(GetThreadID(), &pReturnValue);
	}

	return RET_SUCCESS;
}

	

}

