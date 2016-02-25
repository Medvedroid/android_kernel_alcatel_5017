/*
 *
 *	Copyright (c) 2010, ItsOn, Inc.
 *
 * 	Author: Lisa Stark-Berryman
 *	ItsOn netfilter module.
 *
 */

//#define MODULE
#include <linux/module.h>
#include <linux/moduleparam.h>
#include "../platform.h"
#include "../ItsOnAPI.h"
#include "itson_public.h"

//MODULE_LICENSE("ItsOn, Inc.");
MODULE_LICENSE("Proprietary");
MODULE_AUTHOR("Author");
MODULE_DESCRIPTION("Module");

#undef DBG
#define DBG(X...) LOG_LOCAL_DEBUG(X)
#undef INFO
#define INFO(X...) LOG_LOCAL_INFO(X)
#undef ERR
#define ERR(X...) LOG_LOCAL_ERROR(X)


int initial_service_mode = PROCESSING_MODE_NO_SVC_FAIL_CLOSED;
module_param(initial_service_mode, int, 0000);


int ModuleInit2(void)
{        
	int ret = 0;

	// Enable all debug log

#if 0
	logging_set_mask(LOGGING_ENABLE_ALL_MASK);
	logging_set_mask(logging_create_mask(comp_dn_ip_map));
	logging_set_mask(logging_create_mask(comp_dn_cache) |
					 logging_create_mask(comp_metadata) |
					 logging_create_mask(comp_flow_mgr) |
					 logging_create_mask(comp_l7_parser));

	logging_set_mask(logging_create_mask(comp_classify) |
				logging_create_mask(comp_enforce)|
				logging_create_mask(comp_processing)
				);
	logging_set_mask(logging_create_mask(comp_ceaa_comm));
#endif

	logging_set_mask(LOGGING_DISABLE_ALL_MASK);

	INFO("--------- Initializing logic module -------\n");
	INFO("initial_service_mode %d\n", initial_service_mode);
	runtime_state_set_initial_service_mode(initial_service_mode);

	if (0 != buf_pool_init()) {
		ERR("Could not initialize buffer pools");
		return -ENOMEM;
	}
	if (0 != l7_id_init()) {
		ERR("Could not initialize app id");
		return -ENOMEM;
	}
	if (0 != l7_parser_http_init()) {
		ERR("Could not initialize l7 http parser");
		return -ENOMEM;
	}

	if (0 != dn_cache_init()) {
		ERR("Could not initialize domain name cache");
		return -ENOMEM;
	}

	if (0 != flow_mgr_init() ){
		ERR("ITSON: Could not initialize flow manager\n");
		return -ENOMEM;
	}

	if (0 != dn_uid_map_init()) {
		ERR("Could not initialize uid map");
		return -ENOMEM;
	}

	if (0 != dn_ip_map_init()) {
		ERR("Could not initialize ip map");
		return -ENOMEM;
	}

	if (0 != metadata_init()) {
		ERR("Could not initialize metadata store api");
		return -ENOMEM;
	}

	if (0 != processing_init()) {
		ERR("Could not initialize processing api");
		return -ENOMEM;
	}

	if (0 != classify_init()) {
			ERR("Could not initialize classify api");
		return -ENOMEM;
	}

	if (0 != assc_init()) {
		ERR("Could not initialize associative");
		return -ENOMEM;
	}

#ifndef __LINUX_UNITTESTS__
	if (0 != linux_comm_init()) {
		ERR("Could not initialize communication");
		return -ENOMEM;
	}
#endif

	if (0 != ceaa_comm_init()) {
		ERR("Could not initialize communication");
		return -ENOMEM;
	}

	if (0 != tether_mgr_init()) {
		ERR("Could not initialize tether manager");
		return -ENOMEM;
	}

	 INFO("done initializing modules");


	if (0 !=  throttle_init()) {
		ERR("Could not initialize throttle");
		return -ENOMEM;
	}

	if (0 != notification_init()) {
		ERR("Could not initialize notification");
		return -ENOMEM;
	}

	if (0 != uid_cache_init()) {
		ERR("Could not initialize uid cache");
		return -ENOMEM;
	}

	INFO("++++++++ Initializing logic module - DONE ++++++++\n");

	ret = ItsOnInitCEA(process_pak_shim);
	if (ret)
		return ret;

	 INFO("ITSON: registered netlink successfully\n");

	// Allocate storage for filters
	if (filter_store_init(0) != 0) {
		ERR("Couldn't allocate memory for filters\n");
		return -ENOMEM;
	}

	// Set up the /proc files.
	if (proc_files_init() != 0) {
		ERR("Couldn't set up /proc files\n");
		return -ENOMEM;
	}

	return ret;
}

void ModuleShutdown2(void)
{
	DBG("removing itson_login\n");
	ItsOnExitCEA();
	
	proc_files_destroy();
	
	ceaa_comm_stop_stats_worker();

	// Clean up modules.
	ceaa_comm_destroy();
#ifndef __LINUX_UNITTESTS__
	linux_comm_destroy();
#endif
	filter_store_destroy();
	throttle_destroy();
	dn_cache_destroy();
	dn_ip_map_destroy();
}

module_init(ModuleInit2);
module_exit(ModuleShutdown2);

