/*
 * cdda-solaris.h
 */
#ifndef	__CDDA_SOLARIS_H__
#define	__CDDA_SOLARIS_H__

#include <sys/audio.h>
#include <sys/conf.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/cdio.h>
#include <stdio.h>
#include <stropts.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

/* Basic CDDA read unit */
#define	CDDA_BLKSZ		(2352)

/*
 * We define the CDDA read size to be 75 blocks (equating to 1s
 * of audio data).
 */
#define	CDDA_CHUNK_BLKS		(75)

/*
 * CDDA data is queued up in memory. Here we define (in seconds)
 * the amount of queued data.
 */
#define	CDDA_BUFFER_SECS	(2)

/*
 * During jitter correction we search through CDDA_SRCH_BLOCKS
 * for a match.
 */
#define	CDDA_SRCH_BLKS		(2)

/*
 * In order to perform jitter correction we need to read more
 * CDDA data than we "need". Below we define the amount of extra
 * data required. So each CDDA read will comprise CDDA_CHUNK_BLKS
 * + CDDA_OLAP_BLKS.
 */
#define	CDDA_OLAP_BLKS		(CDDA_SRCH_BLKS * 2)

/*
 * We have found a match when CDDA_STR_LEN bytes are identical in
 * the data and the overlap.
 */
#define	CDDA_STR_LEN		(20)

/* Buffer for audio data transfer */
struct cdda_buffer {
	pthread_mutex_t		mutex;		/* Mutex */
	pthread_t		reader;		/* Thread reading from cd */
	pthread_t		writer;		/* Thread writing to audio */
	pthread_cond_t		more;		/* Wait for data */
	pthread_cond_t		less;		/* Wait for room */
	unsigned int		occupied;	/* Number of filled slots */
	unsigned int		nextin;		/* Next free slot */
	unsigned int		nextout;	/* Next out slot */
	char			*olap;		/* Pointer to overlap */
	char			*data;		/* Pointer to audio data */
	char			*b;		/* The audio data */
};

typedef struct cdda_buffer cdda_buffer_t;

/* Various sizes */
struct cdda_size {
	/* Chunk */
	unsigned int	chunk_blocks;
	unsigned int	chunk_bytes;

	/* Buffer */
	unsigned int	buffer_chunks;
	unsigned int	buffer_blocks;
	unsigned int	buffer_bytes;

	/* Overlap */
	unsigned int	olap_blocks;
	unsigned int	olap_bytes;

	/* Searching */
	unsigned int	search_blocks;
	unsigned int	search_bytes;
	unsigned int	str_length;
};

typedef struct cdda_size cdda_size_t;

/* Overall structure for cdda add on */
struct cdda {
	cdda_buffer_t	cdb;			/* Mutex protected data */
	cdda_size_t	cds;			/* Various sizes stored here */
	unsigned int	start_lba;		/* Start play position */
	unsigned int	end_lba;		/* End play position */
	int		cdrom;			/* Copy of CDROM fd */
	int		audio;			/* Audio device fd */
	char		supports_cdda;		/* Is CDDA supported? */
	char		state;			/* Playing, paused, stopped */
	char		*audio_device;		/* Name of audio device */
};

typedef struct cdda cdda_t;

/* State */
enum {
	CDDA_PLAYING,
	CDDA_STOPPED,
	CDDA_PAUSED,
	CDDA_COMPLETED
};

#endif /* __CDDA_SOLARIS_H__ */
