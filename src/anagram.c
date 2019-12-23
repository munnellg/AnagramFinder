#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define MAX_STR    128
#define ALPHA_SIZE 26

struct word {
	char word[MAX_STR];
	struct word *next;
};

struct trie {
	struct word *words;
	struct trie *child[ALPHA_SIZE];
};

struct state {
	struct trie *trie;

	char **args;
	int numargs;
} state;

static void
word_free(struct word *w)
{
	struct word *p = w;

	while (p) {
		w = w->next;
		free(p);
		p = w;
	}
}

static void
trie_free(struct trie* t)
{
	if (t) {
		for (int i = 0; i < ALPHA_SIZE; i++) {
			trie_free(t->child[i]);
		}

		word_free(t->words);

		free(t);
	}
}

static void
free_all(void)
{
	trie_free(state.trie);
}

static void
die(const char *msg, ...)
{
	va_list args;
	va_start(args, msg);
	fprintf(stderr, "ERROR: ");
	vfprintf(stderr, msg, args);
	fprintf(stderr, "\n");
	va_end(args);

	free_all();
	exit(EXIT_FAILURE);
}

static void
usage(const char *progname)
{
	fprintf(stdout, "usage: %s [options] WORD ...\n", progname);
	fprintf(stdout, "\n");
	fprintf(stdout, "options:\n");
	fprintf(stdout, "\t-h\n");
	fprintf(stdout, "\t\tprint this usage message\n");
	fprintf(stdout, "\t-d DICT\n");
	fprintf(stdout, "\t\tload anagram dictionary from file\n");

	free_all();
	exit(EXIT_SUCCESS);
}

static struct word *
word_new(char *str)
{
	struct word *w = malloc(sizeof(struct word));

	if (w) {
		memset(w, 0, sizeof(struct word));
		strncpy(w->word, str, MAX_STR);
	}

	return w;
}

static struct trie *
trie_new(void)
{
	struct trie *t = malloc(sizeof(struct trie));

	if (t) {
		memset(t, 0, sizeof(struct trie));
	}

	return t;
}

static void
word_insert(struct word **root, char *s)
{
	struct word *w = word_new(s);
	
	if (!w) {
		die("malloc failed");
	}
	
	w->next = *root;
	
	*root = w;
}

static void
trie_insert(struct trie **pt, char *p, char *s)
{
	struct trie *t = *pt;

	while (*p && !isalpha(*p)) {
		p++;
	}

	if (!t) {
		if ((t = trie_new()) == NULL) {
			die("malloc failed");
		}

		*pt = t;
	}

	if (*p) {
		int code = *p - 'a';

		trie_insert(&t->child[code], ++p, s);
	} else {
		word_insert(&(t->words), s);
	}
}

static struct trie *
trie_find(struct trie *t, char *p)
{
	if (!t) {
		return NULL;
	}

	while (*p && !isalpha(*p)) {
		p++;
	}

	if (*p) {
		int code = *p - 'a';

		return trie_find(t->child[code], ++p);
	} else {
		return t;
	}
}

static int
cmp(const void *a, const void *b)
{
	return *((char *) a) - *((char *) b);
}

static void
lowerstrncpy(char *dst, const char *src, int max)
{
	while (--max && *src) {
		*dst++ = tolower(*src++);
	}

	*dst = '\0';
}

static void
sort(char *s)
{
	qsort(s, strlen(s), sizeof(char), cmp);
}

static void
load_dictionary(const char *fname)
{
	FILE *f = fopen(fname, "r");

	char buf[MAX_STR];
	char code[MAX_STR];

	if (!f) {
		die("Failed to open \"%s\"\n", fname);
	}

	while(!feof(f)) {
		fgets(buf, MAX_STR, f);

		lowerstrncpy(code, buf, MAX_STR);

		sort(code);

		trie_insert(&state.trie, code, buf);
	}

	fclose(f);
}

static void
print_anagrams(const char *word)
{
	char buf[MAX_STR];

	lowerstrncpy(buf, word, MAX_STR);

	sort(buf);

	struct trie *t = trie_find(state.trie, buf);

	fprintf(stdout, "Anagrams of %s:\n", word);

	if (t) {
		struct word *w = t->words;

		while (w) {
			fprintf(stdout, "\t%s\n", w->word);
			w = w->next;
		}
	}
}

static void
parse_args(int argc, char *argv[])
{
	const char *progname = argv[0];

	memset(&state, 0, sizeof(struct state));

	state.args = argv;

	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			if (strcmp(argv[i], "-h") == 0) {
				usage(progname);
			} else if (strcmp(argv[i], "-d") == 0) {
				if (++i >= argc) {
					die("\"%s\" flag expects an argument", argv[i - 1]);
				}

				load_dictionary(argv[i]);
			} else {
				die("unknown flag \"%s\"", argv[i]);
			}
		} else {
			state.args[state.numargs++] = argv[i];	
		}
	}
}

int
main(int argc, char *argv[])
{
	parse_args(argc, argv);

	for (int i = 0; i < state.numargs; i++) {
		print_anagrams(state.args[i]);
	}

	free_all();

	return EXIT_SUCCESS;
}