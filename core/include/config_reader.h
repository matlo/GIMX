/*
 Copyright (c) 2011 Mathieu Laurendeau <mat.lau@laposte.net>
 License: GPLv3
 */

#ifndef CONFIG_READER_H_
#define CONFIG_READER_H_

#include <libxml/xmlreader.h>

int read_config_file(const char*);

int GetIntProp(xmlNode*, char*, int*);
int GetUnsignedIntProp(xmlNode*, char*, unsigned int*);
int GetDoubleProp(xmlNode*, char*, double*);

const char* _UTF8_to_8BIT(const char*);

#endif /* CONFIG_READER_H_ */
