/*
 * =====================================================================================
 *
 *       Filename:  dev_mtp.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/05/2015 05:38:43 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#ifndef __MACROS_H__
#define __MACROS_H__

/* USB container types */

#define USB_CONTAINER_UNDEFINED		0x0000
#define USB_CONTAINER_COMMAND		0x0001
#define USB_CONTAINER_DATA			0x0002
#define USB_CONTAINER_RESPONSE		0x0003
#define USB_CONTAINER_EVENT			0x0004

/* Operation Codes */

/* PTP v1.0 operation codes */
#define OC_UNDEFINED                0x1000
#define OC_GETDEVICEINFO            0x1001
#define OC_OPENSESSION              0x1002
#define OC_CLOSESESSION             0x1003
#define OC_GETSTORAGEIDS            0x1004
#define OC_GETSTORAGEINFO           0x1005
#define OC_GETNUMOBJECTS            0x1006
#define OC_GETOBJECTHANDLES         0x1007
#define OC_GETOBJECTINFO            0x1008
#define OC_GETOBJECT                0x1009
#define OC_GETTHUMB                 0x100A
#define OC_DELETEOBJECT             0x100B
#define OC_SENDOBJECTINFO           0x100C
#define OC_SENDOBJECT               0x100D
#define OC_INITIATECAPTURE          0x100E
#define OC_FORMATSTORE              0x100F
#define OC_RESETDEVICE              0x1010
#define OC_SELFTEST                 0x1011
#define OC_SETOBJECTPROTECTION      0x1012
#define OC_POWERDOWN                0x1013
#define OC_GETDEVICEPROPDESC        0x1014
#define OC_GETDEVICEPROPVALUE       0x1015
#define OC_SETDEVICEPROPVALUE       0x1016
#define OC_RESETDEVICEPROPVALUe     0x1017
#define OC_TERMINATEOPENCAPTURe     0x1018
#define OC_MOVEOBJECT               0x1019
#define OC_COPYOBJECT               0x101A
#define OC_GETPARTIALOBJECT         0x101B
#define OC_INITIATEOPENCAPTURE      0x101C
#define OC_GETOBJECTPROPSSUPPORTED  0x9801
#define OC_GETOBJECTPROPDESC		0x9802
#define OC_GETOBJECTPROPVALUE		0x9803
#define OC_SETOBJECTPROPVALUE		0x9804
#define OC_GETOBJECTREFERENCES 		0x9810
#define OC_SETOBJECTREFERENCES		0x9811
#define OC_SKIP						0x9820
#define EOC_GETOBJECTPROPLIST		0x9805
#define EOC_SETOBJECTPROPLIST		0x9806
#define EOC_GETINTERDEPENDENTPROPDESC 0x9807
#define EOC_SENDOBJECTPROPLIST		0x9808

/* PTP v1.1 operation codes */
#define OC_STARTENUMHANDLES			0x101D
#define OC_ENUMHANDLES				0x101E
#define OC_STOPENUMHANDLES			0x101F
#define OC_GETVENDOREXTENSIONMAPS	0x1020
#define OC_GETVENDORDEVICEINFO		0x1021
#define OC_GETRESIZEDIMAGEOBJECT	0x1022
#define OC_GETFILESYSTEMMANIFEST	0x1023
#define OC_GETSTREAMINFO			0x1024
#define OC_GETSTREAM				0x1025

/* Response Codes */

/* PTP v1.0 response codes */
#define RC_UNDEFINED                0x2000
#define RC_OK                       0x2001
#define RC_GENERALERROR             0x2002
#define RC_SESSIONNOTOPEN           0x2003
#define RC_INVALIDTRANSACTIONID     0x2004
#define RC_OPERATIONNOTSUPPORTED    0x2005
#define RC_PARAMETERNOTSUPPORTED    0x2006
#define RC_INCOMPLETETRANSFER       0x2007
#define RC_INVALIDSTORAGEID         0x2008
#define RC_INVALIDOBJECTHANDLE      0x2009
#define RC_DEVICEPROPNOTSUPPORTED   0x200A
#define RC_INVALIDOBJECTFORMATCODE  0x200B
#define RC_STOREFULL                0x200C
#define RC_OBJECTWRITEPROTECTED     0x200D
#define RC_STOREREADONLY            0x200E
#define RC_ACCESSDENIED             0x200F
#define RC_NOTHUMBNAILPRESENT       0x2010
#define RC_SELFTESTFAILED           0x2011
#define RC_PARTIALDELETION          0x2012
#define RC_STORENOTAVAILABLE        0x2013
#define RC_SPECIFICATIONBYFORMATUNSUPPORTED         0x2014
#define RC_NOVALIDOBJECTINFO        0x2015
#define RC_INVALIDCODEFORMAT        0x2016
#define RC_UNKNOWNVENDORCODE        0x2017
#define RC_CAPTUREALREADYTERMINATED 0x2018
#define RC_DEVICEBUSY               0x2019
#define RC_INVALIDPARENTOBJECT      0x201A
#define RC_INVALIDDEVICEPROPFORMAT  0x201B
#define RC_INVALIDDEVICEPROPVALUE   0x201C
#define RC_INVALIDPARAMETER         0x201D
#define RC_SESSIONALREADYOPENED     0x201E
#define RC_TRANSACTIONCANCELED      0x201F
#define RC_SPECIFICATIONOFDESTINATIONUNSUPPORTED            0x2020
/* PTP v1.1 response codes */
#define RC_INVALIDENUMHANDLE		0x2021
#define RC_NOSTREAMENABLED			0x2022
#define RC_INVALIDDATASET			0x2023

#define OFC_UNDEFINED				0x3000
#define OFC_DEFINED					0x3800
#define OFC_ASSOCIATION				0x3001
#define OFC_SCRIPT					0x3002
#define OFC_EXECUTABLE				0x3003
#define OFC_TEXT					0x3004
#define OFC_HTML					0x3005
#define OFC_DPOF					0x3006
#define OFC_AIFF	 				0x3007
#define OFC_WAV						0x3008
#define OFC_MP3						0x3009
#define OFC_AVI						0x300A
#define OFC_MPEG					0x300B
#define OFC_ASF						0x300C
#define OFC_QT						0x300D /* guessing */
/* image formats */
#define OFC_EXIF_JPEG				0x3801
#define OFC_TIFF_EP					0x3802
#define OFC_FLASHPIX				0x3803
#define OFC_BMP						0x3804
#define OFC_CIFF					0x3805
#define OFC_UNDEFINED_0X3806		0x3806
#define OFC_GIF						0x3807
#define OFC_JFIF					0x3808
#define OFC_PCD						0x3809
#define OFC_PICT					0x380A
#define OFC_PNG						0x380B
#define OFC_UNDEFINED_0X380C		0x380C
#define OFC_TIFF					0x380D
#define OFC_TIFF_IT					0x380E
#define OFC_JP2						0x380F
#define OFC_JPX						0x3810
/* ptp v1.1 has only DNG new */
#define OFC_DNG						0x3811
/* Eastman Kodak extension ancillary format */
#define OFC_EK_M3U					0xb002
/* Canon extension */
#define OFC_CANON_CRW				0xb101
#define OFC_CANON_CRW3				0xb103
#define OFC_CANON_MOV				0xb104
#define OFC_CANON_MOV2				0xb105
/* CHDK specific raw mode */
#define OFC_CANON_CHDK_CRW			0xb1ff
/* Sony */
#define OFC_SONY_RAW				0xb101
/* MTP extensions */
#define OFC_MTP_MEDIACARD			0xB211
#define OFC_MTP_MEDIACARDGROUP		0xB212
#define OFC_MTP_ENCOUNTER			0xB213
#define OFC_MTP_ENCOUNTERBOX		0xB214
#define OFC_MTP_M4A					0xB215
#define OFC_MTP_ZUNEUNDEFINED		0xB217 /* Unknown file type */
#define OFC_MTP_FIRMWARE			0xB802
#define OFC_MTP_WINDOWSIMAGEFORMAT	0xB881
#define OFC_MTP_UNDEFINEDAUDIO		0xB900
#define OFC_MTP_WMA					0xB901
#define OFC_MTP_OGG					0xB902
#define OFC_MTP_AAC					0xB903
#define OFC_MTP_AUDIBLECODEC		0xB904
#define OFC_MTP_FLAC				0xB906
#define OFC_MTP_SAMSUNGPLAYLIST		0xB909
#define OFC_MTP_UNDEFINEDVIDEO		0xB980
#define OFC_MTP_WMV					0xB981
#define OFC_MTP_MP4					0xB982
#define OFC_MTP_MP2					0xB983
#define OFC_MTP_3GP					0xB984
#define OFC_MTP_UNDEFINEDCOLLECTION		0xBA00
#define OFC_MTP_ABSTRACTMULTIMEDIAALbum	0xBA01
#define OFC_MTP_ABSTRACTIMAGEALBUM		0xBA02
#define OFC_MTP_ABSTRACTAUDIOALBUM		0xBA03
#define OFC_MTP_ABSTRACTVIDEOALBUM		0xBA04
#define OFC_MTP_ABSTRACTAUDIOVIDEOPLaylist	0xBA05
#define OFC_MTP_ABSTRACTCONTACTGROUP		0xBA06
#define OFC_MTP_ABSTRACTMESSAGEFOLDEr		0xBA07
#define OFC_MTP_ABSTRACTCHAPTEREDPROduction	0xBA08
#define OFC_MTP_ABSTRACTAUDIOPLAYLISt		0xBA09
#define OFC_MTP_ABSTRACTVIDEOPLAYLISt		0xBA0A
#define OFC_MTP_ABSTRACTMEDIACAST			0xBA0B
#define OFC_MTP_WPLPLAYLIST				0xBA10
#define OFC_MTP_M3UPLAYLIST				0xBA11
#define OFC_MTP_MPLPLAYLIST				0xBA12
#define OFC_MTP_ASXPLAYLIST				0xBA13
#define OFC_MTP_PLSPLAYLIST				0xBA14
#define OFC_MTP_UNDEFINEDDOCUMENT		0xBA80
#define OFC_MTP_ABSTRACTDOCUMENT		0xBA81
#define OFC_MTP_XMLDOCUMENT				0xBA82
#define OFC_MTP_MSWORDDOCUMENT			0xBA83
#define OFC_MTP_MHTCOMPILEDHTMLDOCUMENT	0xBA84
#define OFC_MTP_MSEXCELSPREADSHEETXLS	0xBA85
#define OFC_MTP_MSPOWERPOINTPRESENTATIONPPT	0xBA86
#define OFC_MTP_UNDEFINEDMESSAGE		0xBB00
#define OFC_MTP_ABSTRACTMESSAGE			0xBB01
#define OFC_MTP_UNDEFINEDCONTACT		0xBB80
#define OFC_MTP_ABSTRACTCONTACT			0xBB81
#define OFC_MTP_VCARD2				0xBB82
#define OFC_MTP_VCARD3				0xBB83
#define OFC_MTP_UNDEFINEDCALENDARITEM	0xBE00
#define OFC_MTP_ABSTRACTCALENDARITEM	0xBE01
#define OFC_MTP_VCALENDAR1			0xBE02
#define OFC_MTP_VCALENDAR2			0xBE03
#define OFC_MTP_UNDEFINEDWINDOWSEXECUTABLE	0xbe80
#define OFC_MTP_MEDIACAST			0xBE81
#define OFC_MTP_SECTION				0xBE82

#define EC_UNDEFINED		0x4000
#define EC_CANCELTRANSACTION	0x4001
#define EC_OBJECTADDED		0x4002
#define EC_OBJECTREMOVED		0x4003
#define EC_STOREADDED		0x4004
#define EC_STOREREMOVED		0x4005
#define EC_DEVICEPROPCHANGED	0x4006
#define EC_OBJECTINFOCHANGED	0x4007
#define EC_DEVICEINFOCHANGED	0x4008
#define EC_REQUESTOBJECTTRANSFER	0x4009
#define EC_STOREFULL		0x400A
#define EC_DEVICERESET		0x400B
#define EC_STORAGEINFOCHANGED	0x400C
#define EC_CAPTURECOMPLETE		0x400D
#define EC_UNREPORTEDSTATUS		0x400E


/* DataType Codes */

#define DTC_UNDEF		0x0000
#define DTC_INT8		0x0001
#define DTC_UINT8		0x0002
#define DTC_INT16		0x0003
#define DTC_UINT16		0x0004
#define DTC_INT32		0x0005
#define DTC_UINT32		0x0006
#define DTC_INT64		0x0007
#define DTC_UINT64		0x0008
#define DTC_INT128		0x0009
#define DTC_UINT128		0x000A

#define DTC_ARRAY_MASK	0x4000

#define DTC_AINT8		(DTC_ARRAY_MASK | DTC_INT8)
#define DTC_AUINT8		(DTC_ARRAY_MASK | DTC_UINT8)
#define DTC_AINT16		(DTC_ARRAY_MASK | DTC_INT16)
#define DTC_AUINT16		(DTC_ARRAY_MASK | DTC_UINT16)
#define DTC_AINT32		(DTC_ARRAY_MASK | DTC_INT32)
#define DTC_AUINT32		(DTC_ARRAY_MASK | DTC_UINT32)
#define DTC_AINT64		(DTC_ARRAY_MASK | DTC_INT64)
#define DTC_AUINT64		(DTC_ARRAY_MASK | DTC_UINT64)
#define DTC_AINT128		(DTC_ARRAY_MASK | DTC_INT128)
#define DTC_AUINT128	(DTC_ARRAY_MASK | DTC_UINT128)

#define DTC_STR		0xFFFF

/* MTP specific Object Properties */
#define OPC_StorageID				0xDC01
#define OPC_ObjectFormat			0xDC02
#define OPC_ProtectionStatus		0xDC03
#define OPC_ObjectSize				0xDC04
#define OPC_AssociationType			0xDC05
#define OPC_AssociationDesc			0xDC06
#define OPC_ObjectFileName			0xDC07
#define OPC_DateCreated				0xDC08
#define OPC_DateModified			0xDC09
#define OPC_Keywords				0xDC0A
#define OPC_ParentObject			0xDC0B
#define OPC_AllowedFolderContents	0xDC0C
#define OPC_Hidden					0xDC0D
#define OPC_SystemObject			0xDC0E
#define OPC_PersistantUniqueObjectIdentifier	0xDC41
#define OPC_SyncID					0xDC42
#define OPC_PropertyBag				0xDC43
#define OPC_Name					0xDC44
#define OPC_CreatedBy				0xDC45
#define OPC_Artist					0xDC46
#define OPC_DateAuthored			0xDC47
#define OPC_Description				0xDC48
#define OPC_URLReference			0xDC49
#define OPC_LanguageLocale			0xDC4A
#define OPC_CopyrightInformation	0xDC4B
#define OPC_Source					0xDC4C
#define OPC_OriginLocation			0xDC4D
#define OPC_DateAdded				0xDC4E
#define OPC_NonConsumable			0xDC4F
#define OPC_CorruptOrUnplayable		0xDC50
#define OPC_ProducerSerialNumber			0xDC51
#define OPC_RepresentativeSampleFormat		0xDC81
#define OPC_RepresentativeSampleSize		0xDC82
#define OPC_RepresentativeSampleHeight		0xDC83
#define OPC_RepresentativeSampleWidth		0xDC84
#define OPC_RepresentativeSampleDuration	0xDC85
#define OPC_RepresentativeSampleData		0xDC86
#define OPC_Width					0xDC87
#define OPC_Height					0xDC88
#define OPC_Duration				0xDC89
#define OPC_Rating					0xDC8A
#define OPC_Track					0xDC8B
#define OPC_Genre					0xDC8C
#define OPC_Credits					0xDC8D
#define OPC_Lyrics					0xDC8E
#define OPC_SubscriptionContentID	0xDC8F
#define OPC_ProducedBy				0xDC90
#define OPC_UseCount				0xDC91
#define OPC_SkipCount				0xDC92
#define OPC_LastAccessed			0xDC93
#define OPC_ParentalRating			0xDC94
#define OPC_MetaGenre				0xDC95
#define OPC_Composer				0xDC96
#define OPC_EffectiveRating			0xDC97
#define OPC_Subtitle				0xDC98
#define OPC_OriginalReleaseDate		0xDC99
#define OPC_AlbumName				0xDC9A
#define OPC_AlbumArtist				0xDC9B
#define OPC_Mood					0xDC9C
#define OPC_DRMStatus				0xDC9D
#define OPC_SubDescription			0xDC9E
#define OPC_IsCropped				0xDCD1
#define OPC_IsColorCorrected		0xDCD2
#define OPC_ImageBitDepth			0xDCD3
#define OPC_Fnumber					0xDCD4
#define OPC_ExposureTime			0xDCD5
#define OPC_ExposureIndex			0xDCD6
#define OPC_DisplayName				0xDCE0
#define OPC_BodyText				0xDCE1
#define OPC_Subject					0xDCE2
#define OPC_Priority				0xDCE3
#define OPC_GivenName				0xDD00
#define OPC_MiddleNames				0xDD01
#define OPC_FamilyName				0xDD02
#define OPC_Prefix					0xDD03
#define OPC_Suffix					0xDD04

/* PTP Association Types */
#define AT_Undefined			0x0000
#define AT_GenericFolder		0x0001
#define AT_Album				0x0002
#define AT_TimeSequence			0x0003
#define AT_HorizontalPanoramic	0x0004
#define AT_VerticalPanoramic	0x0005
#define AT_2DPanoramic			0x0006
#define AT_AncillaryData		0x0007


#endif

