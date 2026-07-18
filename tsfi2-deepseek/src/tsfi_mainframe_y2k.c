#include "tsfi_mainframe_computerworld.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// Y2K Date Windowing Strategy: Pivot year 50
uint32_t tsfi_cw_y2k_resolve_year(uint32_t two_digit_year) {
    if (two_digit_year < 50) {
        return 2000 + two_digit_year;
    } else {
        return 1900 + two_digit_year;
    }
}

int tsfi_cw_y2k_check_date(uint32_t yy, uint32_t mm, uint32_t dd, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = 0;
    
    if (mm < 1 || mm > 12) return 0;
    
    uint32_t resolved_year = tsfi_cw_y2k_resolve_year(yy);
    int days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    
    // Centurial leap rules: Auncient mainframe Y2K compliance checks
    if ((resolved_year % 4 == 0 && resolved_year % 100 != 0) || (resolved_year % 400 == 0)) {
        days_in_month[1] = 29;
    }
    
    if (dd >= 1 && dd <= (uint32_t)days_in_month[mm - 1]) {
        *is_valid = 1;
    }
    return 0;
}

int tsfi_cw_gregorian_to_julian(uint32_t yy, uint32_t mm, uint32_t dd, char *julian_out, int max_len) {
    if (!julian_out || max_len < 7) return -1;
    
    int is_valid = 0;
    tsfi_cw_y2k_check_date(yy, mm, dd, &is_valid);
    if (!is_valid) return -2;
    
    uint32_t resolved_year = tsfi_cw_y2k_resolve_year(yy);
    int days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if ((resolved_year % 4 == 0 && resolved_year % 100 != 0) || (resolved_year % 400 == 0)) {
        days_in_month[1] = 29;
    }
    
    int day_of_year = 0;
    for (int i = 0; i < (int)mm - 1; i++) {
        day_of_year += days_in_month[i];
    }
    day_of_year += dd;
    
    snprintf(julian_out, max_len, "%02u.%03u", yy % 100, day_of_year);
    return 0;
}

int tsfi_cw_julian_to_gregorian(const char *julian_in, uint32_t *yy_out, uint32_t *mm_out, uint32_t *dd_out) {
    if (!julian_in || !yy_out || !mm_out || !dd_out) return -1;
    
    uint32_t yy = 0, day_of_year = 0;
    if (sscanf(julian_in, "%u.%u", &yy, &day_of_year) != 2) return -2;
    
    uint32_t resolved_year = tsfi_cw_y2k_resolve_year(yy);
    int days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if ((resolved_year % 4 == 0 && resolved_year % 100 != 0) || (resolved_year % 400 == 0)) {
        days_in_month[1] = 29;
    }
    
    if (day_of_year < 1 || day_of_year > 366) return -3;
    
    uint32_t current_days = 0;
    uint32_t mm = 0;
    for (int i = 0; i < 12; i++) {
        if (current_days + days_in_month[i] >= day_of_year) {
            mm = i + 1;
            break;
        }
        current_days += days_in_month[i];
    }
    
    if (mm == 0) return -4;
    
    *yy_out = yy;
    *mm_out = mm;
    *dd_out = day_of_year - current_days;
    return 0;
}

uint32_t tsfi_cw_y2k_resolve_year_ex(uint32_t two_digit_year, uint32_t pivot) {
    if (two_digit_year < pivot) {
        return 2000 + two_digit_year;
    } else {
        return 1900 + two_digit_year;
    }
}

int tsfi_cw_y2k_check_date_ex(uint32_t yy, uint32_t mm, uint32_t dd, uint32_t pivot, int *is_valid) {
    if (!is_valid) return -1;
    *is_valid = 0;
    if (mm < 1 || mm > 12) return 0;
    uint32_t resolved_year = tsfi_cw_y2k_resolve_year_ex(yy, pivot);
    int days_in_month[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if ((resolved_year % 4 == 0 && resolved_year % 100 != 0) || (resolved_year % 400 == 0)) {
        days_in_month[1] = 29;
    }
    if (dd >= 1 && dd <= (uint32_t)days_in_month[mm - 1]) {
        *is_valid = 1;
    }
    return 0;
}

uint32_t tsfi_cw_y2k_resolve_year_multi(uint32_t two_digit_year, uint32_t century_prefix) {
    return century_prefix * 100 + two_digit_year;
}

int tsfi_cw_y2k_adjust_leap_seconds(uint32_t year, int *seconds_offset) {
    if (!seconds_offset) return -1;
    if (year < 1972) {
        *seconds_offset = 0;
    } else if (year < 1980) {
        *seconds_offset = 10;
    } else if (year < 1990) {
        *seconds_offset = 19;
    } else {
        *seconds_offset = 27;
    }
    return 0;
}

int tsfi_cw_y2k_is_century_leap(uint32_t year) {
    if (year % 400 == 0) return 1;
    if (year % 100 == 0) return 0;
    return (year % 4 == 0) ? 1 : 0;
}

int tsfi_cw_parse_multi_format_date(const char *date_str, const char *format, uint32_t *yy, uint32_t *mm, uint32_t *dd) {
    if (!date_str || !format || !yy || !mm || !dd) return -1;
    if (strcmp(format, "MMDDYY") == 0) {
        if (sscanf(date_str, "%2u%2u%2u", mm, dd, yy) != 3) return -2;
        return 0;
    } else if (strcmp(format, "DDMMYY") == 0) {
        if (sscanf(date_str, "%2u%2u%2u", dd, mm, yy) != 3) return -2;
        return 0;
    }
    return -3;
}

int tsfi_cw_julian_standardize(const char *julian_in, uint32_t pivot, char *julian_out, int max_len) {
    if (!julian_in || !julian_out || max_len <= 0) return -1;
    uint32_t yy = 0, ddd = 0;
    if (sscanf(julian_in, "%u.%u", &yy, &ddd) != 2) return -2;
    
    uint32_t full_year = tsfi_cw_y2k_resolve_year_ex(yy, pivot);
    snprintf(julian_out, max_len, "%04u.%03u", full_year, ddd);
    return 0;
}

int tsfi_cw_y2k_check_century_leap_2100(uint32_t year, int *is_leap) {
    if (!is_leap) return -1;
    if (year % 100 == 0) {
        *is_leap = (year % 400 == 0) ? 1 : 0;
    } else {
        *is_leap = (year % 4 == 0) ? 1 : 0;
    }
    return 0;
}

int tsfi_cw_julian_to_gregorian_y2k(const char *julian_in, uint32_t pivot, char *greg_out, int max_len) {
    if (!julian_in || !greg_out || max_len <= 0) return -1;
    uint32_t yy = 0, ddd = 0;
    if (sscanf(julian_in, "%u.%u", &yy, &ddd) != 2) return -2;
    if (ddd < 1 || ddd > 366) return -3;
    
    uint32_t year = tsfi_cw_y2k_resolve_year_ex(yy, pivot);
    int is_leap = 0;
    tsfi_cw_y2k_check_century_leap_2100(year, &is_leap);
    
    int days_in_months[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (is_leap) days_in_months[1] = 29;
    
    uint32_t day_sum = 0;
    uint32_t month = 0;
    uint32_t day = 0;
    for (int m = 0; m < 12; m++) {
        if (day_sum + days_in_months[m] >= ddd) {
            month = m + 1;
            day = ddd - day_sum;
            break;
        }
        day_sum += days_in_months[m];
    }
    
    snprintf(greg_out, max_len, "%04u-%02u-%02u", year, month, day);
    return 0;
}

static int tsfi_cw_days_from_epoch(uint32_t year, uint32_t month, uint32_t day) {
    int days = day;
    int days_in_months[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    for (uint32_t y = 1900; y < year; y++) {
        int leap = 0;
        if (y % 100 == 0) leap = (y % 400 == 0) ? 1 : 0;
        else leap = (y % 4 == 0) ? 1 : 0;
        days += leap ? 366 : 365;
    }
    int is_leap = 0;
    if (year % 100 == 0) is_leap = (year % 400 == 0) ? 1 : 0;
    else is_leap = (year % 4 == 0) ? 1 : 0;
    if (is_leap) days_in_months[1] = 29;
    
    for (uint32_t m = 1; m < month; m++) {
        days += days_in_months[m - 1];
    }
    return days;
}

int tsfi_cw_y2k_date_diff(uint32_t yy1, uint32_t mm1, uint32_t dd1, uint32_t yy2, uint32_t mm2, uint32_t dd2, uint32_t pivot, int *days_out) {
    if (!days_out) return -1;
    uint32_t year1 = tsfi_cw_y2k_resolve_year_ex(yy1, pivot);
    uint32_t year2 = tsfi_cw_y2k_resolve_year_ex(yy2, pivot);
    
    int d1 = tsfi_cw_days_from_epoch(year1, mm1, dd1);
    int d2 = tsfi_cw_days_from_epoch(year2, mm2, dd2);
    
    *days_out = d2 - d1;
    return 0;
}

uint32_t tsfi_cw_y2k_resolve_epoch_base(uint32_t two_digit_year, uint32_t base_epoch) {
    uint32_t pivot = base_epoch % 100;
    uint32_t century = base_epoch / 100;
    if (two_digit_year < pivot) {
        return (century + 1) * 100 + two_digit_year;
    } else {
        return century * 100 + two_digit_year;
    }
}

int tsfi_cw_y2k_check_date_bounds(uint32_t yy, uint32_t mm, uint32_t dd, uint32_t pivot) {
    if (mm < 1 || mm > 12 || dd < 1) return -1;
    uint32_t year = tsfi_cw_y2k_resolve_year_ex(yy, pivot);
    int is_leap = 0;
    tsfi_cw_y2k_check_century_leap_2100(year, &is_leap);
    
    int days_in_months[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (is_leap) days_in_months[1] = 29;
    
    if (dd > (uint32_t)days_in_months[mm - 1]) return -2;
    return 0;
}

int tsfi_cw_y2k_day_of_week(uint32_t yy, uint32_t mm, uint32_t dd, uint32_t pivot, int *dow_out) {
    if (!dow_out) return -1;
    uint32_t year = tsfi_cw_y2k_resolve_year_ex(yy, pivot);
    int days = tsfi_cw_days_from_epoch(year, mm, dd);
    *dow_out = days % 7;
    return 0;
}
