#include "json.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct la_page
{
  struct la_page  *prev;
  size_t          size;
};

enum {
  ALIGNED_PAGE_SIZE = (sizeof(struct la_page) + 15) & ~15
};

struct linear_allocator
{
  struct la_page  *page;
  size_t          offset;
  size_t          default_block_size;
};

void linear_allocator_init(struct linear_allocator *alloc, size_t default_block_size)
{
  alloc->page = NULL;
  alloc->offset = 0;
  alloc->default_block_size = default_block_size;
}

void linear_allocator_destroy(struct linear_allocator *alloc)
{
  struct la_page *page = alloc->page;

  while (page)
  {
    void *memory = page;
    page = page->prev;
    free(memory);
  }

  memset(alloc, 0, sizeof(alloc));
}

void *linear_allocate(struct linear_allocator *alloc, size_t size)
{
  size_t aligned_size = (size + 15) & ~15;
  size_t offset = alloc->offset;
  struct la_page *page = alloc->page;
  size_t remain = page ? page->size - offset : 0;
  char* ptr;

  if (remain < aligned_size)
  {
    size_t default_size = alloc->default_block_size;
    size_t page_size = ALIGNED_PAGE_SIZE + size > default_size ? size : default_size;

    page = malloc(page_size);

    if (!page)
    {
      return NULL;
    }

    page->prev = alloc->page;
    page->size = page_size;
    offset = ALIGNED_PAGE_SIZE;
    alloc->page = page;
  }

  ptr = ((char*) page) + offset;
  alloc->offset = offset + aligned_size;
  return ptr;
}

static char *load_file(const char *filename)
{
  FILE *fp;
  char *buffer = NULL;

  if (NULL != (fp = fopen(filename, "rb")))
  {
    long file_size;

    fseek(fp, 0, SEEK_END);

    file_size = ftell(fp);

    if (NULL != (buffer = malloc(file_size + 1)))
    {
      rewind(fp);

      if (1 != fread(buffer, file_size, 1, fp))
      {
        free(buffer);
        buffer = NULL;
      }

      buffer[file_size] = '\0';
    }

    fclose(fp);
  }

  return buffer;
}

int main(int argc, char **argv)
{
  const int iterations = 50;
  int i;
  char* buffer;
  char error[128];

  if (argc < 3)
  {
    fprintf(stderr, "usage: %s file.json linear/heap\n", argv[0]);
    return 1;
  }

  if (NULL == (buffer = load_file(argv[1])))
  {
    fprintf(stderr, "couldn't load %s\n", argv[1]);
    return 1;
  }

  if (0 == strcmp("heap", argv[2]))
  {
    for (i = 0; i < iterations; ++i)
    {
      json_settings settings = { 0, 0, NULL };
      json_value *root = json_parse_ex(&settings, buffer, error);
      json_value_free(root);
    }
  }
  else if (0 == strcmp("linear", argv[2]))
  {
    for (i = 0; i < iterations; ++i)
    {
      struct linear_allocator allocator;
      json_settings settings = { 0, json_linear_allocate, NULL };
      json_value *root;
      linear_allocator_init(&allocator, 1 * 1024 * 1024);

      settings.allocator = &allocator;
      root = json_parse_ex(&settings, buffer, error);

      linear_allocator_destroy(&allocator);
    }
  }

  free(buffer);
  return 0;
}
