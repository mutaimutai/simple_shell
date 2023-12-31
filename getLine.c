#include "shell.h"

/**
 * input_buf - buffers chained commands
 * @info: struct parameter
 * @buf: buffer address
 * @len: len var address
 *
 * Return: read bytes
 */

ssize_t input_buf(info_t *info, char **buf, size_t *len)

{
	ssize_t x = 0;
	size_t len_p = 0;

	if (!*len) /* if nothing left in the buffer, fill it */
	{
		/*bfree((void **)info->cmd_buf);*/
		free(*buf);
		*buf = NULL;
		signal(SIGINT, sigintHandler);
#if USE_GETLINE
		x = getline(buf, &len_p, stdin);
#else
		x = _getline(info, buf, &len_p);
#endif
		if (x > 0)
		{
			if ((*buf)[x - 1] == '\n')
			{
				(*buf)[x - 1] = '\0';
				x--;
			}
			info->linecount_flag = 1;
			remove_comments(*buf);
			build_history_list(info, *buf, info->histcount++);
			/* if (_strchr(*buf, ';')) is this a command chain? */
			{
				*len = x;
				info->cmd_buf = buf;
			}
		}
	}
	return (x);
}

/**
 * get_input - gets a line minus the newline
 * @info: struct parameter
 *
 * Return: read bytes
 */

ssize_t get_input(info_t *info)

{
	static char *buf; /* the ';' command chain buffer */
	static size_t u, v, len;
	ssize_t x = 0;
	char **buf_p = &(info->arg), *p;

	_putchar(BUF_FLUSH);
	x = input_buf(info, &buf, &len);
	if (x == -1) /* EOF */
		return (-1);
	if (len)
	{
		v = u;
		p = buf + u;

		check_chain(info, buf, &v, u, len);
		while (v < len)
		{
			if (is_chain(info, buf, &v))
				break;
			v++;
		}

		u = v + 1;
		if (u >= len)
		{
			u = len = 0;
			info->cmd_buf_type = CMD_NORM;
		}

		*buf_p = p;
		return (_strlen(p));
	}

	*buf_p = buf;
	return (x);
}

/**
 * read_buf - buffer read
 * @info: parameter struct
 * @buf: buffer
 * @i: size
 *
 * Return: x
 */

ssize_t read_buf(info_t *info, char *buf, size_t *i)

{
	ssize_t x = 0;

	if (*i)
		return (0);
	x = read(info->readfd, buf, READ_BUF_SIZE);
	if (x >= 0)
		*i = x;
	return (x);
}

/**
 * _getline - line input for STDIN
 * @info: struct parameter
 * @ptr: address of pointer to buffer, preallocated or NULL
 * @length: size of preallocated ptr buffer if not NULL
 *
 * Return: s
 */

int _getline(info_t *info, char **ptr, size_t *length)

{
	static char buf[READ_BUF_SIZE];
	static size_t u, len;
	size_t l;
	ssize_t x = 0, s = 0;
	char *p = NULL, *new_p = NULL, *c;

	p = *ptr;
	if (p && length)
		s = *length;
	if (u == len)
		u = len = 0;

	x = read_buf(info, buf, &len);
	if (x == -1 || (x == 0 && len == 0))
		return (-1);

	c = _strchr(buf + u, '\n');
	l = c ? 1 + (unsigned int)(c - buf) : len;
	new_p = _realloc(p, s, s ? s + l : l + 1);
	if (!new_p) /* MALLOC FAILURE! */
		return (p ? free(p), -1 : -1);

	if (s)
		_strncat(new_p, buf + u, l - u);
	else
		_strncpy(new_p, buf + u, l - u + 1);

	s += l - u;
	u = l;
	p = new_p;

	if (length)
		*length = s;
	*ptr = p;
	return (s);
}

/**
 * sigintHandler - ctrl-C blocked
 * @sig_num: number signal
 *
 * Return: void
 */

void sigintHandler(__attribute__((unused))int sig_num)
{
	_puts("\n");
	_puts("$ ");
	_putchar(BUF_FLUSH);
}
