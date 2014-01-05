/*
 * configfile.h
 *
 * Function to read pptpd config file.
 *
 * $Id: configfile.h 1603 2008-05-19 11:41:40Z david $
 */

#ifndef _PPTPD_CONFIGFILE_H
#define _PPTPD_CONFIGFILE_H

int read_config_file(char *filename, char *keyword, char *value);

#endif	/* !_PPTPD_CONFIGFILE_H */
