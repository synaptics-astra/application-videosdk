/* SPDX-License-Identifier: Apache-2.0 */
/* Copyright 2024 Synaptics Incorporated */

#ifdef __cplusplus
extern "C" {
#endif

/** Remove all debug output lines
 */
#define DEB_LEV_NO_OUTPUT  0
/** Messages explaing the reason of critical errors
 */
#define DEB_LEV_ERR        1
/** Messages showing values related to the test and the component/s used
 */
#define DEB_LEV_PARAMS     2
/** Messages representing steps in the execution. These are the simple messages, because
 * they avoid iterations
 */
#define DEB_LEV_SIMPLE_SEQ 4
/** Messages representing steps in the execution. All the steps are described,
 * also with iterations. With this level of output the performance is
 * seriously compromised
 */
#define DEB_LEV_FULL_SEQ   8
/** Messages that indicate the beginning and the end of a function.
 * It can be used to trace the execution
 */
#define DEB_LEV_FUNCTION_NAME 16

/** Messages that are the default test application output. These message should be
	* shown every time
	*/
#define DEFAULT_MESSAGES 32

/** All the messages - max value
 */
#define DEB_ALL_MESS   255


/** \def DEBUG_LEVEL is the current level do debug output on standard err */
//#define DEBUG_LEVEL (DEB_LEV_ERR | DEFAULT_MESSAGES | DEB_LEV_FUNCTION_NAME)
#define DEBUG_LEVEL (DEB_LEV_ERR | DEFAULT_MESSAGES )
//#define DEBUG_LEVEL 0
#if DEBUG_LEVEL > 0
#define DEBUG(n, args...) do { if (DEBUG_LEVEL & (n)){fprintf(stderr, "QtRenderingServer: " args);} } while (0)
#else
#define DEBUG(n, args...)
#endif

#define LOG_ERR(args...) DEBUG(DEB_LEV_ERR, args)
#define LOG_PARAM(args...) DEBUG(DEB_LEV_PARAMS, args)
#define LOG_SEQ(args...) DEBUG(DEB_LEV_SIMPLE_SEQ, args)
#define LOG_SEQ_FULL(args...) DEBUD(DEB_LEV_FULL_SEQ, args)
#define LOG_FUNC(args...) DEBUG(DEB_LEV_FUNCTION_NAME, args)
#define LOG_DEF(args...) DEBUG(DEFAULT_MESSAGES, args)

#ifdef __cplusplus
}
#endif
