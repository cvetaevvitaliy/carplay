/*
 * Generated by plugin-codegen.
 */
#ifndef __MH_DEV_MTP_H__
#define __MH_DEV_MTP_H__

#include <gio/gio.h>
#include "macros.h"
#include <mh_dev.h>
#include <libusb.h>

/*
 * Type Macros
 */

#define MH_TYPE_DEV_MTP \
	(mh_dev_mtp_get_type())
#define MH_DEV_MTP(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), MH_TYPE_DEV_MTP, MHDevMtp))
#define MH_IS_DEV_MTP(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), MH_TYPE_DEV_MTP))
#define MH_DEV_MTP_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), MH_TYPE_DEV_MTP, MHDevMtpClass))
#define MH_IS_DEV_MTP_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), MH_TYPE_DEV_MTP))
#define MH_DEV_MTP_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), MH_TYPE_DEV_MTP, MHDevMtpClass))

typedef struct _MHDevMtp		MHDevMtp;
typedef struct _MHDevMtpClass	MHDevMtpClass;

typedef struct _mtpContainer 
{
	guint32 length;
	guint16 type;
	guint16 code;
	guint32 transactionId;
} mtpContainer;				/* ----------  end of struct mtpContainer  ---------- */

typedef struct _MHMTPStorageIDs 
{
	guint32 count;
	guint32 * ids;
} MHMTPStorageIDs;				/* ----------  end of struct MHMTPStorageIDs  ---------- */

typedef struct _MHMTPStorageInfo 
{
	guint16 storageType;
	guint16 fsType;
	guint16 accessCapability;
	guint64 maxCapability;
	guint64 freeSpaceInBytes;
	guint32 freeSpaceInObjects;
	gchar * storageDescription;
	gchar * volumeLable;
} MHMTPStorageInfo;				/* ----------  end of struct MHMTPStorageInfo  ---------- */

typedef struct _MHMTPDevInfo 
{
	guint16 standardVersion;
	guint32 mtpVendorExtId;
	guint16 mtpVersion;
	gchar * mtpExtensions;
	guint16 functionalMode;
	guint16 * operationSupported;
	guint16 * eventsSupported;
	guint16 * devicePropertiesSupported;
	guint16 * captureFormats;
	guint16 * playbackFormats;
	gchar * manufacturer;
	gchar * model;
	gchar * deviceVersion;
	gchar * serialNumber;

	/* Storage Info */
	MHMTPStorageInfo ** storageInfos;
} MHMTPDevInfo;				/* ----------  end of struct MHMTPDevInfo  ---------- */

typedef struct _MHMTPPropInfo 
{
	guint16 prop_code;
	guint16 datatype;
	guint8 	get_set;
} MHMTPPropInfo;				/* ----------  end of struct MHMTPPropInfo  ---------- */

typedef struct _MHMTPPropInfos 
{
	guint32 size;
	MHMTPPropInfo * info;
} MHMTPPropInfos;				/* ----------  end of struct MHMTPPropInfos  ---------- */

typedef struct _MHMTPObjHandles 
{
	guint32 size;
	guint32 * handles;
} MHMTPObjHandles;				/* ----------  end of struct MHMTPObjHandles  ---------- */

struct _MHDevMtpClass
{
	MHDevClass parent_class;

	/* Class Ios */

	/* Class Methods */

	/* Class Properties */
};

struct _MHDevMtp
{
	MHDev parent;

	/* Instance Members */
	MHMTPDevInfo * devInfo;
	MHMTPStorageIDs * storageIds;

	libusb_device_handle * handle;
	libusb_device * device;

	guint8 intEp;
	guint8 bulkInEp;
	guint16 maxPacketSize;
	guint8 bulkOutEp;

	gboolean unlocked;
	gboolean compatilibility;

	GList * mtpFds;
	
	MHFolder * root;
	
	bool first_music_flag;
	bool first_movie_flag;
	bool first_picture_flag;

	GHashTable * DbHash;
	GHashTable * MtpHash;
};

/* used by MH_TYPE_DEV_MTP */
GType mh_dev_mtp_get_type( void );

/* Ios List */



#endif /* __MH_DEV_MTP_H__ */
