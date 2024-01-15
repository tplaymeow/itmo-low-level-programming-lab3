#include "paging.h"
#include "logger.h"
#include "math_utils.h"

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define PAGING_PAGE_DATA_SIZE (1024)

#define PAGING_INVALID_PAGE_NUMBER UINT64_MAX

struct paging_pager {
  FILE *file;
  uint64_t first_free_page_number;
  uint64_t first_page_type_1_page_number;
  uint64_t first_page_type_2_page_number;
  uint64_t first_page_type_3_page_number;
};

struct paging_file_page_header {
  uint64_t next_page_number;
  uint64_t next_continuation;
};

struct paging_file_header {
  uint64_t first_free_page_number;
  uint64_t first_page_type_1_page_number;
  uint64_t first_page_type_2_page_number;
  uint64_t first_page_type_3_page_number;
};

static bool paging_file_header_read(struct paging_pager *pager) {
  const int seek_result = fseek(pager->file, 0, SEEK_SET);
  if (seek_result != 0) {
    return false;
  }

  struct paging_file_header header;
  const size_t read_count = 1;
  const size_t read_result =
      fread(&header, sizeof(header), read_count, pager->file);
  if (read_result != read_count) {
    return false;
  }

  pager->first_free_page_number = header.first_free_page_number;
  pager->first_page_type_1_page_number = header.first_page_type_1_page_number;
  pager->first_page_type_2_page_number = header.first_page_type_2_page_number;
  pager->first_page_type_3_page_number = header.first_page_type_3_page_number;

  return true;
}

static bool paging_file_header_write(struct paging_pager *pager) {
  const int seek_result = fseek(pager->file, 0, SEEK_SET);
  if (seek_result != 0) {
    return false;
  }

  struct paging_file_header header = {
      .first_free_page_number = pager->first_free_page_number,
      .first_page_type_1_page_number = pager->first_page_type_1_page_number,
      .first_page_type_2_page_number = pager->first_page_type_2_page_number,
      .first_page_type_3_page_number = pager->first_page_type_3_page_number,
  };
  const size_t write_count = 1;
  const size_t write_result =
      fwrite(&header, sizeof(header), write_count, pager->file);
  if (write_result != write_count) {
    return false;
  }

  return true;
}

struct paging_pager *paging_pager_create_and_init(FILE *file) {
  if (file == NULL) {
    return NULL;
  }

  struct paging_pager *pager = malloc(sizeof(struct paging_pager));
  if (pager == NULL) {
    return NULL;
  }

  pager->file = file;
  pager->first_free_page_number = PAGING_INVALID_PAGE_NUMBER;
  pager->first_page_type_1_page_number = PAGING_INVALID_PAGE_NUMBER;
  pager->first_page_type_2_page_number = PAGING_INVALID_PAGE_NUMBER;
  pager->first_page_type_3_page_number = PAGING_INVALID_PAGE_NUMBER;
  if (!paging_file_header_write(pager)) {
    return NULL;
  }

  return pager;
}

struct paging_pager *paging_pager_init(FILE *file) {
  if (file == NULL) {
    return NULL;
  }

  struct paging_pager *pager = malloc(sizeof(struct paging_pager));
  if (pager == NULL) {
    return NULL;
  }

  pager->file = file;
  if (!paging_file_header_read(pager)) {
    return NULL;
  }

  return pager;
}

void paging_pager_destroy(struct paging_pager *pager) {
  if (pager == NULL) {
    return;
  }
  free(pager);
}

static long paging_file_page_header_position(uint64_t page_number) {
  const long page_size =
      (long)sizeof(struct paging_file_page_header) + PAGING_PAGE_DATA_SIZE;
  return (long)sizeof(struct paging_file_header) +
         (long)page_number * page_size;
}

static long paging_file_page_data_position(uint64_t page_number) {
  return paging_file_page_header_position(page_number) +
         (long)sizeof(struct paging_file_page_header);
}

static uint64_t paging_new_page_number(struct paging_pager *pager) {
  if (fseek(pager->file, 0L, SEEK_END) != 0) {
    return 0;
  }

  const long file_size = ftell(pager->file);
  const long page_size =
      (long)sizeof(struct paging_file_page_header) + PAGING_PAGE_DATA_SIZE;
  return (file_size - (long)sizeof(struct paging_file_header)) / page_size;
}

static uint64_t paging_first_page_number(const struct paging_pager *pager,
                                         enum paging_type type) {
  switch (type) {
  case PAGING_TYPE_FREE:
    return pager->first_free_page_number;
  case PAGING_TYPE_1:
    return pager->first_page_type_1_page_number;
  case PAGING_TYPE_2:
    return pager->first_page_type_2_page_number;
  case PAGING_TYPE_3:
    return pager->first_page_type_3_page_number;
  default:
    abort();
  }
}

static void paging_update_first_page_number(struct paging_pager *pager,
                                            enum paging_type type,
                                            uint64_t new_value) {
  switch (type) {
  case PAGING_TYPE_FREE:
    pager->first_free_page_number = new_value;
    break;
  case PAGING_TYPE_1:
    pager->first_page_type_1_page_number = new_value;
    break;
  case PAGING_TYPE_2:
    pager->first_page_type_2_page_number = new_value;
    break;
  case PAGING_TYPE_3:
    pager->first_page_type_3_page_number = new_value;
    break;
  }
}

struct paging_write_result paging_write(struct paging_pager *pager,
                                        enum paging_type type, const void *data,
                                        size_t data_size) {
  if (pager == NULL || data == NULL || data_size == 0) {
    return (struct paging_write_result){.success = false};
  }

  const size_t pages_count = DIV_ROUND_UP(data_size, PAGING_PAGE_DATA_SIZE);
  uint64_t next_page_number = paging_first_page_number(pager, type);
  uint64_t next_page_is_continuation = 0;

  for (size_t i = pages_count; i > 0; i--) {
    uint64_t page_number;

    if (pager->first_free_page_number == PAGING_INVALID_PAGE_NUMBER) {
      page_number = paging_new_page_number(pager);
    } else {
      page_number = pager->first_free_page_number;

      // Update first free page number in pager

      const long seek_position =
          paging_file_page_header_position(pager->first_free_page_number);
      const int seek_result = fseek(pager->file, seek_position, SEEK_SET);
      if (seek_result != 0) {
        warn("Free page header seek error");
        return (struct paging_write_result){.success = false};
      }

      struct paging_file_page_header header;

      const size_t read_count = 1;
      const size_t read_result =
          fread(&header, sizeof(header), read_count, pager->file);
      if (read_result != read_count) {
        warn("Free page header read error");
        return (struct paging_write_result){.success = false};
      }

      pager->first_free_page_number = header.next_page_number;
    }

    const long header_position = paging_file_page_header_position(page_number);
    const int seek_result = fseek(pager->file, header_position, SEEK_SET);
    if (seek_result != 0) {
      warn("Page header seek error");
      return (struct paging_write_result){.success = false};
    }

    const struct paging_file_page_header header = {
        .next_page_number = next_page_number,
        .next_continuation = next_page_is_continuation};

    const size_t write_header_count = 1;
    const size_t write_header_result =
        fwrite(&header, sizeof(header), write_header_count, pager->file);
    if (write_header_result != write_header_count) {
      warn("Page header write error");
      return (struct paging_write_result){.success = false};
    }

    const size_t page_data_offset = PAGING_PAGE_DATA_SIZE * (i - 1);
    const size_t page_data_size =
        MIN(PAGING_PAGE_DATA_SIZE, data_size - page_data_offset);

    const void *page_data = (char *)data + page_data_offset;

    const size_t write_data_count = 1;
    const size_t write_data_result =
        fwrite(page_data, page_data_size, write_data_count, pager->file);
    if (write_data_result != write_data_count) {
      warn("Data write error");
      return (struct paging_write_result){.success = false};
    }

    const long padding_seek_offset = PAGING_PAGE_DATA_SIZE - page_data_size - 1;
    const int padding_seek_result =
        fseek(pager->file, padding_seek_offset, SEEK_CUR);
    if (padding_seek_result != 0) {
      warn("Padding seek error");
      return (struct paging_write_result){.success = false};
    }

    const char data_padding_value = 0;
    const size_t write_data_padding_count = 1;
    const size_t write_data_padding_result =
        fwrite(&data_padding_value, sizeof(data_padding_value),
               write_data_padding_count, pager->file);
    if (write_data_padding_result != write_data_padding_count) {
      warn("Padding write error");
      return (struct paging_write_result){.success = false};
    }

    paging_update_first_page_number(pager, type, page_number);
    next_page_number = page_number;
    next_page_is_continuation = 1;
  }

  if (!paging_file_header_write(pager)) {
    warn("File header write error");
    return (struct paging_write_result){.success = false};
  }

  return (struct paging_write_result){.success = true};
}

struct paging_remove_result paging_remove(struct paging_pager *pager,
                                          struct paging_info info) {
  if (info.current_first_page_number == PAGING_INVALID_PAGE_NUMBER) {
    return (struct paging_remove_result){.success = false};
  }

  bool next_continuation = true;
  uint64_t next_page_number = info.current_first_page_number;

  while (next_continuation) {
    const long seek_position =
        paging_file_page_header_position(next_page_number);

    const int seek_result = fseek(pager->file, seek_position, SEEK_SET);
    if (seek_result != 0) {
      warn("Page header seek error");
      return (struct paging_remove_result){.success = false};
    }

    struct paging_file_page_header header;

    const size_t header_read_count = 1;
    const size_t header_read_result =
        fread(&header, sizeof(header), header_read_count, pager->file);
    if (header_read_result != header_read_count) {
      warn("Read page %" PRIu64 " header error", next_page_number);
      return (struct paging_remove_result){.success = false};
    }

    struct paging_file_page_header new_header = {
        .next_page_number = pager->first_free_page_number,
        .next_continuation = 0};

    const int new_seek_result = fseek(pager->file, seek_position, SEEK_SET);
    if (new_seek_result != 0) {
      warn("Page header seek error");
      return (struct paging_remove_result){.success = false};
    }

    const size_t header_write_count = 1;
    const size_t header_write_result = fwrite(&new_header, sizeof(new_header),
                                              header_write_count, pager->file);
    if (header_write_result != header_write_count) {
      warn("Write page %" PRIu64 " header error", next_page_number);
      return (struct paging_remove_result){.success = false};
    }

    pager->first_free_page_number = next_page_number;
    next_page_number = header.next_page_number;
    next_continuation = header.next_continuation;
  }

  if (paging_first_page_number(pager, info.type) ==
      info.current_first_page_number) {
    paging_update_first_page_number(pager, info.type,
                                    info.next_first_page_number);
  }

  if (!paging_file_header_write(pager)) {
    warn("Write file header error");
    return (struct paging_remove_result){.success = false};
  }

  if (info.previous_last_page_number != PAGING_INVALID_PAGE_NUMBER) {
    const long seek_position =
        paging_file_page_header_position(info.previous_last_page_number);

    const int seek_read_result = fseek(pager->file, seek_position, SEEK_SET);
    if (seek_read_result != 0) {
      warn("Previous page header seek error");
      return (struct paging_remove_result){.success = false};
    }

    struct paging_file_page_header header;

    const size_t header_read_count = 1;
    const size_t header_read_result =
        fread(&header, sizeof(header), header_read_count, pager->file);
    if (header_read_result != header_read_count) {
      warn("Read previous page header error");
      return (struct paging_remove_result){.success = false};
    }

    header.next_page_number = info.next_first_page_number;

    const int seek_write_result = fseek(pager->file, seek_position, SEEK_SET);
    if (seek_write_result != 0) {
      warn("Previous page header seek error");
      return (struct paging_remove_result){.success = false};
    }

    const size_t header_write_count = 1;
    const size_t header_write_result =
        fwrite(&header, sizeof(header), header_write_count, pager->file);
    if (header_write_result != header_write_count) {
      warn("Write previous page header error");
      return (struct paging_remove_result){.success = false};
    }
  }

  return (struct paging_remove_result){.success = true};
}

static struct paging_read_result paging_read(const struct paging_pager *pager,
                                             uint64_t page_number,
                                             void **data) {
  if (page_number == PAGING_INVALID_PAGE_NUMBER) {
    return (struct paging_read_result){.success = false};
  }

  uint64_t current_page_number = page_number;
  uint64_t next_page_number = page_number;
  bool next_continuation = true;
  size_t pages_read = 0;

  *data = NULL;

  while (next_continuation) {
    void *tmp_data = realloc(*data, PAGING_PAGE_DATA_SIZE * (pages_read + 1));
    if (tmp_data == NULL) {
      warn("Re-alloc error");
      free(*data);
      return (struct paging_read_result){.success = false};
    }

    *data = tmp_data;

    const long seek_position =
        paging_file_page_header_position(next_page_number);
    const int seek_result = fseek(pager->file, seek_position, SEEK_SET);
    if (seek_result != 0) {
      warn("Page header seek error");
      free(*data);
      return (struct paging_read_result){.success = false};
    }

    struct paging_file_page_header header;

    const size_t header_read_count = 1;
    const size_t header_read_result =
        fread(&header, sizeof(header), header_read_count, pager->file);
    if (header_read_result != header_read_count) {
      warn("Read page %" PRIu64 " header error", next_page_number);
      free(*data);
      return (struct paging_read_result){.success = false};
    }

    void *data_for_page =
        (PAGING_PAGE_DATA_SIZE * pages_read) + (char *)(*data);
    const size_t data_read_count = 1;
    const size_t data_read_result = fread(data_for_page, PAGING_PAGE_DATA_SIZE,
                                          data_read_count, pager->file);
    if (data_read_result != data_read_count) {
      warn("Read page %" PRIu64 " data error", next_page_number);
      free(*data);
      return (struct paging_read_result){.success = false};
    }

    current_page_number = next_page_number;
    next_page_number = header.next_page_number;
    next_continuation = header.next_continuation == 1;
    pages_read += 1;
  }

  return (struct paging_read_result){
      .success = true,
      .info = {.current_first_page_number = page_number,
               .current_last_page_number = current_page_number,
               .next_first_page_number = next_page_number}};
}

struct paging_read_result paging_read_first(const struct paging_pager *pager,
                                            enum paging_type type,
                                            void **data) {
  const uint64_t page_number = paging_first_page_number(pager, type);
  struct paging_read_result result = paging_read(pager, page_number, data);
  result.info.previous_last_page_number = PAGING_INVALID_PAGE_NUMBER;
  result.info.type = type;
  return result;
}

struct paging_read_result paging_read_next(const struct paging_pager *pager,
                                           struct paging_info info,
                                           void **data) {
  const uint64_t page_number = info.next_first_page_number;
  struct paging_read_result result = paging_read(pager, page_number, data);
  result.info.previous_last_page_number = info.current_last_page_number;
  result.info.type = info.type;
  return result;
}