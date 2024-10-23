/*********************************************************************
 *        _       _         _
 *  _ __ | |_  _ | |  __ _ | |__   ___
 * | '__|| __|(_)| | / _` || '_ \ / __|
 * | |   | |_  _ | || (_| || |_) |\__ \
 * |_|    \__|(_)|_| \__,_||_.__/ |___/
 *
 * http://www.rt-labs.com
 * Copyright 2025 rt-labs AB, Sweden.
 * See LICENSE file in the project root for full license information.
 ********************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "rte_shell.h"

#ifndef NELEMENTS
#define NELEMENTS(a) (sizeof (a) / sizeof ((a)[0]))
#endif

typedef struct shell
{
   const shell_cmd_t ** cmds;
   size_t number_of_cmds;
   const char * prompt;
} shell_t;

shell_t shell;

#if 0
void shell_history_free (gl_state_t * gl, char * line)
{
   free (line);
}

char * shell_history_alloc (gl_state_t * gl, const char * line)
{
   return strdup (line);
}

int shell_completion (gl_state_t * gl, char * line)
{
   unsigned int ix;
   const shell_cmd_t * cmd;

   for (ix = 0; ix < shell.number_of_cmds; ix++)
   {
      cmd = shell.cmds[ix];
      if (strncmp (cmd->name, line, strlen (line)) == 0)
      {
         return snprintf (line, MAX_LINE, "%s", cmd->name);
      }
   }
   return 0;
}
#endif

static const shell_cmd_t * lookup (const char * name)
{
   unsigned int ix;
   const shell_cmd_t * cmd;

   for (ix = 0; ix < shell.number_of_cmds; ix++)
   {
      cmd = shell.cmds[ix];
      if (strcmp (cmd->name, name) == 0)
      {
         return cmd;
      }
   }

   /* Command was not found */
   return NULL;
}

/* not all systems require this function */
__attribute__ ((unused)) int rte_shell_execute_arg (int argc, char * argv[])
{
   const shell_cmd_t * cmd;
   if (argc > 0)
   {
      cmd = lookup (argv[0]);
      if (cmd != NULL)
      {
         return cmd->cmd (argc, argv);
      }
      else
      {
         printf ("Unknown command %s\n", argv[0]);
      }
   }

   return -1;
}

int rte_shell_execute (char * line)
{
   unsigned int argc = 0;
   char * argv[16];
   char * p;
   char * saveptr;

   /* Split line into argv using space as delimiter */
   p = strtok_r (line, " ", &saveptr);
   while (p != NULL && argc < NELEMENTS (argv) - 1)
   {
      argv[argc++] = p;
      p = strtok_r (NULL, " ", &saveptr);
   }

   /* C99 requires argv[argc] to be a NULL pointer */
   argv[argc] = NULL;

   return rte_shell_execute_arg (argc, argv);
}

#if 0
int shell_do_cmd (gl_state_t * gl, const char * prompt)
{
   char * line;

   line = gl_get (gl, prompt);
   if (line != NULL && line[0] != 0)
   {
      gl_history_add (gl, line);
      if (strncmp ("history", line, 7) == 0)
      {
         /* History requires gl context and is handled here */
         gl_history (gl);
      }
      else
      {
         shell_execute (line);
      }
   }

   return line != NULL;
}
#endif

void rte_shell_usage (const char * name, const char * format, ...)
{
   const shell_cmd_t * cmd = lookup (name);

   va_list list;

   printf ("%s: ", name);
   va_start (list, format);
   vprintf (format, list);
   va_end (list);
   printf ("\n\n");

   printf ("usage:\n%s\n", cmd->help_long);
}

int _cmd_help (int argc, char * argv[])
{
   unsigned int ix;
   const shell_cmd_t * cmd;

   if (argc > 2)
   {
      shell_usage (argv[0], "too many arguments");
      return -1;
   }

   if (argc == 1)
   {
      /* List all commands */
      for (ix = 0; ix < shell.number_of_cmds; ix++)
      {
         cmd = shell.cmds[ix];
         printf ("%-20s - %s\n", cmd->name, cmd->help_short);
      }
   }
   else if (argc == 2)
   {
      /* Show long help for given command */
      cmd = lookup (argv[1]);
      if (cmd != NULL)
      {
         printf ("%s\n", cmd->help_long);
      }
      else
      {
         printf ("Unknown command %s\n", argv[1]);
      }
   }

   return 0;
}

void rte_shell_prompt_set (const char * prompt)
{
   shell.prompt = prompt;
}

void rte_shell_init (const char * prompt)
{
   extern uint32_t cmds_start;
   extern uint32_t cmds_end;

   shell.prompt = prompt;
   shell.cmds = (const shell_cmd_t **)&cmds_start;
   shell.number_of_cmds = &cmds_end - &cmds_start;
}

const shell_cmd_t cmd_help = {
   .cmd = _cmd_help,
   .name = "help",
   .help_short = "show help",
   .help_long =
      "help [command]\n"
      "\n"
      "Without argument, show list of available commands. With argument,\n"
      "show help for given command.\n"};

SHELL_CMD (cmd_help);

#if 0
const shell_cmd_t cmd_history =
{
   .cmd = NULL,
   .name = "history",
   .help_short = "show history",
   .help_long =
   "history\n"
   "\n"
   "Show list of previously entered commands.\n"
};
#endif
