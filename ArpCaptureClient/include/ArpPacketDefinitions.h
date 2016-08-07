#ifndef __ARP_PACKET_DEFINITIONS_H__
#define __ARP_PACKET_DEFINITIONS_H__

namespace ARP_CAPTURE_CLIENT
{
	//The struct of erther header
	typedef struct _ether_header
	{
		u_int8_t ether_dest_mac[6];  //Destionation MAC address
		u_int8_t ether_src_mac[6];   //Source MAC address
		u_int16_t ether_proto_type;  //Protocol type
	}__attribute__((packed)) ETHER_HEADER, P_ETHER_HEADER;

	//the struct of arp header
	typedef struct _arp_header
	{
		u_int16_t arp_hardware_type;
		u_int16_t arp_protocol_type;
		u_int8_t  arp_hardware_length;
		u_int8_t  arp_protocol_length;
		u_int16_t arp_operation_code;
		u_int8_t  arp_source_ethernet_address[6];
		u_int32_t arp_source_ip_address;
		u_int8_t  arp_destination_ethernet_address[6];
		u_int32_t arp_destination_ip_address;
	}__attribute__((packed)) ARP_HEADER, *P_ARP_HEADER;

	//the struct of vlan packet
	typedef struct _vlan_packet
	{
		u_int8_t	vlan_dest_mac[6];
		u_int8_t	vlan_src_mac[6];
		u_int16_t   vlan_ptotocol;
		u_int16_t   vlan_id;
		u_int16_t   vlan_ptotocol_type; //0x800=TYPE_IP;0x806=TYPE_ARP;0x835=TYPE_RARP
	}__attribute__((packed)) VLAN_PACKET, *P_VLAN_PACKET;
	
}

#endif
