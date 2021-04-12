


/**
 * Provides a method for retrieving values from the auxiliary vector and
 * possibly running a string comparison.
 *
 * @return Always returns a result.  When the result is 0, check errno
 * to see if an error occurred during processing.
 */
static unsigned long
_rte_cpu_getauxval(unsigned long type, const char *str)
{
	unsigned long val;

	errno = 0;
	val = getauxval(type);

	if (!val && (errno == ENOTSUP || errno == ENOENT)) {
		int auxv_fd = open("/proc/self/auxv", O_RDONLY);
		Internal_Elfx_auxv_t auxv;

		if (auxv_fd == -1)
			return 0;

		errno = ENOENT;
		while (read(auxv_fd, &auxv, sizeof(auxv)) == sizeof(auxv)) {
			if (auxv.a_type == type) {
				errno = 0;
				val = auxv.a_un.a_val;
				if (str)
					val = strcmp((const char *)val, str);
				break;
			}
		}
		close(auxv_fd);
	}

	return val;
}

unsigned long
rte_cpu_getauxval(unsigned long type)
{
	return _rte_cpu_getauxval(type, NULL);
}
