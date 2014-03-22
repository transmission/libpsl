/*
 * Copyright(c) 2014 Tim Ruehsen
 *
 * This file is part of libpsl.
 *
 * Libpsl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Libpsl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libpsl.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Precompile Public Suffix List into
 *
 * Changelog
 * 22.03.2014  Tim Ruehsen  created
 *
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

//#ifdef WITH_LIBIDN2
#	include <idn2.h>
//#endif

#include "psl.c"

static void _print_psl_entries(_psl_vector_t *v, const char *varname)
{
	int it;

	printf("// automatically generated by psl2c\n");
	printf("static _psl_entry_t %s[] = {\n", varname);

	for (it = 0; it < v->cur; it++) {
		_psl_entry_t *e = _vector_get(v, it);

		printf("\t{ \"%s\", NULL, %hd, %hhd, %hhd },\n",
			e->label_buf, e->length, e->nlabels, e->wildcard);
	}

	printf("};\n");
}

static int _str_needs_encoding(const char *s)
{
	while (*s > 0) s++;

	return !!*s;
}

static void _add_punycode_if_needed(_psl_vector_t *v)
{
	int it;

	for (it = 0; it < v->cur; it++) {
		_psl_entry_t *e = _vector_get(v, it);

		if (_str_needs_encoding(e->label_buf)) {
			_psl_entry_t suffix;
			char *asc = NULL;
			int rc;


			if ((rc = idn2_lookup_u8((uint8_t *)e->label_buf, (uint8_t **)&asc, 0)) == IDN2_OK) {
				// fprintf(stderr, "idn2 '%s' -> '%s'\n", e->label_buf, asc);
				_suffix_init(&suffix, asc, strlen(asc));
				suffix.wildcard = e->wildcard;
				_vector_add(v, &suffix);
			} else
				fprintf(stderr, "toASCII(%s) failed (%d): %s\n", e->label_buf, rc, idn2_strerror(rc));
		}
	}

	_vector_sort(v);
}

// int main(int argc, const char **argv)
int main(void)
{
	psl_ctx_t *psl;

	if (!(psl = psl_load_fp(stdin)))
		return 1;

	_add_punycode_if_needed(psl->suffixes);
	_add_punycode_if_needed(psl->suffix_exceptions);

	_print_psl_entries(psl->suffixes, "suffixes");
	_print_psl_entries(psl->suffix_exceptions, "suffix_exceptions");

	psl_free(&psl);
	return 0;
}
