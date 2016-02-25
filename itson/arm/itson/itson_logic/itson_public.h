/*
 * itson_public.h -- public declarations from module 2 used by
 * itson_filter.c.
 *
 *  Created on: Feb 6, 2015
 *      Author: Ian Smith
 */

#ifndef ITSON_PUBLIC_H_
#define ITSON_PUBLIC_H_

#ifdef __cplusplus
extern "C" {
#endif


typedef enum itson_component_t_ {
	INVALID_COMPONENT = -1,
#define DEFINE_COMP(id) \
    comp_##id,
#include "components.h"
#undef DEFINE_COMP
    MAX_COMPONENT,
} itson_component_t;

typedef enum log_severity_t_ {
	LOG_SERV_FATAL,
	LOG_SERV_ERROR,
	LOG_SERV_WARNING,
	LOG_SERV_INFO,
	LOG_SERV_DEBUG
} log_severity_t;

#define LOGGING_ENABLE_ALL_MASK (0xffffffff)
#define LOGGING_DISABLE_ALL_MASK (0x0)



#ifndef PURGE_LOG
#define LOG_ERROR(component, X...) itson_log(component, LOG_SERV_ERROR, __FILE__,__LINE__,__func__, X)
#define LOG_INFO(component, X...) itson_log(component, LOG_SERV_INFO, __FILE__,__LINE__,__func__, X)
#define LOG_DEBUG(component, X...) itson_log(component, LOG_SERV_DEBUG, __FILE__,__LINE__,__func__, X)
#endif

/* These macro should be used when we want to log w/o going through the message queue */
#ifdef PURGE_LOG
/* No Logging */
#define LOG_ERROR(component, X...) ;
#define LOG_INFO(component, X...) ;
#define LOG_DEBUG(component, X...) ;

#define LOG_LOCAL_ERROR(X...) ;
#define LOG_LOCAL_INFO(X...) ;
#define LOG_LOCAL_DEBUG(X...) ;

#elif defined(__KERNEL__)
/* Kernel immediate logging */
#define LOG_LOCAL_ERROR(X...) itson_printf(X)
#define LOG_LOCAL_INFO(X...) itson_printf(X)
#define LOG_LOCAL_DEBUG(X...) ;
#elif defined (ANDROID)
/* Android immediate logging */
#include <android/log.h>
#define LOG_TAG "ItsOn"
#define LOG_LOCAL_ERROR(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__))
#define LOG_LOCAL_INFO(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__))
//#define LOG_LOCAL_DEBUG(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__))
#define LOG_LOCAL_DEBUG(...) ;
#else // PURGE_LOG

/* X86 immediate logging */
#define LOG_LOCAL_DEBUG(X...) ;
#define LOG_LOCAL_INFO(X...) 	printf(X)
#define LOG_LOCAL_ERROR(X...) printf(X)
#endif // PURGE_LOG


extern void itson_log(itson_component_t component, log_severity_t severity,
                      const char *file, int line,
                      const char *func, const char *format, ...);
extern void logging_set_mask(uint64_t new_mask);
extern uint64_t logging_create_mask(itson_component_t comp);
extern int logging_is_enable(itson_component_t component);
extern int logging_set_debug(const char *cmd, int cmd_len);


#define PROCESSING_MODE_NO_SVC_FAIL_OPEN	1
#define PROCESSING_MODE_NO_SVC_FAIL_CLOSED	2
#define PROCESSING_MODE_SVC_PRESENT 		3

extern void runtime_state_set_initial_service_mode(uint32_t mode);


extern int buf_pool_init(void);

extern int l7_id_init(void);

extern int l7_parser_http_init(void);

extern int dn_cache_init(void);
extern int dn_cache_destroy(void);

extern int flow_mgr_init(void);

extern int dn_uid_map_init(void);

extern int dn_ip_map_init(void);
extern int dn_ip_map_destroy(void);

extern int metadata_init(void);

extern int processing_init(void);
extern int process_pak_shim(void *pak, int hook);

extern int classify_init(void);

extern int assc_init(void);

extern int linux_comm_init(void);
extern int linux_comm_destroy(void);

extern int ceaa_comm_init(void);
extern int ceaa_comm_destroy(void);
extern void ceaa_comm_stop_stats_worker(void);

extern int tether_mgr_init(void);

extern int throttle_init(void);
extern void throttle_destroy(void);

extern int notification_init(void);

extern int uid_cache_init(void);

extern int filter_store_init(uint16_t size);
extern int filter_store_destroy(void);

extern int proc_files_init(void);
extern void proc_files_destroy(void);


#ifdef __cplusplus
}
#endif

#endif /* ITSON_PUBLIC_H_ */
