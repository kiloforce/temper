/*
 * Standalone temperature logger
 *
 */

#include <stdio.h>
#include <time.h>
#include "pcsensor.h"

/* Calibration adjustments */
/* See http://www.pitt-pladdy.com/blog/_20110824-191017_0100_TEMPer_under_Linux_perl_with_Cacti/ */
static float scale = 1.0287;
static float offset = -0.85;

int main(){
	int passes = 0;
	float tempc = 0.0000;
	int rc = 0;
	usb_dev_handle *handle_list[MAX_TEMPER_DEVICES];
	int handle_counter;
	usb_dev_handle *lvr_winusb;
	
	for(handle_counter = 0 ; handle_counter < MAX_TEMPER_DEVICES ; handle_counter++)
		handle_list[handle_counter] = NULL;
	
	if(!setup_libusb_access(handle_list)) {
		printf("Error: failed to find devices");
		exit(1);
	}
	
	for(handle_counter = 0 ; handle_counter < MAX_TEMPER_DEVICES ; handle_counter++) {
 		lvr_winusb = handle_list[handle_counter];
 		if(lvr_winusb == NULL) continue;
 	
		do {
			if(!pcsensor_open(lvr_winusb)) {
				/* Open fails sometime, sleep and try again */
				sleep(3);
			}
			else {
		
				tempc = pcsensor_get_temperature(lvr_winusb);
				pcsensor_close(lvr_winusb);
			}
			++passes;
		}
		/* Read fails silently with a 0.0 return, so repeat until not zero
		   or until we have read the same zero value 3 times (just in case
		   temp is really dead on zero */
		while ((tempc > -0.0001 && tempc < 0.0001) || passes >= 4);
	
		if (!((tempc > -0.0001 && tempc < 0.0001) || passes >= 4)) {
			/* Apply calibrations */
			tempc = (tempc * scale) + offset;
	
			struct tm *utc;
			time_t t;
			t = time(NULL);
			utc = gmtime(&t);
			
			char dt[80];
			strftime(dt, 80, "%d-%b-%Y %H:%M", utc);
	
			printf("%d, %s, %f\n", handle_counter, dt, tempc);
			fflush(stdout);
	
		}
		else {
			rc = 1;
		}
	}

	return rc;
}
