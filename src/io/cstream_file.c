#include <errno.h>
#include <io/cstream.h>
#include <stdio.h>
#include <utils/alloc.h>

struct cstream_file
{
    struct cstream base;
    bool fclose_on_free;
    FILE *file;
};

static enum error cstream_file_read(struct cstream *cstream_base, int *c)
{
    struct cstream_file *cstream = (struct cstream_file *)cstream_base;
    // Read a character from the file
    int res = fgetc(cstream->file);

    // If fgetc returned an error, bail out
    if (res == EOF && ferror(cstream->file))
        return error_warn(IO_ERROR, "failed to read from file stream");

    // Otherwise, save c for the called to enjoy
    *c = res;
    return NO_ERROR;
}

static enum error cstream_file_free(struct cstream *cstream_base)
{
    struct cstream_file *cstream = (struct cstream_file *)cstream_base;

    // Check if a stream has to be freed
    FILE *fd = cstream->fclose_on_free ? cstream->file : NULL;
    free(cstream_base);

    // Try to close the stream (if required), and return if this succeeds
    if (fd == NULL || fclose(fd) != EOF)
        return NO_ERROR;

    return error_warn(IO_ERROR, "failed to close file stream");
}

static const struct cstream_type cstream_file_type = {
    .read = cstream_file_read,
    .free = cstream_file_free,
    .reset = NULL,
    .interactive = false,
};

struct cstream *cstream_file_create(FILE *file, bool fclose_on_free)
{
    struct cstream_file *cstream = zalloc(sizeof(*cstream));
    cstream->base.type = &cstream_file_type;
    cstream->file = file;
    cstream->fclose_on_free = fclose_on_free;
    return &cstream->base;
}
