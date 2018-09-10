#ifndef __RESAMPLE_H__
#define __RESAMPLE_H__

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

#include "samplerate.h"

typedef struct  __src{
	
	SRC_STATE *src_state;
	SRC_DATA  src_data;
	long 	  max_input_frames;
	int 	  sample_size;
	int 	  channels;
	float*	  in_buf;
	float*    out_buf;
	double	  in_frames;
	int 	  out_frames;			
	int 	  mute_frames;
	long 	  (*input_callback)(void* context, char** pbuf, long frames);
	void*	 context;
}src;

src* resample_create(double in_rate, double out_rate, int channels, int ms);

int resample_delay(src* s);

int resample_reset(src* s);

int resample_set_ratio(src* s, double ratio);

void resample_insert_mute(src* s, int frames);

long resample_input(src* s);

size_t resample_output(src* s, void* buf, int frames);

int resample_float(src * s, float* in, float* out, int frames);

int resample_regiest_callback(src* s, long(*pfunc)(void *, char **, long), void* context);

int resample_delete(src* s);

#ifdef __cplusplus
}		/* extern "C" */
#endif	/* __cplusplus */

#endif
