/*
 * Code for reading CDDA data from the CD. The cdda_read thread reads
 * data from the CD, bytes swaps if necessary, corrects for jitter and
 * places the audio data in a mutex protected buffer.
 */

#include "cdda-solaris.h"
#include "reader-solaris.h"

/* Private function prototypes */
static void cdda_byte_swap(cdda_t *);
static char *cdda_correct_jitter(cdda_t *);

/*
 * cdda_byte_swap()
 *
 * Description:
 *	Carry out byte swapping. Data coming off the cd is little endian
 *	and so byte swapping is necessary on Sparc.
 *
 * Arguments:
 *	cdda_t			*cdda		Ptr to cdda state structure
 *
 * Returns:
 *	void
 */
static void
cdda_byte_swap(cdda_t *cdda)
{
	unsigned int	i;
	char		temp;

	/* Run through samples */
	for (i = cdda->cds.olap_bytes; i < cdda->cds.chunk_bytes +
			(cdda->cds.olap_bytes << 1); i += 2) {
		/* Byte swapping */
		temp = cdda->cdb.data[i + 1];
		cdda->cdb.data[i + 1] = cdda->cdb.data[i];
		cdda->cdb.data[i] = temp;
	}

} /* cdda_byte_swap() */


/*
 * cdda_correct_jitter()
 *
 * Description:
 *	Correct for jitter. The olap array contains 2 * cdda->cds.search_bytes
 *	of data and the last data written to the device is situated at
 *	olap[cdda->cds.search_bytes]. We try to match the last data read,
 *	which starts at data[cdda->cds.olap_bytes], around the center of olap.
 *	Once matched we may have to copy some data from olap into data
 *	(overshoot) or skip some of the audio in data (undershoot). The
 *	function returns a pointer into data indicating from where we should
 *	start writing.
 *
 * Arguments:
 *	cdda_t		*cdda			Ptr to cdda state structure
 *
 * Returns:
 *	char		*j			Pointer into data
 */
static char *
cdda_correct_jitter(cdda_t *cdda)
{
	int		index, d;
	char		*i, *j, *l;

	/* Find a match in olap, if we can */
	i = cdda->cdb.olap;
	l = &cdda->cdb.olap[(cdda->cds.search_bytes << 1) - 1];
	j = &cdda->cdb.data[cdda->cds.olap_bytes];

	do {
		i = memchr(i, (int)*j, l - i);
		if (i != NULL) {
			/* Make sure we're at a sample boundary */
			if ((i - cdda->cdb.olap) % 2 == 0) {
				d = memcmp(j, i, cdda->cds.str_length);
				if (d == 0)
					break;
			}
			i++;
		}
	} while ((i != NULL) && (i < l));

	/* Did we find a match? */
	if ((i == NULL) || (i >= l)) {
		(void) fprintf(stderr, "Could not correct for jitter\n");
		index = 0;
	} else {
		index = i - cdda->cdb.olap - cdda->cds.search_bytes;
	}

	/* Update pointer */
	j -= index;

	/* Match on RHS, copy some olap into data */
	if (index > 0) {
		(void) memcpy(j, cdda->cdb.olap + cdda->cds.search_bytes,
			index);
	}

	return (j);

} /* cdda_correct_jitter() */


/*
 * cdda_read_init()
 *
 * Description:
 *	Before beginning to play a track we fill the data buffer.
 *
 * Arguments:
 *	cdda_t		*cdda		Ptr to cdda state structure
 *
 * Returns:
 *	void
 */
void
cdda_read_init(cdda_t *cdda)
{
	struct cdrom_cdda	cd_cdda;
	unsigned int		i;
	char			*start, *end, *p;

	/* Initialise */
	start = &cdda->cdb.data[cdda->cds.olap_bytes];

	/* Set up for read */
	cd_cdda.cdda_addr = cdda->start_lba;
	cd_cdda.cdda_length = cdda->cds.chunk_blocks + cdda->cds.olap_blocks;
	cd_cdda.cdda_subcode = CDROM_DA_NO_SUBCODE;
	cd_cdda.cdda_data = (caddr_t)start;

	/* Fill up our buffer */
	i = 0;

	while ((i < cdda->cds.buffer_chunks) &&
			(cdda->state != CDDA_COMPLETED)) {

		/*
		 * If we are passing the end of the track we have to take
		 * special action. We read in up to end_lba and zero out
		 * the buffer. This is equivalent to rounding to the nearest
		 * cdda->cds.chunk_bytes. We also set the COMPLETED flag.
		 */
		if (cd_cdda.cdda_addr + cdda->cds.chunk_blocks >=
				cdda->end_lba) {
			/* Read up to end */
			if (cdda->end_lba - cd_cdda.cdda_addr > 0) {
				cd_cdda.cdda_length = cdda->end_lba -
					cd_cdda.cdda_addr;
			} else {
				cd_cdda.cdda_length = 0;
			}
			(void) memset(cdda->cdb.data, 0, cdda->cds.chunk_bytes +
				(cdda->cds.olap_bytes << 1));
			cdda->state = CDDA_COMPLETED;
		}

		/* Read audio from cd */
		if (ioctl(cdda->cdrom, CDROMCDDA, &cd_cdda) < 0) {
			/* Print error, don't exit as not critical */
			perror("CDROMCDDA");
		}

		/* Do byte swapping */
#if defined(sparc) || defined(__sparc)
		cdda_byte_swap(cdda);
#endif

		/* Do jitter correction */
		if (i == 0) {
			p = &cdda->cdb.data[cdda->cds.olap_bytes];
		} else {
			p = cdda_correct_jitter(cdda);
		}

		/* Data end */
		end = p + cdda->cds.chunk_bytes;

		/* Reinitialise olap */
		(void) memcpy(cdda->cdb.olap, end - cdda->cds.search_bytes,
			cdda->cds.search_bytes << 1);

		/* Copy in */
		(void) memcpy(&cdda->cdb.b[cdda->cds.chunk_bytes *
			cdda->cdb.nextin], p, cdda->cds.chunk_bytes);

		/* Update pointers */
		cdda->cdb.nextin++;
		cdda->cdb.nextin %= cdda->cds.buffer_chunks;
		cdda->cdb.occupied++;
		cd_cdda.cdda_addr += ((end - start) / CDDA_BLKSZ);

		/* Check if we need to round up */
		if (((end - start) % CDDA_BLKSZ) >= (CDDA_BLKSZ >> 1))
			cd_cdda.cdda_addr++;

		i++;
	}

} /* cdda_read_init() */


/*
 * cdda_read()
 *
 * Description:
 *	Thread respsonsible for reading from the cd, byte swapping,
 *	correcting for jitter and writing to the buffer.
 *
 * Arguments:
 *	void			*in_cdda	Ptr to cdda state structure
 *
 * Returns:
 *	void
 */
void *
cdda_read(void *in_cd)
{
	cdda_t			*cdda;
	struct cdrom_cdda	cd_cdda;
	char			*start, *end, *p;

	/* Cast */
	cdda = (cdda_t *)in_cd;

	/* Initialise */
	start = &cdda->cdb.data[cdda->cds.olap_bytes];

	/* Set up for read */
	cd_cdda.cdda_addr = cdda->start_lba + cdda->cds.buffer_blocks;
	cd_cdda.cdda_length = cdda->cds.chunk_blocks + cdda->cds.olap_blocks;
	cd_cdda.cdda_data = (caddr_t)start;
	cd_cdda.cdda_subcode = CDROM_DA_NO_SUBCODE;

	/* While not stopped or completed */
	while ((cdda->state != CDDA_STOPPED) &&
			(cdda->state != CDDA_COMPLETED)) {

		/*
		 * If we are passing the end of the track we have to take
		 * special action. We read in up to end_lba and zero out
		 * the buffer. This is equivalent to rounding to the nearest
		 * cdda->cds.chunk_bytes. We also set the COMPLETED flag.
		 */
		if (cd_cdda.cdda_addr + cdda->cds.chunk_blocks >=
				cdda->end_lba) {
			/* Read up to end */
			if (cdda->end_lba - cd_cdda.cdda_addr > 0) {
				cd_cdda.cdda_length = cdda->end_lba -
					cd_cdda.cdda_addr;
			} else {
				cd_cdda.cdda_length = 0;
			}
			(void) memset(cdda->cdb.data, 0, cdda->cds.chunk_bytes +
				(cdda->cds.olap_bytes << 1));
			cdda->state = CDDA_COMPLETED;
		}

		/* Read audio from cd using ioctl method */
		if (ioctl(cdda->cdrom, CDROMCDDA, &cd_cdda) < 0) {
			/* Print error, don't exit as not critical */
			perror("CDROMCDDA");
		}

		/* Do byte swapping */
#if defined(sparc) || defined(__sparc)
		cdda_byte_swap(cdda);
#endif

		/* Do jitter correction */
		p = cdda_correct_jitter(cdda);

		/* Data end */
		end = p + cdda->cds.chunk_bytes;

		/* Reinitialise cd_olap */
		(void) memcpy(cdda->cdb.olap, end - cdda->cds.search_bytes,
			cdda->cds.search_bytes << 1);

		/* Get lock */
		(void) pthread_mutex_lock(&cdda->cdb.mutex);

		/* Wait until there is room */
		while ((cdda->cdb.occupied >= cdda->cds.buffer_chunks) &&
			(cdda->state != CDDA_STOPPED)) {
				(void) pthread_cond_wait(&cdda->cdb.less,
					&cdda->cdb.mutex);
		}

		/* Break if stopped */
		if (cdda->state == CDDA_STOPPED) {
			(void) pthread_mutex_unlock(&cdda->cdb.mutex);
			break;
		}

		/* Copy in */
		(void) memcpy(&cdda->cdb.b[cdda->cds.chunk_bytes *
			cdda->cdb.nextin], p, cdda->cds.chunk_bytes);

		/* Update pointers */
		cdda->cdb.nextin++;
		cdda->cdb.nextin %= cdda->cds.buffer_chunks;
		cdda->cdb.occupied++;
		cd_cdda.cdda_addr += ((end - start) / CDDA_BLKSZ);

		/* Check if we need to round up */
		if (((end - start) % CDDA_BLKSZ) >= (CDDA_BLKSZ >> 1))
			cd_cdda.cdda_addr++;

		/* Release lock and signal data available */
		(void) pthread_cond_signal(&cdda->cdb.more);
		(void) pthread_mutex_unlock(&cdda->cdb.mutex);
	}

	/* Wake up writer */
	(void) pthread_cond_signal(&cdda->cdb.more);

	return (NULL);

} /* cdda_read() */
