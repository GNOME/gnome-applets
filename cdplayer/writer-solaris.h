/*
 * writer-solaris.h
 */
#ifndef	__WRITER_SOLARIS_H__
#define	__WRITER_SOLARIS_H__

/* Exported function prototypes */
int audio_start(cdda_t *, int, int);
void audio_pause(cdda_t *);
void audio_resume(cdda_t *);
void audio_stop(cdda_t *);
int audio_get_state(cdda_t *, struct cdrom_subchnl *);
void cdda_check(void **, int);
void cdda_cleanup(cdda_t *);

#endif /* __WRITER_SOLARIS_H__ */
