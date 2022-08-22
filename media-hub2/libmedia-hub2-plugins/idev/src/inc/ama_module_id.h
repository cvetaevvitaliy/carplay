/**
	\addtogroup automotive_api_module_id
	Automotive API module id.
	\ingroup automotive_api_base
*/
/*@{*/
/***************************************************************************//**
	\file		ama_module_id.h
	\brief		Definition of module ID.
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
#ifndef _AMA_MODULE_ID_H
#define _AMA_MODULE_ID_H
/*******************************************************************************
	Include Files
*******************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif
/*******************************************************************************
	Macro Definition
*******************************************************************************/

/*******************************************************************************
	Type Definition
*******************************************************************************/
/**
	\enum module_id_t
	Module ID.
*/
typedef enum{
	MODULE_BASE = 0	,	/**< Base */
	MODULE_APP		,	/**< App */
	MODULE_APPMGR	,	/**< App Manager */
	MODULE_APPCTRL	,	/**< App Control */
	MODULE_AUD		,	/**< Audio */
	MODULE_BT		,	/**< Blue Tooth */
	MODULE_BOOT		,	/**< Bootloader */
	MODULE_DLNA		,	/**< DLNA */
	MODULE_DIAG		,	/**< Diagnostic */
	MODULE_IPOD		,	/**< iPod */
	MODULE_MRLK		,	/**< Mirror Link */
	MODULE_NET		,	/**< Network */
	MODULE_NAVI		,	/**< Navigation */
	MODULE_PWR		,	/**< Power */
	MODULE_STR		,	/**< String */
    MODULE_STT      ,   /**< STT */
	MODULE_SYS		,	/**< System */
	MODULE_TEXT		,	/**< Text */
	MODULE_TLMT		,	/**< Telematics */
	MODULE_TNR		,	/**< Tuner */
    MODULE_TTS      ,   /**< TTS */
	MODULE_UI		,	/**< User Interface */
	MODULE_USB		,	/**< USB */
	MODULE_VEHINF	,	/**< Vehicle Info */
	MODULE_VIDEO	,	/**< Video */
	MODULE_CTNS		,	/**< Contents */
	MODULE_IO		,	/**< IO */
	
	MODULE_MAX			/**< Max module ID */
}module_id_t;

/*******************************************************************************
	Prototype  Declaration
*******************************************************************************/

#ifdef __cplusplus
}
#endif
#endif /* _AMA_MODULE_ID_H */
/*@}*/

