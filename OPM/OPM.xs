/* vim:set ts=8 noexpandtab: */
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "../src/opm.h"
#include "../src/opm_types.h"
#include "../src/opm_error.h"

static bool croak_on_error = 0;

typedef struct _OPM OPM_Scan;
typedef struct _OPM_REMOTE OPM_Remote;
typedef int OPM_Error;

/* This is used to store a define -> int mapping */
typedef struct __defines {
   char *text;
   int num;
} _defines;

/* The mappings (called as OPM::xx hence the lack of OPM_ names) */
_defines define[] = {
   {"CALLBACK_END",         OPM_CALLBACK_END},
   {"CALLBACK_ERROR",       OPM_CALLBACK_ERROR},
   {"CALLBACK_NEGFAIL",     OPM_CALLBACK_NEGFAIL},
   {"CALLBACK_OPENPROXY",   OPM_CALLBACK_OPENPROXY},
   {"CALLBACK_TIMEOUT",     OPM_CALLBACK_TIMEOUT},
   {"CONFIG_BIND_IP",       OPM_CONFIG_BIND_IP},
   {"CONFIG_DNSBL_HOST",    OPM_CONFIG_DNSBL_HOST},
   {"CONFIG_FD_LIMIT",      OPM_CONFIG_FD_LIMIT},
   {"CONFIG_MAX_READ",      OPM_CONFIG_MAX_READ},
   {"CONFIG_SCAN_IP",       OPM_CONFIG_SCAN_IP},
   {"CONFIG_SCAN_PORT",     OPM_CONFIG_SCAN_PORT},
   {"CONFIG_TARGET_STRING", OPM_CONFIG_TARGET_STRING},
   {"CONFIG_TIMEOUT",       OPM_CONFIG_TIMEOUT},
   {"ERR_BADADDR",          OPM_ERR_BADADDR},
   {"ERR_NOPROTOCOLS",      OPM_ERR_NOPROTOCOLS},
   {"ERR_BADKEY",           OPM_ERR_BADKEY},
   {"ERR_BADPROTOCOL",      OPM_ERR_BADPROTOCOL},
   {"ERR_BADVALUE",         OPM_ERR_BADVALUE},
   {"ERR_BIND",             OPM_ERR_BIND},
   {"ERR_CBNOTFOUND",       OPM_ERR_CBNOTFOUND},
   {"ERR_MAX_READ",         OPM_ERR_MAX_READ},
   {"ERR_NOFD",             OPM_ERR_NOFD},
   {"STATE_CLOSED",         OPM_STATE_CLOSED},
   {"STATE_ESTABLISHED",    OPM_STATE_ESTABLISHED},
   {"STATE_NEGSENT",        OPM_STATE_NEGSENT},
   {"STATE_UNESTABLISHED",  OPM_STATE_UNESTABLISHED},
   {"SUCCESS",              OPM_SUCCESS},
   {"TYPE_HTTP",            OPM_TYPE_HTTP},
   {"TYPE_ROUTER",          OPM_TYPE_ROUTER},
   {"TYPE_SOCKS4",          OPM_TYPE_SOCKS4},
   {"TYPE_SOCKS5",          OPM_TYPE_SOCKS5},
   {"TYPE_WINGATE",         OPM_TYPE_WINGATE},
   {NULL, 0},
};

static int
constant(char *name)
{
   int i = -1;

   errno = 0;
   while(define[++i].text != NULL)
      if(strEQ(name, define[i].text))
	 return define[i].num;

   errno = EINVAL;
   return 0;
}

static char
*string_error(OPM_Error error)
{
    switch(error)
    {
       case OPM_SUCCESS:
          return "Success";
       case OPM_ERR_BADKEY:
          return "Unknown or bad key value";
       case OPM_ERR_BADVALUE:
          return "Bad value matching key";
       case OPM_ERR_BADPROTOCOL:
          return "Unknown protocol in config";
       case OPM_ERR_MAX_READ:
          return "Socket reached MAX_READ";
       case OPM_ERR_CBNOTFOUND:
          return "Callback is out of range";
       case OPM_ERR_BADADDR:
          return "IP in remote struct is bad";
       case OPM_ERR_BIND:
          return "Error binding to BIND_IP";
       case OPM_ERR_NOFD:
          return "Unable to allocate file descriptor";
       case OPM_ERR_NOPROTOCOLS:
          return "No protocols are configured";
   }
   return "Unknown error";
}

static void error_check(OPM_Error error)
{
   if(croak_on_error)
      croak("%s", string_error(error));

   return;
}

static void perl_callback(OPM_Scan *scanner, OPM_Remote *remote, int val,
     void *func)
{
   dSP;

   ENTER;
   SAVETMPS;

   PUSHMARK(SP);
   
   XPUSHs(sv_2mortal(sv_bless(newRV_inc(newSViv((int)scanner)),
      gv_stashpv("OPM::Scan", 1))));
   XPUSHs(sv_2mortal(sv_bless(newRV_inc(newSViv((int)remote)),
      gv_stashpv("OPM::Remote", 1))));
   XPUSHs(sv_2mortal(newSViv(val)));

   PUTBACK;

   perl_call_sv((SV *) func, G_DISCARD);

   FREETMPS;
   LEAVE;
}

MODULE = OPM		PACKAGE = OPM

int
constant(sv,arg)
    PREINIT:
	STRLEN		len;
    INPUT:
	SV *		sv
	char *		s = SvPV(sv, len);
    CODE:
	RETVAL = constant(s);
    OUTPUT:
	RETVAL

OPM_Scan *
_create()
    CODE:
	RETVAL = opm_create();
	//printf("Created %p\n", RETVAL);
    OUTPUT:
	RETVAL

OPM_Remote *
_remote_create(sv)
    PREINIT:
	STRLEN		len;
    INPUT:
	SV *		sv
	char *		s = SvPV(sv, len);
    CODE:
	RETVAL = opm_remote_create(s);
    OUTPUT:
	RETVAL

void
_fatal_errors()
    CODE:
	croak_on_error = 1;

MODULE = OPM		PACKAGE = OPM::Scan		PREFIX = opm_

void
opm_DESTROY(scanner)
	OPM_Scan *scanner
  CODE:
	opm_free(scanner);

OPM_Error
opm_addtype(scanner, type, port)
 	OPM_Scan *scanner
	int type
	int port
   CODE:
	RETVAL = opm_addtype(scanner, type, port);
	if(RETVAL != OPM_SUCCESS)
	   error_check(RETVAL);
   OUTPUT:
	RETVAL

OPM_Error
opm_addcustom(scanner, type, string, port)
	OPM_Scan *scanner   
	SV* type
	SV* string
	int port
   PREINIT:
	STRLEN		typelen;
	STRLEN		stringlen;
   INPUT:
	char *		stype = SvPV(type, typelen);
	char *		sstring = SvPV(string, stringlen);
   CODE:
   	RETVAL = opm_addcustom(scanner, stype, sstring, port);
	if(RETVAL != OPM_SUCCESS)
	   error_check(RETVAL);
   OUTPUT: 
   	RETVAL

OPM_Error
opm_scan(scanner, remote)
	OPM_Scan *scanner
	OPM_Remote *remote
   CODE:
	RETVAL = opm_scan(scanner, remote);
	if(RETVAL != OPM_SUCCESS)
	   error_check(RETVAL);
   OUTPUT:
	RETVAL

OPM_Error
opm_callback(scanner, type, func)
	OPM_Scan *scanner
	int type
	SV *func
   CODE:
	RETVAL = opm_callback(scanner, type, &perl_callback, (void *)newSVsv(func));
	if(RETVAL != OPM_SUCCESS)
	   error_check(RETVAL);
   OUTPUT:
	RETVAL

void
opm_cycle(scanner)
	OPM_Scan *scanner

OPM_Error
opm__config_int(scanner, type, num)
	OPM_Scan *scanner
	int type
	int num
   CODE:
	RETVAL = opm_config(scanner, type, &num);
	if(RETVAL != OPM_SUCCESS)
	   error_check(RETVAL);
   OUTPUT:
	RETVAL

OPM_Error
opm__config_string(scanner, type, sv)
	OPM_Scan *scanner
	int type
	SV *sv
    PREINIT:
	STRLEN		len;
    INPUT:
	char *		s = SvPV(sv, len);
    CODE: 
	RETVAL = opm_config(scanner, type, s);
	if(RETVAL != OPM_SUCCESS)
	   error_check(RETVAL);
    OUTPUT:
	RETVAL

MODULE = OPM		PACKAGE = OPM::Remote		PREFIX = opm_remote_

void
opm_remote_free(remote)
	OPM_Remote *remote

OPM_Error
opm_remote_callback(remote, type, func)
	OPM_Remote *remote
	int type
	SV *func
    CODE:
	RETVAL = opm_remote_callback(remote, type, &perl_callback, (void *)func);
	if(RETVAL != OPM_SUCCESS)
	   error_check(RETVAL);
    OUTPUT:
	RETVAL

SV *
opm_remote_ip(remote)
	OPM_Remote *remote
    CODE:
        RETVAL = newSVpv(remote->ip, 0);
    OUTPUT:
	RETVAL

SV *
opm_remote_port(remote)
	OPM_Remote *remote
    CODE:
        RETVAL = newSViv(remote->port);
    OUTPUT:
	RETVAL

SV *
opm_remote_protocol(remote)
	OPM_Remote *remote
    CODE:
        RETVAL = newSViv(remote->protocol);
    OUTPUT:
	RETVAL

SV *
opm_remote_bytes_read(remote)
	OPM_Remote *remote
    CODE:
        RETVAL = newSViv(remote->bytes_read);
    OUTPUT:
	RETVAL

MODULE = OPM		PACKAGE = OPM::Error		PREFIX = opm_error_

SV *
opm_error_string(error)
	OPM_Error error
    CODE:
        RETVAL = newSVpv(string_error(error), 0);
    OUTPUT:
	RETVAL

