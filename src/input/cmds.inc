/*
 * Do nothing.
 */
COMMAND (nop)

/*
 * Exit the engine.
 */
COMMAND (exit)

/*
 * Output arguments to stdout
 */
COMMAND (echo)

/*
 * Bind a key to a command.
 * The key's name is the first argument
 * (In key names that contain spaces,
 *  they are replaced with underscores)
 * The command is the rest of the arguments
 */
COMMAND (bind)

/*
 * PAUSE THE ENTIRE ENGINE, read a command
 * from stdin and run it
 */
COMMAND (stdincmd)

/*
 * Run the commands from a file
 */
COMMAND (exec)

/*
 * Move the camera
 */
COMMAND (move)

