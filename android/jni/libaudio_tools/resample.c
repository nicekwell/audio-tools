#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
//#include <cutils/log.h>

#include "resample.h"
#include "samplerate.h"

int resample_delay(src* s){
	s->in_frames -= (double)s->out_frames/s->src_data.src_ratio;
	s->out_frames = 0;

	return (int)s->in_frames + s->mute_frames;
}

int resample_reset(src* s){
	memset((void*)&s->src_data, 0, sizeof(SRC_DATA));
	s->mute_frames = 0;
	return src_reset(s->src_state);
}

int resample_set_ratio(src *s, double ratio){
	s->src_data.src_ratio = ratio;
	s->in_frames = 0;
	s->out_frames = 0;
	s->mute_frames = 0;
	return src_reset(s->src_state);
}

src* resample_create(double in_rate, double out_rate, int channels, int ms){
	int error;

	src* s = malloc(sizeof(src));
	if(!s){
		printf("Error: malloc()");
		return NULL;
	}
	memset(s, 0, sizeof(src));

	s->src_state = src_new(2, channels, &error);
	if(!s->src_state){
		printf("Error : src_new() failed : %s.\n", src_strerror(error));
		goto ERROR_RETURN;
	}

	s->src_data.end_of_input = 0;
	s->src_data.src_ratio = out_rate/in_rate;
	s->src_data.input_frames = 0;
	s->channels = channels;
	s->sample_size = sizeof(double);
	s->max_input_frames = ceil(in_rate*ms/1000);

	s->in_buf = malloc(ceil(in_rate*ms/1000)*s->sample_size*s->channels);
	if(!s->in_buf){
		printf("Error: malloc()");
		goto ERROR_RETURN;
	}
    /*
	s->out_buf = malloc(ceil(out_rate*ms/1000)*s->sample_size*s->channels);
	if(!s->out_buf){
		printf("Error: malloc()");
		goto ERROR_RETURN;
	}
    */
	return s;

ERROR_RETURN:
	if(s->in_buf)
		free(s->in_buf);
	if(s->out_buf)
		free(s->out_buf);
	if(s->src_state)
		src_delete(s->src_state);

	free(s);
	return NULL;
}

long resample_input(src* s){
	long frames = 0;
	short* buf;
	if(s->mute_frames > 0){
		frames = (s->mute_frames > s->max_input_frames)?s->max_input_frames:s->mute_frames;
		s->mute_frames -= frames;
		memset(s->in_buf, 0, frames * s->sample_size);
	}
	else if(s->input_callback){
		frames = s->input_callback(s->context, (char**)&buf, s->max_input_frames);
		src_short_to_float_array(buf, s->in_buf, frames * s->channels);
	}
	s->in_frames += frames;
	return frames;
}

void resample_insert_mute(src* s, int frames){
	s->mute_frames = frames; 
}

size_t resample_output(src* s, void* buf, int frames){

	size_t odone,idone;
	int error;
	long rest = frames;
	
	s->src_data.data_out = buf;
	do{
		if(s->src_data.input_frames == 0){
			idone = resample_input(s);
			if(idone == 0)
				break;
			s->src_data.data_in = s->in_buf;
			s->src_data.input_frames = idone;
		}

		/*frame number  we need cover*/	
		s->src_data.output_frames = rest;

		if ((error = src_process (s->src_state, &(s->src_data)))){
			printf ("\nError : %s\n", src_strerror (error)) ;
			 return 0;
		}
		/*adjuest buffer after oneshot conver process*/
		s->src_data.data_out += s->src_data.output_frames_gen * s->channels;
		s->src_data.data_in += s->src_data.input_frames_used * s->channels;
		s->src_data.input_frames -= s->src_data.input_frames_used;
	
		rest -= s->src_data.output_frames_gen;
	}while(rest > 0);
	frames -= rest;
	s->out_frames += frames;  
	//src_float_to_int_array(s->out_buf, buf, frames * s->channels);
	return frames;
}

//int resample_regiest_callback(src* s, void* pfunc, void* context){
int resample_regiest_callback(src* s, long(*pfunc)(void *, char **, long), void* context) {
	if(pfunc){
		s->input_callback = pfunc;
		s->context = context;
	}
	return 0;
}

int resample_float(src * s, float* in, float* out, int frames){
    
    int error;
    frames /= s->channels;
    s->src_data.data_out = out;
    s->src_data.data_in = in;
    s->src_data.input_frames = frames;
    s->src_data.output_frames = frames * s->src_data.src_ratio;
    s->src_data.input_frames_used = 0;

    if((error = src_process(s->src_state, &(s->src_data)))){
        printf("\nError : %s\n", src_strerror(error));
        return 0;
    };
    return  s->src_data.output_frames_gen * s->channels;
}

int resample_delete(src* s){
	if(s){
		if(s->in_buf)
			free(s->in_buf);
		if(s->out_buf)
			free(s->out_buf);
		if(s->src_state)
			src_delete(s->src_state);
		free(s);
	}
	return 0;
};


#if 0	/*test*/
#define BUFFER_LEN 480
struct context{
	FILE* fp;
	char* buf;
};

long input_callback(void *arg, char**pbuf, long read_frames)
{
	struct context *ctx = (struct context *)arg;
	read_frames = fread(ctx->buf,8,read_frames,ctx->fp);
	*pbuf = ctx->buf;
	return read_frames;
}
 
int main(int argc, char* argv[]){

	FILE *fin = fopen("in.pcm","r");
	FILE *fout = fopen("out.pcm", "w");
	char* in_buf = malloc(BUFFER_LEN*4*2);
	char* out_buf = malloc(BUFFER_LEN*4*2);
	argc--;
	double inrate = argc?atof(argv[1]):48000;
	src* s = resample_create(inrate,16000,2,10);

	if(!fin || !fout || !s || !in_buf || !out_buf){
		perror("init");
		return -1;
	}
	struct context arg={fin,in_buf};
	resample_regiest_callback(s, input_callback, &arg);
	int done;
	while(1){
		done = resample_output(s,out_buf,160);
		if(!done)
			break;
		printf("\r%08ld	%08ld	%lf",s->in_frames, s->out_frames, resample_delay(s));
		fwrite(out_buf,8,done,fout);
	}
	
	free(in_buf);
	free(out_buf);
	fclose(fin);
	fclose(fout);
	resample_delete(s);
	return 0;
}
#endif
