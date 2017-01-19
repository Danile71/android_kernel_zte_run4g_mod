#ifndef _3DNR_CONFIG_H
#define _3DNR_CONFIG_H

// E.g. 600 means that THRESHOLD_LOW is ISO600.
#define ISO_ENABLE_THRESHOLD_LOW        600      

// E.g. 800 means that THRESHOLD_HIGH is ISO600.
#define ISO_ENABLE_THRESHOLD_HIGH       800      

#if 0   // Obsolete
// E.g. 60 means that use 60% of Max Current ISO as THRESHOLD_LOW.
#define ISO_ENABLE_THRESHOLD_LOW_PERCENTAGE        60      

// E.g. 80 means that use 80% of Max Current ISO as THRESHOLD_HIGH.
#define ISO_ENABLE_THRESHOLD_HIGH_PERCENTAGE       80      
#endif  // Obsolete

// E.g. 130 means thatrRaise max ISO limitation to 130% when 3DNR on.
// When set to 100, 3DNR is noise improvement priority. 
// When set to higher than 100, 3DNR is frame rate improvement priority.
#define MAX_ISO_INCREASE_PERCENTAGE     100     

// How many frames should 3DNR HW be turned off (for power saving) if it
// stays at inactive state. (Note: inactive state means ISO is lower than
// ISO_ENABLE_THRESHOLD_LOW).
#define HW_POWER_OFF_THRESHOLD          60

// How many frames should 3DNR HW be turned on again if it returns from 
// inactive state and stays at active state. (Note: active state means
// ISO is higher than ISO_ENABLE_THRESHOLD_LOW).
#define HW_POWER_REOPEN_DELAY           4

int get_3dnr_iso_enable_threshold_low(void);
int get_3dnr_iso_enable_threshold_high(void);
#if 0   // Obsolete
int get_3dnr_iso_enable_threshold_low_percentage(void);
int get_3dnr_iso_enable_threshold_high_percentage(void);
#endif  // Obsolete
int get_3dnr_max_iso_increase_percentage(void);
int get_3dnr_hw_power_off_threshold(void);
int get_3dnr_hw_power_reopen_delay(void);

#endif /* _3DNR_CONFIG_H */


