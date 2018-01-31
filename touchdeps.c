#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERR(FORMAT, ...) \
	fprintf(stderr, "ERROR (%s:%d): " FORMAT "\n", __FILE__, __LINE__, ## __VA_ARGS__)

#define WARN(FORMAT, ...) \
	fprintf(stderr, "warn (%s:%d): " FORMAT "\n", __FILE__, __LINE__, ## __VA_ARGS__)


static inline void print_help() {
	puts(
		"Syntax: touchdeps [-h|--help|<change.csv>]\n\n"
		"touch files read from stdin that have changed according to <change.csv> file"
	);
}

static inline void handle_help(int argc, char *argv[]) {
	if (argc != 2 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
		print_help();
		exit(0);
	} 
}

int main(int argc, char *argv[]) {
	handle_help(argc, argv);
}
