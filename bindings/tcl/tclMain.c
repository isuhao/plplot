/* $Id$
 * $Log$
 * Revision 1.7  1996/06/26 21:35:11  furnish
 * Various hacks to support Tcl 7.5 and Tk 4.1.
 *
 * Revision 1.6  1995/11/30  20:03:04  furnish
 * Correct the order of argument processing so that PLplot command line
 * arguments are removed before the argc/argv list is set for Tcl.  This
 * greatly improves the support for building custom pltcl shells.
 *
 * Revision 1.5  1995/10/22  17:37:25  mjl
 * Added command line argument parsing ala plParseOpts.  Previously, no args
 * except the file name were parsed, they were merely shoved into the Tcl
 * variables argc, argv, and argv0.  Even -f was ignored (try it yourself:
 * "tclsh -f <file>" doesn't work! ..amazing).  Now acts similar to plserver,
 * in that -e and -f options are obeyed as well as all underlying plplot
 * options.
 *
 * Revision 1.4  1994/09/18  07:15:39  mjl
 * Changed the syntax for pltclMain() in order for it to work better with
 * shared libraries.  In particular, Tcl_AppInit is no longer external but
 * passed as a function pointer.
 *
 * Revision 1.3  1994/06/30  18:52:53  mjl
 * Minor change to eliminate a warning.
 *
 * Revision 1.2  1994/06/24  20:40:45  mjl
 * Created function to handle error condition.  Is handled by indirection
 * through a global pointer, so can be replaced.  This call has to bypass
 * the interpreter since it's important that the interp->result string is
 * not modified.
 *
 * Revision 1.1  1994/06/23  22:43:34  mjl
 * Handles nearly all the important setup for extended tclsh's.  Taken from
 * tclMain.c of Tcl 7.3, and modified minimally to support my needs.
*/

/*
 * Modified version of tclMain.c, from Tcl 7.3.
 * Maurice LeBrun
 * 23-Jun-1994
 *
 * Modifications include:
 * 1. main() changed to pltclMain().
 * 2. tcl_RcFileName -> RcFileName, now passed in through the argument list. 
 * 3. Tcl_AppInit -> AppInit, now passed in through the argument list.
 * 4. Changes to work with ANSI C
 * 5. Changes to support user-installable error handler.
 * 6. PLplot argument parsing routine called to handle arguments.
 *
 * The original notes follow.
 */

/* 
 * main.c --
 *
 *	Main program for Tcl shells and other Tcl-based applications.
 *
 * Copyright (c) 1988-1993 The Regents of the University of California.
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#define _POSIX_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <tcl.h>
#include <errno.h>

/*
 * Declarations for various library procedures and variables (don't want
 * to include tclPort.h here, because people might copy this file out of
 * the Tcl source directory to make their own modified versions).
 * Note:  "exit" should really be declared here, but there's no way to
 * declare it without causing conflicts with other definitions elsewher
 * on some systems, so it's better just to leave it out.
 */

extern int		errno;
extern int		isatty _ANSI_ARGS_((int fd));
extern char *		strcpy _ANSI_ARGS_((char *dst, CONST char *src));

static Tcl_Interp *interp;	/* Interpreter for application. */
static Tcl_DString command;	/* Used to buffer incomplete commands being
				 * read from stdin. */
#ifdef TCL_MEM_DEBUG
static char dumpFile[100];	/* Records where to dump memory allocation
				 * information. */
static int quitFlag = 0;	/* 1 means the "checkmem" command was
				 * invoked, so the application should quit
				 * and dump memory allocation information. */
#endif

/*
 * Forward references for procedures defined later in this file:
 */

static int		CheckmemCmd _ANSI_ARGS_((ClientData clientData,
			    Tcl_Interp *interp, int argc, char *argv[]));

static void
ErrorHandler _ANSI_ARGS_((Tcl_Interp *interp, int code, int tty));

/* This is globally visible and can be replaced */

void (*tclErrorHandler)
    _ANSI_ARGS_((Tcl_Interp *interp, int code, int tty)) = ErrorHandler;

/* Options data structure definition. */

#include "plplot.h"

static char *script = NULL;
static char *fileName = NULL;

static PLOptionTable options[] = {
{
    "f",			/* File to read & process */
    NULL,
    NULL,
    &fileName,
    PL_OPT_STRING,
    "-f",
    "File from which to read commands" },
{
    "file",			/* File to read & process (alias) */
    NULL,
    NULL,
    &fileName,
    PL_OPT_STRING | PL_OPT_INVISIBLE,
    "-file",
    "File from which to read commands" },
{
    "e",			/* Script to run on startup */
    NULL,
    NULL,
    &script,
    PL_OPT_STRING,
    "-e",
    "Script to execute on startup" },
{
    NULL,			/* option */
    NULL,			/* handler */
    NULL,			/* client data */
    NULL,			/* address of variable to set */
    0,				/* mode flag */
    NULL,			/* short syntax */
    NULL }			/* long syntax */
};


/*
 *----------------------------------------------------------------------
 *
 * pltclMain --
 *
 *	This is the main program for a Tcl-based shell that reads
 *	Tcl commands from standard input.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Can be almost arbitrary, depending on what the Tcl commands do.
 *
 *----------------------------------------------------------------------
 */

int
pltclMain(int argc, char **argv, char *RcFileName,
	  int (*appInitProc)(Tcl_Interp *interp))
{
    char buffer[1000], *cmd, *args;
    int code, gotPartial, tty;
    int exitCode = 0;

    interp = Tcl_CreateInterp();
#ifdef TCL_MEM_DEBUG
    Tcl_InitMemory(interp);
    Tcl_CreateCommand(interp, "checkmem", CheckmemCmd, (ClientData) 0,
	    (Tcl_CmdDeleteProc *) NULL);
#endif

    /* Now process the args using the PLplot parser. */

    plMergeOpts(options, "pltcl options", NULL);
    (void) plParseOpts(&argc, argv, PL_PARSE_FULL | PL_PARSE_SKIP );
    Tcl_SetVar(interp, "argv0", (fileName != NULL) ? fileName : argv[0],
	    TCL_GLOBAL_ONLY);

    /*
     * Make (remaining) command-line arguments available in the Tcl variables
     * "argc" and "argv".  
     */

    args = Tcl_Merge(argc-1, argv+1);
    Tcl_SetVar(interp, "argv", args, TCL_GLOBAL_ONLY);
    ckfree(args);
    sprintf(buffer, "%d", argc-1);
    Tcl_SetVar(interp, "argc", buffer, TCL_GLOBAL_ONLY);

    /*
     * Set the "tcl_interactive" variable.
     */

    tty = isatty(0);
    Tcl_SetVar(interp, "tcl_interactive",
	    ((fileName == NULL) && tty) ? "1" : "0", TCL_GLOBAL_ONLY);

    /*
     * Invoke application-specific initialization.
     */

    if ((*appInitProc)(interp) != TCL_OK) {
	fprintf(stderr, "application-specific initialization failed: %s\n",
		interp->result);
    }

    /*
     * Process the startup script, if any.
     */

    if (script != NULL) {
	code = Tcl_VarEval(interp, script, (char *) NULL);
	if (code != TCL_OK) {
	    fprintf(stderr, "%s\n", interp->result);
	    exitCode = 1;
	}
    }

    /*
     * If a script file was specified then just source that file
     * and quit.  This is different from how wish proceeds: wish
     * enters an event loop while tclsh is strictly procedural.
     */

    if (fileName != NULL) {
	code = Tcl_EvalFile(interp, fileName);
	if (code != TCL_OK) {
	    fprintf(stderr, "%s\n", interp->result);
	    exitCode = 1;
	}
	goto done;
    }

    /*
     * We're running interactively.  Source a user-specific startup
     * file if the application specified one and if the file exists.
     */

    if (RcFileName != NULL) {
	Tcl_DString buffer;
	char *fullName;
	FILE *f;

	fullName = Tcl_TildeSubst(interp, RcFileName, &buffer);
	if (fullName == NULL) {
	    fprintf(stderr, "%s\n", interp->result);
	} else {
	    f = fopen(fullName, "r");
	    if (f != NULL) {
		code = Tcl_EvalFile(interp, fullName);
		if (code != TCL_OK) {
		    fprintf(stderr, "%s\n", interp->result);
		}
		fclose(f);
	    }
	}
	Tcl_DStringFree(&buffer);
    }

    /*
     * Process commands from stdin until there's an end-of-file.
     */

    gotPartial = 0;
    Tcl_DStringInit(&command);
    while (1) {
	clearerr(stdin);
	if (tty) {
	    char *promptCmd;

	    promptCmd = Tcl_GetVar(interp,
		gotPartial ? "tcl_prompt2" : "tcl_prompt1", TCL_GLOBAL_ONLY);
	    if (promptCmd == NULL) {
		defaultPrompt:
		if (!gotPartial) {
		    fputs("% ", stdout);
		}
	    } else {
		code = Tcl_Eval(interp, promptCmd);
		if (code != TCL_OK) {
		    fprintf(stderr, "%s\n", interp->result);
		    Tcl_AddErrorInfo(interp,
			    "\n    (script that generates prompt)");
		    goto defaultPrompt;
		}
	    }
	    fflush(stdout);
	}
	if (fgets(buffer, 1000, stdin) == NULL) {
	    if (ferror(stdin)) {
		if (errno == EINTR) {
#if TCL_MAJOR_VERSION < 7 || (TCL_MAJOR_VERSION == 7 && TCL_MINOR_VERSION < 5 )
		    if (tcl_AsyncReady) {
#else
		    if (Tcl_AsyncReady()) {
#endif
			(void) Tcl_AsyncInvoke((Tcl_Interp *) NULL, 0);
		    }
		    clearerr(stdin);
		} else {
		    goto done;
		}
	    } else {
		if (!gotPartial) {
		    goto done;
		}
	    }
	    buffer[0] = 0;
	}
	cmd = Tcl_DStringAppend(&command, buffer, -1);
	if ((buffer[0] != 0) && !Tcl_CommandComplete(cmd)) {
	    gotPartial = 1;
	    continue;
	}

	gotPartial = 0;
	code = Tcl_RecordAndEval(interp, cmd, 0);
	Tcl_DStringFree(&command);

	if ((code != TCL_OK) || (tty && (*interp->result != 0)))
	    (*tclErrorHandler)(interp, code, tty);

#ifdef TCL_MEM_DEBUG
	if (quitFlag) {
	    Tcl_DeleteInterp(interp);
	    Tcl_DumpActiveMemory(dumpFile);
	    exit(0);
	}
#endif
    }

    /*
     * Rather than calling exit, invoke the "exit" command so that
     * users can replace "exit" with some other command to do additional
     * cleanup on exit.  The Tcl_Eval call should never return.
     */

    done:
    sprintf(buffer, "exit %d", exitCode);
    Tcl_Eval(interp, buffer);
    return 1;
}

/*
 *----------------------------------------------------------------------
 *
 * ErrorHandler --
 *
 *	Handles error conditions while parsing.  Can be replaced by the
 *	caller, but only via the C API, as otherwise interp->result will
 *	get trashed by the call.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Error info is printed to stdout or stderr.
 *
 *----------------------------------------------------------------------
 */
	/* ARGSUSED */
static void
ErrorHandler(interp, code, tty)
    Tcl_Interp *interp;			/* Interpreter for evaluation. */
    int code;				/* Error code returned by last cmd. */
    int tty;				/* Set if connected to a tty. */
{
    if (code != TCL_OK) {
	fprintf(stderr, "%s\n", interp->result);

    } else if (tty && (*interp->result != 0)) {
	printf("%s\n", interp->result);
    }
}

/*
 *----------------------------------------------------------------------
 *
 * CheckmemCmd --
 *
 *	This is the command procedure for the "checkmem" command, which
 *	causes the application to exit after printing information about
 *	memory usage to the file passed to this command as its first
 *	argument.
 *
 * Results:
 *	Returns a standard Tcl completion code.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */
#ifdef TCL_MEM_DEBUG

	/* ARGSUSED */
static int
CheckmemCmd(clientData, interp, argc, argv)
    ClientData clientData;		/* Not used. */
    Tcl_Interp *interp;			/* Interpreter for evaluation. */
    int argc;				/* Number of arguments. */
    char *argv[];			/* String values of arguments. */
{
    if (argc != 2) {
	Tcl_AppendResult(interp, "wrong # args: should be \"", argv[0],
		" fileName\"", (char *) NULL);
	return TCL_ERROR;
    }
    strcpy(dumpFile, argv[1]);
    quitFlag = 1;
    return TCL_OK;
}
#endif
