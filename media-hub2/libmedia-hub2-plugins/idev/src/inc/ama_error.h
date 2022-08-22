/**
	\addtogroup automotive_api_error_code
	Automotive API error code.
	\ingroup automotive_api_base
*/
/*@{*/
/***************************************************************************//**
	\file		ama_error.h
	\brief		Definition of error code.
	\author		zhaoyg
	\version	1.0
	\date		2013-4-27
	\warning	Copyright (C), 2013, Neusoft.
	\remarks	Revision History\n
------------------------------------------------------------------------------\n
	No.	Author		Date		Version	Description\n
------------------------------------------------------------------------------\n
	001	zhaoyg		2013-4-27	1.0		Create.\n
*******************************************************************************/
#ifndef _AMA_ERROR_H
#define _AMA_ERROR_H
/*******************************************************************************
	Include Files
*******************************************************************************/
#include "ama_module_id.h"

#ifdef __cplusplus
extern "C"
{
#endif
/*******************************************************************************
	Macro Definition
*******************************************************************************/
/**
	\def _E_START(module_id)
	Error code start of specific module.
	\param[in]	module_id : Module ID.
*/
#define _E_START(module_id)		((module_id)<<8)

/*******************************************************************************
	Type Definition
*******************************************************************************/
/**
	\enum result_t
	Return code of API.
*/
typedef enum{
	E_OK = _E_START(MODULE_BASE),	/**< Success. */
	E_NG,				/**< Failure. */
	E_NOT_INIT,			/**< Not initialized. */
	E_BUSY,				/**< Busy. */
	E_NOT_IMPLEMENT,	/**< Not implement. */
	E_INVALID_PARA,		/**< Invalid parameter. */
	E_UNSUPPORTED,		/**< Unsupported. */
	E_TIME_OUT,			/**< Time out. */

	E_USB_NO_DEVICE = _E_START(MODULE_USB),	/**< No device in USB. */
	E_USB_NO_SONG,		/**< No song in USB. */
	
	E_BT_NO_DEVICE = _E_START(MODULE_BT),	/**< No blue tooth device. */
	
	E_IO_EXIST = _E_START(MODULE_IO), /**< fail, the creating item was already 
										exist or the operating item was not 
										exist */
	E_NET_DEVICE_NOT_READY,	/**< Net device not ready. */
	E_MAX				/**< Error code end. */
}result_t;

/*******************************************************************************
	Prototype  Declaration
*******************************************************************************/

#ifdef __cplusplus
}
#endif
#endif /* _AMA_ERROR_H */
/*@}*/

