#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERR(FORMAT, ...) \
	fprintf(stderr, "ERROR (%s:%d): " FORMAT "\n", __FILE__, __LINE__, ## __VA_ARGS__)

#define FAIL(FORMAT, ...) do { ERR(FORMAT, ## __VA_ARGS__); exit(EXIT_FAILURE); } while (false)

#define WARN(FORMAT, ...) \
	fprintf(stderr, "warn (%s:%d): " FORMAT "\n", __FILE__, __LINE__, ## __VA_ARGS__)

#define INFO(FORMAT, ...) printf("info (%s:%d): " FORMAT "\n", __FILE__, __LINE__, ## __VA_ARGS__)

#define HASH_BUFFER_SIZE 28
#define PATH_BUFFER_SIZE 256

typedef struct _node_t node_t;

struct _node_t {
	node_t *link;
	const char *hash;
	const char *path;
};

static node_t *old_hashes = NULL;

static node_t *alloc_node(node_t *link, const char *hash, const char *path) {
	node_t *result = malloc(sizeof(node_t));
	if (result) {
		result->link = link;
		result->hash = strdup(hash);
		if (! result->hash) { FAIL("no memory for hash"); }
		result->path = strdup(path);
		if (! result->path) { FAIL("no memory for path"); }
	} else { FAIL("no memory for node"); }
	return result;
}

static inline void read_old_hashes(const char *path) {
	FILE *in = fopen(path, "r");
	if (! in) { FAIL("can't open [%s]", path); }

	char hash_begin[HASH_BUFFER_SIZE];
	const char *hash_end = hash_begin + sizeof(hash_begin);
	char path_begin[PATH_BUFFER_SIZE];
	const char *path_end = path_begin + sizeof(path_begin);

	unsigned count = 0;

	int ch = getc(in);
	while (ch != EOF) {
		char *hash_cur = hash_begin;
		char *path_cur = path_begin;

		for (; ch != EOF && ch > ' '; ch = getc(in)) {
			if (hash_cur < hash_end) { *hash_cur++ = ch; }
			else { FAIL("hash buffer too small"); }
		}

		while (ch != EOF && ch <= ' ') { ch = getc(in); }

		for (; ch != EOF && ch >= ' '; ch = getc(in)) {
			if (path_cur < path_end) { *path_cur++ = ch; }
			else { FAIL("path buffer too small"); }
		}

		if (hash_cur == hash_begin || path_cur == path_begin) { continue; }

		if (hash_cur < hash_end) { *hash_cur++ = 0; }
		else { FAIL("hash buffer too small"); }

		if (path_cur < path_end) { *path_cur++ = 0; }
		else { FAIL("path buffer too small"); }

		old_hashes = alloc_node(old_hashes, hash_begin, path_end);
		++count;
	}

	fclose(in);

	INFO("read %u entries", count);
}

static inline void print_help() {
	puts(
		"Syntax: touchdeps [-h|--help|<change.csv>]\n\n"
		"touch files read from stdin that have changed according to <change.csv> file"
	);
}

static inline void handle_help(int argc, char *argv[]) {
	if (argc != 2 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
		print_help();
		exit(EXIT_SUCCESS);
	} 
}

void process_file(const char *path) {
	INFO("processing [%s]", path);
}

int main(int argc, char *argv[]) {
	handle_help(argc, argv);
	read_old_hashes(argv[1]);

	char path_begin[PATH_BUFFER_SIZE];
	const char *path_end = path_begin + sizeof(path_begin);

	for (int ch = getchar(); ch != EOF; ch = getchar()) {
		if (ch <= ' ') { continue; }
		char *path_cur = path_begin;
		while (ch != EOF && ch > ' ') {
			if (path_cur < path_end) { *path_cur++ = ch; }
			else { FAIL("path buffer too small"); }
		}
		if (path_cur < path_end) { *path_cur++ = 0; }
		else { FAIL("path buffer too small"); }

		process_file(path_begin);
	}
}
