/*
 * =====================================================================================
 *
 *       Filename:  order.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  09/09/2014 10:27:40 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  jiang pengfei (), jiang.pf@neusoft.com
 *   Organization:  www.neusoft.com
 *
 * =====================================================================================
 */
#include <mh_item.h>
#include <string.h>
#include <stdio.h>
#include <mh_api.h>
#include <ctype.h>
#include <stdlib.h>

typedef unsigned char       uint8;
typedef signed char         sint8;
typedef unsigned short int  uint16;
typedef signed short int    sint16;
typedef unsigned int        uint32;
typedef signed int          sint32;
typedef unsigned long long  uint64;
typedef signed long long    sint64;

extern const unsigned short UniCode2Pinyin_Tbl[0x10000];
const unsigned short UniCode2ASCII_Tbl[0x80] =
{
	0x0000,0x0035,0x0036,0x0037,0x0038,0x0039,0x003A,0x003B,0x003C,0x003D,0x003E,0x003F,0x0040,0x0041,0x0042,0x0043,
	0x0044,0x0045,0x0046,0x0047,0x0048,0x0049,0x004A,0x004B,0x004C,0x004D,0x004E,0x004F,0x0050,0x0051,0x0052,0x0053,
	0x0054,0x0055,0x0056,0x0057,0x0058,0x0059,0x005A,0x005B,0x005C,0x005D,0x005E,0x005F,0x0060,0x0061,0x0062,0x0063,
	0x0064,0x0065,0x0066,0x0067,0x0068,0x0069,0x006A,0x006B,0x006C,0x006D,0x006E,0x006F,0x0070,0x0071,0x0072,0x0073,
	0x0074,0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,0x0008,0x0009,0x000A,0x000B,0x000C,0x000D,0x000E,0x000F,
	0x0010,0x0011,0x0012,0x0013,0x0014,0x0015,0x0016,0x0017,0x0018,0x0019,0x001A,0x0075,0x0076,0x0077,0x0078,0x0079,
	0x007A,0x001B,0x001C,0x001D,0x001E,0x001F,0x0020,0x0021,0x0022,0x0023,0x0024,0x0025,0x0026,0x0027,0x0028,0x0029,
	0x002A,0x002B,0x002C,0x002D,0x002E,0x002F,0x0030,0x0031,0x0032,0x0033,0x0034,0x007B,0x007C,0x007D,0x007E,0x007F,
};


#define SORT_ORDER_INPUT_SAME				(0)
#define SORT_ORDER_INPUT1_FIRST				(1)
#define SORT_ORDER_INPUT2_FIRST				(-1)
#define SORT_ORDER_INPUT_INVALID			(0x7F)


#define INPUT_CHAR_INDEX_VALID				(0)
#define INPUT_CHAR_INDEX_INVALID			(-1)


#define LANGUAGE_CATEGORY_MASK				(0xFF00)

#define KANA_CATEGORY_TABLE1				(0x3000)
#define KANA_CATEGORY_TABLE2				(0xFF00)
#define GREEK_CATEGORY_TABLE				(0x0300)
#define EUROPE_CATEGORY_TABLE1				(0x0000)
#define EUROPE_CATEGORY_TABLE2				(0x0100)

typedef struct {
	uint32				index_exact;
	uint32				index_group;
}UNICODE_MATCH_INFO;


#define UNICODE_JAPAN_T1_OFFSET					(12353U)
#define UNICODE_JAPAN_T1_SUB_PHASE				(85U)
#define UNICODE_JAPAN_T1_SUB_OFFSET				(10U)

const UNICODE_MATCH_INFO unicode_japan_table1[] =
{
            /* char unicode utf8 */  
        {   3   ,   1   },      /*  ぁ   3041    E3  81  81  */
        {   6   ,   1   },      /*  あ   3042    E3  81  82  */
        {   9   ,   1   },      /*  ぃ   3043    E3  81  83  */
        {   12  ,   1   },      /*  い   3044    E3  81  84  */
        {   15  ,   1   },      /*  ぅ   3045    E3  81  85  */
        {   18  ,   1   },      /*  う   3046    E3  81  86  */
        {   23  ,   1   },      /*  ぇ   3047    E3  81  87  */
        {   26  ,   1   },      /*  え   3048    E3  81  88  */
        {   29  ,   1   },      /*  ぉ   3049    E3  81  89  */
        {   32  ,   1   },      /*  お   304A    E3  81  8A  */
        {   37  ,   2   },      /*  か   304B    E3  81  8B  */
        {   39  ,   2   },      /*  が   304C    E3  81  8C  */
        {   42  ,   2   },      /*  き   304D    E3  81  8D  */
        {   44  ,   2   },      /*  ぎ   304E    E3  81  8E  */
        {   47  ,   2   },      /*  く   304F    E3  81  8F  */
        {   49  ,   2   },      /*  ぐ   3050    E3  81  90  */
        {   54  ,   2   },      /*  け   3051    E3  81  91  */
        {   56  ,   2   },      /*  げ   3052    E3  81  92  */
        {   59  ,   2   },      /*  こ   3053    E3  81  93  */
        {   61  ,   2   },      /*  ご   3054    E3  81  94  */
        {   64  ,   3   },      /*  さ   3055    E3  81  95  */
        {   66  ,   3   },      /*  ざ   3056    E3  81  96  */
        {   69  ,   3   },      /*  し   3057    E3  81  97  */
        {   71  ,   3   },      /*  じ   3058    E3  81  98  */
        {   74  ,   3   },      /*  す   3059    E3  81  99  */
        {   76  ,   3   },      /*  ず   305A    E3  81  9A  */
        {   79  ,   3   },      /*  せ   305B    E3  81  9B  */
        {   81  ,   3   },      /*  ぜ   305C    E3  81  9C  */
        {   84  ,   3   },      /*  そ   305D    E3  81  9D  */
        {   86  ,   3   },      /*  ぞ   305E    E3  81  9E  */
        {   89  ,   4   },      /*  た   305F    E3  81  9F  */
        {   91  ,   4   },      /*  だ   3060    E3  81  A0  */
        {   94  ,   4   },      /*  ち   3061    E3  81  A1  */
        {   96  ,   4   },      /*  ぢ   3062    E3  81  A2  */
        {   99  ,   4   },      /*  っ   3063    E3  81  A3  */
        {   102 ,   4   },      /*  つ   3064    E3  81  A4  */
        {   104 ,   4   },      /*  づ   3065    E3  81  A5  */
        {   107 ,   4   },      /*  て   3066    E3  81  A6  */
        {   109 ,   4   },      /*  で   3067    E3  81  A7  */
        {   112 ,   4   },      /*  と   3068    E3  81  A8  */
        {   114 ,   4   },      /*  ど   3069    E3  81  A9  */
        {   117 ,   5   },      /*  な   306A    E3  81  AA  */
        {   120 ,   5   },      /*  に   306B    E3  81  AB  */
        {   123 ,   5   },      /*  ぬ   306C    E3  81  AC  */
        {   126 ,   5   },      /*  ね   306D    E3  81  AD  */
        {   129 ,   5   },      /*  の   306E    E3  81  AE  */
        {   132 ,   6   },      /*  は   306F    E3  81  AF  */
        {   134 ,   6   },      /*  ば   3070    E3  81  B0  */
        {   136 ,   6   },      /*  ぱ   3071    E3  81  B1  */
        {   139 ,   6   },      /*  ひ   3072    E3  81  B2  */
        {   141 ,   6   },      /*  び   3073    E3  81  B3  */
        {   143 ,   6   },      /*  ぴ   3074    E3  81  B4  */
        {   146 ,   6   },      /*  ふ   3075    E3  81  B5  */
        {   148 ,   6   },      /*  ぶ   3076    E3  81  B6  */
        {   150 ,   6   },      /*  ぷ   3077    E3  81  B7  */
        {   153 ,   6   },      /*  へ   3078    E3  81  B8  */
        {   155 ,   6   },      /*  べ   3079    E3  81  B9  */
        {   157 ,   6   },      /*  ぺ   307A    E3  81  BA  */
        {   160 ,   6   },      /*  ほ   307B    E3  81  BB  */
        {   162 ,   6   },      /*  ぼ   307C    E3  81  BC  */
        {   164 ,   6   },      /*  ぽ   307D    E3  81  BD  */
        {   167 ,   7   },      /*  ま   307E    E3  81  BE  */
        {   170 ,   7   },      /*  み   307F    E3  81  BF  */
        {   173 ,   7   },      /*  む   3080    E3  82  80  */
        {   176 ,   7   },      /*  め   3081    E3  82  81  */
        {   179 ,   7   },      /*  も   3082    E3  82  82  */
        {   182 ,   8   },      /*  ゃ   3083    E3  82  83  */
        {   185 ,   8   },      /*  や   3084    E3  82  84  */
        {   188 ,   8   },      /*  ゅ   3085    E3  82  85  */
        {   191 ,   8   },      /*  ゆ   3086    E3  82  86  */
        {   194 ,   8   },      /*  ょ   3087    E3  82  87  */
        {   197 ,   8   },      /*  よ   3088    E3  82  88  */
        {   200 ,   9   },      /*  ら   3089    E3  82  89  */
        {   203 ,   9   },      /*  り   308A    E3  82  8A  */
        {   206 ,   9   },      /*  る   308B    E3  82  8B  */
        {   209 ,   9   },      /*  れ   308C    E3  82  8C  */
        {   212 ,   9   },      /*  ろ   308D    E3  82  8D  */
        {   214 ,   10  },      /*  ゎ   308E    E3  82  8E  */
        {   217 ,   10  },      /*  わ   308F    E3  82  8F  */
        {   220 ,   10  },      /*  ゐ   3090    E3  82  90  */
        {   223 ,   10  },      /*  ゑ   3091    E3  82  91  */
        {   227 ,   10  },      /*  を   3092    E3  82  92  */
        {   231 ,   10  },      /*  ん   3093    E3  82  93  */
        {   20  ,   1   },      /*  ゔ   3094    E3  82  94  */
        {   34  ,   2   },      /*  ゕ   3095    E3  82  95  */
        {   51  ,   2   },      /*  ゖ   3096    E3  82  96  */
        {   1   ,   1   },      /*  ァ   30A1    E3  82  A1  *//* index is not consecutive in here */
        {   4   ,   1   },      /*  ア   30A2    E3  82  A2  */
        {   7   ,   1   },      /*  ィ   30A3    E3  82  A3  */
        {   10  ,   1   },      /*  イ   30A4    E3  82  A4  */
        {   13  ,   1   },      /*  ゥ   30A5    E3  82  A5  */
        {   16  ,   1   },      /*  ウ   30A6    E3  82  A6  */
        {   21  ,   1   },      /*  ェ   30A7    E3  82  A7  */
        {   24  ,   1   },      /*  エ   30A8    E3  82  A8  */
        {   27  ,   1   },      /*  ォ   30A9    E3  82  A9  */
        {   30  ,   1   },      /*  オ   30AA    E3  82  AA  */
        {   35  ,   2   },      /*  カ   30AB    E3  82  AB  */
        {   38  ,   2   },      /*  ガ   30AC    E3  82  AC  */
        {   40  ,   2   },      /*  キ   30AD    E3  82  AD  */
        {   43  ,   2   },      /*  ギ   30AE    E3  82  AE  */
        {   45  ,   2   },      /*  ク   30AF    E3  82  AF  */
        {   48  ,   2   },      /*  グ   30B0    E3  82  B0  */
        {   52  ,   2   },      /*  ケ   30B1    E3  82  B1  */
        {   55  ,   2   },      /*  ゲ   30B2    E3  82  B2  */
        {   57  ,   2   },      /*  コ   30B3    E3  82  B3  */
        {   60  ,   2   },      /*  ゴ   30B4    E3  82  B4  */
        {   62  ,   3   },      /*  サ   30B5    E3  82  B5  */
        {   65  ,   3   },      /*  ザ   30B6    E3  82  B6  */
        {   67  ,   3   },      /*  シ   30B7    E3  82  B7  */
        {   70  ,   3   },      /*  ジ   30B8    E3  82  B8  */
        {   72  ,   3   },      /*  ス   30B9    E3  82  B9  */
        {   75  ,   3   },      /*  ズ   30BA    E3  82  BA  */
        {   77  ,   3   },      /*  セ   30BB    E3  82  BB  */
        {   80  ,   3   },      /*  ゼ   30BC    E3  82  BC  */
        {   82  ,   3   },      /*  ソ   30BD    E3  82  BD  */
        {   85  ,   3   },      /*  ゾ   30BE    E3  82  BE  */
        {   87  ,   4   },      /*  タ   30BF    E3  82  BF  */
        {   90  ,   4   },      /*  ダ   30C0    E3  83  80  */
        {   92  ,   4   },      /*  チ   30C1    E3  83  81  */
        {   95  ,   4   },      /*  ヂ   30C2    E3  83  82  */
        {   97  ,   4   },      /*  ッ   30C3    E3  83  83  */
        {   100 ,   4   },      /*  ツ   30C4    E3  83  84  */
        {   103 ,   4   },      /*  ヅ   30C5    E3  83  85  */
        {   105 ,   4   },      /*  テ   30C6    E3  83  86  */
        {   108 ,   4   },      /*  デ   30C7    E3  83  87  */
        {   110 ,   4   },      /*  ト   30C8    E3  83  88  */
        {   113 ,   4   },      /*  ド   30C9    E3  83  89  */
        {   115 ,   5   },      /*  ナ   30CA    E3  83  8A  */
        {   118 ,   5   },      /*  ニ   30CB    E3  83  8B  */
        {   121 ,   5   },      /*  ヌ   30CC    E3  83  8C  */
        {   124 ,   5   },      /*  ネ   30CD    E3  83  8D  */
        {   127 ,   5   },      /*  ノ   30CE    E3  83  8E  */
        {   130 ,   6   },      /*  ハ   30CF    E3  83  8F  */
        {   133 ,   6   },      /*  バ   30D0    E3  83  90  */
        {   135 ,   6   },      /*  パ   30D1    E3  83  91  */
        {   137 ,   6   },      /*  ヒ   30D2    E3  83  92  */
        {   140 ,   6   },      /*  ビ   30D3    E3  83  93  */
        {   142 ,   6   },      /*  ピ   30D4    E3  83  94  */
        {   144 ,   6   },      /*  フ   30D5    E3  83  95  */
        {   147 ,   6   },      /*  ブ   30D6    E3  83  96  */
        {   149 ,   6   },      /*  プ   30D7    E3  83  97  */
        {   151 ,   6   },      /*  ヘ   30D8    E3  83  98  */
        {   154 ,   6   },      /*  ベ   30D9    E3  83  99  */
        {   156 ,   6   },      /*  ペ   30DA    E3  83  9A  */
        {   158 ,   6   },      /*  ホ   30DB    E3  83  9B  */
        {   161 ,   6   },      /*  ボ   30DC    E3  83  9C  */
        {   163 ,   6   },      /*  ポ   30DD    E3  83  9D  */
        {   165 ,   7   },      /*  マ   30DE    E3  83  9E  */
        {   168 ,   7   },      /*  ミ   30DF    E3  83  9F  */
        {   171 ,   7   },      /*  ム   30E0    E3  83  A0  */
        {   174 ,   7   },      /*  メ   30E1    E3  83  A1  */
        {   177 ,   7   },      /*  モ   30E2    E3  83  A2  */
        {   180 ,   8   },      /*  ャ   30E3    E3  83  A3  */
        {   183 ,   8   },      /*  ヤ   30E4    E3  83  A4  */
        {   186 ,   8   },      /*  ュ   30E5    E3  83  A5  */
        {   189 ,   8   },      /*  ユ   30E6    E3  83  A6  */
        {   192 ,   8   },      /*  ョ   30E7    E3  83  A7  */
        {   195 ,   8   },      /*  ヨ   30E8    E3  83  A8  */
        {   198 ,   9   },      /*  ラ   30E9    E3  83  A9  */
        {   201 ,   9   },      /*  リ   30EA    E3  83  AA  */
        {   204 ,   9   },      /*  ル   30EB    E3  83  AB  */
        {   207 ,   9   },      /*  レ   30EC    E3  83  AC  */
        {   210 ,   9   },      /*  ロ   30ED    E3  83  AD  */
        {   213 ,   10  },      /*  ヮ   30EE    E3  83  AE  */
        {   215 ,   10  },      /*  ワ   30EF    E3  83  AF  */
        {   219 ,   10  },      /*  ヰ   30F0    E3  83  B0  */
        {   222 ,   10  },      /*  ヱ   30F1    E3  83  B1  */
        {   225 ,   10  },      /*  ヲ   30F2    E3  83  B2  */
        {   229 ,   10  },      /*  ン   30F3    E3  83  B3  */
        {   19  ,   1   },      /*  ヴ   30F4    E3  83  B4  */
        {   33  ,   2   },      /*  ヵ   30F5    E3  83  B5  */
        {   50  ,   2   },      /*  ヶ   30F6    E3  83  B6  */
        {   218 ,   10  },      /*  ヷ   30F7    E3  83  B7  */
        {   221 ,   10  },      /*  ヸ   30F8    E3  83  B8  */
        {   224 ,   10  },      /*  ヹ   30F9    E3  83  B9  */
        {   228 ,   10  }      /*  ヺ   30FA    E3  83  BA  */
};



#define UNICODE_JAPAN_T2_OFFSET				(65382U)


const UNICODE_MATCH_INFO unicode_japan_table2[] =
{
              /* char unicode utf8 */       
        {   226 ,   10  },      /*  ｦ   FF66    EF  BD  A6  *//* table2 start */
        {   2   ,   1   },      /*  ｧ   FF67    EF  BD  A7  */
        {   8   ,   1   },      /*  ｨ   FF68    EF  BD  A8  */
        {   14  ,   1   },      /*  ｩ   FF69    EF  BD  A9  */
        {   22  ,   1   },      /*  ｪ   FF6A    EF  BD  AA  */
        {   28  ,   1   },      /*  ｫ   FF6B    EF  BD  AB  */
        {   181 ,   8   },      /*  ｬ   FF6C    EF  BD  AC  */
        {   187 ,   8   },      /*  ｭ   FF6D    EF  BD  AD  */
        {   193 ,   8   },      /*  ｮ   FF6E    EF  BD  AE  */
        {   98  ,   4   },      /*  ｯ   FF6F    EF  BD  AF  */
   { 232,10  },      /*  ｰ   FF70    EF  BD  B0  */
        {   5   ,   1   },      /*  ｱ   FF71    EF  BD  B1  */
        {   11  ,   1   },      /*  ｲ   FF72    EF  BD  B2  */
        {   17  ,   1   },      /*  ｳ   FF73    EF  BD  B3  */
        {   25  ,   1   },      /*  ｴ   FF74    EF  BD  B4  */
        {   31  ,   1   },      /*  ｵ   FF75    EF  BD  B5  */
        {   36  ,   2   },      /*  ｶ   FF76    EF  BD  B6  */
        {   41  ,   2   },      /*  ｷ   FF77    EF  BD  B7  */
        {   46  ,   2   },      /*  ｸ   FF78    EF  BD  B8  */
        {   53  ,   2   },      /*  ｹ   FF79    EF  BD  B9  */
        {   58  ,   2   },      /*  ｺ   FF7A    EF  BD  BA  */
        {   63  ,   3   },      /*  ｻ   FF7B    EF  BD  BB  */
        {   68  ,   3   },      /*  ｼ   FF7C    EF  BD  BC  */
        {   73  ,   3   },      /*  ｽ   FF7D    EF  BD  BD  */
        {   78  ,   3   },      /*  ｾ   FF7E    EF  BD  BE  */
        {   83  ,   3   },      /*  ｿ   FF7F    EF  BD  BF  */
        {   88  ,   4   },      /*  ﾀ   FF80    EF  BE  80  */
        {   93  ,   4   },      /*  ﾁ   FF81    EF  BE  81  */
        {   101 ,   4   },      /*  ﾂ   FF82    EF  BE  82  */
        {   106 ,   4   },      /*  ﾃ   FF83    EF  BE  83  */
        {   111 ,   4   },      /*  ﾄ   FF84    EF  BE  84  */
        {   116 ,   5   },      /*  ﾅ   FF85    EF  BE  85  */
        {   119 ,   5   },      /*  ﾆ   FF86    EF  BE  86  */
        {   122 ,   5   },      /*  ﾇ   FF87    EF  BE  87  */
        {   125 ,   5   },      /*  ﾈ   FF88    EF  BE  88  */
        {   128 ,   5   },      /*  ﾉ   FF89    EF  BE  89  */
        {   131 ,   6   },      /*  ﾊ   FF8A    EF  BE  8A  */
        {   138 ,   6   },      /*  ﾋ   FF8B    EF  BE  8B  */
        {   145 ,   6   },      /*  ﾌ   FF8C    EF  BE  8C  */
        {   152 ,   6   },      /*  ﾍ   FF8D    EF  BE  8D  */
        {   159 ,   6   },      /*  ﾎ   FF8E    EF  BE  8E  */
        {   166 ,   7   },      /*  ﾏ   FF8F    EF  BE  8F  */
        {   169 ,   7   },      /*  ﾐ   FF90    EF  BE  90  */
        {   172 ,   7   },      /*  ﾑ   FF91    EF  BE  91  */
        {   175 ,   7   },      /*  ﾒ   FF92    EF  BE  92  */
        {   178 ,   7   },      /*  ﾓ   FF93    EF  BE  93  */
        {   184 ,   8   },      /*  ﾔ   FF94    EF  BE  94  */
        {   190 ,   8   },      /*  ﾕ   FF95    EF  BE  95  */
        {   196 ,   8   },      /*  ﾖ   FF96    EF  BE  96  */
        {   199 ,   9   },      /*  ﾗ   FF97    EF  BE  97  */
        {   202 ,   9   },      /*  ﾘ   FF98    EF  BE  98  */
        {   205 ,   9   },      /*  ﾙ   FF99    EF  BE  99  */
        {   208 ,   9   },      /*  ﾚ   FF9A    EF  BE  9A  */
        {   211 ,   9   },      /*  ﾛ   FF9B    EF  BE  9B  */
        {   216 ,   10  },      /*  ﾜ   FF9C    EF  BE  9C  */
        {   230 ,   10  },      /*  ﾝ   FF9D    EF  BE  9D  */
};


#define UNICODE_EUROPE_T3_CAPICAL_OFFSET			(65U)
#define UNICODE_EUROPE_T3_LOWCASE_OFFSET			(97U)
#define UNICODE_EUROPE_T3_SUB_OFFSET				(26U)

const UNICODE_MATCH_INFO unicode_europe_table3[] =
{
        {   233 ,   11  },  /*  A   41  */
        {   234 ,   12  },  /*  B   42  */
        {   235 ,   13  },  /*  C   43  */
        {   236 ,   14  },  /*  D   44  */
        {   237 ,   15  },  /*  E   45  */
        {   238 ,   16  },  /*  F   46  */
        {   239 ,   17  },  /*  G   47  */
        {   240 ,   18  },  /*  H   48  */
        {   241 ,   19  },  /*  I   49  */
        {   242 ,   20  },  /*  J   4A  */
        {   243 ,   21  },  /*  K   4B  */
        {   244 ,   22  },  /*  L   4C  */
        {   245 ,   23  },  /*  M   4D  */
        {   246 ,   24  },  /*  N   4E  */
        {   247 ,   25  },  /*  O   4F  */
        {   248 ,   26  },  /*  P   50  */
        {   249 ,   27  },  /*  Q   51  */
        {   250 ,   28  },  /*  R   52  */
        {   251 ,   29  },  /*  S   53  */
        {   252 ,   30  },  /*  T   54  */
        {   253 ,   31  },  /*  U   55  */
        {   254 ,   32  },  /*  V   56  */
        {   255 ,   33  },  /*  W   57  */
        {   256 ,   34  },  /*  X   58  */
        {   257 ,   35  },  /*  Y   59  */
        {   258 ,   36  },  /*  Z   5A  */
        {   259 ,   11  },  /*  a   61  */
        {   260 ,   12  },  /*  b   62  */
        {   261 ,   13  },  /*  c   63  */
        {   262 ,   14  },  /*  d   64  */
        {   263 ,   15  },  /*  e   65  */
        {   264 ,   16  },  /*  f   66  */
        {   265 ,   17  },  /*  g   67  */
        {   266 ,   18  },  /*  h   68  */
        {   267 ,   19  },  /*  i   69  */
        {   268 ,   20  },  /*  j   6A  */
        {   269 ,   21  },  /*  k   6B  */
        {   270 ,   22  },  /*  l   6C  */
        {   271 ,   23  },  /*  m   6D  */
        {   272 ,   24  },  /*  n   6E  */
        {   273 ,   25  },  /*  o   6F  */
        {   274 ,   26  },  /*  p   70  */
        {   275 ,   27  },  /*  q   71  */
        {   276 ,   28  },  /*  r   72  */
        {   277 ,   29  },  /*  s   73  */
        {   278 ,   30  },  /*  t   74  */
        {   279 ,   31  },  /*  u   75  */
        {   280 ,   32  },  /*  v   76  */
        {   281 ,   33  },  /*  w   77  */
        {   282 ,   34  },  /*  x   78  */
        {   283 ,   35  },  /*  y   79  */
        {   284 ,   36  },  /*  z   7A  */
};


#define UNICODE_EUROPE_T4_OFFSET			(192U)

const UNICODE_MATCH_INFO unicode_europe_table4[] =
{
        {   285 ,   37  },  /*  À   C0  */
        {   286 ,   38  },  /*  Á   C1  */
        {   287 ,   39  },  /*  Â   C2  */
        {   288 ,   40  },  /*  Ã   C3  */
        {   289 ,   41  },  /*  Ä   C4  */
        {   290 ,   42  },  /*  Å   C5  */
        {   291 ,   43  },  /*  Æ   C6  */
        {   292 ,   44  },  /*  Ç   C7  */
        {   293 ,   45  },  /*  È   C8  */
        {   294 ,   46  },  /*  É   C9  */
        {   295 ,   47  },  /*  Ê   CA  */
        {   296 ,   48  },  /*  Ë   CB  */
        {   297 ,   49  },  /*  Ì   CC  */
        {   298 ,   50  },  /*  Í   CD  */
        {   299 ,   51  },  /*  Î   CE  */
        {   300 ,   52  },  /*  Ï   CF  */
        {   301 ,   53  },  /*  Ð   D0  */
        {   302 ,   54  },  /*  Ñ   D1  */
        {   303 ,   55  },  /*  Ò   D2  */
        {   304 ,   56  },  /*  Ó   D3  */
        {   305 ,   57  },  /*  Ô   D4  */
        {   306 ,   58  },  /*  Õ   D5  */
        {   307 ,   59  },  /*  Ö   D6  */
        {   308 ,   60  },  /*  ×   D7  */
        {   309 ,   61  },  /*  Ø   D8  */
        {   310 ,   62  },  /*  Ù   D9  */
        {   311 ,   63  },  /*  Ú   DA  */
        {   312 ,   64  },  /*  Û   DB  */
        {   313 ,   65  },  /*  Ü   DC  */
        {   314 ,   66  },  /*  Ý   DD  */
        {   315 ,   67  },  /*  Þ   DE  */
        {   316 ,   68  },  /*  ß   DF  */
        {   382 ,   37  },  /*  à   E0  */
        {   383 ,   38  },  /*  á   E1  */
        {   384 ,   39  },  /*  â   E2  */
        {   385 ,   40  },  /*  ã   E3  */
        {   386 ,   41  },  /*  ä   E4  */
        {   387 ,   42  },  /*  å   E5  */
        {   388 ,   43  },  /*  æ   E6  */
        {   389 ,   44  },  /*  ç   E7  */
        {   390 ,   45  },  /*  è   E8  */
        {   391 ,   46  },  /*  é   E9  */
        {   392 ,   47  },  /*  ê   EA  */
        {   393 ,   48  },  /*  ë   EB  */
        {   394 ,   49  },  /*  ì   EC  */
        {   395 ,   50  },  /*  í   ED  */
        {   396 ,   51  },  /*  î   EE  */
        {   397 ,   52  },  /*  ï   EF  */
        {   398 ,   53  },  /*  ð   F0  */
        {   399 ,   54  },  /*  ñ   F1  */
        {   400 ,   55  },  /*  ò   F2  */
        {   401 ,   56  },  /*  ó   F3  */
        {   402 ,   57  },  /*  ô   F4  */
        {   403 ,   58  },  /*  õ   F5  */
        {   404 ,   59  },  /*  ö   F6  */
        {   405 ,   60  },  /*  ÷   F7  */
        {   406 ,   61  },  /*  ø   F8  */
        {   407 ,   62  },  /*  ù   F9  */
        {   408 ,   63  },  /*  ú   FA  */
        {   409 ,   64  },  /*  û   FB  */
        {   410 ,   65  },  /*  ü   FC  */
        {   411 ,   66  },  /*  ý   FD  */
        {   412 ,   67  },  /*  þ   FE  */
        {   413 ,   130 },  /*  ÿ   FF  */
        {   317 ,   69  },  /*  Ā   100 */
        {   414 ,   69  },  /*  ā   101 */
        {   318 ,   70  },  /*  Ă   102 */
        {   415 ,   70  },  /*  ă   103 */
        {   319 ,   71  },  /*  Ą   104 */
        {   416 ,   71  },  /*  ą   105 */
        {   320 ,   72  },  /*  Ć   106 */
        {   417 ,   72  },  /*  ć   107 */
        {   321 ,   73  },  /*  Ĉ   108 */
        {   418 ,   73  },  /*  ĉ   109 */
        {   322 ,   74  },  /*  Ċ   10A */
        {   419 ,   74  },  /*  ċ   10B */
        {   323 ,   75  },  /*  Č   10C */
        {   420 ,   75  },  /*  č   10D */
        {   324 ,   76  },  /*  Ď   10E */
        {   421 ,   76  },  /*  ď   10F */
        {   325 ,   77  },  /*  Đ   110 */
        {   422 ,   77  },  /*  đ   111 */
        {   326 ,   78  },  /*  Ē   112 */
        {   423 ,   78  },  /*  ē   113 */
        {   327 ,   79  },  /*  Ĕ   114 */
        {   424 ,   79  },  /*  ĕ   115 */
        {   328 ,   80  },  /*  Ė   116 */
        {   425 ,   80  },  /*  ė   117 */
        {   329 ,   81  },  /*  Ę   118 */
        {   426 ,   81  },  /*  ę   119 */
        {   330 ,   82  },  /*  Ě   11A */
        {   427 ,   82  },  /*  ě   11B */
        {   331 ,   83  },  /*  Ĝ   11C */
        {   428 ,   83  },  /*  ĝ   11D */
        {   332 ,   84  },  /*  Ğ   11E */
        {   429 ,   84  },  /*  ğ   11F */
        {   333 ,   85  },  /*  Ġ   120 */
        {   430 ,   85  },  /*  ġ   121 */
        {   334 ,   86  },  /*  Ģ   122 */
        {   431 ,   86  },  /*  ģ   123 */
        {   335 ,   87  },  /*  Ĥ   124 */
        {   432 ,   87  },  /*  ĥ   125 */
        {   336 ,   88  },  /*  Ħ   126 */
        {   433 ,   88  },  /*  ħ   127 */
        {   337 ,   89  },  /*  Ĩ   128 */
        {   434 ,   89  },  /*  ĩ   129 */
        {   338 ,   90  },  /*  Ī   12A */
        {   435 ,   90  },  /*  ī   12B */
        {   339 ,   91  },  /*  Ĭ   12C */
        {   436 ,   91  },  /*  ĭ   12D */
        {   340 ,   92  },  /*  Į   12E */
        {   437 ,   92  },  /*  į   12F */
        {   341 ,   93  },  /*  İ   130 */
        {   438 ,   93  },  /*  ı   131 */
        {   342 ,   94  },  /*  Ĳ   132 */
        {   439 ,   94  },  /*  ĳ   133 */
        {   343 ,   95  },  /*  Ĵ   134 */
        {   440 ,   95  },  /*  ĵ   135 */
        {   344 ,   96  },  /*  Ķ   136 */
        {   441 ,   96  },  /*  ķ   137 */
        {   345 ,   97  },  /*  ĸ   138 */
        {   346 ,   98  },  /*  Ĺ   139 */
        {   442 ,   98  },  /*  ĺ   13A */
        {   347 ,   99  },  /*  Ļ   13B */
        {   443 ,   99  },  /*  ļ   13C */
        {   348 ,   100 },  /*  Ľ   13D */
        {   444 ,   100 },  /*  ľ   13E */
        {   349 ,   101 },  /*  Ŀ   13F */
        {   445 ,   101 },  /*  ŀ   140 */
        {   350 ,   102 },  /*  Ł   141 */
        {   446 ,   102 },  /*  ł   142 */
        {   351 ,   103 },  /*  Ń   143 */
        {   447 ,   103 },  /*  ń   144 */
        {   352 ,   104 },  /*  Ņ   145 */
        {   448 ,   104 },  /*  ņ   146 */
        {   353 ,   105 },  /*  Ň   147 */
        {   449 ,   105 },  /*  ň   148 */
        {   354 ,   106 },  /*  ŉ   149 */
        {   355 ,   107 },  /*  Ŋ   14A */
        {   450 ,   107 },  /*  ŋ   14B */
        {   356 ,   108 },  /*  Ō   14C */
        {   451 ,   108 },  /*  ō   14D */
        {   357 ,   109 },  /*  Ŏ   14E */
        {   452 ,   109 },  /*  ŏ   14F */
        {   358 ,   110 },  /*  Ő   150 */
        {   453 ,   110 },  /*  ő   151 */
        {   359 ,   111 },  /*  Œ   152 */
        {   454 ,   111 },  /*  œ   153 */
        {   360 ,   112 },  /*  Ŕ   154 */
        {   455 ,   112 },  /*  ŕ   155 */
        {   361 ,   113 },  /*  Ŗ   156 */
        {   456 ,   113 },  /*  ŗ   157 */
        {   362 ,   114 },  /*  Ř   158 */
        {   457 ,   114 },  /*  ř   159 */
        {   363 ,   115 },  /*  Ś   15A */
        {   458 ,   115 },  /*  ś   15B */
        {   364 ,   116 },  /*  Ŝ   15C */
        {   459 ,   116 },  /*  ŝ   15D */
        {   365 ,   117 },  /*  Ş   15E */
        {   460 ,   117 },  /*  ş   15F */
        {   366 ,   118 },  /*  Š   160 */
        {   461 ,   118 },  /*  š   161 */
        {   367 ,   119 },  /*  Ţ   162 */
        {   462 ,   119 },  /*  ţ   163 */
        {   368 ,   120 },  /*  Ť   164 */
        {   463 ,   120 },  /*  ť   165 */
        {   369 ,   121 },  /*  Ŧ   166 */
        {   464 ,   121 },  /*  ŧ   167 */
        {   370 ,   122 },  /*  Ũ   168 */
        {   465 ,   122 },  /*  ũ   169 */
        {   371 ,   123 },  /*  Ū   16A */
        {   466 ,   123 },  /*  ū   16B */
        {   372 ,   124 },  /*  Ŭ   16C */
        {   467 ,   124 },  /*  ŭ   16D */
        {   373 ,   125 },  /*  Ů   16E */
        {   468 ,   125 },  /*  ů   16F */
        {   374 ,   126 },  /*  Ű   170 */
        {   469 ,   126 },  /*  ű   171 */
        {   375 ,   127 },  /*  Ų   172 */
        {   470 ,   127 },  /*  ų   173 */
        {   376 ,   128 },  /*  Ŵ   174 */
        {   471 ,   128 },  /*  ŵ   175 */
        {   377 ,   129 },  /*  Ŷ   176 */
        {   472 ,   129 },  /*  ŷ   177 */
        {   378 ,   130 },  /*  Ÿ   178 */
        {   379 ,   131 },  /*  Ź   179 */
        {   473 ,   131 },  /*  ź   17A */
        {   380 ,   132 },  /*  Ż   17B */
        {   474 ,   132 },  /*  ż   17C */
        {   381 ,   133 },  /*  Ž   17D */
        {   475 ,   133 },  /*  ž   17E */

};



#define UNICODE_GREEK_T5_OFFSET			(902U)

const UNICODE_MATCH_INFO unicode_greek_table5[] =
{
        {   524 ,   158 },  /*  Ά   386 */
        {   65535,  65535   },  /*  null    387 */
        {   525 ,   159 },  /*  Έ   388 */
        {   526 ,   160 },  /*  Ή   389 */
        {   527 ,   161 },  /*  Ί   38A */
        {   65535,  65535   },  /*  null    38B */
        {   529 ,   163 },  /*  Ό   38C */
        {   65535   ,   65535   },  /*  null    38D */
        {   530 ,   164 },  /*  Ύ   38E */
        {   532 ,   166 },  /*  Ώ   38F */
        {   65535,  65535   },  /*  null    390 */
        {   476 ,   134 },  /*  Α   391 */
        {   477 ,   135 },  /*  Β   392 */
        {   478 ,   136 },  /*  Γ   393 */
        {   479 ,   137 },  /*  Δ   394 */
        {   480 ,   138 },  /*  Ε   395 */
        {   481 ,   139 },  /*  Ζ   396 */
        {   482 ,   140 },  /*  Η   397 */
        {   483 ,   141 },  /*  Θ   398 */
        {   484 ,   142 },  /*  Ι   399 */
        {   485 ,   143 },  /*  Κ   39A */
        {   486 ,   144 },  /*  Λ   39B */
        {   487 ,   145 },  /*  Μ   39C */
        {   488 ,   146 },  /*  Ν   39D */
        {   489 ,   147 },  /*  Ξ   39E */
        {   490 ,   148 },  /*  Ο   39F */
        {   491 ,   149 },  /*  Π   3A0 */
        {   492 ,   150 },  /*  Ρ   3A1 */
        {   65535,  65535   },  /*  null    3a2 */
        {   493 ,   151 },  /*  Σ   3A3 */
        {   494 ,   152 },  /*  Τ   3A4 */
        {   495 ,   153 },  /*  Υ   3A5 */
        {   496 ,   154 },  /*  Φ   3A6 */
        {   497 ,   155 },  /*  Χ   3A7 */
        {   498 ,   156 },  /*  Ψ   3A8 */
        {   499 ,   157 },  /*  Ω   3A9 */
        {   528 ,   162 },  /*  Ϊ   3AA */
        {   531 ,   165 },  /*  Ϋ   3AB */
        {   533 ,   158 },  /*  ά   3AC */
        {   534 ,   159 },  /*  έ   3AD */
        {   535 ,   160 },  /*  ή   3AE */
        {   536 ,   161 },  /*  ί   3AF */
        {   65535,  65535   },  /*  null    3b0 */
        {   500 ,   134 },  /*  α   3B1 */
        {   501 ,   135 },  /*  β   3B2 */
        {   502 ,   136 },  /*  γ   3B3 */
        {   503 ,   137 },  /*  δ   3B4 */
        {   504 ,   138 },  /*  ε   3B5 */
        {   505 ,   139 },  /*  ζ   3B6 */
        {   506 ,   140 },  /*  η   3B7 */
        {   507 ,   141 },  /*  θ   3B8 */
        {   508 ,   142 },  /*  ι   3B9 */
        {   509 ,   143 },  /*  κ   3BA */
        {   510 ,   144 },  /*  λ   3BB */
        {   511 ,   145 },  /*  μ   3BC */
        {   512 ,   146 },  /*  ν   3BD */
        {   513 ,   147 },  /*  ξ   3BE */
        {   514 ,   148 },  /*  ο   3BF */
        {   515 ,   149 },  /*  π   3C0 */
        {   516 ,   150 },  /*  ρ   3C1 */
        {   65535,  65535   },  /*  null    3c2 */
        {   517 ,   151 },  /*  σ   3C3 */
        {   518 ,   152 },  /*  τ   3C4 */
        {   519 ,   153 },  /*  υ   3C5 */
        {   520 ,   154 },  /*  φ   3C6 */
        {   521 ,   155 },  /*  χ   3C7 */
        {   522 ,   156 },  /*  ψ   3C8 */
        {   523 ,   157 },  /*  ω   3C9 */
        {   537 ,   162 },  /*  ϊ   3CA */
        {   540 ,   165 },  /*  ϋ   3CB */
        {   538 ,   163 },  /*  ό   3CC */
        {   539 ,   164 },  /*  ύ   3CD */
        {   541 ,   166 },  /*  ώ   3CE */

};



sint8 japanese_kana_find_sorted_index(uint16 input_code, uint32* group_index, uint32* exact_index);
sint8 greek_find_sorted_index(uint16 input_code, uint32* group_index, uint32* exact_index);
sint8 europe_find_sorted_index(uint16 input_code, uint32* group_index, uint32* exact_index);


sint8 character_find_sorted_index(uint16 input_code, uint32* group_index, uint32* exact_index)
{
	sint8 ret = 0;

	switch (input_code & LANGUAGE_CATEGORY_MASK)
	{
	case KANA_CATEGORY_TABLE1:
	case KANA_CATEGORY_TABLE2:
		ret = japanese_kana_find_sorted_index(input_code, group_index, exact_index);
		break;
	case GREEK_CATEGORY_TABLE:
		ret = greek_find_sorted_index(input_code, group_index, exact_index);
		break;
	case EUROPE_CATEGORY_TABLE1:
	case EUROPE_CATEGORY_TABLE2:
		ret = europe_find_sorted_index(input_code, group_index, exact_index);
		break;
	default:
		ret = INPUT_CHAR_INDEX_INVALID;
		break;
	}

	return ret;
}

sint8 japanese_kana_find_sorted_index(uint16 input_code, uint32* group_index, uint32* exact_index)
{
	uint32 offset = 0;

	if (input_code >= UNICODE_JAPAN_T2_OFFSET)	/* input character is allocated in table2 */
	{
		/* calculate the offset according to input character unicode value */
		offset = input_code - UNICODE_JAPAN_T2_OFFSET;

		if ((offset < sizeof(unicode_japan_table2)/sizeof(UNICODE_MATCH_INFO))
				&& (65535 != unicode_japan_table2[offset].index_exact))	/* the line in the table is valid */
		{
			if (NULL != group_index)
			{
				*group_index = unicode_japan_table2[offset].index_group;
			}

			if (NULL != exact_index)
			{
				*exact_index = unicode_japan_table2[offset].index_exact;
			}
		}
		else
		{
			return INPUT_CHAR_INDEX_INVALID;
		}
	}
	else
	{
		/* calculate the offset according to input character unicode value */
		offset = input_code - UNICODE_JAPAN_T1_OFFSET;

		/* For table1, there are two part. In each part, index is consecutive. */
		/* But between these two parts, index is not consecutive. So offset need to update. */
		if (offset > UNICODE_JAPAN_T1_SUB_PHASE)
		{
			offset -= UNICODE_JAPAN_T1_SUB_OFFSET;
		}
		else
		{

		}

		if ((offset < sizeof(unicode_japan_table1)/sizeof(UNICODE_MATCH_INFO))
			&& (65535 != unicode_japan_table1[offset].index_exact))	/* the line in the table is valid */
		{
			if (NULL != group_index)
			{
				*group_index = unicode_japan_table1[offset].index_group;
			}

			if (NULL != exact_index)
			{
				*exact_index = unicode_japan_table1[offset].index_exact;
			}
		}
		else
		{
			return INPUT_CHAR_INDEX_INVALID;
		}
	}

	return INPUT_CHAR_INDEX_VALID;
}


sint8 greek_find_sorted_index(uint16 input_code, uint32* group_index, uint32* exact_index)
{
	uint32 offset = 0;

	if (input_code >= UNICODE_GREEK_T5_OFFSET)	/* input character is allocated in table5 */
	{
		/* calculate the offset according to input character unicode value */
		offset = input_code - UNICODE_GREEK_T5_OFFSET;

		if ((offset < sizeof(unicode_greek_table5)/sizeof(UNICODE_MATCH_INFO))
				&& (65535 != unicode_greek_table5[offset].index_exact))	/* the line in the table is valid */
		{
			if (NULL != group_index)
			{
				*group_index = unicode_greek_table5[offset].index_group;
			}

			if (NULL != exact_index)
			{
				*exact_index = unicode_greek_table5[offset].index_exact;
			}
		}
		else
		{
			return INPUT_CHAR_INDEX_INVALID;
		}
	}
	else
	{
		return INPUT_CHAR_INDEX_INVALID;
	}

	return INPUT_CHAR_INDEX_VALID;
}


sint8 europe_find_sorted_index(uint16 input_code, uint32* group_index, uint32* exact_index)
{
	uint32 offset = 0;

	if (input_code >= UNICODE_EUROPE_T4_OFFSET)	/* input character is allocated in table5 */
	{
		/* calculate the offset according to input character unicode value */
		offset = input_code - UNICODE_EUROPE_T4_OFFSET;

		if ((offset < sizeof(unicode_europe_table4)/sizeof(UNICODE_MATCH_INFO))
				&& (65535 != unicode_europe_table4[offset].index_exact))	/* the line in the table is valid */
		{
			if (NULL != group_index)
			{
				*group_index = unicode_europe_table4[offset].index_group;
			}

			if (NULL != exact_index)
			{
				*exact_index = unicode_europe_table4[offset].index_exact;
			}
		}
		else
		{
			return INPUT_CHAR_INDEX_INVALID;
		}
	}
	else
	{
		/* calculate the offset according to input character unicode value */
		offset = input_code - UNICODE_EUROPE_T3_CAPICAL_OFFSET;

		if (offset >= UNICODE_EUROPE_T3_SUB_OFFSET)
		{
			offset = input_code - UNICODE_EUROPE_T3_LOWCASE_OFFSET + UNICODE_EUROPE_T3_SUB_OFFSET;
		}

		if ((offset < sizeof(unicode_europe_table3)/sizeof(UNICODE_MATCH_INFO))
				&& (65535 != unicode_europe_table3[offset].index_exact))	/* the line in the table is valid */
		{
			if (NULL != group_index)
			{
				*group_index = unicode_europe_table3[offset].index_group;
			}

			if (NULL != exact_index)
			{
				*exact_index = unicode_europe_table3[offset].index_exact;
			}
		}
		else
		{
			return INPUT_CHAR_INDEX_INVALID;
		}
	}

	return INPUT_CHAR_INDEX_VALID;
}

sint8 character_group_compare(uint16 input1, uint16 input2)
{
	uint32 input1_group_index = 0;
	uint32 input2_group_index = 0;
	sint8  input1_ret = 0;
	sint8  input2_ret = 0;
	sint8  ret = 0;

	input1_ret = character_find_sorted_index(input1, &input1_group_index, NULL);
	input2_ret = character_find_sorted_index(input2, &input2_group_index, NULL);

	if ((-1 == input1_ret) && (0 == input2_ret))    /* input1 is not a legal character */
    {
        return SORT_ORDER_INPUT2_FIRST;
    }
    if ((-1 == input2_ret) && (0 == input1_ret))    /* input2 is not a legal character */
    {
        return SORT_ORDER_INPUT1_FIRST;
    }
    if ((input1_ret) && (input2_ret))   /* both are not legal characters */
    {
        if (input1 < input2)
        {
            return SORT_ORDER_INPUT1_FIRST;
        }
        else if (input1 > input2)
        {
            return SORT_ORDER_INPUT2_FIRST;
        }
        else
        {
            ret = SORT_ORDER_INPUT_SAME;
        }
    }

	if (input1_group_index > input2_group_index)
	{
		ret = SORT_ORDER_INPUT2_FIRST;
	}
	else if (input1_group_index < input2_group_index)
	{
		ret = SORT_ORDER_INPUT1_FIRST;
	}
	else
	{
		ret = SORT_ORDER_INPUT_SAME;
	}

	return ret;
}


sint8 character_compare(uint16 input1, uint16 input2)
{
	uint32 input1_exact_index = 0;
	uint32 input2_exact_index = 0;
	sint8  input1_ret = 0;
	sint8  input2_ret = 0;
	sint8  ret = SORT_ORDER_INPUT_SAME;

	input1_ret = character_find_sorted_index(input1, NULL, &input1_exact_index);
	input2_ret = character_find_sorted_index(input2, NULL, &input2_exact_index);

	if ((-1 == input1_ret) && (0 == input2_ret))	/* input1 is not a legal character */
	{
		return SORT_ORDER_INPUT2_FIRST;
	}
	if ((-1 == input2_ret) && (0 == input1_ret))	/* input2 is not a legal character */
	{
		return SORT_ORDER_INPUT1_FIRST;
	}
	if ((input1_ret) && (input2_ret))	/* both are not legal characters */
	{
	    if (input1 < input2)
	    {
	        return SORT_ORDER_INPUT1_FIRST;
	    }
	    else if (input1 > input2)
	    {
	        return SORT_ORDER_INPUT2_FIRST;
	    }
	    else
	    {
	        ret = SORT_ORDER_INPUT_SAME;
	    }
	}

	if (input1_exact_index > input2_exact_index)
	{
		ret = SORT_ORDER_INPUT2_FIRST;
	}
	else if (input1_exact_index < input2_exact_index)
	{
		ret = SORT_ORDER_INPUT1_FIRST;
	}
	else
	{
		ret = SORT_ORDER_INPUT_SAME;
	}

	return ret;
}



sint8 string_compare(uint16* input1, uint16 input1_size, uint16* input2, uint16 input2_size)
{
	uint16 loopcnt = 0;
	sint8  compare_ret = SORT_ORDER_INPUT_SAME;

	for (loopcnt = 0; (loopcnt < input1_size) && (loopcnt < input2_size); loopcnt++)
	{
		compare_ret = character_compare(input1[loopcnt], input2[loopcnt]);

		if (SORT_ORDER_INPUT_SAME != compare_ret)
		{
			if (SORT_ORDER_INPUT_INVALID == compare_ret)
			{
				if (input1[loopcnt] < input2[loopcnt])
				{
					compare_ret = SORT_ORDER_INPUT1_FIRST;

					break;
				}
				else if (input1[loopcnt] > input2[loopcnt])
				{
					compare_ret = SORT_ORDER_INPUT2_FIRST;

					break;
				}
				else
				{}
			}
			else
			{
				break;
			}
		}
	}

	if (SORT_ORDER_INPUT_SAME == compare_ret)
	{
		if (input1_size < input2_size)
		{
			compare_ret = SORT_ORDER_INPUT1_FIRST;
		}
		else if (input1_size > input2_size)
		{
			compare_ret = SORT_ORDER_INPUT2_FIRST;
		}
		else
		{}
	}

	return compare_ret;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  string_orderByAlphabetForNagiviJp
 *  Description:  
 * =====================================================================================
 */
gint string_orderByAlphabetForNagiviJp(gconstpointer a, gconstpointer b )
{
	char ** _str1	=	( char ** )a;
	char ** _str2	=	( char ** )b;
	char * _str1_collateKey;
	char * _str2_collateKey;

	static GIConv _conv =	NULL;
	gchar * _in = NULL, * _out;
	gushort * _collate, * _off1, * _off2;
	int i = 0;
	uint16 input1_size = 0, input2_size = 0;
	gint res = 0;
	
	if ((*_str1 == NULL || g_strcmp0(*_str1,"") == 0) 
		&& (*_str2 == NULL ||  g_strcmp0(*_str2,"") == 0))
	{
		return 0;
	}
	else if ((*_str1 == NULL || g_strcmp0(*_str1,"") == 0)
		&& *_str2 != NULL)
	{
		return 1;
	}
	else if (*_str1 != NULL 
		&& (*_str2 == NULL ||  g_strcmp0(*_str2,"") == 0))
	{
		return -1;
	}


	if( _conv == NULL )
	{
		_conv	=	g_iconv_open( "utf-16le", "utf-8" );
	}

	gchar _buf[512] =	{ 0 };
	gsize _insize, _outsize =	sizeof( _buf );
	char  fc;
	char  tmp_buf[8];

	_in 	=	*_str1;
	_out	=	_buf;
	if (_in == NULL)
	{
		_insize = 0;
	}
	else
	{
		_insize	=	strlen( _in );
	}

	//g_message("_in = %s ", _in);
	if( g_iconv( _conv, &_in, &_insize, &_out, &_outsize ) != -1 )
	{
		_str1_collateKey	=	g_malloc0( sizeof( _buf ) - _outsize + 2 );
		input1_size = (sizeof( _buf ) - _outsize + 2) / 2;

		_collate	=	( gushort * )_str1_collateKey;

		for( i = 0; i < sizeof( _buf ) - _outsize; i += 2, _collate ++ )
		{
			* _collate = ( guint8 )_buf[i + 1] << 8 | ( guint8 )_buf[ i ];
			//printf("0x%4x ", * _collate);
		}
		//printf("size = %d \n", input1_size);
		* _collate	=	0x0000;
	}
	else
	{
		perror( "string_orderByAlphabetForNagiviJp" );
		return -1;
	}


	_outsize = sizeof( _buf );

	_in 	=	*_str2;
	_out	=	_buf;

	if (_in == NULL)
	{
		_insize = 0;
	}
	else
	{
		_insize	=	strlen( _in );
	}

	if( g_iconv( _conv, &_in, &_insize, &_out, &_outsize ) != -1 )
	{
		_str2_collateKey	=	g_malloc0( sizeof( _buf ) - _outsize + 2 );
		input2_size = (sizeof( _buf ) - _outsize + 2) / 2;

		_collate	=	( gushort * )_str2_collateKey;

		for( i = 0; i < sizeof( _buf ) - _outsize; i += 2, _collate ++ )
		{
			* _collate = ( guint8 )_buf[i + 1] << 8 | ( guint8 )_buf[ i ];
		}

		* _collate	=	0x0000;
	}
	else
	{
		perror( "string_orderByAlphabetForNagiviJp" );
		g_free( _str1_collateKey );
		return -1;
	}

	_off1	=	( gushort * )_str1_collateKey;
	_off2	=	( gushort * )_str2_collateKey;

	if( _off1 == NULL || _off2 == NULL ) return 0;

	res = string_compare(_off1, input1_size, _off2, input2_size);
	if (res == -1)
		res = 1;
	else if (res == 1)
		res = -1;
	else
		res = 0;
	g_free( _str1_collateKey );
	g_free( _str2_collateKey );
	
	return res;
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  string_orderByAlphabetForNagivi
 *  Description:  
 * =====================================================================================
 */
gint string_orderByAlphabetForNagivi(gconstpointer a, gconstpointer b )
{
	char ** _str1	=	( char ** )a;
	char ** _str2	=	( char ** )b;
	char * _str1_collateKey;
	char * _str2_collateKey;

	static GIConv _conv =	NULL;
	gchar * _in = NULL, * _out;
	gushort * _collate, * _off1, * _off2;
	gushort arrIndex = 0;
	int i;
	int min = 0;
	
	if ((*_str1 == NULL || g_strcmp0(*_str1,"") == 0) 
		&& (*_str2 == NULL ||  g_strcmp0(*_str2,"") == 0))
	{
		return 0;
	}
	else if ((*_str1 == NULL || g_strcmp0(*_str1,"") == 0)
		&& *_str2 != NULL)
	{
		return 1;
	}
	else if (*_str1 != NULL 
		&& (*_str2 == NULL ||  g_strcmp0(*_str2,"") == 0))
	{
		return -1;
	}

	if( _conv == NULL )
	{
		_conv	=	g_iconv_open( "utf-16le", "utf-8" );
	}

	gchar _buf[512] =	{ 0 };
	gsize _insize, _outsize =	sizeof( _buf );
	char  fc;
	char  tmp_buf[8];

	_in 	=	*_str1;
	_out	=	_buf;

	if (_in == NULL)
	{
		_insize = 0;
	}
	else
	{
		_insize	=	strlen( _in );
	}

	const char * _skipArticleStr = getenv("MH_ORDER_ARTICLE_STATE");
	_skipArticleStr = _skipArticleStr ? _skipArticleStr : "0";
	guint _skipArticle = (guint)atoi(_skipArticleStr);
	if(0 == _skipArticle && _insize > 3)
	{
		/* skip capital: A, AN, THE */
		fc = toupper(_in[0]);
		memset(tmp_buf, 0, sizeof(tmp_buf));
		if(_insize > 3 && ((fc == 'A') || (fc == 'T')))
		{
			min = 5 < _insize ? 5 : _insize;
			for(i=0; i<min; i++)
			{
				tmp_buf[i] = toupper(_in[i]);
			}

			if((memcmp("A ", tmp_buf, 2) == 0) && (_insize > 2))
			{
				_in = (_in + 2);
				_insize -= 2;
			}
			else if((memcmp("AN ", tmp_buf, 3) == 0) && (_insize > 3))
			{
				_in = (_in + 3);
				_insize -= 3;
			}
			else if((memcmp("THE ", tmp_buf, 4) == 0) && (_insize > 4))
			{
				_in = (_in + 4);
				_insize -= 4;
			}
		}
	}


	//g_message("_in = %s ", _in);
	if( g_iconv( _conv, &_in, &_insize, &_out, &_outsize ) != -1 )
	{
		_str1_collateKey	=	g_malloc0( sizeof( _buf ) - _outsize + 2 );

		_collate	=	( gushort * )_str1_collateKey;

		for( i = 0; i < sizeof( _buf ) - _outsize; i += 2, _collate ++ )
		{
			arrIndex = ( guint8 )_buf[i + 1] << 8 | ( guint8 )_buf[ i ];
			if (arrIndex < 0x80)
			{
				* _collate	=	UniCode2ASCII_Tbl[ arrIndex];
			}
			else
			{
				* _collate	=	arrIndex;
			}
			//printf("0x%x ", * _collate);
		}
		//printf("\n");
		* _collate	=	0xFFFF;
	}
	else
	{
		perror( "string_orderByPinyin" );
		return -1;
	}


	_outsize = sizeof( _buf );

	_in 	=	*_str2;
	_out	=	_buf;

	if (_in == NULL)
	{
		_insize = 0;
	}
	else
	{
		_insize	=	strlen( _in );
	}

	if(0 == _skipArticle && _insize > 3)
	{
		/* skip capital: A, AN, THE */
		fc = toupper(_in[0]);
		memset(tmp_buf, 0, sizeof(tmp_buf));
		if(_insize > 3 && ((fc == 'A') || (fc == 'T')))
		{
			min = 5 < _insize ?	5 : _insize;
			for(i=0; i<min; i++)
			{
				tmp_buf[i] = toupper(_in[i]);
			}

			if((memcmp("A ", tmp_buf, 2) == 0) && (_insize > 2))
			{
				_in = (_in + 2);
				_insize -= 2;
			}
			else if((memcmp("AN ", tmp_buf, 3) == 0) && (_insize > 3))
			{
				_in = (_in + 3);
				_insize -= 3;
			}
			else if((memcmp("THE ", tmp_buf, 4) == 0) && (_insize > 4))
			{
				_in = (_in + 4);
				_insize -= 4;
			}
		}
	}

	if( g_iconv( _conv, &_in, &_insize, &_out, &_outsize ) != -1 )
	{
		_str2_collateKey	=	g_malloc0( sizeof( _buf ) - _outsize + 2 );

		_collate	=	( gushort * )_str2_collateKey;

		for( i = 0; i < sizeof( _buf ) - _outsize; i += 2, _collate ++ )
		{
			arrIndex = ( guint8 )_buf[i + 1] << 8 | ( guint8 )_buf[ i ];
			if (arrIndex < 0x80)
			{
				* _collate	=	UniCode2ASCII_Tbl[ arrIndex];
			}
			else
			{
				* _collate	=	arrIndex;
			}
		}

		* _collate	=	0xFFFF;
	}
	else
	{

		perror( "string_orderByPinyin" );
		g_free( _str1_collateKey );
		return -1;
	}

	_off1	=	( gushort * )_str1_collateKey;
	_off2	=	( gushort * )_str2_collateKey;

	if( _off1 == NULL || _off2 == NULL ) return 0;

	while( TRUE )
	{
		if( * _off1 == 0xFFFF ) 
		{
			g_free( _str1_collateKey );
			g_free( _str2_collateKey );
			return -1;

		}	
		if( * _off2 == 0xFFFF ) 
		{
			g_free( _str1_collateKey );
			g_free( _str2_collateKey );
			return 1;

		}
		if( * _off1 == * _off2 )
		{
			_off1 ++;
			_off2 ++;

			continue;
		}

		if( * _off1 == 0 )
		{
			_off1 ++;

			continue;
		}

		if( * _off2 == 0 )
		{
			_off2 ++;

			continue;
		}

		if( * _off1 > * _off2 ) 
		{
			g_free( _str1_collateKey );
			g_free( _str2_collateKey );
			return 1;
		}
		if( * _off1 < * _off2 ) 
		{
			g_free( _str1_collateKey );
			g_free( _str2_collateKey );
			return -1;
		}
	}

}		/* -----  end of function string_orderByAlphabet  ----- */


/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  string_orderByAlphabet
 *  Description:  
 * =====================================================================================
 */
gint string_orderByAlphabet(gconstpointer a, gconstpointer b )
{
	gint _res;
	char ** _str1	=	( char ** )a;
	char ** _str2	=	( char ** )b;
	
	char * _str1_collateKey;
	char * _str2_collateKey;

	_str1_collateKey	=	g_utf8_collate_key_for_filename( *_str1, strlen( *_str1 ));

	_str2_collateKey	=	g_utf8_collate_key_for_filename( *_str2, strlen( *_str2 ));

	_res	=	g_strcmp0( _str1_collateKey, _str2_collateKey );

	g_free( _str1_collateKey );
	g_free( _str2_collateKey );

	return _res;
}		/* -----  end of function string_orderByAlphabet  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  string_orderByPinyin
 *  Description:  
 * =====================================================================================
 */
gint string_orderByPinyin(gconstpointer a, gconstpointer b )
{

	char ** _str1	=	( char ** )a;
	char ** _str2	=	( char ** )b;
	char * _str1_collateKey;
	char * _str2_collateKey;

	static GIConv _conv	=	NULL;
	gchar * _in = NULL, * _out;
	gushort * _collate, * _off1, * _off2;
	int i;

	if( _conv == NULL )
	{
		_conv   =   g_iconv_open( "utf-16le", "utf-8" );
	}

	gchar _buf[512]	=	{ 0 };
	gsize _insize, _outsize	=	sizeof( _buf );

	_in		=	*_str1;
	_out	=	_buf;

	if (_in == NULL)
	{
		_insize = 0;
	}
	else
	{
		_insize	=	strlen( _in );
	}

	if( g_iconv( _conv, &_in, &_insize, &_out, &_outsize ) != -1 )
	{
		_str1_collateKey	=	g_malloc0( sizeof( _buf ) - _outsize + 2 );

		_collate	=	( gushort * )_str1_collateKey;

		for( i = 0; i < sizeof( _buf ) - _outsize; i += 2, _collate ++ )
		{
			* _collate	=	UniCode2Pinyin_Tbl[ ( guint8 )_buf[i + 1] << 8 | ( guint8 )_buf[ i ] ];
			/*
				abc:0x01xx--0x0300 
				123:0x09xx--0x09xx
				ascii:0x0Axx--0x0Axx
				other:0x0AFF
			 */
			if(* _collate == 0x0000)
				* _collate = 0x0AFF;

			if(* _collate < 0x0080)
				* _collate = * _collate | 0x0A00;			
/* 				if( * _collate == 0 )
 * 				{
 * 					g_warning( "Convert %02x%02x to pinyin failed(%s)", ( guint8 )_buf[i + 1], ( guint8 )_buf[ i ], _item1->name );
 * 				}
 */
		}

		* _collate	=	0xFFFF;
	}
	else
	{
		perror( "string_orderByPinyin" );
		return -1;
	}


	_outsize = sizeof( _buf );
	_in = NULL;

	_in		=	*_str2;
	_out	=	_buf;

	if (_in == NULL)
	{
		_insize = 0;
	}
	else
	{
		_insize	=	strlen( _in );
	}

	if( g_iconv( _conv, &_in, &_insize, &_out, &_outsize ) != -1 )
	{
		_str2_collateKey	=	g_malloc0( sizeof( _buf ) - _outsize + 2 );

		_collate	=	( gushort * )_str2_collateKey;

		for( i = 0; i < sizeof( _buf ) - _outsize; i += 2, _collate ++ )
		{
			* _collate	=	UniCode2Pinyin_Tbl[ ( guint8 )_buf[i + 1] << 8 | ( guint8 )_buf[ i ] ];
			
			if(* _collate == 0x0000)
				* _collate = 0x0AFF;

			if(* _collate < 0x0080)
				* _collate = * _collate | 0x0A00;

/* 				if( * _collate == 0 )
 * 				{
 * 					g_warning( "Convert %02x%02x to pinyin failed(%s)", ( guint8 )_buf[i + 1], ( guint8 )_buf[ i ], _item2->name );
 * 				}
 */
		}

		* _collate	=	0xFFFF;
	}
	else
	{

		perror( "string_orderByPinyin" );
		g_free( _str1_collateKey );
		return -1;
	}

	_off1	=	( gushort * )_str1_collateKey;
	_off2	=	( gushort * )_str2_collateKey;

	if( _off1 == NULL || _off2 == NULL ) return 0;

	while( TRUE )
	{
		if( * _off1 == 0xFFFF ) 
		{
			g_free( _str1_collateKey );
			g_free( _str2_collateKey );
			return -1;

		}	
		if( * _off2 == 0xFFFF ) 
		{
			g_free( _str1_collateKey );
			g_free( _str2_collateKey );
			return 1;

		}
		if( * _off1 == * _off2 )
		{
			_off1 ++;
			_off2 ++;

			continue;
		}

		if( * _off1 == 0 )
		{
			_off1 ++;

			continue;
		}

		if( * _off2 == 0 )
		{
			_off2 ++;

			continue;
		}

		if( * _off1 > * _off2 ) 
		{
			g_free( _str1_collateKey );
			g_free( _str2_collateKey );
			return 1;
		}
		if( * _off1 < * _off2 ) 
		{
			g_free( _str1_collateKey );
			g_free( _str2_collateKey );
			return -1;
		}
	}
}		/* -----  end of function string_orderByPinyin  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  orderByAlphabetForNagiviJp
 *  Description:  
 * =====================================================================================
 */
gint orderByAlphabetForNagiviJp( gconstpointer a, gconstpointer b )
{
	MHItem * _item1	=	( MHItem * )a;
	MHItem * _item2	=	( MHItem * )b;

	if( _item1->type != _item2->type )
	{
		if( _item1->type == MH_ITEM_FOLDER )
			return -1;
		else if( _item2->type == MH_ITEM_FOLDER )
			return 1;
	}
	return string_orderByAlphabetForNagiviJp( &(_item1->name), &( _item2->name ));
}		/* -----  end of function orderByAlphabetForNagiviJp  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  orderByAlphabetForNagivi
 *  Description:  
 * =====================================================================================
 */
gint orderByAlphabetForNagivi( gconstpointer a, gconstpointer b )
{
	MHItem * _item1	=	( MHItem * )a;
	MHItem * _item2	=	( MHItem * )b;

	if( _item1->type != _item2->type )
	{
		if( _item1->type == MH_ITEM_FOLDER )
			return -1;
		else if( _item2->type == MH_ITEM_FOLDER )
			return 1;
	}
	return string_orderByAlphabetForNagivi( &(_item1->name), &( _item2->name ));
}		/* -----  end of function orderByAlphabetForNagivi  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  orderByAlphabet
 *  Description:  
 * =====================================================================================
 */
gint orderByAlphabet( gconstpointer a, gconstpointer b )
{
	MHItem * _item1	=	( MHItem * )a;
	MHItem * _item2	=	( MHItem * )b;

	if( _item1->type != _item2->type )
	{
		if( _item1->type == MH_ITEM_FOLDER )
			return -1;
		else if( _item2->type == MH_ITEM_FOLDER )
			return 1;
	}
	return string_orderByAlphabet( &(_item1->name), &( _item2->name ));
}		/* -----  end of function orderByAlphabet  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  orderByPinyin
 *  Description:  
 * =====================================================================================
 */
gint orderByPinyin( gconstpointer a, gconstpointer b )
{
	MHItem * _item1	=	( MHItem * )a;
	MHItem * _item2	=	( MHItem * )b;

	if( _item1->type != _item2->type )
	{
		if( _item1->type == MH_ITEM_FOLDER )
			return -1;
		else if( _item2->type == MH_ITEM_FOLDER )
			return 1;
	}
	
	return string_orderByPinyin( &(_item1->name), &(_item2->name));

}		/* -----  end of function orderByPinyin  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  playlist_orderByAlphabetForNagivi
 *  Description:  
 * =====================================================================================
 */
gint playlist_orderByAlphabetForNagiviJp( gconstpointer a, gconstpointer b, gpointer user_data )
{
	gint _res;
	MHItemData * _item_a	=	*( MHItemData **)a;
	MHItemData * _item_b	=	*( MHItemData **)b;
	MHSortType *_type	=	(MHSortType *)user_data;
	char * _str1;
	char * _str2;
	char * _str1_collateKey;
	char * _str2_collateKey;

	switch( *_type )
	{
		case MH_SORT_TITLE:
			_str1	=	_item_a->metadata.music.title;
			_str2	=	_item_b->metadata.music.title;
			break;
		case MH_SORT_ALBUM:
			_str1	=	_item_a->metadata.music.album;
			_str2	=	_item_b->metadata.music.album;
			break;
		case MH_SORT_ARTIST:
			_str1	=	_item_a->metadata.music.artist;
			_str2	=	_item_b->metadata.music.artist;
			break;
		case MH_SORT_NAME:
			_str1	=	_item_a->name;
			_str2	=	_item_b->name;
			break;
	}	
	return string_orderByAlphabetForNagiviJp( &_str1, &_str2);
}		/* -----  end of function playlist_orderByAlphabetForNagiviJp  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  playlist_orderByAlphabetForNagivi
 *  Description:  
 * =====================================================================================
 */
gint playlist_orderByAlphabetForNagivi( gconstpointer a, gconstpointer b, gpointer user_data )
{
	gint _res;
	MHItemData * _item_a	=	*( MHItemData **)a;
	MHItemData * _item_b	=	*( MHItemData **)b;
	MHSortType *_type	=	(MHSortType *)user_data;
	char * _str1;
	char * _str2;
	char * _str1_collateKey;
	char * _str2_collateKey;

	switch( *_type )
	{
		case MH_SORT_TITLE:
			_str1	=	_item_a->metadata.music.title;
			_str2	=	_item_b->metadata.music.title;
			break;
		case MH_SORT_ALBUM:
			_str1	=	_item_a->metadata.music.album;
			_str2	=	_item_b->metadata.music.album;
			break;
		case MH_SORT_ARTIST:
			_str1	=	_item_a->metadata.music.artist;
			_str2	=	_item_b->metadata.music.artist;
			break;
		case MH_SORT_NAME:
			_str1	=	_item_a->name;
			_str2	=	_item_b->name;
			break;
	}	
	return string_orderByAlphabetForNagivi( &_str1, &_str2);
}		/* -----  end of function playlist_orderByAlphabetForNagivi  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  playlist_orderByAlphabet
 *  Description:  
 * =====================================================================================
 */
gint playlist_orderByAlphabet( gconstpointer a, gconstpointer b, gpointer user_data )
{
	gint _res;
	MHItemData * _item_a	=	*( MHItemData **)a;
	MHItemData * _item_b	=	*( MHItemData **)b;
	MHSortType *_type	=	(MHSortType *)user_data;
	char * _str1;
	char * _str2;
	char * _str1_collateKey;
	char * _str2_collateKey;

	switch( *_type )
	{
		case MH_SORT_TITLE:
			_str1	=	_item_a->metadata.music.title;
			_str2	=	_item_b->metadata.music.title;
			break;
		case MH_SORT_ALBUM:
			_str1	=	_item_a->metadata.music.album;
			_str2	=	_item_b->metadata.music.album;
			break;
		case MH_SORT_ARTIST:
			_str1	=	_item_a->metadata.music.artist;
			_str2	=	_item_b->metadata.music.artist;
			break;
		case MH_SORT_NAME:
			_str1	=	_item_a->name;
			_str2	=	_item_b->name;
			break;
	}	
	return string_orderByAlphabet( &_str1, &_str2);
}		/* -----  end of function playlist_orderByAlphabet  ----- */
/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  playlist_orderByPinyin
 *  Description:  
 * =====================================================================================
 */
gint playlist_orderByPinyin( gconstpointer a, gconstpointer b, gpointer user_data)
{
	MHItemData * _item_a	=	*( MHItemData **)a;
	MHItemData * _item_b	=	*( MHItemData **)b;
	MHSortType *_type	=	(MHSortType *)user_data;
	char * _str1;
	char * _str2;
	switch( *_type )
	{
		case MH_SORT_TITLE:
			_str1	=	_item_a->metadata.music.title;
			_str2	=	_item_b->metadata.music.title;
			break;
		case MH_SORT_ALBUM:
			_str1	=	_item_a->metadata.music.album;
			_str2	=	_item_b->metadata.music.album;
			break;
		case MH_SORT_ARTIST:
			_str1	=	_item_a->metadata.music.artist;
			_str2	=	_item_b->metadata.music.artist;
			break;
		case MH_SORT_NAME:
			_str1	=	_item_a->name;
			_str2	=	_item_b->name;
			break;
	}	
	return string_orderByPinyin( &_str1, &_str2);
}		/* -----  end of function playlist_orderByPinyin  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  playlist_orderByTrackId
 *  Description:  
 * =====================================================================================
 */
gint playlist_orderByTrackId( gconstpointer a, gconstpointer b, gpointer user_data)
{
	MHItemData * _item_a	=	*( MHItemData **)a;
	MHItemData * _item_b	=	*( MHItemData **)b;
	int32_t _track_a	=	_item_a->metadata.music.track;
	int32_t _track_b	=	_item_b->metadata.music.track;
	if( _track_a < _track_b)
	{
		return -1;
	}
	else if( _track_a > _track_b )
	{
		return 1;
	}
	else
	{
		return string_orderByPinyin( &(_item_a->metadata.music.title), &(_item_b->metadata.music.title));
		
	}
}		/* -----  end of function playlist_orderByTrackId  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  album_orderByAlphabetForNagiviJp
 *  Description:  
 * =====================================================================================
 */
gint album_orderByAlphabetForNagiviJp( gconstpointer a, gconstpointer b )
{
	char * _album1	=	((MHAlbumInfo*)a)->album_title;
	char * _album2	=	((MHAlbumInfo*)b)->album_title;
	return string_orderByAlphabetForNagiviJp( &_album1, &_album2);
	
}		/* -----  end of static function album_orderByAlphabetForNagiviJp  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  album_orderByAlphabetForNagivi
 *  Description:  
 * =====================================================================================
 */
gint album_orderByAlphabetForNagivi( gconstpointer a, gconstpointer b )
{
	char * _album1	=	((MHAlbumInfo*)a)->album_title;
	char * _album2	=	((MHAlbumInfo*)b)->album_title;
	return string_orderByAlphabetForNagivi( &_album1, &_album2);
	
}		/* -----  end of static function string_orderByAlphabetForNagivi  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  album_orderByAlphabet
 *  Description:  
 * =====================================================================================
 */
gint album_orderByAlphabet( gconstpointer a, gconstpointer b )
{
	char * _album1	=	((MHAlbumInfo*)a)->album_title;
	char * _album2	=	((MHAlbumInfo*)b)->album_title;
	return string_orderByAlphabet( &_album1, &_album2);
	
}		/* -----  end of static function album_orderByAlphabet  ----- */

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  album_orderByPinyin
 *  Description:  
 * =====================================================================================
 */
gint album_orderByPinyin( gconstpointer a, gconstpointer b )
{
	char * _album1	=	((MHAlbumInfo*)a)->album_title;
	char * _album2	=	((MHAlbumInfo*)b)->album_title;
	return string_orderByPinyin( &_album1, &_album2);
}		/* -----  end of static function album_orderByPinyin  ----- */

