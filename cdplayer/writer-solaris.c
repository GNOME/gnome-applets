/*
 * Code for writing CDDA data to the Solaris audio device. The
 * audio_write thread retrieves data from a mutex protected
 * buffer and sends it to the audio device.
 */

#include "cdda-solaris.h"
#include "reader-solaris.h"
#include "writer-solaris.h"
#include <errno.h>

/* Private function prototypes */
static void audio_get_info(cdda_t *, audio_info_t *);
static void audio_set_info(cdda_t *, audio_info_t *);
static void audio_flush(cdda_t *);
static void audio_drain(cdda_t *);
static int cdda_audio_open(cdda_t *);
static void cdda_audio_close(cdda_t *);
static void audio_config(cdda_t *);
static int audio_supports_analog(cdda_t *);
static void audio_write_chunk(cdda_t *, char *);
static void audio_write_eof(cdda_t *);
static void *audio_write(void *);
static void cdda_init(cdda_t *);

/*
 * audio_get_info()
 *
 * Description:
 *	Wrapper for AUDIO_GETINFO ioctl.
 *
 * Arguments:
 *	cdda_t			*cdda		Ptr to cdda state structure
 *	audio_info_t		*info		Info to get
 *
 * Returns:
 *	void
 */
static void
audio_get_info(cdda_t *cdda, audio_info_t *info)
{
	/* Return if not open */
	if (cdda->audio < 0) {
		return;
	}

	/* Get */
	if (ioctl(cdda->audio, AUDIO_GETINFO, info) < 0) {
		perror("AUDIO_GETINFO");
	}

} /* audio_get_info() */


/*
 * audio_set_info()
 *
 * Description:
 *	Wrapper for AUDIO_SETINFO ioctl.
 *
 * Arguments:
 *	cdda_t			*cdda		Ptr to cdda state structure
 *	audio_info_t		*info		Info to set
 *
 * Returns:
 *	void
 */
static void
audio_set_info(cdda_t *cdda, audio_info_t *info)
{
	/* Return if not open */
	if (cdda->audio < 0) {
		return;
	}

	/* Set */
	if (ioctl(cdda->audio, AUDIO_SETINFO, info) < 0) {
		perror("AUDIO_SETINFO");
	}

} /* audio_set_info() */


/*
 * audio_flush()
 *
 * Description:
 *	Wrapper for STREAMS I_FLUSH ioctl.
 *
 * Arguments:
 *	cdda_t		*cdda		Ptr to cdda state structure
 *
 * Returns:
 *	void
 */
static void
audio_flush(cdda_t *cdda)
{
	/* Return if not open */
	if (cdda->audio < 0) {
		return;
	}

	/* Flush */
	if (ioctl(cdda->audio, I_FLUSH, FLUSHRW) < 0) {
		perror("I_FLUSH");
	}

} /* audio_flush() */


/*
 * audio_drain()
 *
 * Description:
 *	Wrapper for AUDIO_DRAIN ioctl.
 *
 * Arguments:
 *	cdda_t		*cdda		Ptr to cdda state structure
 *
 * Returns:
 *	void
 */
static void
audio_drain(cdda_t *cdda)
{
	/* Return if not open */
	if (cdda->audio < 0) {
		return;
	}

	/* Drain */
	if (ioctl(cdda->audio, AUDIO_DRAIN, NULL) < 0) {
		perror("AUDIO_DRAIN");
	}

} /* audio_drain() */


/*
 * cdda_audio_open()
 *
 * Description:
 *	Opens the audio device.
 *
 * Arguments:
 *	cdda_t		*cdda		Ptr to cdda state structure
 *
 * Returns:
 *	int: The error number if open fails
 */
static int
cdda_audio_open(cdda_t *cdda)
{
	extern int errno;
	errno = 0;
	/* Return if already open */
	if (cdda->audio >= 0) {
		return errno;
	}

	/* Open audio device */
	if ((cdda->audio = open(cdda->audio_device, O_WRONLY | O_NDELAY)) < 0) {
		perror("open()");
		return errno;
	}

	fcntl (cdda->audio, F_SETFL, O_WRONLY);
	return errno;
} /* cdda_audio_open() */


/*
 * cdda_audio_close()
 *
 * Description:
 *	Closes the audio device.
 *
 * Arguments:
 *	cdda_t		*cdda		Ptr to cdda state structure
 *
 * Returns:
 *	void
 */
static void
cdda_audio_close(cdda_t *cdda)
{
	/* Return if already closed */
	if (cdda->audio < 0) {
		return;
	}

	/* Close audio device */
	if (close(cdda->audio) < 0) {
		perror("close()");
	} else {
		cdda->audio = -1;
	}

} /* cdda_audio_close() */


/*
 * audio_config()
 *
 * Description:
 *	Sets up audio device for playing cd quality audio.
 *
 * Arguments:
 *	cdda_t		*cdda		Ptr to cdda state structure
 *
 * Returns:
 *	void
 */
static void
audio_config(cdda_t *cdda)
{
	audio_info_t	audio_info;

	/* Initialise */
	AUDIO_INIT(&audio_info, sizeof (audio_info));

	/* Configure for 44.1kHz 16-bit stereo */
	audio_info.play.sample_rate = 44100;
	audio_info.play.channels = 2;
	audio_info.play.precision = 16;
	audio_info.play.encoding = AUDIO_ENCODING_LINEAR;

	/* Apply new settings */
	audio_set_info(cdda, &audio_info);

} /* audio_config() */


/*
 * audio_supports_analog()
 *
 * Description:
 *	Returns 1 if input ports contains AUDIO_CD, zero otherwise.
 *	If AUDIO_CD available then the monitor gain and cd volume
 *	are set to their maxmimum values to ensure output is audible.
 *
 * Arguments:
 *	cdda_t		*cdda		Ptr to cdda state stucture
 *
 * Returns:
 *	0		AUDIO_CD unavailable
 *	1		AUDIO_CD available
 */
static int
audio_supports_analog(cdda_t *cdda)
{
	audio_info_t		audio_info;
	struct cdrom_volctrl	volctrl;

	/* Retrieve info */
	audio_get_info(cdda, &audio_info);

	/* Check for AUDIO_CD */
	if (audio_info.record.avail_ports & AUDIO_CD) {

		/* Initialise */
		AUDIO_INIT(&audio_info, sizeof (audio_info));

		/* Set monitor gain */
		audio_info.monitor_gain = AUDIO_MAX_GAIN;

		/* Apply */
		audio_set_info(cdda, &audio_info);

		/* Configure for CD audio */
		audio_config(cdda);

		/* Set CD volume to max */
		volctrl.channel0 = AUDIO_MAX_GAIN;
		volctrl.channel1 = AUDIO_MAX_GAIN;

		/* Apply */
		if (ioctl(cdda->cdrom, CDROMVOLCTRL, &volctrl) < 0) {
			perror("CDROMVOLCTRL");
		}

		/* Return success */
		return (1);
	}

	/* Return failure */
	return (0);

} /* audio_supports_analog() */


/*
 * audio_pause()
 *
 * Description:
 *	Pause audio output.
 *
 * Arguments:
 *	cdda_t		*cdda		Ptr to cdda state structure
 *
 * Returns:
 *	void
 */
void
audio_pause(cdda_t *cdda)
{
	audio_info_t		audio_info;

	/* If not playing do nothing */
	if (cdda->state == CDDA_STOPPED) {
		return;
	}

	/* Initialise */
	AUDIO_INIT(&audio_info, sizeof (audio_info));

	/* Turn on pause */
	audio_info.play.pause = 1;

	/* Apply */
	audio_set_info(cdda, &audio_info);

	/* Set paused flag */
	cdda->state = CDDA_PAUSED;

} /* audio_pause() */


/*
 * audio_resume()
 *
 * Description:
 *	Resume audio output.
 *
 * Arguments:
 *	cdda_t		*cdda		Ptr to cdda state structure
 *
 * Returns:
 *	void
 */
void
audio_resume(cdda_t *cdda)
{
	audio_info_t		audio_info;

	/* If not paused do nothing */
	if (cdda->state != CDDA_PAUSED) {
		return;
	}

	/* Initialise */
	AUDIO_INIT(&audio_info, sizeof (audio_info));

	/* Turn off pause */
	audio_info.play.pause = 0;

	/* Apply */
	audio_set_info(cdda, &audio_info);

	/* Clear paused flag */
	cdda->state = CDDA_PLAYING;

} /* audio_resume() */


/*
 * audio_write_chunk()
 *
 * Description:
 *	Writes data to the device.
 *
 * Arguments:
 *	cdda_t			*cdda			Ptr to cdda state
 *	char			*data			Data to write
 *
 * Returns:
 *	void
 */
static void
audio_write_chunk(cdda_t *cdda, char *data)
{

	if (write(cdda->audio, data, cdda->cds.chunk_bytes) <
			cdda->cds.chunk_bytes) {
		perror("write()");
	}

} /* audio_write_chunk() */


/*
 * audio_write_eof()
 *
 * Description:
 *	Puts an eof marker in the audio stream in order to keep track
 *	of where we are on the cd.
 *
 * Arguments:
 *	cdda_t		*cdda		Ptr to cdda state structure
 *
 * Returns:
 *	void
 */
static void
audio_write_eof(cdda_t *cdda)
{
	if (write(cdda->audio, 0, 0) < 0) {
		perror("write()");
	}

} /* audio_write_eof() */


/*
 * audio_stop()
 *
 * Description:
 *	Shuts down reader/writer threads and closes the audio device.
 *
 * Arguments:
 *	cdda_t		*cdda		Ptr to cdda state structure
 *
 * Returns:
 *	void
 */
void
audio_stop(cdda_t *cdda)
{
	/* If stopped do nothing */
	if (cdda->state == CDDA_STOPPED) {
		return;
	}

	/* May be paused, in which case clear the paused flag */
	if (cdda->state == CDDA_PAUSED) {
		audio_resume(cdda);
	}

	/* Set status */
	cdda->state = CDDA_STOPPED;

	/* Wait for threads */
	if (pthread_join(cdda->cdb.writer, NULL) < 0) {
		perror("pthread_join()");
	}

	if (pthread_join(cdda->cdb.reader, NULL) < 0) {
		perror("pthread_join()");
	}

	/* Flush the device */
	audio_flush(cdda);

	/* Close the device */
	cdda_audio_close(cdda);

} /* audio_stop() */


/*
 * audio_write()
 *
 * Description:
 *	Thread respsonsible for writing from the buffer to the audio
 *	device. Each chunk is followed by an eof marker.
 *
 * Arguments:
 *	void		*in_cdda		Ptr to cdda state structure
 *
 * Returns:
 *	void
 */
static void *
audio_write(void *in_cdda)
{
	char		*data;
	cdda_t		*cdda;

	/* Cast */
	cdda = (cdda_t *)in_cdda;

	/* Get some memory */
	data = malloc(cdda->cds.chunk_bytes);

	if (data == NULL) {
		perror("malloc()");
		exit(EXIT_FAILURE);
	}

	/* While not stopped */
	while (cdda->state != CDDA_STOPPED) {

		/* Get lock */
		(void) pthread_mutex_lock(&cdda->cdb.mutex);

		/* End if finished and nothing in the buffer */
		if ((cdda->cdb.occupied == 0) &&
				(cdda->state == CDDA_COMPLETED)) {
			audio_drain(cdda);
			cdda->state = CDDA_STOPPED;
			cdda_audio_close(cdda);
			(void) pthread_mutex_unlock(&cdda->cdb.mutex);
			break;
		}

		/* Wait until there is something there */
		while ((cdda->cdb.occupied == 0) &&
				(cdda->state != CDDA_STOPPED)) {
			(void) pthread_cond_wait(&cdda->cdb.more,
					&cdda->cdb.mutex);
		}

		/* Break if stopped */
		if (cdda->state == CDDA_STOPPED) {
			(void) pthread_mutex_unlock(&cdda->cdb.mutex);
			break;
		}

		/* Read a chunk from the nextout slot */
		(void) memcpy(data, &cdda->cdb.b[cdda->cds.chunk_bytes *
			cdda->cdb.nextout], cdda->cds.chunk_bytes);

		/* Update pointers */
		cdda->cdb.nextout++;
		cdda->cdb.nextout %= cdda->cds.buffer_chunks;
		cdda->cdb.occupied--;

		/* Release lock and signal room available */
		(void) pthread_cond_signal(&cdda->cdb.less);
		(void) pthread_mutex_unlock(&cdda->cdb.mutex);

		/* Write to device */
		audio_write_chunk(cdda, data);

		/* Write eof marker */
		audio_write_eof(cdda);
	}

	/* Wake up reader */
	(void) pthread_cond_signal(&cdda->cdb.less);

	/* Free memory */
	free(data);

	return (NULL);

} /* audio_write() */


/*
 * audio_get_state()
 *
 * Description:
 *	Fills in the subchannel information, based on the current eof
 *	count. The returned status is one of CDROM_AUDIO_NOSTATUS,
 *	CDROM_AUDIO_PLAY or CDROM_AUDIO_PAUSED.
 *
 * Arguments:
 *	cdda_t			*cdda		Ptr to cdda state structure
 *	struct cdrom_subchnl	*subchnl	Subchannel information
 *
 * Returns:
 *	int			lba		Current logical block addr
 */
int
audio_get_state(cdda_t *cdda, struct cdrom_subchnl *subchnl)
{
	audio_info_t		audio_info;
	int			curr_lba;

	/* Just return if not playing */
	if (cdda->state == CDDA_STOPPED) {
		subchnl->cdsc_audiostatus = CDROM_AUDIO_NO_STATUS;
		return (0);
	}

	/* Retrieve current setting */
	audio_get_info(cdda, &audio_info);

	/* Get current lba */
	curr_lba = cdda->start_lba + audio_info.play.eof * 75;

	if (cdda->state == CDDA_PAUSED) {
		subchnl->cdsc_audiostatus = CDROM_AUDIO_PAUSED;
	} else {
		/*
		 * The state must either be CDDA_PLAYING or CDDA_COMPLETED
		 * in which case we are still playing.
		 */
		subchnl->cdsc_audiostatus = CDROM_AUDIO_PLAY;
	}

	/* Return lba and let caller figure out track no */
	return (curr_lba);

} /* audio_get_state() */


/*
 * audio_start()
 *
 * Description:
 *	Opens audio device and launches reader/writer threads.
 *
 * Arguments:
 *	cdda_t		*cdda		Ptr to cdda state structure
 *	int		start_lba	Start play address
 *	int		end_lba		End play address
 *
 * Returns:
 *	int: -1 for failure and 1 for sucess 
 */
int
audio_start(cdda_t *cdda, int start_lba, int end_lba)
{
	int errno;
	/* If playing do nothing */
	if (cdda->state != CDDA_STOPPED) {
		return 1;
	}

	/* Initialise */
	cdda->start_lba = start_lba;
	cdda->end_lba = end_lba;

	/* Buffer variables */
	cdda->cdb.occupied = 0;
	cdda->cdb.nextin = 0;
	cdda->cdb.nextout = 0;

	/* Open the audio device */
	errno = cdda_audio_open(cdda);
	if (cdda->audio < 0 && errno == EBUSY)
		return -1;

	/* Configure for CD audio */
	audio_config(cdda);

	/* Fill buffers */
	cdda_read_init(cdda);

	/* Set status */
	cdda->state = CDDA_PLAYING;

	/* Start reading */
	if (pthread_create(&cdda->cdb.reader, NULL, cdda_read,
				(void *)cdda) < 0) {
		perror("pthread_create()");
		exit(EXIT_FAILURE);
	}

	/* Start writing */
	if (pthread_create(&cdda->cdb.writer, NULL, audio_write,
				(void *)cdda) < 0) {
		perror("pthread_create()");
		exit(EXIT_FAILURE);
	}

	return 1;
} /* audio_start() */


/*
 * cdda_cleanup()
 *
 * Description:
 *	Frees memory and destroys mutexes and condition variables.
 *
 * Arguments:
 *	cdda_t		*cdda		Ptr to cdda state structure
 *
 * Returns:
 *	void
 */
void
cdda_cleanup(cdda_t *cdda)
{
	/* Free memory and destroy mutexes, condition variables */
	free(cdda->audio_device);
	pthread_mutex_destroy(&cdda->cdb.mutex);
	pthread_cond_destroy(&cdda->cdb.more);
	pthread_cond_destroy(&cdda->cdb.less);
	free(cdda->cdb.b);
	free(cdda->cdb.data);
	free(cdda->cdb.olap);
	free(cdda);
	cdda = NULL;

} /* cdda_cleanup() */


/*
 * cdda_check()
 *
 * Description:
 *	Checks whether analog playback is supported. If analog playback
 *	is not supported then sets up for cdda playback.
 *
 * Arguments:
 *	void		*in_cdda		Ptr to uninitialised structure
 *	int		cdrom			CDROM file descriptor
 *
 * Returns:
 *	void
 */
void
cdda_check(void **in_cdda, int cdrom)
{
	cdda_t	*cdda;

	/* Allocate cdda structure */
	cdda = malloc(sizeof (*cdda));

	if (cdda == NULL) {
		perror("malloc()");
		exit(EXIT_FAILURE);
	}

	/* Retrieve AUDIODEV, if set */
	if (getenv("AUDIODEV") != NULL) {
		cdda->audio_device = strdup(getenv("AUDIODEV"));
	} else {
		cdda->audio_device = strdup("/dev/audio");
	}
	if (cdda->audio_device == NULL) {
		perror("strdup()");
		exit(EXIT_FAILURE);
	}

	/* Initialise fds */
	cdda->audio = -1;
	cdda->cdrom = cdrom;

	/* Open the device */
	cdda_audio_open(cdda);

	/* Check for analog support */
	if (audio_supports_analog(cdda)) {
		/* Clean up */
		cdda_audio_close(cdda);
		free(cdda->audio_device);
		free(cdda);
		*in_cdda = NULL;
		return;
	}

	/*
	 * Analog playback is not supported so we can go ahead
	 * and initialise the cdda structure.
	 */
	cdda_audio_close(cdda);
	cdda_init(cdda);
	*in_cdda = cdda;

} /* cdda_check() */


/*
 * cdda_init()
 *
 * Description:
 *	Sets up mutexes and condition variables and allocates memory
 *	required for cdda.
 *
 * Arguments:
 *	cdda_t		*cdda		Ptr to cdda state structure
 *
 * Returns:
 *	void
 */
static void
cdda_init(cdda_t *cdda)
{
	/* Initialise mutex and condition variables */
	if (pthread_mutex_init(&cdda->cdb.mutex, NULL) < 0) {
		perror("pthread_mutex_init()");
		exit(EXIT_FAILURE);
	}
	if (pthread_cond_init(&cdda->cdb.more, NULL) < 0) {
		perror("pthread_cond_init()");
		exit(EXIT_FAILURE);
	}
	if (pthread_cond_init(&cdda->cdb.less, NULL) < 0) {
		perror("pthread_cond_init()");
		exit(EXIT_FAILURE);
	}

	/* Chunk blocks, bytes */
	cdda->cds.chunk_blocks = CDDA_CHUNK_BLKS;
	cdda->cds.chunk_bytes = cdda->cds.chunk_blocks * CDDA_BLKSZ;

	/* Buffer chunks, blocks, bytes */
	cdda->cds.buffer_chunks = CDDA_BUFFER_SECS;
	cdda->cds.buffer_blocks = cdda->cds.buffer_chunks *
		cdda->cds.chunk_blocks;
	cdda->cds.buffer_bytes = cdda->cds.buffer_blocks *
		cdda->cds.chunk_bytes;

	/* Overlap block, bytes */
	cdda->cds.olap_blocks = CDDA_OLAP_BLKS;
	cdda->cds.olap_bytes = cdda->cds.olap_blocks * CDDA_BLKSZ;

	/* Search blocks, bytes */
	cdda->cds.search_blocks = CDDA_SRCH_BLKS;
	cdda->cds.search_bytes = cdda->cds.search_blocks * CDDA_BLKSZ;

	/* Jitter correction match length */
	cdda->cds.str_length = CDDA_STR_LEN;

	/* Allocate space for data */
	cdda->cdb.b = malloc(cdda->cds.buffer_bytes);

	if (cdda->cdb.b == NULL) {
		perror("malloc()");
		exit(EXIT_FAILURE);
	}

	/* Allocate space for read */
	cdda->cdb.data = malloc(cdda->cds.chunk_bytes +
		(cdda->cds.olap_bytes << 1));

	if (cdda->cdb.data == NULL) {
		perror("malloc()");
		exit(EXIT_FAILURE);
	}

	/* Allocate space for overlap */
	cdda->cdb.olap = malloc(cdda->cds.search_bytes << 1);

	if (cdda->cdb.olap == NULL) {
		perror("malloc()");
		exit(EXIT_FAILURE);
	}

	/* Set state */
	cdda->state = CDDA_STOPPED;

} /* cdda_init() */
