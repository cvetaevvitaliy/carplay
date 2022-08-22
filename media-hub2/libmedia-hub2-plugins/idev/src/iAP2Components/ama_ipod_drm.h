/**
    \addtogroup automotive_api_ipod_drm
    Automotive API of iPod.
*/
/*@{*/

/*******************************************************************************//**
\file       ama_ipod_drm.h
\brief      Core of ipod drm.
\author     tangwei
\version    0.3.0
\date       2013-8-6
\warning    Copyright   (C),  2013,  Neusoft.\n
\remarks    Revision   History\n
--------------------------------------------------------------------------------\n
  No.   Author      Date             Version    Description\n
--------------------------------------------------------------------------------\n
  01    tangwei     2013-08-06        0.3.0      Create.\n
********************************************************************************/

#ifndef        _AMA_IPOD_DRM_H
#define        _AMA_IPOD_DRM_H

/*******************************************************************************
    Include Files
*******************************************************************************/
#include <ama_types.h>
#include <ama_error.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
    Macro Definition
*******************************************************************************/
#define SZ_CHALLENGE_LEN     20
#define SZ_SIGNATURE_LEN     128
#define SZ_CERTIFICATE_LEN   1920

/*******************************************************************************
    Type Definition
*******************************************************************************/
/**
    \struct drm_ctrl_t
    Drm data structure.
*/
typedef struct {
//  uint8   major_version;                        /**<Device version.         */
//  uint8   minor_version;                        /**<Firmware version.       */
    uint8   auth_major_version;                   /**<protocol major version. */
    uint8   auth_minor_version;                   /**<protocol minor version. */
    uint32  device_id;                            /**<Device ID.              */
    uint16  challenge_length;                     /**<Challenge data length.  */
    uint8   challenge_data[SZ_CHALLENGE_LEN];     /**<Challenge data.         */
    uint16  signature_length;                     /**<Signature data length.  */
    uint8   signature_data[SZ_SIGNATURE_LEN];     /**<Signature data.         */
    uint16  certificate_length;                   /**<Certificate data length.*/
    uint8   certificate_data[SZ_CERTIFICATE_LEN]; /**<Certificate data.       */
} drm_ctrl_t;

/*******************************************************************************
    Prototype  Declaration
*******************************************************************************/
extern result_t ipod_drm_open(void);
extern void     ipod_drm_close(void);
extern result_t ipod_drm_write_challenge_data(void);
extern result_t ipod_drm_read_signature_data(void);
extern void     ipod_drm_get_data(drm_ctrl_t **);
//extern void   ipod_drm_get_drm_version(iap_version_t *);

#ifdef __cplusplus
}
#endif
#endif  /* _AMA_IPOD_DRM_H */
/*@}*/
