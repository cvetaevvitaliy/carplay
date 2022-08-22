/**
	\addtogroup automotive_api_basic_date_types
	Automotive API basic data types.
	\ingroup automotive_api_base
*/
/*@{*/
/***************************************************************************//**
	\file		ama_types.h
	\brief		Definition of basic data type.
	\author		zhaoyg
	\version	1.0
	\date		2013-4-23
	\warning	Copyright (C), 2013, Neusoft.
	\remarks	Revision History\n
------------------------------------------------------------------------------\n
	No.	Author		Date		Version	Description\n
------------------------------------------------------------------------------\n
	001	zhaoyg		2013-4-23	1.0		Create.\n
*******************************************************************************/
#ifndef _AMA_TYPES_H
#define _AMA_TYPES_H
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
/**
	\def TRUE
	Bool value true.
*/
#ifndef TRUE
#define	TRUE	(1)
#endif

/**
	\def FALSE
	Bool value false.
*/
#ifndef FALSE
#define FALSE	(0)
#endif

/**
	\def NULL
	Null pointer.
*/
#ifndef NULL
#define	NULL	((void *)0)
#endif

/**
	\def ARRAY_SIZE(a)
	Element total of array.
	\param[in] a : Array name.
*/
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)		(sizeof(a)/sizeof(a[0]))
#endif

/*******************************************************************************
	Type Definition
*******************************************************************************/
/**
	\typedef unsigned char uint8
	Unsigned single byte integer type.
*/
typedef unsigned char		uint8;

/**
	\typedef signed char sint8
	Signed single byte integer type.
*/
typedef signed char			sint8;

/**
	\typedef unsigned short int uint16
	Unsigned double bytes integer type.
*/
typedef unsigned short int	uint16;

/**
	\typedef signed short int sint16
	Signed double bytes integer type.
*/
typedef signed short int	sint16;

/**
	\typedef unsigned int uint32
	Unsigned four bytes integer type.
*/
typedef unsigned int		uint32;

/**
	\typedef signed int sint32
	Signed four bytes integer type.
*/
typedef signed int			sint32;

/**
	\typedef unsigned long long	uint64
	Unsigned eight bytes integer type.
*/
typedef unsigned long long	uint64;

/**
	\typedef signed long long	sint64
	Signed eight bytes integer type.
	*/
typedef signed long long	sint64;

/**
	\typedef float float32
	Single-precision float type.
*/
typedef float				float32;

/**
	\typedef double float64
	Double-precision float type.
*/
typedef double				float64;

/**
	\typedef unsigned char bool_t
	Bool type.
*/
typedef unsigned char		bool_t;

/**
	\typedef char char_t
	Character type.
*/
typedef char				char_t;

/*******************************************************************************
	Prototype  Declaration
*******************************************************************************/

#ifdef __cplusplus
}
#endif
#endif /* _AMA_TYPES_H */
/*@}*/

