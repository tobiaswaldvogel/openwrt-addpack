

typedef struct CronFile {
	struct CronFile *cf_next;
	struct CronLine *cf_lines;
	smallint cf_wants_starting;     /* bool: one or more jobs ready */
	smallint cf_has_running;        /* bool: one or more jobs running */
	smallint cf_deleted;            /* marked for deletion (but still has running jobs) */
} CronFile;

typedef struct CronLine {
	struct CronLine *cl_next;
	char *cl_cmd;                   /* shell command */
	pid_t cl_pid;                   /* >0:running, <0:needs to be started in this minute, 0:dormant */
	char *cl_shell;
	/* ordered by size, not in natural order. makes code smaller: */
	char cl_Dow[7];                 /* 0-6, beginning sunday */
	char cl_Mons[12];               /* 0-11 */
	char cl_Hrs[24];                /* 0-23 */
	char cl_Days[32];               /* 1-31 */
	char cl_Mins[60];               /* 0-59 */
} CronLine;

/*

enum {
	OPT_l = (1 << 0),
	OPT_L = (1 << 1),
	OPT_f = (1 << 2),
	OPT_b = (1 << 3),
	OPT_S = (1 << 4),
	OPT_c = (1 << 5),
	OPT_d = (1 << 6) * ENABLE_FEATURE_CROND_D,
};
*/

struct globals {
	unsigned log_level; /* = 8; */
	time_t crontab_dir_mtime;
	const char *log_filename;
	const char *crontab_dir_name; /* = CRONTABS; */
	CronFile *cron_files;
} FIX_ALIASING;
#define G (*(struct globals*)&bb_common_bufsiz1)
#define INIT_G() do { \
	G.log_level = 8; \
	G.crontab_dir_name = CRONTABS; \
} while (0)

static const char days[] = "sun""mon""tue""wed""thu""fri""sat";
static const char months[] = "jan""feb""mar""apr""may""jun""jul""aug""sep""oct""nov""dec";

static void parse_field(char *user, char *ary, int modvalue, int off,
				const char *names, char *ptr)
/* 'names' is a pointer to a set of 3-char abbreviations */
{
	char *base = ptr;
	int n1 = -1;
	int n2 = -1;

	// this can't happen due to config_read()
	/*if (base == NULL)
		return;*/

	while (1) {
		int skip = 0;

		/* Handle numeric digit or symbol or '*' */
		if (*ptr == '*') {
			n1 = 0;  /* everything will be filled */
			n2 = modvalue - 1;
			skip = 1;
			++ptr;
		} else if (isdigit(*ptr)) {
			char *endp;
			if (n1 < 0) {
				n1 = strtol(ptr, &endp, 10) + off;
			} else {
				n2 = strtol(ptr, &endp, 10) + off;
			}
			ptr = endp; /* gcc likes temp var for &endp */
			skip = 1;
		} else if (names) {
			int i;

			for (i = 0; names[i]; i += 3) {
				/* was using strncmp before... */
				if (strncasecmp(ptr, &names[i], 3) == 0) {
					ptr += 3;
					if (n1 < 0) {
						n1 = i / 3;
					} else {
						n2 = i / 3;
					}
					skip = 1;
					break;
				}
			}
		}

		/* handle optional range '-' */
		if (skip == 0) {
			goto err;
		}
		if (*ptr == '-' && n2 < 0) {
			++ptr;
			continue;
		}

		/*
		 * collapse single-value ranges, handle skipmark, and fill
		 * in the character array appropriately.
		 */
		if (n2 < 0) {
			n2 = n1;
		}
		if (*ptr == '/') {
			char *endp;
			skip = strtol(ptr + 1, &endp, 10);
			ptr = endp; /* gcc likes temp var for &endp */
		}

		/*
		 * fill array, using a failsafe is the easiest way to prevent
		 * an endless loop
		 */
		{
			int s0 = 1;
			int failsafe = 1024;

			--n1;
			do {
				n1 = (n1 + 1) % modvalue;

				if (--s0 == 0) {
					ary[n1 % modvalue] = 1;
					s0 = skip;
				}
				if (--failsafe == 0) {
					goto err;
				}
			} while (n1 != n2);
		}
		if (*ptr != ',') {
			break;
		}
		++ptr;
		n1 = -1;
		n2 = -1;
	}

	if (*ptr) {
 err:
		bb_error_msg("user %s: parse error at %s", user, base);
		return;
	}

	/* can't use log5 (it inserts newlines), open-coding it */
	if (G.log_level <= 5 && logmode != LOGMODE_SYSLOG) {
		int i;
		for (i = 0; i < modvalue; ++i)
			fprintf(stderr, "%d", (unsigned char)ary[i]);
		bb_putchar_stderr('\n');
	}
}

static void FixDayDow(CronLine *line)
{
	unsigned i;
	int weekUsed = 0;
	int daysUsed = 0;

	for (i = 0; i < ARRAY_SIZE(line->cl_Dow); ++i) {
		if (line->cl_Dow[i] == 0) {
			weekUsed = 1;
			break;
		}
	}
	for (i = 0; i < ARRAY_SIZE(line->cl_Days); ++i) {
		if (line->cl_Days[i] == 0) {
			daysUsed = 1;
			break;
		}
	}
	if (weekUsed != daysUsed) {
		if (weekUsed)
			memset(line->cl_Days, 0, sizeof(line->cl_Days));
		else /* daysUsed */
			memset(line->cl_Dow, 0, sizeof(line->cl_Dow));
	}
}

/
static void load_crontab(const char *fileName)
{
	struct parser_t *parser;
	struct stat sbuf;
	int maxLines;
	char *tokens[6];
#if ENABLE_FEATURE_CROND_CALL_SENDMAIL
	char *mailTo = NULL;
#endif
	char *shell = NULL;

	delete_cronfile(fileName);

	if (!getpwnam(fileName)) {
		log7("ignoring file '%s' (no such user)", fileName);
		return;
	}

	parser = config_open(fileName);
	if (!parser)
		return;

	maxLines = (strcmp(fileName, "root") == 0) ? 65535 : MAXLINES;

	if (fstat(fileno(parser->fp), &sbuf) == 0 && sbuf.st_uid == DAEMON_UID) {
		CronFile *file = xzalloc(sizeof(CronFile));
		CronLine **pline;
		int n;

		file->cf_username = xstrdup(fileName);
		pline = &file->cf_lines;

		while (1) {
			CronLine *line;

			if (!--maxLines) {
				bb_error_msg("user %s: too many lines", fileName);
				break;
			}

			n = config_read(parser, tokens, 6, 1, "# \t", PARSE_NORMAL | PARSE_KEEP_COPY);
			if (!n)
				break;

			log5("user:%s entry:%s", fileName, parser->data);

			/* check if line is setting MAILTO= */
			if (is_prefixed_with(tokens[0], "MAILTO=")) {
#if ENABLE_FEATURE_CROND_CALL_SENDMAIL
				free(mailTo);
				mailTo = (tokens[0][7]) ? xstrdup(&tokens[0][7]) : NULL;
#endif /* otherwise just ignore such lines */
				continue;
			}
			if (is_prefixed_with(tokens[0], "SHELL=")) {
				free(shell);
				shell = xstrdup(&tokens[0][6]);
				continue;
			}
//TODO: handle HOME= too? "man crontab" says:
//name = value
//
//where the spaces around the equal-sign (=) are optional, and any subsequent
//non-leading spaces in value will be part of the value assigned to name.
//The value string may be placed in quotes (single or double, but matching)
//to preserve leading or trailing blanks.
//
//Several environment variables are set up automatically by the cron(8) daemon.
//SHELL is set to /bin/sh, and LOGNAME and HOME are set from the /etc/passwd
//line of the crontab's owner. HOME and SHELL may be overridden by settings
//in the crontab; LOGNAME may not.

			/* check if a minimum of tokens is specified */
			if (n < 6)
				continue;
			*pline = line = xzalloc(sizeof(*line));
			/* parse date ranges */
			ParseField(file->cf_username, line->cl_Mins, 60, 0, NULL, tokens[0]);
			ParseField(file->cf_username, line->cl_Hrs, 24, 0, NULL, tokens[1]);
			ParseField(file->cf_username, line->cl_Days, 32, 0, NULL, tokens[2]);
			ParseField(file->cf_username, line->cl_Mons, 12, -1, MonAry, tokens[3]);
			ParseField(file->cf_username, line->cl_Dow, 7, 0, DowAry, tokens[4]);
			/*
			 * fix days and dow - if one is not "*" and the other
			 * is "*", the other is set to 0, and vise-versa
			 */
			FixDayDow(line);
#if ENABLE_FEATURE_CROND_CALL_SENDMAIL
			/* copy mailto (can be NULL) */
			line->cl_mailto = xstrdup(mailTo);
#endif
			line->cl_shell = xstrdup(shell);
			/* copy command */
			line->cl_cmd = xstrdup(tokens[5]);
			pline = &line->cl_next;
//bb_error_msg("M[%s]F[%s][%s][%s][%s][%s][%s]", mailTo, tokens[0], tokens[1], tokens[2], tokens[3], tokens[4], tokens[5]);
		}
		*pline = NULL;

		file->cf_next = G.cron_files;
		G.cron_files = file;
	}
	config_close(parser);
#if ENABLE_FEATURE_CROND_CALL_SENDMAIL
	free(mailTo);
#endif
	free(shell);
}


static void change_user(struct passwd *pas)
{
	/* careful: we're after vfork! */
	change_identity(pas); /* - initgroups, setgid, setuid */
	if (chdir(pas->pw_dir) < 0) {
		bb_error_msg("can't change directory to '%s'", pas->pw_dir);
		xchdir(CRON_DIR);
	}
}

static void start_one_job(const char *user, CronLine *line)
{
	const char *shell;
	struct passwd *pas;
	pid_t pid;

	pas = getpwnam(user);
	if (!pas) {
		bb_error_msg("can't get uid for %s", user);
		goto err;
	}

	/* Prepare things before vfork */
	shell = line->cl_shell ? line->cl_shell : DEFAULT_SHELL;
	set_env_vars(pas, shell);

	/* Fork as the user in question and run program */
	pid = vfork();
	if (pid == 0) {
		/* CHILD */
		/* initgroups, setgid, setuid, and chdir to home or CRON_DIR */
		change_user(pas);
		log5("child running %s", shell);
		/* crond 3.0pl1-100 puts tasks in separate process groups */
		bb_setpgrp();
		execl(shell, shell, "-c", line->cl_cmd, (char *) NULL);
		bb_error_msg_and_die("can't execute '%s' for user %s", shell, user);
	}
	if (pid < 0) {
		bb_perror_msg("vfork");
 err:
		pid = 0;
	}
	line->cl_pid = pid;
}

#define process_finished_job(user, line)  ((line)->cl_pid = 0)


/*
 * Determine which jobs need to be run.  Under normal conditions, the
 * period is about a minute (one scan).  Worst case it will be one
 * hour (60 scans).
 */
static void flag_starting_jobs(time_t t1, time_t t2)
{
	time_t t;

	/* Find jobs > t1 and <= t2 */

	for (t = t1 - t1 % 60; t <= t2; t += 60) {
		struct tm *ptm;
		CronFile *file;
		CronLine *line;

		if (t <= t1)
			continue;

		ptm = localtime(&t);
		for (file = G.cron_files; file; file = file->cf_next) {
			log5("file %s:", file->cf_username);
			if (file->cf_deleted)
				continue;
			for (line = file->cf_lines; line; line = line->cl_next) {
				log5(" line %s", line->cl_cmd);
				if (line->cl_Mins[ptm->tm_min]
				 && line->cl_Hrs[ptm->tm_hour]
				 && (line->cl_Days[ptm->tm_mday] || line->cl_Dow[ptm->tm_wday])
				 && line->cl_Mons[ptm->tm_mon]
				) {
					log5(" job: %d %s",
							(int)line->cl_pid, line->cl_cmd);
					if (line->cl_pid > 0) {
						log8("user %s: process already running: %s",
							file->cf_username, line->cl_cmd);
					} else if (line->cl_pid == 0) {
						line->cl_pid = -1;
						file->cf_wants_starting = 1;
					}
				}
			}
		}
	}
}

static void start_jobs(void)
{
	CronFile *file;
	CronLine *line;

	for (file = G.cron_files; file; file = file->cf_next) {
		if (!file->cf_wants_starting)
			continue;

		file->cf_wants_starting = 0;
		for (line = file->cf_lines; line; line = line->cl_next) {
			pid_t pid;
			if (line->cl_pid >= 0)
				continue;

			start_one_job(file->cf_username, line);
			pid = line->cl_pid;
			log8("USER %s pid %3d cmd %s",
				file->cf_username, (int)pid, line->cl_cmd);
			if (pid < 0) {
				file->cf_wants_starting = 1;
			}
			if (pid > 0) {
				file->cf_has_running = 1;
			}
		}
	}
}

/*
 * Check for job completion, return number of jobs still running after
 * all done.
 */
static int check_completions(void)
{
	CronFile *file;
	CronLine *line;
	int num_still_running = 0;

	for (file = G.cron_files; file; file = file->cf_next) {
		if (!file->cf_has_running)
			continue;

		file->cf_has_running = 0;
		for (line = file->cf_lines; line; line = line->cl_next) {
			int r;

			if (line->cl_pid <= 0)
				continue;

			r = waitpid(line->cl_pid, NULL, WNOHANG);
			if (r < 0 || r == line->cl_pid) {
				process_finished_job(file->cf_username, line);
				if (line->cl_pid == 0) {
					/* sendmail was not started for it */
					continue;
				}
				/* else: sendmail was started, job is still running, fall thru */
			}
			/* else: r == 0: "process is still running" */
			file->cf_has_running = 1;
		}
//FIXME: if !file->cf_has_running && file->deleted: delete it!
//otherwise deleted entries will stay forever, right?
		num_still_running += file->cf_has_running;
	}
	return num_still_running;
}

static void reopen_logfile_to_stderr(void)
{
	if (G.log_filename) {
		int logfd = open_or_warn(G.log_filename, O_WRONLY | O_CREAT | O_APPEND);
		if (logfd >= 0)
			xmove_fd(logfd, STDERR_FILENO);
	}
}

int crond_main(int argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
int crond_main(int argc UNUSED_PARAM, char **argv)
{
	time_t t2;
	unsigned rescan;
	unsigned sleep_time;
	unsigned opts;

	/* Main loop */
	t2 = time(NULL);
	rescan = 60;
	sleep_time = 60;
	for (;;) {
		struct stat sbuf;
		time_t t1;
		long dt;

		/* Synchronize to 1 minute, minimum 1 second */
		t1 = t2;
		sleep(sleep_time - (time(NULL) % sleep_time));
		t2 = time(NULL);
		dt = (long)t2 - (long)t1;

		reopen_logfile_to_stderr();

		/*
		 * The file 'cron.update' is checked to determine new cron
		 * jobs.  The directory is rescanned once an hour to deal
		 * with any screwups.
		 *
		 * Check for time jump.  Disparities over an hour either way
		 * result in resynchronization.  A negative disparity
		 * less than an hour causes us to effectively sleep until we
		 * match the original time (i.e. no re-execution of jobs that
		 * have just been run).  A positive disparity less than
		 * an hour causes intermediate jobs to be run, but only once
		 * in the worst case.
		 *
		 * When running jobs, the inequality used is greater but not
		 * equal to t1, and less then or equal to t2.
		 */
		if (stat(G.crontab_dir_name, &sbuf) != 0)
			sbuf.st_mtime = 0; /* force update (once) if dir was deleted */
		if (G.crontab_dir_mtime != sbuf.st_mtime) {
			G.crontab_dir_mtime = sbuf.st_mtime;
			rescan = 1;
		}
		if (--rescan == 0) {
			rescan = 60;
			rescan_crontab_dir();
		}
		process_cron_update_file();
		log5("wakeup dt=%ld", dt);
		if (dt < -60 * 60 || dt > 60 * 60) {
			bb_error_msg("time disparity of %ld minutes detected", dt / 60);
			/* and we do not run any jobs in this case */
		} else if (dt > 0) {
			/* Usual case: time advances forward, as expected */
			flag_starting_jobs(t1, t2);
			start_jobs();
			sleep_time = 60;
			if (check_completions() > 0) {
				/* some jobs are still running */
				sleep_time = 10;
			}
		}
		/* else: time jumped back, do not run any jobs */
	} /* for (;;) */

	return 0; /* not reached */
}
