#ifndef PARSERUTILS_H_
#define PARSERUTILS_H_

#include "internal.h"

gboolean rayem_parser_read_single_line(FILE *input,GString *str,
		int max_str_len,
		int *eos);

#endif /* PARSERUTILS_H_ */
