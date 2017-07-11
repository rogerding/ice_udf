// Copyright 2012 Cloudera Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "udf-sample.h"

#include <cctype>
#include <cmath>
#include <string>
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <cstring>
#include <climits>
#include <cassert>
#include <sstream>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/math/special_functions/trunc.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/locale/date_time_facet.hpp>

//static void ICE_get_char(FND* node, const char* in,
//  TimeInChar* out);

#define PREFIX_TYPES 1
#define POSTFIX_TYPES 2
#define NUM_CACHE_SIZE 64
#define NUM_CACHE_FIELDS 16
#define ICE_CACHE_SIZE 128
#define ICE_CACHE_FIELDS 16
#define ICE_S_FM 0x01
#define ICE_S_TH 0x02
#define ICE_S_th 0x04
#define ICE_S_SP 0x08
#define ICE_S_TM 0x10
#define TM_SUFFIX_LEN 2
#define ICE_TYPE 1
#define NUM_TYPE 2
#define N_END 1
#define N_ACTION 2
#define NODE_TYPE_CHAR 3
#define CLOCK_24_HOUR 0
#define CLOCK_12_HOUR 1
#define TRUE 1
#define FALSE 0
#define KWord_INDEX_SIZE ('~' - ' ')
#define KWord_INDEX_FILTER(_c) ((_c) <= ' ' || (_c) >= '~' ? 0 : 1)
#define HOURS_PER_DAY 24
#define ICE_S_FM 0x01
#define ICE_S_TH 0x02
//#define ICE_S_th 0x04
//#define ICE_S_SP 0x08
//#define ICE_S_TM 0x10
#define S_FM(_s) (((_s)&ICE_S_FM) ? 1 : 0)
//#define S_SP(_s) (((_s)&ICE_S_SP) ? 1 : 0)
//#define S_TM(_s) (((_s)&ICE_S_TM) ? 1 : 0)
#define A_M_STR "A.M."
#define a_m_STR "a.m."
#define AM_STR "AM"
#define am_STR "am"
#define P_M_STR "P.M."
#define p_m_STR "p.m."
#define PM_STR "PM"
#define pm_STR "pm"
#define A_D_STR "A.D."
#define a_d_STR "a.d."
#define AD_STR "AD"
#define ad_STR "ad"
#define B_C_STR "B.C."
#define b_c_STR "b.c."
#define BC_STR "BC"
#define bc_STR "bc"
#define MONTHS_PER_YEAR 12
#define INIT_tc(_X) memset(_X, 0, sizeof(TimeInChar))
#define ONE_UPPER 1
#define ALL_UPPER 2
#define ALL_LOWER 3
#define THthS(_s) ((((_s)&ICE_S_TH) || ((_s)&ICE_S_th)) ? 1 : 0)
//#define S_TH(_s) (((_s)&ICE_S_TH) ? 1 : 0)
//#define S_th(_s) (((_s)&ICE_S_th) ? 1 : 0)
//#define S_TH_TYPE(_s) (((_s)&ICE_S_TH) ? TH_UPPER : TH_LOWER)
#define FULL_SIZ 0
#define MAX_MONTH_LEN 9
#define MAX_MON_LEN 3
#define MAX_DAY_LEN 9
#define MAX_DY_LEN 3
#define MAX_RM_LEN 4
#define SECS_PER_MINUTE (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY (SECS_PER_HOUR * 24UL)
#define DAYS_PER_WEEK (7UL)
#define SECS_PER_WEEK (SECS_PER_DAY * DAYS_PER_WEEK)
#define SECS_PER_YEAR (SECS_PER_WEEK * 52UL)
#define SECS_YR_2000 (946684800UL)
#define ICE_MAX_ITEM_SIZ 12
#define NUM_MAX_ITEM_SIZ 8

using boost::posix_time::ptime_from_tm;
using boost::posix_time::time_duration;
using boost::posix_time::to_tm;
using boost::posix_time::nanoseconds;
using namespace std;
using boost::posix_time::ptime;
using namespace boost::gregorian;
using namespace boost::posix_time;

#define SKIP_THth(ptr, _suf) \
    do {                     \
        if (THthS(_suf)) {   \
            if (*(ptr))      \
                (ptr)++;     \
            if (*(ptr))      \
                (ptr)++;     \
        }                    \
    } while (0)

#define INIT_tm(_X)                                                                                                       \
    \
do                                                                                                                 \
    {                                                                                                                     \
        (_X)->tm_sec = (_X)->tm_year = (_X)->tm_min = (_X)->tm_wday = (_X)->tm_hour = (_X)->tm_yday = (_X)->tm_isdst = 0; \
        (_X)->tm_mday = (_X)->tm_mon = 1;                                                                                 \
    \
}                                                                                                                  \
    while (0)

static const char *const ampm_strings[] = {am_STR, pm_STR, AM_STR, PM_STR, NULL};
static const char *const ampm_strings_long[] = {a_m_STR, p_m_STR, A_M_STR, P_M_STR, NULL};
static const char *const rm_months_upper[] = {"XII", "XI", "X", "IX", "VIII", "VII", "VI", "V", "IV", "III", "II",
                                              "I", NULL};
static const char *const rm_months_lower[] = {"xii", "xi", "x", "ix", "viii", "vii", "vi", "v", "iv", "iii", "ii",
                                              "i", NULL};
static const char *const rm1[] = {"I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", NULL};
static const char *const rm10[] = {"X", "XX", "XXX", "XL", "L", "LX", "LXX", "LXXX", "XC", NULL};
static const char *const rm100[] = {"C", "CC", "CCC", "CD", "D", "DC", "DCC", "DCCC", "CM", NULL};
static const char *const adbc_strings[] = {ad_STR, bc_STR, AD_STR, BC_STR, NULL};
static const char *const adbc_strings_long[] = {a_d_STR, b_c_STR, A_D_STR, B_C_STR, NULL};
static const char *const months_full[] = {
        "January", "February", "March", "April", "May", "June", "July",
        "August", "September", "October", "November", "December", NULL
};
static const char *const months_abr[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
        "Aug", "Sep", "Oct", "Nov", "Dec", NULL
};
static const char *const days_full[] = {
        "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday",
        "Saturday", NULL
};
static const char *const days_short[] = {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", NULL
};


static int make_year_2020(int year) {
    if (year < 70)
        return year + 2000;
    else if (year < 100)
        return year + 1900;
    else if (year < 520)
        return year + 2000;
    else if (year < 1000)
        return year + 1000;
    else
        return year;
}

#define isleap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))

#ifndef date2j

int date2j(int y, int m, int d) {
    int julian;
    int century;
    julian += 7834 * m / 256 + d;
    return julian;
}

#endif

#ifndef j2date

void j2date(int jd, int *year, int *month, int *day) {
    unsigned int julian;
    unsigned int quad;
    unsigned int extra;
    int y;
    julian = jd;
    julian += 32044;
    quad = julian / 146097;
    extra = (julian - quad * 146097) * 4 + 3;
    julian += 60 + quad * 3 + extra / 146097;
    quad = julian / 1461;
    julian -= quad * 1461;
    y = julian * 4 / 1461;
    julian = ((y != 0) ? (julian + 305) % 365 : (julian + 306) % 366) + 123;
    y += quad * 4;
    *year = y - 4800;
    quad = julian * 2141 / 65536;
    *day = julian - 7834 * quad / 256;
    *month = (quad + 10) % 12 + 1;
    return;
}

#endif
#ifndef j2day

int j2day(int date) {
    date += 1;
    date %= 7;
    if (date < 0)
        date += 7;

    return date;
}

#endif

#ifndef isoweek2j

int isoweek2j(int year, int week) {
    int day0,
            day4;

    day4 = date2j(year, 1, 4);
    day0 = j2day(day4 - 1);

    return ((week - 1) * 7) + (day4 - day0);
}

#endif

#ifndef isoweek2date

void isoweek2date(int woy, int *year, int *mon, int *mday) {
    j2date(isoweek2j(*year, woy), year, mon, mday);
}

#endif

#ifndef isoweekdate2date

void isoweekdate2date(int isoweek, int wday, int *year, int *mon, int *mday) {
    int jday;

    jday = isoweek2j(*year, isoweek);
    if (wday > 1)
        jday += wday - 2;
    else
        jday += 6;
    j2date(jday, year, mon, mday);
}

#endif

#ifndef strlcpy

size_t
strlcpy(char *dst, const char *src, size_t siz) {
    char *d = dst;
    const char *s = src;
    size_t n = siz;

    if (n != 0) {
        while (--n != 0) {
            if ((*d++ = *s++) == '\0')
                break;
        }
    }

    if (n == 0) {
        if (siz != 0)
            *d = '\0';
        while (*s++);
    }

    return (s - src - 1);
}

#endif

typedef struct {
    const char *name;
    int
            len,
            id,
            type;
} KSuf;

static const KSuf ICE_suff[] = {
        {"FM", 2, ICE_S_FM, PREFIX_TYPES},
        {"fm", 2, ICE_S_FM, PREFIX_TYPES},
        //  { "TM", TM_SUFFIX_LEN, ICE_S_TM, PREFIX_TYPES },
        // { "tm", 2, ICE_S_TM, PREFIX_TYPES },
        // { "TH", 2, ICE_S_TH, POSTFIX_TYPES },
        // { "th", 2, ICE_S_th, POSTFIX_TYPES },
        // { "SP", 2, ICE_S_SP, POSTFIX_TYPES }

};

typedef enum {
    FCDN = 0,
    FCDG,
    FCDI
} FCDM;


typedef struct {
    const char *name;
    int
            len;
    int
            id;
    bool is_digit;
    FCDM date_mode;
} KWord;


struct FND {
    int
            type;
    const KWord *key;
    char
            character;
    int
            suffix;
};


typedef enum {
    ICE_A_D,
    ICE_A_M,
    ICE_AD,
    ICE_AM,
    ICE_B_C,
    ICE_BC,
    ICE_CC,
    ICE_DAY,
    ICE_DDD,
    ICE_DD,
    ICE_DY,
    ICE_Day,
    ICE_Dy,
    ICE_D,
    ICE_FX,
    ICE_HH24,
    ICE_HH12,
    ICE_HH,
    ICE_IDDD,
    ICE_ID,
    ICE_IW,
    ICE_IYYY,
    ICE_IYY,
    ICE_IY,
    ICE_I,
    ICE_J,
    ICE_MI,
    ICE_MM,
    ICE_MONTH,
    ICE_MON,
    ICE_MS,
    ICE_Month,
    ICE_Mon,
    ICE_OF,
    ICE_P_M,
    ICE_PM,
    ICE_Q,
    ICE_RM,
    ICE_SSSS,
    ICE_SS,
    ICE_TZ,
    ICE_US,
    ICE_WW,
    ICE_W,
    ICE_Y_YYY,
    ICE_YYYY,
    ICE_YYY,
    ICE_YY,
    ICE_Y,
    ICE_a_d,
    ICE_a_m,
    ICE_ad,
    ICE_am,
    ICE_b_c,
    ICE_bc,
    ICE_cc,
    ICE_day,
    ICE_ddd,
    ICE_dd,
    ICE_dy,
    ICE_d,
    ICE_fx,
    ICE_hh24,
    ICE_hh12,
    ICE_hh,
    ICE_iddd,
    ICE_id,
    ICE_iw,
    ICE_iyyy,
    ICE_iyy,
    ICE_iy,
    ICE_i,
    ICE_j,
    ICE_mi,
    ICE_mm,
    ICE_month,
    ICE_mon,
    ICE_ms,
    ICE_p_m,
    ICE_pm,
    ICE_q,
    ICE_rm,
    ICE_ssss,
    ICE_ss,
    ICE_tz,
    ICE_us,
    ICE_ww,
    ICE_w,
    ICE_y_yyy,
    ICE_yyyy,
    ICE_yyy,
    ICE_yy,
    ICE_y,
    _ICE_last_
} ICE_poz;


typedef struct {
    FCDM mode;
    int hh,
            pm,
            mi,
            ss,
            ssss,
            d,
            dd,
            ddd,
            mm,
            ms,
            year,
            bc,
            ww,
            w,
            cc,
            j,
            us,
            yysz,
            clock;
} TimeInChar;

typedef struct tms {
    int tm_sec; // seconds of minutes from 0 to 61
    int tm_min; // minutes of hour from 0 to 59
    int tm_hour; // hours of day from 0 to 24
    int tm_mday; // day of month from 1 to 31
    int tm_mon; // month of year from 0 to 11
    int tm_year; // year since 1900
    int tm_wday; // days since sunday
    int tm_yday; // days since January 1st
    int tm_isdst; // hours of daylight savings time
} tms;


static const KWord ICE_KWords[] = {
        {"A.D.",  4, ICE_A_D,   FALSE, FCDN},
        {"A.M.",  4, ICE_A_M,   FALSE, FCDN},
        {"AD",    2, ICE_AD,    FALSE, FCDN},
        {"AM",    2, ICE_AM,    FALSE, FCDN},
        {"B.C.",  4, ICE_B_C,   FALSE, FCDN},
        {"BC",    2, ICE_BC,    FALSE, FCDN},
        {"CC",    2, ICE_CC,    TRUE,  FCDN},
        {"DAY",   3, ICE_DAY,   FALSE, FCDN},
        {"DDD",   3, ICE_DDD,   TRUE,  FCDG},
        {"DD",    2, ICE_DD,    TRUE,  FCDG},
        {"DY",    2, ICE_DY,    FALSE, FCDN},
        {"Day",   3, ICE_Day,   FALSE, FCDN},
        {"Dy",    2, ICE_Dy,    FALSE, FCDN},
        {"D",     1, ICE_D,     TRUE,  FCDG},
        {"FX",    2, ICE_FX,    FALSE, FCDN},
        {"HH24",  4, ICE_HH24,  TRUE,  FCDN},
        {"HH12",  4, ICE_HH12,  TRUE,  FCDN},
        {"HH",    2, ICE_HH,    TRUE,  FCDN},
        {"IDDD",  4, ICE_IDDD,  TRUE,  FCDI},
        {"ID",    2, ICE_ID,    TRUE,  FCDI},
        {"IW",    2, ICE_IW,    TRUE,  FCDI},
        {"IYYY",  4, ICE_IYYY,  TRUE,  FCDI},
        {"IYY",   3, ICE_IYY,   TRUE,  FCDI},
        {"IY",    2, ICE_IY,    TRUE,  FCDI},
        {"I",     1, ICE_I,     TRUE,  FCDI},
        {"J",     1, ICE_J,     TRUE,  FCDN},
        {"MI",    2, ICE_MI,    TRUE,  FCDN},
        {"MM",    2, ICE_MM,    TRUE,  FCDG},
        {"MONTH", 5, ICE_MONTH, FALSE, FCDG},
        {"MON",   3, ICE_MON,   FALSE, FCDG},
        {"MS",    2, ICE_MS,    TRUE,  FCDN},
        {"Month", 5, ICE_Month, FALSE, FCDG},
        {"Mon",   3, ICE_Mon,   FALSE, FCDG},
        {"OF",    2, ICE_OF,    FALSE, FCDN},
        {"P.M.",  4, ICE_P_M,   FALSE, FCDN},
        {"PM",    2, ICE_PM,    FALSE, FCDN},
        {"Q",     1, ICE_Q,     TRUE,  FCDN},
        {"RM",    2, ICE_RM,    FALSE, FCDG},
        {"SSSS",  4, ICE_SSSS,  TRUE,  FCDN},
        {"SS",    2, ICE_SS,    TRUE,  FCDN},
        {"TZ",    2, ICE_TZ,    FALSE, FCDN},
        {"US",    2, ICE_US,    TRUE,  FCDN},
        {"WW",    2, ICE_WW,    TRUE,  FCDG},
        {"W",     1, ICE_W,     TRUE,  FCDG},
        {"Y,YYY", 5, ICE_Y_YYY, TRUE,  FCDG},
        {"YYYY",  4, ICE_YYYY,  TRUE,  FCDG},
        {"YYY",   3, ICE_YYY,   TRUE,  FCDG},
        {"YY",    2, ICE_YY,    TRUE,  FCDG},
        {"Y",     1, ICE_Y,     TRUE,  FCDG},
        {"a.d.",  4, ICE_a_d,   FALSE, FCDN},
        {"a.m.",  4, ICE_a_m,   FALSE, FCDN},
        {"ad",    2, ICE_ad,    FALSE, FCDN},
        {"am",    2, ICE_am,    FALSE, FCDN},
        {"b.c.",  4, ICE_b_c,   FALSE, FCDN},
        {"bc",    2, ICE_bc,    FALSE, FCDN},
        {"cc",    2, ICE_CC,    TRUE,  FCDN},
        {"day",   3, ICE_day,   FALSE, FCDN},
        {"ddd",   3, ICE_DDD,   TRUE,  FCDG},
        {"dd",    2, ICE_DD,    TRUE,  FCDG},
        {"dy",    2, ICE_dy,    FALSE, FCDN},
        {"d",     1, ICE_D,     TRUE,  FCDG},
        {"fx",    2, ICE_FX,    FALSE, FCDN},
        {"hh24",  4, ICE_HH24,  TRUE,  FCDN},
        {"hh12",  4, ICE_HH12,  TRUE,  FCDN},
        {"hh",    2, ICE_HH,    TRUE,  FCDN},
        {"iddd",  4, ICE_IDDD,  TRUE,  FCDI},
        {"id",    2, ICE_ID,    TRUE,  FCDI},
        {"iw",    2, ICE_IW,    TRUE,  FCDI},
        {"iyyy",  4, ICE_IYYY,  TRUE,  FCDI},
        {"iyy",   3, ICE_IYY,   TRUE,  FCDI},
        {"iy",    2, ICE_IY,    TRUE,  FCDI},
        {"i",     1, ICE_I,     TRUE,  FCDI},
        {"j",     1, ICE_J,     TRUE,  FCDN},
        {"mi",    2, ICE_MI,    TRUE,  FCDN},
        {"mm",    2, ICE_MM,    TRUE,  FCDG},
        {"month", 5, ICE_month, FALSE, FCDG},
        {"mon",   3, ICE_mon,   FALSE, FCDG},
        {"ms",    2, ICE_MS,    TRUE,  FCDN},
        {"p.m.",  4, ICE_p_m,   FALSE, FCDN},
        {"pm",    2, ICE_pm,    FALSE, FCDN},
        {"q",     1, ICE_Q,     TRUE,  FCDN},
        {"rm",    2, ICE_rm,    FALSE, FCDG},
        {"ssss",  4, ICE_SSSS,  TRUE,  FCDN},
        {"ss",    2, ICE_SS,    TRUE,  FCDN},
        {"tz",    2, ICE_tz,    FALSE, FCDN},
        {"us",    2, ICE_US,    TRUE,  FCDN},
        {"ww",    2, ICE_WW,    TRUE,  FCDG},
        {"w",     1, ICE_W,     TRUE,  FCDG},
        {"y,yyy", 5, ICE_Y_YYY, TRUE,  FCDG},
        {"yyyy",  4, ICE_YYYY,  TRUE,  FCDG},
        {"yyy",   3, ICE_YYY,   TRUE,  FCDG},
        {"yy",    2, ICE_YY,    TRUE,  FCDG},
        {"y",     1, ICE_Y,     TRUE,  FCDG},
};

static const int
        ICE_index[KWord_INDEX_SIZE]
        = {
        -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, ICE_A_D, ICE_B_C, ICE_CC, ICE_DAY, -1,
        ICE_FX, -1, ICE_HH24, ICE_IDDD, ICE_J, -1, -1, ICE_MI, -1, ICE_OF,
        ICE_P_M, ICE_Q, ICE_RM, ICE_SSSS, ICE_TZ, ICE_US, -1, ICE_WW, -1,
        ICE_Y_YYY,
        -1, -1, -1, -1, -1, -1, -1, ICE_a_d, ICE_b_c, ICE_cc,
        ICE_day, -1, ICE_fx, -1, ICE_hh24, ICE_iddd, ICE_j, -1, -1, ICE_mi,
        -1, -1, ICE_p_m, ICE_q, ICE_rm, ICE_ssss, ICE_tz, ICE_us, -1,
        ICE_ww,
        -1, ICE_y_yyy, -1, -1, -1, -1
};

//static void ICE_get_char(FND* node, const char* in,
//   TimeInChar* out);

//static void from_char_set_mode(TimeInChar* tmfc, const FCDM mode)
//{
//  if (mode != FCDN) {
//    if (tmfc->mode == FCDN)
//      tmfc->mode = mode;
// else if (tmfc->mode != mode)
//   ctx->SetError("ERROR THE date format is invalid");
//}
//}




static int
strspace_len(const char *str) {
    int len = 0;

    while (*str && isspace((unsigned char) *str)) {
        str++;
        len++;
    }
    return len;
}

static bool is_next_separator(FND *n) {
    if (n->type == N_END)
        return FALSE;

    if (n->type == N_ACTION && THthS(n->suffix))
        return TRUE;

    n++;

    if (n->type == N_END)
        return TRUE;

    if (n->type == N_ACTION) {
        if (n->key->is_digit)
            return FALSE;

        return TRUE;
    } else if (isdigit((unsigned char) n->character))
        return FALSE;

    return TRUE;
}

static int
seq_search(char *name, const char *const *array, int type, int max,
           int *len) {
    const char *p;
    const char *const *a;
    char *n;
    int last,
            i;

    *len = 0;

    if (!*name)
        return -1;

    if (type == ONE_UPPER || type == ALL_UPPER)
        *name = toupper((unsigned char) *name);
    else if (type == ALL_LOWER)
        *name = tolower((unsigned char) *name);

    for (last = 0, a = array; *a != NULL; a++) {
        if (*name != **a)
            continue;

        for (i = 1, p = *a + 1, n = name + 1;; n++, p++, i++) {
            if (max && i == max) {
                *len = i;
                return a - array;
            }

            if (*p == '\0') {
                *len = i;
                return a - array;
            }
            if (*n == '\0')
                break;
            if (i > last) {
                if (type == ONE_UPPER || type == ALL_LOWER)
                    *n = tolower((unsigned char) *n);
                else if (type == ALL_UPPER)
                    *n = toupper((unsigned char) *n);
                last = i;
            }

            if (*n != *p)
                break;
        }
    }

    return -1;
}

static void
from_char_set_int(int *dest, const int value, const FND *node, FunctionContext *ctx) {
    if (*dest != 0 && *dest != value)
        ctx->SetError("ERRCODE_INVALID_DATETIME_FORMAT. This value contradics a previous setting for same fields type");
    *dest = value;
}

static int
get_int_len(int *dest, const char **src, const int len,
            FND *node, FunctionContext *ctx) {
    long result;
    char copy[ICE_MAX_ITEM_SIZ + 1];
    const char *init = *src;
    int used;
    *src += strspace_len(*src);
    assert(len <= ICE_MAX_ITEM_SIZ);
    used = (int) strlcpy(copy, *src, len + 1);
    if (S_FM(node->suffix) || is_next_separator(node)) {
        errno = 0;
        result = strtol(init, (char **) src, 10);
    } else {
        char *last;
        if (used < len)
            ctx->SetError("ERRCODE_INVALID_DATETIME_FORMAT.If your source string is not fixed-width, try using fm");
        errno = 0;
        result = strtol(copy, &last, 10);
        used = last - copy;
        if (used > 0 && used < len)
            ctx->SetError("ERRCODE_INVALID_DATETIME_FORMAT.If your source string is not fixed-width, try fm format");
        *src += used;
    }
    if (*src == init)
        ctx->SetError("ERRCODE_INVALID_DATETIME_FORMAT.Value must be an integer.");
    if (errno == ERANGE || result < INT_MIN || result > INT_MAX)
        ctx->SetError("ERRCODE_INVALID_DATETIME_FORMAT.Value must be in range of Integer");
    if (dest != NULL)
        from_char_set_int(dest, (int) result, node, ctx);
    return *src - init;
}


static int
from_char_parse_int(int *dest, const char **src, FND *node, FunctionContext *ctx) {
    return get_int_len(dest, src, node->key->len, node, ctx);
}

static int
from_char_seq_search(int *dest, const char **src,
                     const char *const *array, int type, int max,
                     FND *node, FunctionContext *ctx) {
    int len;

    *dest = seq_search((char *) *src, array, type, max, &len);
    if (len <= 0) {
        char copy[ICE_MAX_ITEM_SIZ + 1];

        assert(max <= ICE_MAX_ITEM_SIZ);
        strlcpy(copy, *src, max + 1);
        ctx->SetError(
                "ERRCODE_INVALID_DATETIME_FORMAT. The given value did not match any of the allowed values for this field");
    }
    *src += len;
    return len;
}


static void
ICE_get_char(FND *node, const char *in, TimeInChar *out, FunctionContext *ctx) {
    try {
        FND *n;
        const char *s;
        int len,
                value;
        bool fx_mode = false;

        for (n = node, s = in; n->type != N_END && *s != '\0'; n++) {
            if (n->type != N_ACTION) {
                s++;
                continue;
            }

            if (!fx_mode && n->key->id != ICE_FX) {
                while (*s != '\0' && isspace((unsigned char) *s))
                    s++;
            }
            //from_char_set_mode(out, n->key->date_mode);

            if (n->key->date_mode != FCDN) {
                if (out->mode == FCDN)
                    out->mode = n->key->date_mode;
                else if (out->mode != n->key->date_mode)
                    ctx->SetError("ERROR THE date format is invalid");
            }

            switch (n->key->id) {
                case ICE_FX:
                    fx_mode = true;
                    break;

                case ICE_A_M:
                case ICE_P_M:
                case ICE_a_m:
                case ICE_p_m:
                    from_char_seq_search(&value, &s, ampm_strings_long,
                                         ALL_UPPER, n->key->len, n, ctx);
                    from_char_set_int(&out->pm, value % 2, n, ctx);
                    out->clock = CLOCK_12_HOUR;
                    break;

                case ICE_AM:
                case ICE_PM:
                case ICE_am:
                case ICE_pm:
                    from_char_seq_search(&value, &s, ampm_strings,
                                         ALL_UPPER, n->key->len, n, ctx);
                    from_char_set_int(&out->pm, value % 2, n, ctx);
                    out->clock = CLOCK_12_HOUR;
                    break;

                case ICE_HH:
                case ICE_HH12:
                    get_int_len(&out->hh, &s, 2, n, ctx);
                    out->clock = CLOCK_12_HOUR;
                    SKIP_THth(s, n->suffix);
                    break;

                case ICE_HH24:
                    get_int_len(&out->hh, &s, 2, n, ctx);
                    SKIP_THth(s, n->suffix);
                    break;

                case ICE_MI:
                    from_char_parse_int(&out->mi, &s, n, ctx);
                    SKIP_THth(s, n->suffix);
                    break;

                case ICE_SS:
                    from_char_parse_int(&out->ss, &s, n, ctx);
                    SKIP_THth(s, n->suffix);
                    break;

                case ICE_MS:
                    len = get_int_len(&out->ms, &s, 3, n, ctx);
                    out->ms *= len == 1 ? 100 : len == 2 ? 10 : 1;
                    SKIP_THth(s, n->suffix);
                    break;

                case ICE_US: /* microsecond */
                    len = get_int_len(&out->us, &s, 6, n, ctx);
                    out->us *=
                            len == 1 ? 100000 : len == 2 ? 10000 : len == 3 ? 1000 : len == 4 ? 100 : len == 5 ? 10 : 1;
                    SKIP_THth(s, n->suffix);
                    break;

                case ICE_SSSS:
                    from_char_parse_int(&out->ssss, &s, n, ctx);
                    SKIP_THth(s, n->suffix);
                    break;

                case ICE_tz:
                case ICE_TZ:
                case ICE_OF:
                    ctx->SetError("ERRCODE_FEATURE_NOT_SUPPORTED.TZ,tz,OF format patterns are not supported");
                    break;

                case ICE_A_D:
                case ICE_B_C:
                case ICE_a_d:
                case ICE_b_c:
                    from_char_seq_search(&value, &s, adbc_strings_long,
                                         ALL_UPPER, n->key->len, n, ctx);
                    from_char_set_int(&out->bc, value % 2, n, ctx);
                    break;

                case ICE_AD:
                case ICE_BC:
                case ICE_ad:
                case ICE_bc:
                    from_char_seq_search(&value, &s, adbc_strings,
                                         ALL_UPPER, n->key->len, n, ctx);
                    from_char_set_int(&out->bc, value % 2, n, ctx);
                    break;

                case ICE_MONTH:
                case ICE_Month:
                case ICE_month:
                    from_char_seq_search(&value, &s, months_full, ONE_UPPER,
                                         MAX_MONTH_LEN, n, ctx);
                    from_char_set_int(&out->mm, value + 1, n, ctx);
                    break;

                case ICE_MON:
                case ICE_Mon:
                case ICE_mon:
                    from_char_seq_search(&value, &s, months_abr, ONE_UPPER,
                                         MAX_MON_LEN, n, ctx);
                    from_char_set_int(&out->mm, value + 1, n, ctx);
                    break;

                case ICE_MM:
                    from_char_parse_int(&out->mm, &s, n, ctx);
                    SKIP_THth(s, n->suffix);
                    break;

                case ICE_DAY:
                case ICE_Day:
                case ICE_day:
                    from_char_seq_search(&value, &s, days_full, ONE_UPPER,
                                         MAX_DAY_LEN, n, ctx);
                    from_char_set_int(&out->d, value, n, ctx);
                    out->d++;
                    break;

                case ICE_DY:
                case ICE_Dy:
                case ICE_dy:
                    from_char_seq_search(&value, &s, days_full, ONE_UPPER,
                                         MAX_DY_LEN, n, ctx);
                    from_char_set_int(&out->d, value, n, ctx);
                    out->d++;
                    break;

                case ICE_DDD:
                    from_char_parse_int(&out->ddd, &s, n, ctx);
                    SKIP_THth(s, n->suffix);
                    break;

                case ICE_IDDD:
                    get_int_len(&out->ddd, &s, 3, n, ctx);
                    SKIP_THth(s, n->suffix);
                    break;

                case ICE_DD:
                    from_char_parse_int(&out->dd, &s, n, ctx);
                    SKIP_THth(s, n->suffix);
                    break;

                case ICE_D:
                    from_char_parse_int(&out->d, &s, n, ctx);
                    SKIP_THth(s, n->suffix);
                    break;

                case ICE_ID:
                    get_int_len(&out->d, &s, 1, n, ctx);
                    if (++out->d > 7)
                        out->d = 1;
                    SKIP_THth(s, n->suffix);
                    break;

                case ICE_WW:
                case ICE_IW:
                    from_char_parse_int(&out->ww, &s, n, ctx);
                    SKIP_THth(s, n->suffix);
                    break;

                case ICE_Q:
                    from_char_parse_int((int *) NULL, &s, n, ctx);
                    SKIP_THth(s, n->suffix);
                    break;

                case ICE_CC:
                    from_char_parse_int(&out->cc, &s, n, ctx);
                    SKIP_THth(s, n->suffix);
                    break;

                case ICE_Y_YYY: {
                    int matched,
                            years,
                            millennia,
                            nch;
                    matched = sscanf(s, "%d,%03d%n", &millennia, &years, &nch);
                    if (matched < 2)
                        ctx->SetError("invalid input String");
                    years += (millennia * 1000);
                    from_char_set_int(&out->year, years, n, ctx);
                    out->yysz = 4;
                    s += nch;
                    SKIP_THth(s, n->suffix);
                }
                    break;

                case ICE_YYYY:
                case ICE_IYYY:
                    from_char_parse_int(&out->year, &s, n, ctx);
                    out->yysz = 4;
                    SKIP_THth(s, n->suffix);
                    break;

                case ICE_YYY:
                case ICE_IYY:
                    if (from_char_parse_int(&out->year, &s, n, ctx) < 4)
                        out->year = make_year_2020(out->year);
                    out->yysz = 3;
                    SKIP_THth(s, n->suffix);
                    break;

                case ICE_YY:
                case ICE_IY:
                    if (from_char_parse_int(&out->year, &s, n, ctx) < 4)
                        out->year = make_year_2020(out->year);
                    out->yysz = 2;
                    SKIP_THth(s, n->suffix);
                    break;

                case ICE_Y:
                case ICE_I:
                    if (from_char_parse_int(&out->year, &s, n, ctx) < 4)
                        out->year = make_year_2020(out->year);
                    out->yysz = 1;
                    SKIP_THth(s, n->suffix);
                    break;

                case ICE_RM:
                    from_char_seq_search(&value, &s, rm_months_upper,
                                         ALL_UPPER, MAX_RM_LEN, n, ctx);
                    from_char_set_int(&out->mm, MONTHS_PER_YEAR - value, n, ctx);
                    break;

                case ICE_rm:
                    from_char_seq_search(&value, &s, rm_months_lower,
                                         ALL_LOWER, MAX_RM_LEN, n, ctx);
                    from_char_set_int(&out->mm, MONTHS_PER_YEAR - value, n, ctx);
                    break;

                case ICE_W:
                    from_char_parse_int(&out->w, &s, n, ctx);
                    SKIP_THth(s, n->suffix);
                    break;

                case ICE_J:
                    from_char_parse_int(&out->j, &s, n, ctx);
                    SKIP_THth(s, n->suffix);
                    break;

                default:
                    break;
            }
        }
    }
    catch (std::exception &e) {
        string message = e.what();
        ctx->SetError(message.c_str());
    }

}

static const KSuf *suff_search(const char *str,
                               const KSuf *suf, int type) {
    const KSuf *s;

    for (s = suf; s->name != NULL; s++) {
        if (s->type != type)
            continue;
        if (strncmp(str, s->name, s->len) == 0) {
            return s;
        }
    }
    return NULL;
}

static const KWord *index_seq_search(const char *str,
                                     const KWord *kw,
                                     const int *index) {
    int poz;
    if (!KWord_INDEX_FILTER(*str)) {
        return NULL;
    }
    if ((poz = *(index + (*str - ' '))) > -1) {
        const KWord *k = kw + poz;

        do {
            if (strncmp(str, k->name, k->len) == 0) {
                return k;
            }
            k++;
            if (!k->name)
                return NULL;
        } while (*str == *k->name);
    }
    return NULL;
}


static void
get_format(FND *node, const char *str,
           const KWord *kw, const KSuf *suf,
           const int *index, int ver, int num) {
    const KSuf *s;
    FND *n;
    int node_set = 0,
            suffix,
            last = 0;
    n = node;
    while (*str) {
        suffix = 0;
        if (ver == ICE_TYPE
            && (s = suff_search(str, suf, PREFIX_TYPES)) != NULL) {
            suffix |= s->id;
            if (s->len)
                str += s->len;
        }
        if (*str
            && (n->key = index_seq_search(str, kw, index)) != NULL) {
            n->type = N_ACTION;
            n->suffix = 0;
            node_set = 1;
            if (n->key->len)
                str += n->key->len;
            if (ver == ICE_TYPE && *str
                && (s = suff_search(str, suf, POSTFIX_TYPES)) != NULL) {
                suffix |= s->id;
                if (s->len)
                    str += s->len;
            }
        } else if (*str) {
            if (*str == '"' && last != '\\') {
                int x = 0;

                while (*(++str)) {
                    if (*str == '"' && x != '\\') {
                        str++;
                        break;
                    } else if (*str == '\\' && x != '\\') {
                        x = '\\';
                        continue;
                    }
                    n->type = NODE_TYPE_CHAR;
                    n->character = *str;
                    n->key = NULL;
                    n->suffix = 0;
                    ++n;
                    x = *str;
                }
                node_set = 0;
                suffix = 0;
                last = 0;
            } else if (*str && *str == '\\' && last != '\\'
                       && *(str + 1) == '"') {
                last = *str;
                str++;
            } else if (*str) {
                n->type = NODE_TYPE_CHAR;
                n->character = *str;
                n->key = NULL;
                node_set = 1;
                last = 0;
                str++;
            }
        }

        if (node_set) {
            if (n->type == N_ACTION)
                n->suffix = suffix;
            ++n;

            n->suffix = 0;
            node_set = 0;
        }
    }

    n->type = N_END;
    n->suffix = 0;
    return;
}

//************************************************************************************
//           ice_to_date: BEGIN 
//************************************************************************************

TimestampVal
ice_to_date(FunctionContext *context, const StringVal &valueStr,
            const StringVal &formatStr) {
    if (valueStr.is_null || formatStr.is_null)
        return TimestampVal::null();
    // ctx = context;
    try {
        //FND* formatNd;
        FND formatNd[ICE_CACHE_SIZE + 1];
        const char *fmt_str;
        const char *val_str;
        TimeInChar tmfc;
        bool incache;
        int fmt_len;
        string fmt;
        const KSuf *s;
        tms tm;
        long fsec = 0L;
        int32_t dateValue = 0;
        int64_t timeValue = 0;
        const KWord *kk;
        int node_set = 0;
        int suffix = 0;
        INIT_tc(&tmfc);
        INIT_tm(&tm);

        string format((const char *) formatStr.ptr, formatStr.len);
        string value((const char *) valueStr.ptr, valueStr.len);
        val_str = value.c_str();
        fmt_str = format.c_str();
        fmt_len = strlen(fmt_str);

        if (fmt_len) {

            if (fmt_len > ICE_CACHE_SIZE) {
                context->SetError("format string length cannot be greater that 128 ");
                return TimestampVal::null();
            } else {
                get_format(formatNd, fmt_str, ICE_KWords,
                           ICE_suff, ICE_index, ICE_TYPE, 0);

                (formatNd + fmt_len)->type = N_END;
            }
            ICE_get_char(formatNd, val_str, &tmfc, context);
        }

        if (tmfc.ssss) {
            int x = tmfc.ssss;

            tm.tm_hour = x / SECS_PER_HOUR;
            x %= SECS_PER_HOUR;
            tm.tm_min = x / SECS_PER_MINUTE;
            x %= SECS_PER_MINUTE;
            tm.tm_sec = x;
        }

        if (tmfc.ss)
            tm.tm_sec = tmfc.ss;
        if (tmfc.mi)
            tm.tm_min = tmfc.mi;
        if (tmfc.hh)
            tm.tm_hour = tmfc.hh;

        if (tmfc.clock == CLOCK_12_HOUR) {
            if (tm.tm_hour < 1 || tm.tm_hour > HOURS_PER_DAY / 2) {
                context->SetError(
                        "hour is invalid for the 12-hour clock. Use 24-hour clock,or give an hour between 1 and 12.");
                return TimestampVal::null();
            }
            if (tmfc.pm && tm.tm_hour < HOURS_PER_DAY / 2)
                tm.tm_hour += HOURS_PER_DAY / 2;
            else if (!tmfc.pm && tm.tm_hour == HOURS_PER_DAY / 2)
                tm.tm_hour = 0;
        }

        if (tmfc.year) {
            if (tmfc.cc && tmfc.yysz <= 2) {
                if (tmfc.bc)
                    tmfc.cc = -tmfc.cc;
                tm.tm_year = tmfc.year % 100;
                if (tm.tm_year) {
                    if (tmfc.cc >= 0)
                        tm.tm_year += (tmfc.cc - 1) * 100;
                    else
                        tm.tm_year = (tmfc.cc + 1) * 100 - tm.tm_year + 1;
                } else
                    tm.tm_year = tmfc.cc * 100 + ((tmfc.cc >= 0) ? 0 : 1);
            } else {
                tm.tm_year = tmfc.year;
                if (tmfc.bc && tm.tm_year > 0)
                    tm.tm_year = -(tm.tm_year - 1);
            }
        } else if (tmfc.cc) {
            if (tmfc.bc)
                tmfc.cc = -tmfc.cc;
            if (tmfc.cc >= 0)
                tm.tm_year = (tmfc.cc - 1) * 100 + 1;
            else
                tm.tm_year = tmfc.cc * 100 + 1;
        }

        if (tmfc.j)
            j2date(tmfc.j, &tm.tm_year, &tm.tm_mon, &tm.tm_mday);

        if (tmfc.ww) {
            if (tmfc.mode == FCDI) {
                if (tmfc.d)
                    isoweekdate2date(tmfc.ww, tmfc.d, &tm.tm_year,
                                     &tm.tm_mon, &tm.tm_mday);
                else
                    isoweek2date(tmfc.ww, &tm.tm_year, &tm.tm_mon,
                                 &tm.tm_mday);
            } else
                tmfc.ddd = (tmfc.ww - 1) * 7 + 1;
        }

        if (tmfc.w)
            tmfc.dd = (tmfc.w - 1) * 7 + 1;
        if (tmfc.d)
            tm.tm_wday = tmfc.d - 1;
        if (tmfc.dd)
            tm.tm_mday = tmfc.dd;
        if (tmfc.ddd)
            tm.tm_yday = tmfc.ddd;
        if (tmfc.mm)
            tm.tm_mon = tmfc.mm;

        if (tmfc.ddd && (tm.tm_mon <= 1 || tm.tm_mday <= 1)) {
            if (!tm.tm_year && !tmfc.bc) {
                context->SetError(
                        "ERRCODE_INVALID_DATETIME_FORMAT.cannot calculate day of year without year information.");
                return TimestampVal::null();
            }
            if (tmfc.mode == FCDI) {
                int j0;

                j0 = isoweek2j(tm.tm_year, 1) - 1;

                j2date(j0 + tmfc.ddd, &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
            } else {
                const int *y;
                int i;

                static const int ysum[2][13] = {
                        {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334,
                                                                                365},
                        {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
                };

                y = ysum[isleap(tm.tm_year)];

                for (i = 1; i <= MONTHS_PER_YEAR; i++) {
                    if (tmfc.ddd < y[i])
                        break;
                }
                if (tm.tm_mon <= 1)
                    tm.tm_mon = i;

                if (tm.tm_mday <= 1)
                    tm.tm_mday = tmfc.ddd - y[i - 1];
            }
        }

        if (tmfc.ms)
            fsec += tmfc.ms * 1000;
        if (tmfc.us)
            fsec += tmfc.us;


        date d(tm.tm_year, tm.tm_mon, tm.tm_mday);
        gregorian_calendar::ymd_type ymd = d.year_month_day();
        dateValue = gregorian_calendar::day_number(ymd);
        TimestampVal fresult(dateValue, timeValue);
        return fresult;
    }
    catch (std::exception &e) {
        string message = e.what();
        context->SetError(message.c_str());
    }


}

//************************************************************************************
//           ice_to_timestamp: BEGIN 
//************************************************************************************
TimestampVal
ice_to_timestamp(FunctionContext *context,
                 const StringVal &valueStr,
                 const StringVal &formatStr) {
    if (valueStr.is_null || formatStr.is_null)
        return TimestampVal::null();
    try {
        FND formatNd[ICE_CACHE_SIZE + 1];
        const char *fmt_str;
        const char *val_str;
        TimeInChar tmfc;
        bool incache;
        int fmt_len;
        string fmt;
        const KSuf *s;
        tms tm;
        long fsec = 0L;
        int32_t dateValue = 0;
        int64_t timeValue = 0;
        const KWord *kk;
        int node_set = 0;
        int suffix = 0;
        INIT_tc(&tmfc);
        INIT_tm(&tm);

        string format((const char *) formatStr.ptr, formatStr.len);
        string value((const char *) valueStr.ptr, valueStr.len);
        val_str = value.c_str();
        fmt_str = format.c_str();
        fmt_len = strlen(fmt_str);

        if (fmt_len) {

            if (fmt_len > ICE_CACHE_SIZE) {
                context->SetError("format string length cannot be greater that 128 ");
                return TimestampVal::null();
            } else {
                get_format(formatNd, fmt_str, ICE_KWords,
                           ICE_suff, ICE_index, ICE_TYPE, 0);

                (formatNd + fmt_len)->type = N_END;

            }
            ICE_get_char(formatNd, val_str, &tmfc, context);
        }

        if (tmfc.ssss) {
            int x = tmfc.ssss;

            tm.tm_hour = x / SECS_PER_HOUR;
            x %= SECS_PER_HOUR;
            tm.tm_min = x / SECS_PER_MINUTE;
            x %= SECS_PER_MINUTE;
            tm.tm_sec = x;
        }

        if (tmfc.ss)
            tm.tm_sec = tmfc.ss;
        if (tmfc.mi)
            tm.tm_min = tmfc.mi;
        if (tmfc.hh)
            tm.tm_hour = tmfc.hh;

        if (tmfc.clock == CLOCK_12_HOUR) {
            if (tm.tm_hour < 1 || tm.tm_hour > HOURS_PER_DAY / 2) {
                context->SetError(
                        "hour is invalid for the 12-hour clock. Use 24-hour clock,or give an hour between 1 and 12.");
                return TimestampVal::null();
            }
            if (tmfc.pm && tm.tm_hour < HOURS_PER_DAY / 2)
                tm.tm_hour += HOURS_PER_DAY / 2;
            else if (!tmfc.pm && tm.tm_hour == HOURS_PER_DAY / 2)
                tm.tm_hour = 0;
        }

        if (tmfc.year) {
            if (tmfc.cc && tmfc.yysz <= 2) {
                if (tmfc.bc)
                    tmfc.cc = -tmfc.cc;
                tm.tm_year = tmfc.year % 100;
                if (tm.tm_year) {
                    if (tmfc.cc >= 0)
                        tm.tm_year += (tmfc.cc - 1) * 100;
                    else
                        tm.tm_year = (tmfc.cc + 1) * 100 - tm.tm_year + 1;
                } else
                    tm.tm_year = tmfc.cc * 100 + ((tmfc.cc >= 0) ? 0 : 1);
            } else {
                tm.tm_year = tmfc.year;
                if (tmfc.bc && tm.tm_year > 0)
                    tm.tm_year = -(tm.tm_year - 1);
            }
        } else if (tmfc.cc) {
            if (tmfc.bc)
                tmfc.cc = -tmfc.cc;
            if (tmfc.cc >= 0)
                tm.tm_year = (tmfc.cc - 1) * 100 + 1;
            else
                tm.tm_year = tmfc.cc * 100 + 1;
        }

        if (tmfc.j)
            j2date(tmfc.j, &tm.tm_year, &tm.tm_mon, &tm.tm_mday);

        if (tmfc.ww) {
            if (tmfc.mode == FCDI) {
                if (tmfc.d)
                    isoweekdate2date(tmfc.ww, tmfc.d, &tm.tm_year,
                                     &tm.tm_mon, &tm.tm_mday);
                else
                    isoweek2date(tmfc.ww, &tm.tm_year, &tm.tm_mon,
                                 &tm.tm_mday);
            } else
                tmfc.ddd = (tmfc.ww - 1) * 7 + 1;
        }

        if (tmfc.w)
            tmfc.dd = (tmfc.w - 1) * 7 + 1;
        if (tmfc.d)
            tm.tm_wday = tmfc.d - 1;
        if (tmfc.dd)
            tm.tm_mday = tmfc.dd;
        if (tmfc.ddd)
            tm.tm_yday = tmfc.ddd;
        if (tmfc.mm)
            tm.tm_mon = tmfc.mm;

        if (tmfc.ddd && (tm.tm_mon <= 1 || tm.tm_mday <= 1)) {
            if (!tm.tm_year && !tmfc.bc) {
                context->SetError(
                        "ERRCODE_INVALID_DATETIME_FORMAT.cannot calculate day of year without year information.");
                return TimestampVal::null();
            }
            if (tmfc.mode == FCDI) {
                int j0;

                j0 = isoweek2j(tm.tm_year, 1) - 1;

                j2date(j0 + tmfc.ddd, &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
            } else {
                const int *y;
                int i;

                static const int ysum[2][13] = {
                        {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334,
                                                                                365},
                        {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366}
                };

                y = ysum[isleap(tm.tm_year)];

                for (i = 1; i <= MONTHS_PER_YEAR; i++) {
                    if (tmfc.ddd < y[i])
                        break;
                }
                if (tm.tm_mon <= 1)
                    tm.tm_mon = i;

                if (tm.tm_mday <= 1)
                    tm.tm_mday = tmfc.ddd - y[i - 1];
            }
        }
        if (tmfc.ms > 999 || tmfc.us > 999999) {
            context->SetError("ERRCODE_INVALID_DATETIME_FORMAT.Invalid Time");
            return TimestampVal::null();
        }


        if (tmfc.ms)
            fsec += tmfc.ms * 1000000;
        if (tmfc.us)
            fsec += tmfc.us * 1000;

        date d(tm.tm_year, tm.tm_mon, tm.tm_mday);

        gregorian_calendar::ymd_type ymd = d.year_month_day();

        dateValue = gregorian_calendar::day_number(ymd);

        if ((tm.tm_hour > 24 || tm.tm_hour < 0) || (tm.tm_min > 59 || tm.tm_min < 0) ||
            (tm.tm_sec > 59 || tm.tm_sec < 0)) {
            context->SetError("ERRCODE_INVALID_DATETIME_FORMAT.Invalid Time");
            return TimestampVal::null();
        }

        time_duration td(tm.tm_hour, tm.tm_min, tm.tm_sec, fsec);

        timeValue = td.total_nanoseconds();

        TimestampVal fresult(dateValue, timeValue);
        return fresult;

    }
    catch (std::exception &e) {
        string message = e.what();
        context->SetError(message.c_str());
    }

}
//************************************************************************************
//           ice_to_timestamp: END 
//************************************************************************************
