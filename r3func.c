/*
	r3func.c
	Capyright (c) 1999 Johan Schoen. All rights reserved.

	revision history:

	0.01	1999-03-22	schoen
		first version created

	0.20	1999-10-28	schoen
		last changes before first upload to CPAN
 
*/

#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <saprfc.h>
#include <sapitab.h>
#include <ctype.h>
#include "r3rfc.h"

typedef RFC_CHAR SYST_LANGU[1];

typedef struct {
	RFC_CHAR Paramclass[1];
	RFC_CHAR Parameter[30];
	RFC_CHAR Tabname[30];
	RFC_CHAR Fieldname[30];
	RFC_CHAR Exid[1];
	RFC_INT Position;
	RFC_INT Offset;
	RFC_INT Intlength;
	RFC_INT Decimals;
	RFC_CHAR Default[21];
	RFC_CHAR Paramtext[79]; 
} RFC_FUNINT;

static RFC_TYPEHANDLE handleOfRFC_FUNINT;

static RFC_TYPE_ELEMENT typeOfRFC_FUNINT[] = {
  {"PARAMCLASS", TYPC, 1, 0},
  {"PARAMETER", TYPC, 30, 0},
  {"TABNAME", TYPC, 30, 0},
  {"FIELDNAME", TYPC, 30, 0},
  {"EXID", TYPC, 1, 0},
  {"POSITION", TYPINT, sizeof(RFC_INT), 0},
  {"OFFSET", TYPINT, sizeof(RFC_INT), 0},
  {"INTLENGTH", TYPINT, sizeof(RFC_INT), 0},
  {"DECIMALS", TYPINT, sizeof(RFC_INT), 0},
  {"DEFAULT", TYPC, 21, 0},
  {"PARAMTEXT", TYPC, 79, 0}
};
#define ENTRIES(tab) (sizeof(tab)/sizeof((tab)[0]))

H_R3RFC_FUNC r3_new_func(H_R3RFC_CONN h_conn,
			char * function_name)
{
	static H_R3RFC_FUNC h;
	static RFC_FUNCTIONNAME eFuncname;
	static SYST_LANGU eLanguage;
	RFC_PARAMETER Exporting[3];
	RFC_PARAMETER Importing[1];
	RFC_TABLE Tables[2];
	RFC_RC RfcRc;
	char *RfcException = NULL;
	ITAB_H thParams;
	int i;
	RFC_FUNINT * tParams;

	/* install structures */
	if (handleOfRFC_FUNINT==0) 
	{
		RfcRc = RfcInstallStructure(
				"RFC_FUNINT",
				typeOfRFC_FUNINT,
                                ENTRIES(typeOfRFC_FUNINT),
                                &handleOfRFC_FUNINT);
		if (RfcRc != RFC_OK) 
		{
			if (RfcRc == RFC_MEMORY_INSUFFICIENT)
				r3_set_rfcapi_exception("RFC_MEMORY_INSUFFICIENT");
			else
				r3_set_rfcapi_exception("UNKNOWN_ERROR");
			return NULL;	
		}
	}
	/* define export params */
	memset(eFuncname,' ',sizeof(eFuncname));
	strncpy(eFuncname, function_name, strlen(function_name));
	memset(eLanguage,' ',sizeof(eLanguage));
	eLanguage[0]='E'; /* this shouldn't be hardcoded */

	Exporting[0].name = "FUNCNAME";
	Exporting[0].nlen = 8;
	Exporting[0].type = TYPC;
	Exporting[0].leng = sizeof(RFC_FUNCTIONNAME);
	Exporting[0].addr = eFuncname;

	Exporting[1].name = "LANGUAGE";
	Exporting[1].nlen = 8;
	Exporting[1].type = TYPC;
	Exporting[1].leng = sizeof(SYST_LANGU);
	Exporting[1].addr = eLanguage;

	Exporting[2].name = NULL;

	/* no import parameters */
	Importing[0].name = NULL;

	/* define internal tables */
	thParams = ItCreate("PARAMS",sizeof(RFC_FUNINT),0,0);
	if (thParams==ITAB_NULL) 
	{
		r3_set_rfcapi_exception("RFC_MEMORY_INSUFFICIENT");
		return NULL;
	}

	Tables[0].name     = "PARAMS";
	Tables[0].nlen     = 6;
	Tables[0].type     = handleOfRFC_FUNINT;
	Tables[0].ithandle = thParams;

	Tables[1].name = NULL;

	/* call function module */
	RfcRc = RfcCallReceive(h_conn->h_rfc,
			"RFC_GET_FUNCTION_INTERFACE",
			Exporting,
			Importing,
			Tables,
			&RfcException);
	if (RfcRc != RFC_OK)
	{
		switch (RfcRc)
		{
		case RFC_FAILURE:
			r3_set_rfc_sys_exception("RFC_FAILURE");
			break;
		case RFC_EXCEPTION:
			r3_set_rfc_exception(RfcException);
			break;
		case RFC_SYS_EXCEPTION:
			r3_set_rfc_sys_exception(RfcException);
			break;
		case RFC_CALL:
			r3_set_rfc_exception("RFC_CALL");
			break;
		default:
			r3_set_rfcapi_exception("UNKNOWN_ERROR");
		}
		ItDelete(thParams);
		return NULL;
	}

	/* allocate memory for the function */
	if (!(h=malloc(sizeof(R3RFC_FUNC))))
	{
		r3_set_rfcapi_exception("MALLOC_FAILED");
		ItDelete(thParams);
		return NULL;
	}
	memset(h, 0, sizeof(R3RFC_FUNC));
	h->h_conn=h_conn;

        /* get interface */
	strcpy(h->name, function_name);
	h->n_interface = ItFill(thParams);	
	h->interface = malloc(h->n_interface*sizeof(R3RFC_FUNCINT));
	if (h->interface==NULL)
	{
		r3_set_rfcapi_exception("MALLOC_FAILED");
		r3_del_func(h);
		ItDelete(thParams);
		return NULL;
	}
	memset(h->interface, 0, h->n_interface*sizeof(R3RFC_FUNCINT));
        for (i=0; i<h->n_interface; i++) 
	{
		/* ABAP arrays are 1-based */
		tParams = ItGetLine(thParams, i+1); 
		if (tParams == NULL)
		{
			r3_set_rfcapi_exception("RFC_MEMORY_INSUFFICIENT");
			r3_del_func(h);
			ItDelete(thParams);
			return NULL;
		}

		strncpy(h->interface[i].paramclass,
			tParams->Paramclass,
			sizeof(tParams->Paramclass));
		r3_stbl(h->interface[i].paramclass);

		switch (h->interface[i].paramclass[0])
		{
			case 'I':
				h->n_exporting++;
				break;
			case 'E':
				h->n_importing++;
				break;
			case 'T':
				h->n_tables++;
				break;
		}

		strncpy(h->interface[i].parameter,
			tParams->Parameter,
			sizeof(tParams->Parameter));
		r3_stbl(h->interface[i].parameter);

		strncpy(h->interface[i].tabname,
			tParams->Tabname,
			sizeof(tParams->Tabname));
		r3_stbl(h->interface[i].tabname);

		strncpy(h->interface[i].fieldname,
			tParams->Fieldname,
			sizeof(tParams->Fieldname));
		r3_stbl(h->interface[i].fieldname);

		strncpy(h->interface[i].exid,
			tParams->Exid,
			sizeof(tParams->Exid));
		r3_stbl(h->interface[i].exid);
			
		h->interface[i].position=tParams->Position;
		h->interface[i].offset=tParams->Offset;
		h->interface[i].intlength=tParams->Intlength;
		h->interface[i].decimal=tParams->Decimals;

		strncpy(h->interface[i].zdefault,
			tParams->Default,
			sizeof(tParams->Default));
		r3_stbl(h->interface[i].zdefault);

		strncpy(h->interface[i].paramtext,
			tParams->Paramtext,
			sizeof(tParams->Paramtext));
		r3_stbl(h->interface[i].paramtext);

		h->interface[i].buffer = malloc(h->interface[i].intlength);
		if (!h->interface[i].buffer)
		{
			r3_set_rfcapi_exception("MALLOC_FAILED");
			r3_del_func(h);
			ItDelete(thParams);
			return NULL;
		}
	}

	/* delete table */
	ItDelete(thParams);

	h->exporting=malloc((1+h->n_exporting)*sizeof(RFC_PARAMETER));
	h->importing=malloc((1+h->n_importing)*sizeof(RFC_PARAMETER));
	h->tables=malloc((1+h->n_tables)*sizeof(RFC_TABLE));
	if (!h->exporting || !h->importing || !h->tables)
	{
		r3_set_rfcapi_exception("MALLOC_FAILED");
		r3_del_func(h);
		return NULL;
	}
	return h;
}

void r3_del_func(H_R3RFC_FUNC h)
{
	int i;
	if (!h)
		return;
	if (!h->interface)
	{
		for (i=0; i<h->n_interface; i++)
		{
			if (h->interface[i].buffer)
				free(h->interface[i].buffer);
		}
		free(h->interface);
	}
	if (!h->exporting)
		free(h->exporting);
	if (!h->importing)
		free(h->importing);
	if (!h->tables)
		free(h->tables);
	free(h);
}

int r3_call_func(H_R3RFC_FUNC h)
{
	RFC_RC RfcRc;
	char *RfcException = NULL;
	RfcRc = RfcCallReceive(h->h_conn->h_rfc,
			h->name,
			h->exporting,
			h->importing,
			h->tables,
			&RfcException);
	switch(RfcRc)
	{
		case RFC_OK:
			break;
		case RFC_FAILURE:
			r3_set_rfc_sys_exception("RFC_FAILURE");
			break;
		case RFC_EXCEPTION:
			r3_set_f_rfc_exception(h, RfcException);
			break;
		case RFC_SYS_EXCEPTION:
			r3_set_rfc_sys_exception(RfcException);
			break;
		case RFC_CALL:
			r3_set_rfc_exception("RFC_CALL");
			break;
		default:
			r3_set_rfcapi_exception("UNKNOWN_ERROR");
	}
	return (RfcRc != RFC_OK);
}

void r3_clear_params(H_R3RFC_FUNC h)
{
	int i, imp;
	memset(h->exporting, 0, (1+h->n_exporting)*sizeof(RFC_PARAMETER));
	memset(h->importing, 0, (1+h->n_importing)*sizeof(RFC_PARAMETER));
	memset(h->tables, 0, (1+h->n_tables)*sizeof(RFC_TABLE));
	imp=0;
	for (i=0; i<h->n_interface; i++)
	{
		if (h->interface[i].paramclass[0]=='E')
		{
			h->importing[imp].name= 
				h->interface[i].parameter;
			h->importing[imp].nlen=
				strlen(h->interface[i].parameter);
			h->importing[imp].type=
				r3_exid2type(h->interface[i].exid[0]);
			h->importing[imp].leng=h->interface[i].intlength;
			h->importing[imp].addr=h->interface[i].buffer;
			memset(h->importing[imp].addr, 0, 
				h->importing[imp].leng);
			imp++;
		}
	}
}

int r3_get_ino(H_R3RFC_FUNC h, char * pc, char * fn)
{
	int i;
	for (i=0; i<h->n_interface; i++)
	{
		if (!(strncmp(pc, h->interface[i].paramclass, 1)))
		{
			if (!(strcmp(fn, h->interface[i].parameter)))
			{
				return i;
			}
		}
	}
	r3_set_rfcapi_exception("NO_SUCH_PARAM");
	return -1;
}

int r3_set_export_value(H_R3RFC_FUNC h, char * export, char * value)
{
	int ino;
	ino=r3_get_ino(h, "I", export);
	if (ino<0)
	{
		r3_set_error_message(export);
		return 1;
	}
	return r3_set_exp_val(h, ino, value);
}

int r3_set_exp_val(H_R3RFC_FUNC h, int ino, char * value)
{
	int eno;
	if (ino<0 || ino>=h->n_interface)
	{
		r3_set_rfcapi_exception("NO_SUCH_PARAM");
		return 1;
	}
	for (eno=0; eno<h->n_exporting; eno++)
	{
		if (h->exporting[eno].name==NULL)
			break;
		if (!(strcmp(h->exporting[eno].name, 
			h->interface[ino].parameter)))
			break;
	}	
	h->exporting[eno].name=h->interface[ino].parameter; /* schoen */
	h->exporting[eno].nlen=strlen(h->interface[ino].parameter);
	h->exporting[eno].type=r3_exid2type(h->interface[ino].exid[0]);
	h->exporting[eno].leng=h->interface[ino].intlength;
	h->exporting[eno].addr=h->interface[ino].buffer;
	switch(h->exporting[eno].type)
	{
		case TYPC:
			r3_setchar(h->exporting[eno].addr,
				h->exporting[eno].leng,
				value);	
			break;
		case TYPX:
			r3_setbyte(h->exporting[eno].addr,
				h->exporting[eno].leng,
				value);	
			break;
		case TYPP:
			r3_setbcd(h->exporting[eno].addr,
				h->exporting[eno].leng,
				h->interface[ino].decimal,	
				value);	
			break;
		case TYPINT:
			r3_setint(h->exporting[eno].addr,
				value);	
			break;
		case TYPFLOAT:
			r3_setfloat(h->exporting[eno].addr,
				value);	
			break;
		case TYPDATE:
			r3_setdate(h->exporting[eno].addr,
				value);	
			break;
		case TYPTIME:
			r3_settime(h->exporting[eno].addr,
				value);	
			break;
		default:
			r3_set_rfcapi_exception("UNKNOWN_PARAM_TYPE");	
			return 1;
	}
	return 0;
}

char * r3_get_import_value(H_R3RFC_FUNC h, char * import)
{
	int ino;
	ino=r3_get_ino(h, "E", import);
	return r3_get_imp_val(h, ino);
}

char * r3_get_imp_val(H_R3RFC_FUNC h, int ino)
{
	int eno;
	if (ino<0 || ino>=h->n_interface)
		return NULL;
	for (eno=0; eno<h->n_importing; eno++)
	{
		if (h->importing[eno].name==NULL)
			break;
		if (!(strcmp(h->importing[eno].name, 
			h->interface[ino].parameter)))
			break;
	}	
	switch(h->importing[eno].type)
	{
		case TYPC:
			return r3_getchar(h->importing[eno].addr,
				h->importing[eno].leng);	
		case TYPX:
			return r3_getbyte(h->importing[eno].addr,
				h->importing[eno].leng);	
		case TYPP:
			return r3_getbcd(h->importing[eno].addr,
				h->importing[eno].leng,
				h->interface[ino].decimal);
		case TYPINT:
			return r3_getint(h->importing[eno].addr);	
		case TYPFLOAT:
			return r3_getfloat(h->importing[eno].addr);
		case TYPDATE:
			return r3_getdate(h->importing[eno].addr);
		case TYPTIME:
			return r3_gettime(h->importing[eno].addr);
		default:
			r3_set_rfcapi_exception("UNKNOWN_PARAM_TYPE");	
			return NULL;
	}
	r3_set_rfcapi_exception("ERROR");	
	return NULL; /* should never reach here */
}

int r3_set_table(H_R3RFC_FUNC h, char * table, H_R3RFC_ITAB h_table)
{
	int ino;
	int eno;
	ino=r3_get_ino(h, "T", table);
	if (ino<0)
	{
		r3_set_rfcapi_exception("NO_SUCH_TABLE");
		return 1;
	}
	for (eno=0; eno<h->n_tables; eno++)
	{
		if (h->tables[eno].name==NULL)
			break;
		if (!(strcmp(h->tables[eno].name, table)))
			break;
	}	
	h->tables[eno].name=h->interface[ino].parameter; /* schoen */
	h->tables[eno].nlen=strlen(h->interface[ino].parameter);
	h->tables[eno].type=h_table->h_type;
	h->tables[eno].leng=h_table->rec_size;
	h->tables[eno].ithandle=h_table->h_itab;
	return 0;
}

int r3_get_params(H_R3RFC_FUNC h)
{
	return h->n_interface;
}

char * r3_get_param_name(H_R3RFC_FUNC h, int ino)
{
	return h->interface[ino].parameter;
}

char * r3_get_param_class(H_R3RFC_FUNC h, int ino)
{
	return h->interface[ino].paramclass;
}

/* EOF r3func.c */


