/*
 * Copyright (c) 2004 Topspin Communications. All rights reserved.
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <endian.h>

#include <infiniband/verbs.h>

static const char *transport_str(enum ibv_transport_type transport)
{
	switch (transport) {
	case IBV_TRANSPORT_IB:
		return "InfiniBand";
	case IBV_TRANSPORT_IWARP:
		return "iWARP";
	case IBV_TRANSPORT_USNIC:
		return "usNIC";
	case IBV_TRANSPORT_USNIC_UDP:
		return "usNIC UDP";
	case IBV_TRANSPORT_UNSPECIFIED:
		return "unspecified";
	default:
		return "invalid transport";
	}
}

static const char *port_state_str(enum ibv_port_state state)
{
	switch (state) {
	case IBV_PORT_DOWN:
		return "DOWN";
	case IBV_PORT_INIT:
		return "INIT";
	case IBV_PORT_ARMED:
		return "ARMED";
	case IBV_PORT_ACTIVE:
		return "ACTIVE";
	default:
		return "UNKNOWN";
	}
}

static const char *link_layer_str(uint8_t link_layer)
{
	switch (link_layer) {
	case IBV_LINK_LAYER_INFINIBAND:
		return "InfiniBand";
	case IBV_LINK_LAYER_ETHERNET:
		return "Ethernet";
	case IBV_LINK_LAYER_UNSPECIFIED:
		return "Unspecified";
	default:
		return "Unknown";
	}
}

static const char *mtu_str(enum ibv_mtu mtu)
{
	switch (mtu) {
	case IBV_MTU_256:
		return "256";
	case IBV_MTU_512:
		return "512";
	case IBV_MTU_1024:
		return "1024";
	case IBV_MTU_2048:
		return "2048";
	case IBV_MTU_4096:
		return "4096";
	default:
		return "unknown";
	}
}

static const char *width_str(uint8_t width)
{
	switch (width) {
	case 1:
		return "1X";
	case 2:
		return "4X";
	case 4:
		return "8X";
	case 8:
		return "12X";
	case 16:
		return "2X";
	default:
		return "unknown";
	}
}

static const char *speed_str(uint32_t speed)
{
	switch (speed) {
	case 1:
		return "2.5 Gbps";
	case 2:
		return "5.0 Gbps";
	case 4:
	case 8:
		return "10.0 Gbps";
	case 16:
		return "14.0 Gbps";
	case 32:
		return "25.0 Gbps";
	case 64:
		return "50.0 Gbps";
	case 128:
		return "100.0 Gbps";
	case 256:
		return "200.0 Gbps";
	default:
		return "unknown";
	}
}

static void format_guid(__be64 guid_be, char *buffer, size_t buffer_size)
{
	uint64_t guid = be64toh(guid_be);

	snprintf(buffer, buffer_size,
	         "%04x:%04x:%04x:%04x",
	         (unsigned int)((guid >> 48) & 0xffff),
	         (unsigned int)((guid >> 32) & 0xffff),
	         (unsigned int)((guid >> 16) & 0xffff),
	         (unsigned int)(guid & 0xffff));
}

int main(void)
{
	struct ibv_device **device_list;
	int num_devices;

	device_list = ibv_get_device_list(&num_devices);
	if (device_list == NULL) {
		perror("ibv_get_device_list");
		return EXIT_FAILURE;
	}

	printf("Found %d RDMA device%s\n\n",
	       num_devices,
	       num_devices == 1 ? "" : "s");

	for (int i = 0; i < num_devices; ++i) {
		struct ibv_device *device = device_list[i];
		struct ibv_context *context = NULL;
		struct ibv_device_attr_ex device_attr = {};
		char node_guid[32];
		char system_guid[32];

		context = ibv_open_device(device);
		if (context == NULL) {
			fprintf(stderr,
			        "Failed to open device %s\n",
			        ibv_get_device_name(device));
			continue;
		}

		if (ibv_query_device_ex(context, NULL, &device_attr) != 0) {
			fprintf(stderr,
			        "Failed to query device %s\n",
			        ibv_get_device_name(device));
			ibv_close_device(context);
			continue;
		}

		format_guid(device_attr.orig_attr.node_guid,
		            node_guid, sizeof(node_guid));
		format_guid(device_attr.orig_attr.sys_image_guid,
		            system_guid, sizeof(system_guid));

		printf("Device %d\n", i);
		printf("  Name:              %s\n",
		       ibv_get_device_name(device));
		printf("  Transport:         %s\n",
		       transport_str(device->transport_type));

		if (device_attr.orig_attr.fw_ver[0] != '\0') {
			printf("  Firmware version:  %s\n",
			       device_attr.orig_attr.fw_ver);
		}

		printf("  Node GUID:         %s\n", node_guid);
		printf("  System image GUID: %s\n", system_guid);
		printf("  Vendor ID:         0x%04x\n",
		       device_attr.orig_attr.vendor_id);
		printf("  Vendor part ID:    %u\n",
		       device_attr.orig_attr.vendor_part_id);
		printf("  Hardware version:  0x%x\n",
		       device_attr.orig_attr.hw_ver);
		printf("  Physical ports:    %u\n",
		       device_attr.orig_attr.phys_port_cnt);
		printf("  Completion vectors:%d\n",
		       context->num_comp_vectors);

		printf("  Max QPs:           %d\n",
		       device_attr.orig_attr.max_qp);
		printf("  Max QP WRs:        %d\n",
		       device_attr.orig_attr.max_qp_wr);
		printf("  Max CQ entries:    %d\n",
		       device_attr.orig_attr.max_cqe);
		printf("  Max MRs:           %d\n",
		       device_attr.orig_attr.max_mr);
		printf("  Max PDs:           %d\n",
		       device_attr.orig_attr.max_pd);

		for (uint32_t port = 1;
		     port <= device_attr.orig_attr.phys_port_cnt;
		     ++port) {
			struct ibv_port_attr port_attr;

			if (ibv_query_port(context, port, &port_attr) != 0) {
				fprintf(stderr,
				        "Failed to query port %u on %s\n",
				        port,
				        ibv_get_device_name(device));
				continue;
			}

			uint32_t active_speed =
				port_attr.active_speed_ex
					? port_attr.active_speed_ex
					: port_attr.active_speed;

			printf("\n  Port %u\n", port);
			printf("    State:           %s\n",
			       port_state_str(port_attr.state));
			printf("    Link layer:      %s\n",
			       link_layer_str(port_attr.link_layer));
			printf("    LID:             %u\n",
			       port_attr.lid);
			printf("    SM LID:          %u\n",
			       port_attr.sm_lid);
			printf("    Active MTU:      %s bytes\n",
			       mtu_str(port_attr.active_mtu));
			printf("    Maximum MTU:     %s bytes\n",
			       mtu_str(port_attr.max_mtu));
			printf("    Active width:    %s\n",
			       width_str(port_attr.active_width));
			printf("    Active speed:    %s\n",
			       speed_str(active_speed));
			printf("    GID table size:  %d\n",
			       port_attr.gid_tbl_len);
		}

		putchar('\n');

		if (ibv_close_device(context) != 0) {
			fprintf(stderr,
			        "Failed to close device %s\n",
			        ibv_get_device_name(device));
		}
	}

	ibv_free_device_list(device_list);
	return EXIT_SUCCESS;
}