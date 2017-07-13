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
#include <sstream>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/math/special_functions/trunc.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/locale/date_time_facet.hpp>

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
#define S_FM(_s) (((_s)&ICE_S_FM) ? 1 : 0)
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

#define FORMAT_STRING_LEN_MAX 50
#define DECIMAL_FORMAT_STRING_LEN_MAX 38

// STRUCTs
typedef struct {
    const char *name;
    int
            len,
            id,
            type;
} KSuf;

static const KSuf ICE_suff[] = {
        {"FM", 2, ICE_S_FM, PREFIX_TYPES},
        {"fm", 2, ICE_S_FM, PREFIX_TYPES}
};

typedef enum {
    FCDN = 0,
    FCDG,
    FCDI
} FCDM;

typedef struct FND
        FND;

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


static const KSuf *suff_search(const char *str,
                               const KSuf *suf, int type);

static const KWord *index_seq_search(char *str, const KWord *kw,
                                     const int *index);

static int get_int_len(int *dest, char **src,
                       const int len, FND *node, FunctionContext *context);

static int seq_search(char *name, const char *const *array,
                      int type, int max, int *len);

using boost::posix_time::ptime_from_tm;
using boost::posix_time::time_duration;
using boost::posix_time::to_tm;
using boost::posix_time::nanoseconds;
using namespace std;
using boost::posix_time::ptime;
using namespace boost::gregorian;
using namespace boost::posix_time;

string get_roman(unsigned int value) {
    struct romandata_t {
        unsigned int value;
        char const *numeral;
    };
    const struct romandata_t romandata[] = {
            {1000, "M"},
            {900,  "CM"},
            {500,  "D"},
            {400,  "CD"},
            {100,  "C"},
            {90,   "XC"},
            {50,   "L"},
            {40,   "XL"},
            {10,   "X"},
            {9,    "IX"},
            {5,    "V"},
            {4,    "IV"},
            {1,    "I"},
            {0, NULL}
    };

    std::string result;
    for (const romandata_t *current = romandata; current->value > 0;
         ++current) {
        while (value >= current->value) {
            result += current->numeral;
            value -= current->value;
        }
    }
    return result;
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


static string
ICE_conv_fmt_to_char(FND *ifmt, ptime &pt, FunctionContext *context) {
    try {
        FND *localfmt;
        string result("");
        date_facet *datef = new date_facet();
        boost::posix_time::time_facet *timef = new boost::posix_time::time_facet();
        stringstream oss;
        stringstream tss;

        oss.imbue(std::locale(std::locale(""), datef));
        tss.imbue(std::locale(std::locale(""), timef));
        if (ifmt != NULL) {

            for (localfmt = ifmt; localfmt->type != N_END; localfmt++) {

                if (localfmt->type != N_ACTION) {
                    oss.str("");
                    oss << localfmt->character;
                    result.append(oss.str());
                    continue;
                }

                switch (localfmt->key->id) {

                    case ICE_A_M:
                    case ICE_P_M:
                        (pt.time_of_day().hours() % HOURS_PER_DAY >= HOURS_PER_DAY / 2)
                        ? result.append(P_M_STR)
                        : result.append(A_M_STR);
                        break;

                    case ICE_AM:
                    case ICE_PM:
                        (pt.time_of_day().hours() % HOURS_PER_DAY >= HOURS_PER_DAY / 2)
                        ? result.append(PM_STR)
                        : result.append(AM_STR);
                        break;

                    case ICE_a_m:
                    case ICE_p_m:
                        (pt.time_of_day().hours() % HOURS_PER_DAY >= HOURS_PER_DAY / 2)
                        ? result.append(p_m_STR)
                        : result.append(a_m_STR);
                        break;

                    case ICE_am:
                    case ICE_pm:
                        (pt.time_of_day().hours() % HOURS_PER_DAY >= HOURS_PER_DAY / 2)
                        ? result.append(pm_STR)
                        : result.append(am_STR);
                        break;

                    case ICE_HH:
                    case ICE_HH12:
                        tss.str("");
                        (S_FM(localfmt->suffix)) ? timef->time_duration_format("%l") : timef->time_duration_format(
                                "%I");
                        tss << pt.time_of_day();
                        result.append(tss.str());
                        break;

                    case ICE_HH24:
                        tss.str("");
                        (S_FM(localfmt->suffix)) ? timef->time_duration_format("%k") : timef->time_duration_format(
                                "%H");
                        tss << pt.time_of_day();
                        result.append(tss.str());
                        tss.str("");
                        break;

                    case ICE_MI:
                        tss.str("");
                        timef->time_duration_format("%M");
                        tss << pt.time_of_day();
                        result.append(tss.str());
                        tss.str("");
                        break;

                    case ICE_SS:
                        tss.str("");
                        timef->time_duration_format("%S");
                        tss << pt.time_of_day();
                        result.append(tss.str());
                        tss.str("");
                        break;

                    case ICE_MS:
                        tss.str("");
                        timef->time_duration_format("%f");
                        tss << pt.time_of_day();
                        result.append(tss.str().substr(0, 3));
                        break;

                    case ICE_US:
                        tss.str("");
                        timef->time_duration_format("%f");
                        tss << pt.time_of_day();
                        result.append(tss.str().substr(0, 6));
                        break;

                    case ICE_SSSS:
                        tss.str("");
                        timef->time_duration_format("%f");
                        tss << boost::format("%d") % (pt.time_of_day().total_seconds());
                        result.append(tss.str());
                        break;

                    case ICE_tz:
                        break;

                    case ICE_OF:
                        break;

                    case ICE_A_D:
                    case ICE_B_C:
                        (pt.date().year() <= 0
                         ? result.append(B_C_STR)
                         : result.append(A_D_STR));
                        break;

                    case ICE_AD:
                    case ICE_BC:
                        (pt.date().year() <= 0
                         ? result.append(BC_STR)
                         : result.append(AD_STR));
                        break;

                    case ICE_a_d:
                    case ICE_b_c:
                        (pt.date().year() <= 0
                         ? result.append(b_c_STR)
                         : result.append(a_d_STR));
                        break;

                    case ICE_ad:
                    case ICE_bc:
                        (pt.date().year() <= 0
                         ? result.append(bc_STR)
                         : result.append(ad_STR));
                        break;

                    case ICE_MONTH:
                        oss.str("");
                        datef->format("%B");
                        oss << pt.date();
                        result.append(boost::to_upper_copy<std::string>(oss.str()));
                        break;

                    case ICE_Month:
                        oss.str("");
                        datef->format("%B");
                        oss << pt.date();
                        result.append(oss.str());
                        break;

                    case ICE_month:
                        oss.str("");
                        datef->format("%B");
                        oss << pt.date();
                        result.append(boost::to_lower_copy<std::string>(oss.str()));
                        break;

                    case ICE_MON:
                        oss.str("");
                        datef->format("%b");
                        oss << pt.date();
                        result.append(boost::to_upper_copy<std::string>(oss.str()));
                        break;

                    case ICE_Mon:
                        oss.str("");
                        datef->format("%b");
                        oss << pt.date();
                        result.append(oss.str());
                        break;

                    case ICE_mon:
                        oss.str("");
                        datef->format("%b");
                        oss << pt.date();
                        result.append(boost::to_lower_copy<std::string>(oss.str()));
                        break;

                    case ICE_MM:
                        oss.str("");
                        datef->format("%m");
                        oss << pt.date();
                        result.append(oss.str());
                        break;

                    case ICE_DAY:
                        oss.str("");
                        datef->format("%A");
                        oss << pt.date();
                        result.append(boost::to_upper_copy<std::string>(oss.str()));
                        break;

                    case ICE_Day:
                        oss.str("");
                        datef->format("%A");
                        oss << pt.date();
                        result.append(oss.str());
                        break;

                    case ICE_day:
                        oss.str("");
                        datef->format("%a");
                        oss << pt.date();
                        result.append(boost::to_lower_copy<std::string>(oss.str()));
                        break;

                    case ICE_DY:
                        oss.str("");
                        datef->format("%a");
                        oss << pt.date();
                        result.append(boost::to_upper_copy<std::string>(oss.str()));
                        break;

                    case ICE_Dy:

                        oss.str("");
                        datef->format("%a");
                        oss << pt.date();
                        result.append(oss.str());
                        break;

                    case ICE_dy:
                        oss.str("");
                        datef->format("%a");
                        oss << pt.date();
                        result.append(boost::to_lower_copy<std::string>(oss.str()));
                        break;

                    case ICE_DDD:
                        oss.str("");
                        datef->format("%j");
                        oss << pt.date();
                        result.append(oss.str());
                        break;

                    case ICE_IDDD:
                        break;

                    case ICE_DD:
                        oss.str("");
                        datef->format("%d");
                        oss << pt.date();
                        result.append(oss.str());
                        break;

                    case ICE_D:
                        oss.str("");
                        datef->format("%u");
                        oss << pt.date();
                        result.append(oss.str());
                        break;

                    case ICE_ID:
                        break;

                    case ICE_WW:
                        oss.str("");
                        datef->format("%U");
                        oss << pt.date();
                        result.append(boost::to_upper_copy<std::string>(oss.str()));
                        break;

                    case ICE_IW:
                        oss.str("");
                        oss << boost::format("%d") % (pt.date().week_number());
                        result.append(oss.str());
                        break;

                    case ICE_Q:
                        oss.str("");
                        oss << boost::format("%d") % ((pt.date().month() - 1) / 3 + 1);
                        result.append(oss.str());
                        break;

                    case ICE_CC:
                        oss.str("");
                        oss << boost::format("%d") % ((pt.date().year() + 99) / 100);
                        result.append(oss.str());
                        break;

                    case ICE_Y_YYY:
                        break;

                    case ICE_YYYY:
                        oss.str("");
                        datef->format("%Y");
                        oss << pt.date();
                        result.append(oss.str());
                        break;
                    case ICE_IYYY:
                        break;

                    case ICE_YYY:
                        oss.str("");
                        datef->format("%Y");
                        oss << pt.date();
                        result.append(oss.str().substr(1, 4));
                        break;
                    case ICE_IYY:
                        break;

                    case ICE_YY:
                        oss.str("");
                        datef->format("%Y");
                        oss << pt.date();
                        result.append(oss.str().substr(2, 4));
                        break;

                    case ICE_IY:
                        break;

                    case ICE_Y:
                        oss.str("");
                        datef->format("%Y");
                        oss << pt.date();
                        result.append(oss.str().substr(3, 4));
                        break;

                    case ICE_I:
                        break;

                    case ICE_RM:
                        result.append(get_roman(pt.date().month()));
                        break;

                    case ICE_rm:
                        result.append(boost::to_lower_copy<std::string>(get_roman(pt.date().month())));
                        break;
                    case ICE_W:
                        oss.str("");
                        oss << (pt.date().day() - 1) / 7 + 1;
                        result.append(oss.str());
                        break;

                    case ICE_J:
                        oss.str("");
                        datef->format("%W");
                        oss << boost::format("%d") % (pt.date().julian_day());
                        result.append(oss.str());
                        break;
                }
            }
        }

        return result;
    }
    catch (std::exception &e) {
        string message = e.what();
        context->SetError("Exception handling is being tested");
        return NULL;
    }

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


void get_int_value(int i, char *result, char *signValue) {
    i < 0 ? *signValue = '-' : *signValue = ' ';

    sprintf(result, "%d", i);
}


void get_double_value(double d, int afterDecimal,
                      char *result, char *signValue) {
    d < 0 ? *signValue = '-' : *signValue = ' ';

    switch (afterDecimal) {
        case 0:
            sprintf(result, "%50.0f", d);
            break;
        case 1:
            sprintf(result, "%50.1f", d);
            break;
        case 2:
            sprintf(result, "%50.2f", d);
            break;
        case 3:
            sprintf(result, "%50.3f", d);
            break;
        case 4:
            sprintf(result, "%50.4f", d);
            break;
        case 5:
            sprintf(result, "%50.5f", d);
            break;
        case 6:
            sprintf(result, "%50.6f", d);
            break;
        case 7:
            sprintf(result, "%50.7f", d);
            break;
        case 8:
            sprintf(result, "%50.8f", d);
            break;
        case 9:
            sprintf(result, "%50.9f", d);
            break;
        case 10:
            sprintf(result, "%50.10f", d);
            break;
        case 11:
            sprintf(result, "%50.11f", d);
            break;
        Default:
            sprintf(result, "%50.12f", d);
            break;
    }
}

bool checkSpecialCase(char *in, int afterDecimal) {
    bool ret = true;

    for (int i = 0; i < afterDecimal; i++) {
        if (in[i] != '9') {
            ret = false;
            break;
        }
    }
    if (ret) {
        memset(in, 0, sizeof(in));
        for (int i = 0; i < afterDecimal; i++) {
            in[i] = '0';
        }
    }

    return ret;
}

void
get_decimal_value(
        int64_t val,        // in: decimal value, 64 bit
        int32_t scale,      // in: scale in decimal value
        int afterDecimal,   // in: how many digits after "." in output
        char *result,       // Out: result string
        char *signValue     // Out: + or - sign
) {
    bool isSpecialCase = false; // all 9s in fractional_part
    int64_t whole_part;
    int64_t fractional_part;

    val < 0 ? *signValue = '-' : *signValue = ' ';

    whole_part = val;
    for (int i = 0; i < scale; i++) {
        whole_part /= 10;
    }


    char fractional_str[FORMAT_STRING_LEN_MAX + 1];  // scale
    memset(fractional_str, 0, sizeof(fractional_str));

    int fractional_part_digit = 0;

    val < 0 ? fractional_part = val * -1 : fractional_part = val;

    for (int j = 0; j < scale; j++) {
        fractional_part_digit = fractional_part % 10;
        fractional_part /= 10;
        fractional_str[scale - 1 - j] = fractional_part_digit + '0';
    }

    if (scale < afterDecimal) {
        for (int k = strlen(fractional_str); k < afterDecimal; k++) {
            fractional_str[k] = '0';
        }
    } else if (scale > afterDecimal) {
        // do round
        if (fractional_str[afterDecimal] > '4') {
            isSpecialCase = checkSpecialCase(fractional_str, afterDecimal);
            if (isSpecialCase) {
                // whole part + 1
                val < 0 ? whole_part -= 1 : whole_part += 1;
            } else {
                for (int mm = (afterDecimal - 1); mm >= 0; mm--) {
                    if (fractional_str[mm] == '9') {
                        fractional_str[mm] = '0';
                    } else {
                        fractional_str[mm] = fractional_str[mm] + 1;
                        break;
                    }
                }
            }
        }
        fractional_str[afterDecimal] = '\0';
    }

    // copy to result
    if ( (val < 0) && (whole_part == 0) ) {
        strcpy(result, "-0");
    } else {
        sprintf(result, "%39lld", whole_part);
    }
    if (afterDecimal > 0) {
        strcat(result, ".");
        strcat(result, fractional_str);
    }
}


int
parse_format_string(
        char *format,       // in
        int *total,         // out: format string length
        int *afterDecimal,  // out: # of digits after decimal
        int *beforeDecimal, // out: # of digits before decimal, include zerofill
        int *zerofill       // out: # of zero fill digits
) {
    int numChar = 0;
    bool isDecimal = false;
    int numInvalidDigit = 0;
    int numNineAfterDecimal = 0;
    int numZeroAfterDecimal = 0;
    int numNineBeforeDecimal = 0;
    int numZeroBeforeDecimal = 0;

    int tot = strlen(format);
    for (int i = 0; i < tot; i++) {
        char c = format[i];
        if (c == '.')
            isDecimal = true;
        if (c >= ' ' && c <= '~')
            numChar++;
        if (c > '0' && c < '9')
            numInvalidDigit++;
        if (c == '0')
            (isDecimal == false) ? numZeroBeforeDecimal++ : numZeroAfterDecimal++;
        if (c == '9')
            (isDecimal == false) ? numNineBeforeDecimal++ : numNineAfterDecimal++;
    }
    *total = tot;
    *zerofill = numZeroBeforeDecimal;
    *afterDecimal = numNineAfterDecimal;
    *beforeDecimal = numZeroBeforeDecimal + numNineBeforeDecimal;

    return 0;
}

void
copy_value(
        //int intOrDouble,  // 1 -> int; 2 -> double/decimal
        bool isInt,             // IN: if ture -> int; false -> double/decimal
        char *value_withSign,   // IN: input value; string length: FORMAT_STRING_LEN_MAX+1
        char *format,           // IN: format string; string max length: FORMAT_STRING_LEN_MAX
        int afterDecimal,       // IN: number of digits after decimal(.)
        int zerofill,           // IN: number of leading "0" padding digits
        char signValue,         // IN: ' ' OR '-'
        char *finalResult       // OUT: result, string length: FORMAT_STRING_LEN_MAX+2
) {
    char value[FORMAT_STRING_LEN_MAX+1];
    char result[FORMAT_STRING_LEN_MAX+1];

    memset(value, 0, sizeof(value));
    memset(result, 0, sizeof(result));

    // remove leading space
    int index = 0;
    for (index = 0; index < strlen(value_withSign), value_withSign[index] == ' '; index++) {}
    if (index == strlen(value_withSign)) {
        finalResult[0] = '\0';
        return;
    }

    // skip the '-' sign
    if (signValue == '-') {
        strcpy(value, value_withSign + index + 1);
    } else
    {
        strcpy(value, value_withSign + index);
    }

    int tot = strlen(format);

    // If int, should automatically ignore the decimal part if included in format
    // (for example, 999,999,999,999,999.99 => 999,999,999,999,999)
    if (isInt) {
        for (int i = 0; i < tot; i++) {
            char c = format[i];
            if (c == '.') {
                format[i] = '\0';
                break;
            }
        }

        afterDecimal = 0;
        tot = strlen(format);
    } else {
        if (afterDecimal > 0) {
            // ToDo: if double, should automatically convert the format according to the value of afterDecimal
            // (for example, if afterDecimal == 1, 999,999,999,999,999.09  => 999,999,999,999,999.9)
            // (for example, if afterDecimal == 2, 999,999,999,999,999.909 => 999,999,999,999,999.99)
            int j = 0;
            for (int i = 0; i < tot; i++) {
                char c = format[i];
                if (c == '.') {
                    j = i + 1;
                    break;
                }
            }

            // validate decimal format, must be .99999...
            // otherwise, ignore. for example: ".989798" will be convert to ".999"
            int k1 = j;
            for (int k2 = 0; k2 < afterDecimal; k2++) {
                format[k1] = '9';
                k1++;
            }
            format[k1] = '\0';

            // recalculate format strlen
            tot = strlen(format);
        } else {   // afterDecimal == 0
            for (int i = 0; i < tot; i++) {
                char c = format[i];
                if (c == '.') {
                    format[i] = '\0';
                    break;
                }
            }

            tot = strlen(format);
        }
    }

    int resultPtr = tot - 1;
    int valuePtr = strlen(value) - 1;

    if (afterDecimal > 0) {
        for (int i = resultPtr; i >= tot - afterDecimal - 1; i--) {
            if (isInt) {
                result[resultPtr] = '0';
                resultPtr--;
            } else {
                result[resultPtr] = value[valuePtr];
                resultPtr--;
                valuePtr--;
            }
        }
        if (isInt) {
            result[resultPtr + 1] = '.';
        }
    }

    for (int i = resultPtr; i >= 0; i--) {
        int found = 0;
        if (format[i] == '0') {
            if (valuePtr >= 0) {
                if (value[valuePtr] >= '0' && value[valuePtr] <= '9') {
                    result[resultPtr] = value[valuePtr];
                    valuePtr--;
                    found = 1;
                }
                if (value[valuePtr] == '+' || value[valuePtr] == '-') {
                    valuePtr--;
                }
            }
            if (found == 0) {
                result[resultPtr] = '0';
            }
        } else if (format[i] >= '1' && format[i] <= '9') {
            if (valuePtr >= 0) {
                if (value[valuePtr] == '+' || value[valuePtr] == '-'
                    || (value[valuePtr] >= '0' && value[valuePtr] <= '9'))
                {
                    result[resultPtr] = value[valuePtr];
                    valuePtr--;
                    found = 1;
                }
            }
            if (found == 0) {
                zerofill > 0 ? result[resultPtr] = '0' : result[resultPtr] = ' ';

            }
        } else { // "," sign
            if (valuePtr >= 0) {
                if (value[valuePtr] >= '0' && value[valuePtr] <= '9') {
                    result[resultPtr] = ',';
                    found = 1;
                }
                if (value[valuePtr] == '+' || value[valuePtr] == '-') {
                    result[resultPtr] = value[valuePtr];
                    valuePtr--;
                    found = 1;
                }
            }
            if (found == 0) {
                zerofill > 0 ? result[resultPtr] = ',' : result[resultPtr] = ' ';
            }
        }
        resultPtr--;
    }

    result[tot] = '\0';

    // remove leading space
    index = 0;
    for (index = 0; index < strlen(result), result[index] == ' '; index++) {}

    if (signValue == '-') {
        finalResult[0]='-';
    }
    strcat(finalResult, result + index);

//    if (zerofill != 0 && signValue == '-')
//        result[0] = signValue;
}


// support precision upto 18
// whole_part range supported : -9223372036853 to 9223372036853 if scale = 6.
StringVal
ice_to_char_decimal(
        FunctionContext *context,
        const DecimalVal &valueDecimal,
        const IntVal &scale,
        const StringVal &formatStr
) {
    int64_t val64;
    //ctx = context;
    if (valueDecimal.is_null || formatStr.is_null) {
        context->SetError("format string or value can not be NULL");
        return StringVal::null();
    }
    if (formatStr.len > FORMAT_STRING_LEN_MAX) {
        context->SetError("format string length can not great than FORMAT_STRING_LEN_MAX(50)");
        return StringVal::null();
    }
    if (scale.val > DECIMAL_FORMAT_STRING_LEN_MAX) {
        context->SetError("DECIMAL data SCALE value can not great than DECIMAL_FORMAT_STRING_LEN_MAX(38)");
        return StringVal::null();
    }
    try {
        char format[FORMAT_STRING_LEN_MAX+1];
        char msg[FORMAT_STRING_LEN_MAX+2];

        memset(format, 0, sizeof(format));
        memset(msg, 0, sizeof(msg));

        memcpy(format, (const char *) formatStr.ptr, formatStr.len);
        val64 = valueDecimal.val8;

        int total,
                afterDecimal,
                digitsBeforeDecimal,
                zerofill;
        if (parse_format_string(format, &total, &afterDecimal, &digitsBeforeDecimal,
                                &zerofill)
            == 0) {
            char signValue;
            char valueStr[FORMAT_STRING_LEN_MAX+1];

            memset(valueStr, 0, sizeof(valueStr));
            get_decimal_value(val64, scale.val, afterDecimal, valueStr, &signValue);

            copy_value(false, valueStr, format, afterDecimal, zerofill, signValue, msg);
        }

        StringVal result(context, strlen(msg));
        memcpy(result.ptr, msg, strlen(msg));
        return result;
    }
    catch (std::exception &e) {
        string message = e.what();
        context->SetError(message.c_str());
        return StringVal::null();
    }


}

StringVal ice_to_char_double(
        FunctionContext *context,
        const DoubleVal &valueDbl,
        const StringVal &formatStr) {
    try {
        if (valueDbl.is_null || formatStr.is_null) {
            context->SetError("format string or value can not be NULL");
            return StringVal::null();
        }
        if (formatStr.len > FORMAT_STRING_LEN_MAX) {
            context->SetError("format string length can not great than FORMAT_STRING_LEN_MAX(50)");
            return StringVal::null();
        }

        char format[FORMAT_STRING_LEN_MAX+1];
        char msg[FORMAT_STRING_LEN_MAX+2];
        memset(format, 0, sizeof(format));
        memset(msg, 0, sizeof(msg));

        memcpy(format, (const char *) formatStr.ptr, formatStr.len);
        double value = valueDbl.val;

        int total,
                afterDecimal,
                digitsBeforeDecimal,
                zerofill;
        if (parse_format_string(format, &total, &afterDecimal, &digitsBeforeDecimal,
                                &zerofill)
            == 0) {
            char signValue;
            char valueStr[FORMAT_STRING_LEN_MAX+1];
            memset(valueStr, 0, sizeof(valueStr));

            get_double_value(value, afterDecimal, valueStr, &signValue);
            copy_value(false, valueStr, format, afterDecimal, zerofill,
                       signValue, msg);
        }

        StringVal result(context, strlen(msg));
        memcpy(result.ptr, msg, strlen(msg));
        return result;
    }
    catch (std::exception &e) {
        string message = e.what();
        context->SetError(message.c_str());
        return StringVal::null();
    }

}

StringVal ice_to_char_int(
        FunctionContext *context,
        const IntVal &valueInt,
        const StringVal &formatStr) {
    try {
        if (valueInt.is_null || formatStr.is_null) {
            context->SetError("format string or value can not be NULL");
            return StringVal::null();
        }
        if (formatStr.len > FORMAT_STRING_LEN_MAX) {
            context->SetError("format string length can not greater than FORMAT_STRING_LEN_MAX(50) ");
            return StringVal::null();
        }


        char format[FORMAT_STRING_LEN_MAX + 1];
        char msg[FORMAT_STRING_LEN_MAX+2];
        memset(format, 0, sizeof(format));
        memset(msg, 0, sizeof(msg));

        memcpy(format, (const char *) formatStr.ptr, formatStr.len);
        int value = valueInt.val;

        int total,
                afterDecimal,
                digitsBeforeDecimal,
                zerofill;
        if (parse_format_string(format, &total, &afterDecimal, &digitsBeforeDecimal,
                                &zerofill)
            == 0) {
            char signValue;
            char valueStr[FORMAT_STRING_LEN_MAX+1];
            memset(valueStr, 0, sizeof(valueStr));

            get_int_value(value, valueStr, &signValue);
            copy_value(true, valueStr, format, afterDecimal, zerofill,
                       signValue, msg);
        }

        StringVal result(context, strlen(msg));
        memcpy(result.ptr, msg, strlen(msg));
        return result;
    }
    catch (std::exception &e) {
        string message = e.what();
        context->SetError(message.c_str());
        return StringVal::null();
    }

}


//************************************************************************************
//           ice_to_char: BEGIN 
//************************************************************************************


StringVal
ice_to_char(FunctionContext *context,
            const TimestampVal &valueStr,
            const StringVal &formatStr) {
    if (formatStr.is_null) {
        context->SetError("Format string cannot be null");
        return StringVal::null();
    }
    if (valueStr.is_null) {
        //context->AddWarning("The input value passed is null. Please ensure the data is in the timestamp format");
        return StringVal::null();
    }
    try {
        // FND* formatNd;
        FND formatNd[ICE_CACHE_SIZE + 1];
        const char *fmt_str;
        string result("");
        //bool incache;
        int fmt_len;
        string fmt;
        const KSuf *s;
        const KWord *kk;
        int node_set = 0;
        int suffix = 0;

        int32_t dateValue = valueStr.date;
        int64_t time1 = valueStr.time_of_day;

        if (dateValue <= 0) {
            //context->SetError("Invalid date value");
            return StringVal::null();
        }
        gregorian_calendar::ymd_type ymd = gregorian_calendar::from_day_number(dateValue);
        boost::gregorian::date d(ymd.year, ymd.month, ymd.day);
        time_duration td = nanoseconds(time1);
        ptime pt(d, td);
        //ptime pt(boost::gregorian::date(ymd.year, ymd.month, ymd.day),nanoseconds(time1));
        string format((const char *) formatStr.ptr,
                      formatStr.len);
        fmt_str = format.c_str();
        fmt_len = strlen(fmt_str);
        if (fmt_len > ICE_CACHE_SIZE) {
            context->SetError("format string length cannot be greater that 128 ");
            return StringVal::null();
        }

        get_format(formatNd, fmt_str, ICE_KWords,
                   ICE_suff, ICE_index, ICE_TYPE, 0);
        formatNd[fmt_len].type = N_END;


        result = ICE_conv_fmt_to_char(formatNd, pt, context);

        StringVal fresult(context, result.size());
        memcpy(fresult.ptr, result.c_str(), result.size());
        return fresult;
    }
    catch (std::exception &e) {
        string message = e.what();
        context->SetError(message.c_str());
        return StringVal::null();
    }
}

//************************************************************************************
//           ice_to_char: END 
//************************************************************************************
