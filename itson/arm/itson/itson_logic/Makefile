THIS_DIR=$(realpath $(dir $(lastword $(MAKEFILE_LIST))))

ifeq ($(TARGET_BUILD_VARIANT),user)
$(warning Stripping debug info)
EXTRA_LDFLAGS += --strip-debug -x
endif

ifeq ($(TARGET_PRODUCT),full_huawei8160)
EXTRA_CFLAGS += -DFULL_HUAWEI8160
$(warning Using FULL_HUAWEI8160 memory configuration)
endif

ifeq ($(TARGET_PRODUCT),full_marvel)
EXTRA_CFLAGS += -DFULL_MARVEL
$(warning Using FULL_MARVEL memory configuration)
endif

ifeq ($(TARGET_PRODUCT),samsung_d2vmu)
EXTRA_CFLAGS += -DSAMSUNG_D2VMU
$(warning Using GALAXY S3 memory configuration)
endif

ifeq ($(TARGET_PRODUCT),samsung_goghvmu)
EXTRA_CFLAGS += -DSAMSUNG_D2VMU
$(warning Using Victory memory configuration)
endif


ifeq ($(DO_PURGE_LOG),1)
EXTRA_CFLAGS += -DPURGE_LOG
$(warning Purging all logs)
endif


ifneq (${ITSON_UID},)
$(warning ItsOn UID is ${ITSON_UID})
else
$(error ITSON_UID is not set)
endif

EXTRA_CFLAGS += -DCRYPTO_AUTH=none

$(warning FLAGS is ${EXTRA_CFLAGS})

shipped-objs := processing.o l7_id.o l7_parser.o l3_parser.o \
	l4_parser.o l7_parser_http.o l7_parser_ssl.o dn_uid_map.o dn_ip_map.o \
	dn_cache.o metadata.o filter_store.o flow_mgr.o flow_stat.o buf_pool.o \
	kernel_comm.o classify.o associative.o linux_comm.o ceaa_comm.o \
	notification.o iface_map.o app_state.o enforce.o logging.o regulate.o \
	tether_mgr.o uid_cache.o runtime_state.o proc_files.o ns_parse.o mtregex.o

module2-objs := itson_filter.o $(shipped-objs)

obj-$(CONFIG_ITSON_LOGIC) += module2.o


clean-files := *.ko *.o *.mod.c modules.order .*.cmd ext/*.o

clean:
	rm -f *.ko *.o *.mod.c modules.order .*.cmd
	rm -f ext/*.o
