/*
	r3rfc.c
	Copyright (c) 1999 Johan Schon. All rights reserved.

	revision history:
	0.01	1999-03-22	schoen
		created first version

	0.20	1999-10-28	schoen
		last changes before first upload to CPAN

	0.30	1999-11-06	schoen
		changed format string in r3_getfloat
*/

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <saprfc.h>
#include <sapitab.h>
#include <ctype.h>
#include "r3rfc.h"

static char buf[65535L*2L+1L];

void r3_stbl(char *s)
{
	int i;
	for (i=strlen(s)-1; i>=0; i--)
	{
		if (s[i]==' ')
			s[i]=0;
		else
			return;
	}
}

void r3_ftbl(char *s, int n)
{
	int i;
	for (i=strlen(s); i<n; i++)
		s[i]=' ';
}


void r3_setchar(char * var, size_t n, char * str)
{
	strncpy(var, str, n); 
	if (n>strlen(str)) 
		memset(var+strlen(str),' ',n-strlen(str));
}

char * r3_getchar(char * var, size_t n)
{
	strncpy(buf, var, n);
	buf[n]=0;
	r3_stbl(buf);
	return buf;
}

void r3_setdate(char * var, char * str)
{
	strncpy(var,str,sizeof(RFC_DATE));
	memset(var+strlen(str),'0',sizeof(RFC_DATE)-strlen(str));
}

char * r3_getdate(char * var)
{
	strncpy(buf, var, sizeof(RFC_DATE));
	buf[sizeof(RFC_DATE)]=0;
	return buf;
}

void r3_setfloat(double * var, char * str)
{
	*var=atof(str);
}

char * r3_getfloat(double * var)
{
	sprintf(buf, "%.23e", *var);
	return buf;
}

void r3_setint(long * var, char * str)
{
	*var=atol(str);
}

char * r3_getint(long * var)
{
	sprintf(buf, "%ld", *var);
	return buf;
}


void r3_setnum(char * var, size_t n, char * str)
{
	strncpy(var+n-strlen(str), str, strlen(str)); 
	if (n>strlen(str)) 
		memset(var,'0',n-strlen(str));
}

char * r3_getnum(char * var, size_t n)
{
	strncpy(buf, var, n);
	buf[n]=0;
	r3_stbl(buf);
	return buf;
}


void r3_settime(char * var, char * str)
{
	strncpy(var,str,sizeof(RFC_TIME));
	memset(var+strlen(str),'0',sizeof(RFC_TIME)-strlen(str));
}

char * r3_gettime(char * var)
{
	strncpy(buf, var, sizeof(RFC_TIME));
	buf[sizeof(RFC_TIME)]=0;
	return buf;
}

static int h16(int x)
{
	if (x>='0' && x<='9')
		return x-'0';
	if (x>='A' && x<='F')
		return 10+x-'A';
	if (x>='a' && x<='f')
		return 10+x-'a';
	return 0;
}

void r3_setbyte(unsigned char * var, size_t n, char * str)
{
	size_t i;
	strcpy(buf, str);
	memset(buf+strlen(str), '0', strlen(str)+n*2);		
	for (i=0; i<n; i++)
	{
		var[i]=h16(buf[i*2])*16+h16(buf[i*2+1]);
	} 
}

char * r3_getbyte(unsigned char * var, size_t n)
{
	size_t i;
	for (i=0; i<n; i++) 
		sprintf(buf+i*2, "%02X", var[i]);
	return buf;
}

void r3_setbcd(unsigned char * bcd, size_t n, int decimals, char * str)
{
	char s[256];
	char *p; 
	size_t c; 
	if (str[0]=='-') 
	{
		s[n*2-1]='D'; 
		str++;
	}
	else 
	{
		s[n*2-1]='C'; 
		if (str[0]=='+') 
			str++;
	}
	if ((p=strchr(str,'.'))) 
		*(p++)=0; 
	else 
		p=str+strlen(str);
	c=n*2-1-decimals;
	if (strlen(str)>c) 
		memcpy(s,str+strlen(str)-c,c);
	else 
	{
		memset(s,'0',c-strlen(str)); 
		memcpy(s+c-strlen(str),str,strlen(str));
	}
  	while((*p) && (c < n*2-1)) 
		s[c++]=*(p++);
  	while(c < n*2-1) 
		s[c++]='0';
	s[n*2]=0;
	r3_setbyte(bcd, n, s);
}

char * r3_getbcd(unsigned char * bcd, size_t n, int decimals)
{
	char s[256]; 
	size_t c;
	char * str;
	str=buf;
	strcpy(s, r3_getbyte(bcd, n));
	if (s[n*2-1]=='D') 
		*(str++)='-'; 
	else 
		*(str++)='+';
  	for(c=0; c<n*2-decimals-1; c++) 
		*(str++)=s[c];
	if (decimals>0) 
	{ 
		*(str++)='.'; 
		while (c<n*2-1) 
			*(str++)=s[c++];
	}
	*str=0;
	return buf;
}

