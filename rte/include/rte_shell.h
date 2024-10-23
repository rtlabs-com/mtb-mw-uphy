/*********************************************************************
 *        _       _         _
 *  _ __ | |_  _ | |  __ _ | |__   ___
 * | '__|| __|(_)| | / _` || '_ \ / __|
 * | |   | |_  _ | || (_| || |_) |\__ \
 * |_|    \__|(_)|_| \__,_||_.__/ |___/
 *
 * www.rt-labs.com
 * Copyright 2015 rt-labs AB, Sweden.
 *
 * This software is licensed under the terms of the BSD 3-clause
 * license. See the file LICENSE distributed with this software for
 * full license information.
 ********************************************************************/

/**
 * \defgroup shell Command-line shell
 * \{
 *
 * A command consists of the function that executes the command and a
 * command declaration that contains the function pointer as well
 * as help information for the command. Example:
 *
 * \code
 * // Command function
 * int _cmd_hello (int argc, char * argv[])
 * {
 *    if (argc > 2)
 *    {
 *       shell_usage (argv[0], "too many arguments");
 *       return -1;
 *    }
 *
 *    printf ("Hello World\n");
 *
 *    if (argc == 2)
 *    {
 *       printf ("%s\n", argv[1]);
 *    }
 *
 *    return 0;
 * }
 *
 * // Command declaration
 * static const shell_cmd_t cmd_hello =
 * {
 *    .cmd = _cmd_hello,
 *    .name = "hello",
 *    .help_short = "print hello world",
 *    .help_long =
 *    "hello [msg]\n"
 *    "\n"
 *    "Prints Hello World and an optional message.\n"
 * };
 *
 * // Add command to command table
 * SHELL_CMD (cmd_hello);
 * \endcode
 */

#ifndef RTE_SHELL_H
#define RTE_SHELL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* remap internal lib calls to rte variant */
#define shell_usage rte_shell_usage

/** Command definition */
typedef struct shell_cmd
{
   int (*cmd) (int argc, char * argv[]); /**< command function */
   const char * name;                    /**< command name */
   const char * help_short;              /**< short help for command */
   const char * help_long;               /**< long help for command */
} shell_cmd_t;

#define CC_ATTRIBUTE_SECTION(name) __attribute__ ((section (name)))

/** This macro adds a command to the command table. */
#define SHELL_CMD(cmd)                                                         \
   const shell_cmd_t * _decl_##cmd CC_ATTRIBUTE_SECTION (".cmds." #cmd) = &cmd

/**
 * Executes shell command
 *
 * The string is executed as a shell command.
 *
 * \param str           String containing command to execute
 *
 * \return result of command
 */
int rte_shell_execute (char * str);

/**
 * Executes shell command from arglist
 *
 * The string is executed as a shell command.
 *
 * \param str           String containing command to execute
 *
 * \return result of command
 */

int rte_shell_execute_arg (int argc, char * argv[]);

/**
 * Prints welcome message
 *
 * The default welcome message can be overridden by the application by
 * defining shell_banner() in the application code.
 */
void rte_shell_banner (void);

/**
 * Prints usage for command
 *
 * This function should be called from other commands when a user
 * supplies invalid arguments. It prints \a msg followed by the long
 * help of the command.
 *
 * \param name          Name of the command
 * \param msg           Error message
 */
void rte_shell_usage (const char * name, const char * msg, ...);

/**
 * Set the shell prompt
 *
 * This function changes the shell prompt.
 *
 * \param prompt        Prompt
 */
void rte_shell_prompt_set (const char * prompt);

/**
 * Initialise shell
 *
 * This function initialises the shell. The \a prompt will be printed
 * at the start of the command line.
 *
 * \param prompt        Prompt
 */
void rte_shell_init (const char * prompt);

#ifdef __cplusplus
}
#endif

#endif /* SHELL_H */

/**
 * \}
 */
