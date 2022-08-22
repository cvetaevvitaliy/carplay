/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 18 "../mDNSShared/dnsextd_parser.y"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mDNSEmbeddedAPI.h"
#include "DebugServices.h"
#include "dnsextd.h"

void yyerror( void *context, const char* error );
int  yylex(void);


typedef struct StringListElem
{
	char					*	string;
	struct StringListElem	*	next;
} StringListElem;


typedef struct OptionsInfo
{
	char	server_address[ 256 ];
	int		server_port;
	char	source_address[ 256 ];
	int		source_port;
	int		private_port;
	int		llq_port;
} OptionsInfo;


typedef struct ZoneInfo
{
	char	name[ 256 ];
	char	certificate_name[ 256 ];
	char	allow_clients_file[ 256 ];
	char	allow_clients[ 256 ];
	char	key[ 256 ];
} ZoneInfo;


typedef struct KeySpec
{
	char 				name[ 256 ];
	char				algorithm[ 256 ];
	char				secret[ 256 ];
	struct KeySpec	*	next;
} KeySpec;


typedef struct ZoneSpec
{
	char				name[ 256 ];
	DNSZoneSpecType		type;
	StringListElem	*	allowUpdate;
	StringListElem	*	allowQuery;
	char				key[ 256 ];
	struct ZoneSpec	*	next;
} ZoneSpec;


static StringListElem	*	g_stringList = NULL;
static KeySpec			*	g_keys;
static ZoneSpec			*	g_zones;
static ZoneSpec				g_zoneSpec;
static const char		*	g_filename;

void
SetupOptions
	(
	OptionsInfo	*	info,
	void		*	context
	);


#line 146 "objects/prod/dnsextd_parser.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

#include "dnsextd_parser.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_OPTIONS = 3,                    /* OPTIONS  */
  YYSYMBOL_LISTEN_ON = 4,                  /* LISTEN_ON  */
  YYSYMBOL_NAMESERVER = 5,                 /* NAMESERVER  */
  YYSYMBOL_PORT = 6,                       /* PORT  */
  YYSYMBOL_ADDRESS = 7,                    /* ADDRESS  */
  YYSYMBOL_LLQ = 8,                        /* LLQ  */
  YYSYMBOL_PUBLIC = 9,                     /* PUBLIC  */
  YYSYMBOL_PRIVATE = 10,                   /* PRIVATE  */
  YYSYMBOL_ALLOWUPDATE = 11,               /* ALLOWUPDATE  */
  YYSYMBOL_ALLOWQUERY = 12,                /* ALLOWQUERY  */
  YYSYMBOL_KEY = 13,                       /* KEY  */
  YYSYMBOL_ALGORITHM = 14,                 /* ALGORITHM  */
  YYSYMBOL_SECRET = 15,                    /* SECRET  */
  YYSYMBOL_ISSUER = 16,                    /* ISSUER  */
  YYSYMBOL_SERIAL = 17,                    /* SERIAL  */
  YYSYMBOL_ZONE = 18,                      /* ZONE  */
  YYSYMBOL_TYPE = 19,                      /* TYPE  */
  YYSYMBOL_ALLOW = 20,                     /* ALLOW  */
  YYSYMBOL_OBRACE = 21,                    /* OBRACE  */
  YYSYMBOL_EBRACE = 22,                    /* EBRACE  */
  YYSYMBOL_SEMICOLON = 23,                 /* SEMICOLON  */
  YYSYMBOL_IN = 24,                        /* IN  */
  YYSYMBOL_DOTTED_DECIMAL_ADDRESS = 25,    /* DOTTED_DECIMAL_ADDRESS  */
  YYSYMBOL_WILDCARD = 26,                  /* WILDCARD  */
  YYSYMBOL_DOMAINNAME = 27,                /* DOMAINNAME  */
  YYSYMBOL_HOSTNAME = 28,                  /* HOSTNAME  */
  YYSYMBOL_QUOTEDSTRING = 29,              /* QUOTEDSTRING  */
  YYSYMBOL_NUMBER = 30,                    /* NUMBER  */
  YYSYMBOL_YYACCEPT = 31,                  /* $accept  */
  YYSYMBOL_commands = 32,                  /* commands  */
  YYSYMBOL_command = 33,                   /* command  */
  YYSYMBOL_options_set = 34,               /* options_set  */
  YYSYMBOL_optionscontent = 35,            /* optionscontent  */
  YYSYMBOL_optionsstatements = 36,         /* optionsstatements  */
  YYSYMBOL_optionsstatement = 37,          /* optionsstatement  */
  YYSYMBOL_key_set = 38,                   /* key_set  */
  YYSYMBOL_zone_set = 39,                  /* zone_set  */
  YYSYMBOL_zonecontent = 40,               /* zonecontent  */
  YYSYMBOL_zonestatements = 41,            /* zonestatements  */
  YYSYMBOL_zonestatement = 42,             /* zonestatement  */
  YYSYMBOL_addresscontent = 43,            /* addresscontent  */
  YYSYMBOL_addressstatements = 44,         /* addressstatements  */
  YYSYMBOL_addressstatement = 45,          /* addressstatement  */
  YYSYMBOL_keycontent = 46,                /* keycontent  */
  YYSYMBOL_keystatements = 47,             /* keystatements  */
  YYSYMBOL_keystatement = 48,              /* keystatement  */
  YYSYMBOL_networkaddress = 49,            /* networkaddress  */
  YYSYMBOL_block = 50,                     /* block  */
  YYSYMBOL_statements = 51,                /* statements  */
  YYSYMBOL_statement = 52                  /* statement  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   63

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  31
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  22
/* YYNRULES -- Number of rules.  */
#define YYNRULES  43
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  79

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   285


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   135,   135,   137,   142,   144,   146,   151,   158,   161,
     163,   168,   170,   174,   178,   182,   186,   191,   198,   219,
     241,   265,   267,   269,   273,   278,   283,   289,   297,   301,
     303,   309,   316,   320,   322,   328,   349,   351,   353,   357,
     360,   362,   366,   371
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "OPTIONS", "LISTEN_ON",
  "NAMESERVER", "PORT", "ADDRESS", "LLQ", "PUBLIC", "PRIVATE",
  "ALLOWUPDATE", "ALLOWQUERY", "KEY", "ALGORITHM", "SECRET", "ISSUER",
  "SERIAL", "ZONE", "TYPE", "ALLOW", "OBRACE", "EBRACE", "SEMICOLON", "IN",
  "DOTTED_DECIMAL_ADDRESS", "WILDCARD", "DOMAINNAME", "HOSTNAME",
  "QUOTEDSTRING", "NUMBER", "$accept", "commands", "command",
  "options_set", "optionscontent", "optionsstatements", "optionsstatement",
  "key_set", "zone_set", "zonecontent", "zonestatements", "zonestatement",
  "addresscontent", "addressstatements", "addressstatement", "keycontent",
  "keystatements", "keystatement", "networkaddress", "block", "statements",
  "statement", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-19)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int8 yypact[] =
{
     -19,     2,   -19,   -11,   -18,     6,    13,   -19,   -19,   -19,
     -19,   -19,     8,    -7,   -19,    -4,    22,   -19,    17,   -19,
       1,    32,    34,    35,   -19,    19,    -8,    14,    -3,   -19,
      15,   -19,   -19,     0,    16,    18,   -19,   -19,   -19,   -19,
     -19,    21,    26,    26,    23,   -19,    27,    28,     5,   -19,
     -19,   -19,    45,   -19,   -19,    12,    30,   -19,   -19,   -19,
     -19,   -19,   -19,   -19,   -19,   -19,    31,    25,    33,   -19,
     -10,   -19,   -19,   -19,    36,   -19,    37,   -19,   -19
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       2,     0,     1,     0,     0,     0,     0,     4,     6,     5,
       9,     7,     0,     0,     3,    40,     0,    22,     0,    19,
       0,     0,     0,     0,     8,     0,    11,     0,     0,    20,
       0,    29,    12,     0,     0,     0,    10,    22,    43,    42,
      41,     0,     0,     0,     0,    21,     0,     0,     0,    36,
      38,    37,    14,    17,    16,     0,     0,    33,    26,    27,
      24,    25,    23,    13,    28,    31,     0,     0,     0,    18,
       0,    30,    15,    39,     0,    32,     0,    35,    34
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -19,   -19,   -19,   -19,   -19,   -19,   -19,   -19,   -19,    39,
      24,   -19,    11,   -19,   -19,    10,   -19,   -19,   -19,   -19,
     -19,   -19
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,     1,     6,     7,    11,    15,    25,     8,     9,    19,
      28,    46,    32,    48,    66,    58,    70,    76,    52,    39,
      26,    40
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int8 yytable[] =
{
      20,    21,     2,    74,    22,     3,    23,    30,    42,    43,
      10,    12,    75,    37,    17,     4,    44,    18,    24,    45,
       5,    38,    31,    42,    43,    49,    50,    64,    51,    16,
      65,    44,    60,    61,    68,    13,    14,    27,    17,    33,
      34,    35,    36,    41,    56,    47,    53,    57,    54,    31,
      62,    67,    69,    59,    71,    72,    73,    29,    63,     0,
      78,    55,     0,    77
};

static const yytype_int8 yycheck[] =
{
       4,     5,     0,    13,     8,     3,    10,     6,    11,    12,
      21,    29,    22,    21,    21,    13,    19,    24,    22,    22,
      18,    29,    21,    11,    12,    25,    26,    22,    28,    21,
      25,    19,     9,    10,    22,    29,    23,    15,    21,     7,
       6,     6,    23,    29,    23,    30,    30,    21,    30,    21,
      23,     6,    22,    43,    23,    30,    23,    18,    47,    -1,
      23,    37,    -1,    27
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    32,     0,     3,    13,    18,    33,    34,    38,    39,
      21,    35,    29,    29,    23,    36,    21,    21,    24,    40,
       4,     5,     8,    10,    22,    37,    51,    15,    41,    40,
       6,    21,    43,     7,     6,     6,    23,    21,    29,    50,
      52,    29,    11,    12,    19,    22,    42,    30,    44,    25,
      26,    28,    49,    30,    30,    41,    23,    21,    46,    46,
       9,    10,    23,    43,    22,    25,    45,     6,    22,    22,
      47,    23,    30,    23,    13,    22,    48,    27,    23
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    31,    32,    32,    33,    33,    33,    34,    35,    36,
      36,    37,    37,    37,    37,    37,    37,    37,    38,    39,
      39,    40,    41,    41,    42,    42,    42,    42,    43,    44,
      44,    45,    46,    47,    47,    48,    49,    49,    49,    50,
      51,    51,    52,    52
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     0,     3,     1,     1,     1,     2,     3,     0,
       3,     1,     2,     4,     3,     5,     3,     3,     7,     3,
       4,     3,     0,     3,     2,     2,     2,     2,     3,     0,
       3,     1,     3,     0,     3,     2,     1,     1,     1,     4,
       0,     2,     1,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (context, YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value, context); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void *context)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  YY_USE (context);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep, void *context)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep, context);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule, void *context)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)], context);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, context); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep, void *context)
{
  YY_USE (yyvaluep);
  YY_USE (context);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void *context)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 7: /* options_set: OPTIONS optionscontent  */
#line 152 "../mDNSShared/dnsextd_parser.y"
                {
			// SetupOptions( &g_optionsInfo, context );
		}
#line 1236 "objects/prod/dnsextd_parser.c"
    break;

  case 12: /* optionsstatement: LISTEN_ON addresscontent  */
#line 171 "../mDNSShared/dnsextd_parser.y"
                {
		}
#line 1243 "objects/prod/dnsextd_parser.c"
    break;

  case 13: /* optionsstatement: LISTEN_ON PORT NUMBER addresscontent  */
#line 175 "../mDNSShared/dnsextd_parser.y"
                {
		}
#line 1250 "objects/prod/dnsextd_parser.c"
    break;

  case 14: /* optionsstatement: NAMESERVER ADDRESS networkaddress  */
#line 179 "../mDNSShared/dnsextd_parser.y"
                {
		}
#line 1257 "objects/prod/dnsextd_parser.c"
    break;

  case 15: /* optionsstatement: NAMESERVER ADDRESS networkaddress PORT NUMBER  */
#line 183 "../mDNSShared/dnsextd_parser.y"
                {
		}
#line 1264 "objects/prod/dnsextd_parser.c"
    break;

  case 16: /* optionsstatement: PRIVATE PORT NUMBER  */
#line 187 "../mDNSShared/dnsextd_parser.y"
                {
			( ( DaemonInfo* ) context )->private_port = mDNSOpaque16fromIntVal( (yyvsp[0].number) );
		}
#line 1272 "objects/prod/dnsextd_parser.c"
    break;

  case 17: /* optionsstatement: LLQ PORT NUMBER  */
#line 192 "../mDNSShared/dnsextd_parser.y"
                {
			( ( DaemonInfo* ) context )->llq_port = mDNSOpaque16fromIntVal( (yyvsp[0].number) );
		}
#line 1280 "objects/prod/dnsextd_parser.c"
    break;

  case 18: /* key_set: KEY QUOTEDSTRING OBRACE SECRET QUOTEDSTRING SEMICOLON EBRACE  */
#line 199 "../mDNSShared/dnsextd_parser.y"
        {
			KeySpec	* keySpec;

			keySpec = ( KeySpec* ) malloc( sizeof( KeySpec ) );

			if ( !keySpec )
				{
				LogMsg("ERROR: memory allocation failure");
				YYABORT;
				}

			strncpy( keySpec->name, (yyvsp[-5].string), sizeof( keySpec->name ) );
			strncpy( keySpec->secret, (yyvsp[-2].string), sizeof( keySpec->secret ) );

			keySpec->next	= g_keys;
			g_keys			= keySpec;
        }
#line 1302 "objects/prod/dnsextd_parser.c"
    break;

  case 19: /* zone_set: ZONE QUOTEDSTRING zonecontent  */
#line 220 "../mDNSShared/dnsextd_parser.y"
                {
			ZoneSpec * zoneSpec;

			zoneSpec = ( ZoneSpec* ) malloc( sizeof( ZoneSpec ) );

			if ( !zoneSpec )
				{
				LogMsg("ERROR: memory allocation failure");
				YYABORT;
				}

			strncpy( zoneSpec->name, (yyvsp[-1].string), sizeof( zoneSpec->name ) );
			zoneSpec->type = g_zoneSpec.type;
			strcpy( zoneSpec->key, g_zoneSpec.key );
			zoneSpec->allowUpdate = g_zoneSpec.allowUpdate;
			zoneSpec->allowQuery = g_zoneSpec.allowQuery;

			zoneSpec->next = g_zones;
			g_zones = zoneSpec;
		}
#line 1327 "objects/prod/dnsextd_parser.c"
    break;

  case 20: /* zone_set: ZONE QUOTEDSTRING IN zonecontent  */
#line 242 "../mDNSShared/dnsextd_parser.y"
        {
			ZoneSpec * zoneSpec;

			zoneSpec = ( ZoneSpec* ) malloc( sizeof( ZoneSpec ) );

			if ( !zoneSpec )
				{
				LogMsg("ERROR: memory allocation failure");
				YYABORT;
				}

			strncpy( zoneSpec->name, (yyvsp[-2].string), sizeof( zoneSpec->name ) );
			zoneSpec->type = g_zoneSpec.type;
			strcpy( zoneSpec->key, g_zoneSpec.key );
			zoneSpec->allowUpdate = g_zoneSpec.allowUpdate;
			zoneSpec->allowQuery = g_zoneSpec.allowQuery;

			zoneSpec->next = g_zones;
			g_zones = zoneSpec;
		}
#line 1352 "objects/prod/dnsextd_parser.c"
    break;

  case 24: /* zonestatement: TYPE PUBLIC  */
#line 274 "../mDNSShared/dnsextd_parser.y"
                {
			g_zoneSpec.type = kDNSZonePublic;
		}
#line 1360 "objects/prod/dnsextd_parser.c"
    break;

  case 25: /* zonestatement: TYPE PRIVATE  */
#line 279 "../mDNSShared/dnsextd_parser.y"
                {
			g_zoneSpec.type = kDNSZonePrivate;
		}
#line 1368 "objects/prod/dnsextd_parser.c"
    break;

  case 26: /* zonestatement: ALLOWUPDATE keycontent  */
#line 284 "../mDNSShared/dnsextd_parser.y"
                {
			g_zoneSpec.allowUpdate = g_stringList;
			g_stringList = NULL;
		}
#line 1377 "objects/prod/dnsextd_parser.c"
    break;

  case 27: /* zonestatement: ALLOWQUERY keycontent  */
#line 290 "../mDNSShared/dnsextd_parser.y"
                {
			g_zoneSpec.allowQuery = g_stringList;
			g_stringList = NULL;
		}
#line 1386 "objects/prod/dnsextd_parser.c"
    break;

  case 28: /* addresscontent: OBRACE addressstatements EBRACE  */
#line 298 "../mDNSShared/dnsextd_parser.y"
                {
		}
#line 1393 "objects/prod/dnsextd_parser.c"
    break;

  case 30: /* addressstatements: addressstatements addressstatement SEMICOLON  */
#line 304 "../mDNSShared/dnsextd_parser.y"
                {
		}
#line 1400 "objects/prod/dnsextd_parser.c"
    break;

  case 31: /* addressstatement: DOTTED_DECIMAL_ADDRESS  */
#line 310 "../mDNSShared/dnsextd_parser.y"
                {
		}
#line 1407 "objects/prod/dnsextd_parser.c"
    break;

  case 32: /* keycontent: OBRACE keystatements EBRACE  */
#line 317 "../mDNSShared/dnsextd_parser.y"
                {
		}
#line 1414 "objects/prod/dnsextd_parser.c"
    break;

  case 34: /* keystatements: keystatements keystatement SEMICOLON  */
#line 323 "../mDNSShared/dnsextd_parser.y"
                {
		}
#line 1421 "objects/prod/dnsextd_parser.c"
    break;

  case 35: /* keystatement: KEY DOMAINNAME  */
#line 329 "../mDNSShared/dnsextd_parser.y"
                {
			StringListElem * elem;

			elem = ( StringListElem* ) malloc( sizeof( StringListElem ) );

			if ( !elem )
				{
				LogMsg("ERROR: memory allocation failure");
				YYABORT;
				}

			elem->string = (yyvsp[0].string);

			elem->next		= g_stringList;
			g_stringList	= elem;
		}
#line 1442 "objects/prod/dnsextd_parser.c"
    break;

  case 42: /* statement: block  */
#line 367 "../mDNSShared/dnsextd_parser.y"
                {
			(yyval.string) = NULL;
		}
#line 1450 "objects/prod/dnsextd_parser.c"
    break;

  case 43: /* statement: QUOTEDSTRING  */
#line 372 "../mDNSShared/dnsextd_parser.y"
                {
			(yyval.string) = (yyvsp[0].string);
		}
#line 1458 "objects/prod/dnsextd_parser.c"
    break;


#line 1462 "objects/prod/dnsextd_parser.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (context, YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, context);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp, context);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (context, YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, context);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp, context);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 375 "../mDNSShared/dnsextd_parser.y"


int yywrap(void);

extern int yylineno;

void yyerror( void *context, const char *str )
{
        fprintf( stderr,"%s:%d: error: %s\n", g_filename, yylineno, str );
}
 
int yywrap()
{
        return 1;
} 


int
ParseConfig
	(
	DaemonInfo	*	d,
	const char	*	file
	)
	{
	extern FILE		*	yyin;
	DNSZone			*	zone;
	DomainAuthInfo	*	key;
	KeySpec			*	keySpec;
	ZoneSpec		*	zoneSpec;
	int					err = 0;

	g_filename = file;

	// Tear down the current zone specifiers

	zone = d->zones;

	while ( zone )
		{
		DNSZone * next = zone->next;

		key = zone->updateKeys;

		while ( key )
			{
			DomainAuthInfo * nextKey = key->next;

			free( key );

			key = nextKey;
			}

		key = zone->queryKeys;

		while ( key )
			{
			DomainAuthInfo * nextKey = key->next;

			free( key );

			key = nextKey;
			}

		free( zone );

		zone = next;
		}

	d->zones = NULL;
	
	yyin = fopen( file, "r" );
	require_action( yyin, exit, err = 0 );

	err = yyparse( ( void* ) d );
	require_action( !err, exit, err = 1 );

	for ( zoneSpec = g_zones; zoneSpec; zoneSpec = zoneSpec->next )
		{
		StringListElem  *   elem;
		mDNSu8			*	ok;

		zone = ( DNSZone* ) malloc( sizeof( DNSZone ) );
		require_action( zone, exit, err = 1 );
		memset( zone, 0, sizeof( DNSZone ) );

		zone->next	= d->zones;
		d->zones	= zone;

		// Fill in the domainname

		ok = MakeDomainNameFromDNSNameString( &zone->name, zoneSpec->name );
		require_action( ok, exit, err = 1 );

		// Fill in the type

		zone->type = zoneSpec->type;

		// Fill in the allow-update keys

		for ( elem = zoneSpec->allowUpdate; elem; elem = elem->next )
			{
			mDNSBool found = mDNSfalse;

			for ( keySpec = g_keys; keySpec; keySpec = keySpec->next )
				{
				if ( strcmp( elem->string, keySpec->name ) == 0 )
					{
					DomainAuthInfo	*	authInfo = malloc( sizeof( DomainAuthInfo ) );
					mDNSs32				keylen;
					require_action( authInfo, exit, err = 1 );
					memset( authInfo, 0, sizeof( DomainAuthInfo ) );

					ok = MakeDomainNameFromDNSNameString( &authInfo->keyname, keySpec->name );
					if (!ok) { free(authInfo); err = 1; goto exit; }

					keylen = DNSDigest_ConstructHMACKeyfromBase64( authInfo, keySpec->secret );
					if (keylen < 0) { free(authInfo); err = 1; goto exit; }

					authInfo->next = zone->updateKeys;
					zone->updateKeys = authInfo;

					found = mDNStrue;

					break;
					}
				}

			// Log this
			require_action( found, exit, err = 1 );
			}

		// Fill in the allow-query keys

		for ( elem = zoneSpec->allowQuery; elem; elem = elem->next )
			{
			mDNSBool found = mDNSfalse;

			for ( keySpec = g_keys; keySpec; keySpec = keySpec->next )
				{
				if ( strcmp( elem->string, keySpec->name ) == 0 )
					{
					DomainAuthInfo	*	authInfo = malloc( sizeof( DomainAuthInfo ) );
					mDNSs32				keylen;
					require_action( authInfo, exit, err = 1 );
					memset( authInfo, 0, sizeof( DomainAuthInfo ) );

					ok = MakeDomainNameFromDNSNameString( &authInfo->keyname, keySpec->name );
					if (!ok) { free(authInfo); err = 1; goto exit; }

					keylen = DNSDigest_ConstructHMACKeyfromBase64( authInfo, keySpec->secret );
					if (keylen < 0) { free(authInfo); err = 1; goto exit; }

					authInfo->next = zone->queryKeys;
					zone->queryKeys = authInfo;

					found = mDNStrue;

					break;
					}
				}

			// Log this
			require_action( found, exit, err = 1 );
			}
		}

exit:

	return err;
	}


void
SetupOptions
	(
	OptionsInfo	*	info,
	void		*	context
	)
	{
	DaemonInfo * d = ( DaemonInfo* ) context;

	if ( strlen( info->source_address ) )
		{
		inet_pton( AF_INET, info->source_address, &d->addr.sin_addr );
		}

	if ( info->source_port )
		{
		d->addr.sin_port = htons( ( mDNSu16 ) info->source_port );
		}
				
	if ( strlen( info->server_address ) )
		{
		inet_pton( AF_INET, info->server_address, &d->ns_addr.sin_addr );
		}

	if ( info->server_port )
		{
		d->ns_addr.sin_port = htons( ( mDNSu16 ) info->server_port );
		}

	if ( info->private_port )
		{
		d->private_port = mDNSOpaque16fromIntVal( info->private_port );
		}

	if ( info->llq_port )
		{
		d->llq_port = mDNSOpaque16fromIntVal( info->llq_port );
		}
	}
