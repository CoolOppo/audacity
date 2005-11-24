/* fileio.h -- Nyquist code to read sound files */

/* for multiple channel files, one susp is shared by all sounds */
/* the susp in turn must point back to all sound list tails */

typedef struct read_susp_struct {
    snd_susp_node susp;
    snd_node snd;
    snd_list_type *chan;	/* array of back pointers */
    long bytes_per_sample;	/* handy for calculations */
    long cnt;	/* how many sample frames to read */
    cvtfn_type cvtfn;
} read_susp_node, *read_susp_type;


LVAL snd_make_read(unsigned char *filename, time_type offset, time_type t0,
        long *format, long *channels, long *mode, long *bits, long *swap,
        double *srate, double *dur, long *flags, long *byte_offset);
/* LISP: (SND-READ STRING ANYNUM ANYNUM FIXNUM* FIXNUM* FIXNUM* FIXNUM* FIXNUM* ANYNUM* ANYNUM* FIXNUM^ FIXNUM^) */

void read_free();
