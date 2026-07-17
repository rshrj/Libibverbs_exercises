/*
 * Copyright (c) 2026 Rishi Raj. All rights reserved.
 */

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <infiniband/verbs.h>
#include <infiniband/arch.h>

static const char *node_type_str(enum ibv_node_type type)
{
	switch (type) {
	case IBV_NODE_CA:
		return "Channel Adapter";
	case IBV_NODE_SWITCH:
		return "Switch";
	case IBV_NODE_ROUTER:
		return "Router";
	case IBV_NODE_RNIC:
		return "RNIC";
	case IBV_NODE_USNIC:
		return "usNIC";
	case IBV_NODE_USNIC_UDP:
		return "usNIC UDP";
	default:
		return "Unknown";
	}
}

static const char *transport_type_str(enum ibv_transport_type type)
{
	switch (type) {
	case IBV_TRANSPORT_IB:
		return "InfiniBand";
	case IBV_TRANSPORT_IWARP:
		return "iWARP";
	case IBV_TRANSPORT_USNIC:
		return "usNIC";
	case IBV_TRANSPORT_USNIC_UDP:
		return "usNIC UDP";
	case IBV_TRANSPORT_UNSPECIFIED:
	default:
		return "Unspecified";
	}
}

int main(void)
{
	struct ibv_device **dev_list;
	int num_devices;

	dev_list = ibv_get_device_list(&num_devices);
	if (!dev_list) {
		perror("Failed to get RDMA device list");
		return EXIT_FAILURE;
	}

	printf("Found %d RDMA device(s)\n\n", num_devices);

	for (int i = 0; i < num_devices; ++i) {
		struct ibv_device *device = dev_list[i];
		struct ibv_context *context;
		struct ibv_device_attr attr;

		printf("Device %d\n", i);
		printf("  Name:              %s\n",
		       ibv_get_device_name(device));
		printf("  Node type:         %s\n",
		       node_type_str(ibv_get_device_node_type(device)));
		printf("  Transport:         %s\n",
		       transport_type_str(ibv_get_device_transport_type(device)));
		printf("  Node GUID:         %016llx\n",
		       (unsigned long long)
		       ntohll(ibv_get_device_guid(device)));

		context = ibv_open_device(device);
		if (!context) {
			perror("  Failed to open device");
			putchar('\n');
			continue;
		}

		if (ibv_query_device(context, &attr)) {
			perror("  Failed to query device");
			ibv_close_device(context);
			putchar('\n');
			continue;
		}

		printf("  Firmware version:  %s\n", attr.fw_ver);
		printf("  System image GUID: %016llx\n",
		       (unsigned long long) attr.sys_image_guid);
		printf("  Vendor ID:         0x%06x\n", attr.vendor_id);
		printf("  Vendor part ID:    %u\n", attr.vendor_part_id);
		printf("  Hardware version:  %u\n", attr.hw_ver);
		printf("  Physical ports:    %u\n", attr.phys_port_cnt);
		printf("  Max QPs:           %d\n", attr.max_qp);
		printf("  Max QP WRs:        %d\n", attr.max_qp_wr);
		printf("  Max CQ entries:    %d\n", attr.max_cqe);
		printf("  Max MRs:           %d\n", attr.max_mr);
		printf("  Max PDs:           %d\n", attr.max_pd);

		ibv_close_device(context);
		putchar('\n');
	}

	ibv_free_device_list(dev_list);
	return EXIT_SUCCESS;
}