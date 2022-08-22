/**
    \addtogroup automotive_api_ipod_drm
    Automotive API of iPod.
*/
/*@{*/
/***************************************************************************//**
    \file       ama_ipod_drm.c
    \brief      Automotive API implementation of ipod drm.
    \author     tangwei
    \version    0.3.0
    \date       2013-8-7
    \warning    Copyright (C), 2013, Neusoft.\n
    \remarks    Revision History\n
------------------------------------------------------------------------------\n
    No.    Author        Date        Version    Description\n
------------------------------------------------------------------------------\n
    001    tangwei      2013-8-7      0.3.0        Create.\n
*******************************************************************************/

/*******************************************************************************
    Include Files
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <glib.h>
#include <unistd.h>
#include <errno.h>

#include "ama_ipod_drm.h"

/*******************************************************************************
    Macro Definition
*******************************************************************************/
#define DRM_RETRY_CNT       (3)
#define DRM_RETRY_INTERVAL  (200000) /* uS */
#define BUFFER_SIZE         2000
#define DRM_DEV_PATH        "/dev/ipod_drmic_dev"

/* ioctl() Command ID */
#if 0
#define READ_MAJOR_VERSION          (0x00)
#define READ_MINOR_VERSION          (0x01)
#define READ_AUTH_MAJOR_VERSION     (0x63)
#define READ_AUTH_MINOR_VERSION     (0x03)
#define READ_DEVICE_ID              (0x04)
#define READ_SIGNATURE_DATA         (0x12)
#define WRITE_CHALLENGE_DATA        (0x21)
#define READ_CERTIFICATE_DATA       (0x31)
#endif

enum IPOD_DRM_COMMAND {
    READ_MAJOR_VERSION          = _IO('i', 0),
    READ_MINOR_VERSION,
    READ_AUTH_MAJOR_VERSION,
    READ_AUTH_MINOR_VERSION,
    READ_DEVICE_ID,
    READ_CERTIFICATE_DATA,    
    WRITE_CHALLENGE_DATA,
    READ_SIGNATURE_DATA,
};

/*******************************************************************************
    Type Definition
*******************************************************************************/

/*******************************************************************************
    Prototype  Declaration
*******************************************************************************/
static result_t ipod_drm_init_data(void);

/*******************************************************************************
    Variable  Definition
*******************************************************************************/
static drm_ctrl_t   ipod_drm_data;      /**<Drm data of Authentication. */
static sint16       drm_fd = -1;        /**<IIC Device file descriptor. */

/*******************************************************************************
    Function  Definition
*******************************************************************************/

/***************************************************************************//**
    \fn         result_t ipod_drm_open(void)
    \brief      Open the DRM device.
    \return     E_OK : Success.\n
                Others : Error.
    \remarks    Sync/Async : Synchronous.\n
                Reentrancy : Non-Reentrant.
*******************************************************************************/
result_t ipod_drm_open(void)
{
    uint8    i;
    result_t result = E_NG;

    ipod_drm_close();

    for(i=0; i<DRM_RETRY_CNT; i++)
    {
        /* Open DRM Device */
        drm_fd = open(DRM_DEV_PATH, O_RDWR);
        if (drm_fd < 0)
        {
            usleep(DRM_RETRY_INTERVAL);
        }
        else
        {
            result = E_OK;
            break;
        }
    }

    if(E_OK == result)
    {
        /* Get data */
        for(i=0; i<DRM_RETRY_CNT ; i++)
        {
            result = ipod_drm_init_data();
            if(E_OK != result)
            {
                /* Get data failed, retry */
                usleep(DRM_RETRY_INTERVAL);
            }
            else
            {
                break;
            }
        }
    }

    return result;
}

/***************************************************************************//**
    \fn         void ipod_drm_close(void)
    \brief      Close the DRM device.
    \return     E_OK : Success.\n
                Others : Error.
    \remarks    Sync/Async : Synchronous.\n
                Reentrancy : Non-Reentrant.
*******************************************************************************/
void ipod_drm_close(void)
{
    if(drm_fd >= 0)
    {
        close(drm_fd);
        drm_fd = -1;
    }
}

/***************************************************************************//**
    \fn         result_t ipod_drm_init_data(void)
    \brief      DRM data initialization.
    \return     E_OK : Success.\n
                Others : Error.
    \remarks    Sync/Async : Synchronous.\n
                Reentrancy : Non-Reentrant.
*******************************************************************************/
static result_t ipod_drm_init_data(void)
{
    result_t    result;
    char_t      buffer[BUFFER_SIZE];
    sint16      status;


    (void)memset(&ipod_drm_data, 0, sizeof(drm_ctrl_t));
    ipod_drm_data.device_id = 0xFFFFFFFF;

    /****************************************/
    /* DRM Device Open                      */
    /****************************************/
    if (drm_fd < 0)
    {
        result = ipod_drm_open();
    }
    else
    {
        result = E_OK;
    }

#if 0
    /****************************************/
    /* Major Version Read                    */
    /****************************************/
    if(E_OK == result)
    {
        (void)memset(buffer, 0, BUFFER_SIZE);
        status = ioctl(drm_fd, READ_MAJOR_VERSION, buffer);
        if (status <0)
        {
            result = E_NG;
        }
        else
        {
            ipod_drm_data.major_version =(uint8)buffer[2];
            result = E_OK;
        }
    }

    /****************************************/
    /* Minor Version Read                    */
    /****************************************/
    if(E_OK == result)
    {
        (void)memset(buffer, 0, BUFFER_SIZE);
        status = ioctl(drm_fd, READ_MINOR_VERSION, buffer);
        if (status <0)
        {
            result = E_NG;
        }
        else
        {
            ipod_drm_data.minor_version =(uint8)buffer[2];
            result = E_OK;
        }
    }
#endif
    /************************************************/
    /* Authentication Protocol Major Version Read    */
    /************************************************/
    if(E_OK == result)
    {
        (void)memset(buffer, 0, BUFFER_SIZE);
        status = ioctl(drm_fd, READ_AUTH_MAJOR_VERSION, buffer);
        if (status <0)
        {
            result = E_NG;
        }
        else
        {
            ipod_drm_data.auth_major_version =(uint8)buffer[2];
            result = E_OK;
        }
    }

    /************************************************/
    /* Authentication Protocol Minor Version Read    */
    /************************************************/
    if(E_OK == result)
    {
        (void)memset(buffer, 0, BUFFER_SIZE);
        status = ioctl(drm_fd, READ_AUTH_MINOR_VERSION, buffer);
        if (status <0)
        {
            result = E_NG;
        }
        else
        {
            ipod_drm_data.auth_minor_version =(uint8)buffer[2];
            result = E_OK;
        }
    }

    /****************************************/
    /* Device ID Read                        */
    /****************************************/
    if(E_OK == result)
    {
        (void)memset(buffer, 0, BUFFER_SIZE);
        status = ioctl(drm_fd, READ_DEVICE_ID, buffer);
        if (status <0)
        {
            result = E_NG;
        }
        else
        {
            ipod_drm_data.device_id =(uint32)(buffer[2]<<24 | buffer[3]<<16 | buffer[4]<<8 | buffer[5]);
            result = E_OK;
        }
    }

    /****************************************/
    /* Certificate Data Read                */
    /****************************************/
    if(E_OK == result)
    {
        (void)memset(buffer, 0, BUFFER_SIZE);
        status = ioctl(drm_fd, READ_CERTIFICATE_DATA, buffer);
        if (status <0)
        {
            result = E_NG;
        }
        else
        {
            ipod_drm_data.certificate_length =(uint16)(buffer[0]<<8 | buffer[1]);
            (void)memcpy(ipod_drm_data.certificate_data ,&buffer[2], ipod_drm_data.certificate_length);
            result = E_OK;
        }
    }

    return result;
}

/***************************************************************************//**
    \fn         result_t ipod_drm_write_challenge_data(void)
    \brief      write challenge data to CP.
    \return     E_OK : Success.\n
                Others : Error.
    \remarks    Sync/Async : Synchronous.\n
                Reentrancy : Non-Reentrant.
*******************************************************************************/
result_t ipod_drm_write_challenge_data(void)
{
    result_t    result;
    char_t      buffer[BUFFER_SIZE];
    sint16      status;

    if (drm_fd < 0)
    {
        result = E_NG;
    }
    else
    {
        result = E_OK;
    }

    /****************************************/
    /* Write Challenge Data                 */
    /****************************************/
    if(E_OK == result)
    {
        (void)memset(buffer, 0, BUFFER_SIZE);
        buffer[0] = (char_t)(ipod_drm_data.challenge_length>>8);
        buffer[1] = (char_t)(ipod_drm_data.challenge_length);
        (void)memcpy(&buffer[2], ipod_drm_data.challenge_data, ipod_drm_data.challenge_length);

        status = ioctl(drm_fd, WRITE_CHALLENGE_DATA, buffer);
        if (status <0)
        {
            result = E_NG;
        }
        else
        {
            result = E_OK;
        }
    }

    return result;
}

/***************************************************************************//**
    \fn         result_t ipod_drm_read_signature_data(void)
    \brief      read authentication signature data from CP.
    \return     E_OK : Success.\n
                Others : Error.
    \remarks    Sync/Async : Synchronous.\n
                Reentrancy : Non-Reentrant.
*******************************************************************************/
result_t ipod_drm_read_signature_data(void)
{
    result_t    result;
    char_t      buffer[BUFFER_SIZE];
    sint16      status;

    if (drm_fd < 0)
    {
        result = E_NG;
    }
    else
    {
        result = E_OK;
    }

    /****************************************/
    /* Read Signature Data                  */
    /****************************************/
    if(E_OK == result)
    {
        (void)memset(buffer, 0, BUFFER_SIZE);
        status = ioctl(drm_fd, READ_SIGNATURE_DATA, buffer);
        if (status <0)
        {
            result = E_NG;
        }
        else
        {
            ipod_drm_data.signature_length = (uint16)(buffer[0]<<8 | buffer[1]);
            (void)memcpy(ipod_drm_data.signature_data, &buffer[2], ipod_drm_data.signature_length);
            result = E_OK;
        }
    }

    return result;
}

/***************************************************************************//**
    \fn         void ipod_drm_get_data(drm_ctrl_t **drm_data)
    \brief      Get the drm data
    \param[out] drm_data : Drm data.\n
    \return     E_OK : Success.\n
                Others : Error.
    \remarks    Sync/Async : Synchronous.\n
                Reentrancy : Non-Reentrant.
*******************************************************************************/
void ipod_drm_get_data(drm_ctrl_t **drm_data)
{
    *drm_data = &ipod_drm_data;
}

#if 0
/***************************************************************************//**
    \fn         void ipod_drm_get_drm_version(iap_version_t *version)
    \brief      Get the device and firmware version.
    \param[out] options_p : Drm version.\n
    \return     E_OK : Success.\n
                Others : Error.
    \remarks    Sync/Async : Synchronous.\n
                Reentrancy : Non-Reentrant.
*******************************************************************************/
void ipod_drm_get_drm_version(iap_version_t *version)
{
    version->major = ipod_drm_data.major_version;
    version->minor = ipod_drm_data.minor_version;
}
#endif

/*@}*/
