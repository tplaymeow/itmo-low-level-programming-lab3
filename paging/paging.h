#ifndef LOW_LEVEL_PROGRAMMING_LAB3_PAGING_H
#define LOW_LEVEL_PROGRAMMING_LAB3_PAGING_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

struct paging_pager;

enum paging_type {
  PAGING_TYPE_FREE,
  PAGING_TYPE_1,
  PAGING_TYPE_2,
  PAGING_TYPE_3,
};

struct paging_info {
  enum paging_type type;
  uint64_t previous_last_page_number;
  uint64_t current_first_page_number;
  uint64_t current_last_page_number;
  uint64_t next_first_page_number;
};

struct paging_write_result {
  bool success;
};

struct paging_remove_result {
  bool success;
};

struct paging_read_result {
  bool success;
  struct paging_info info;
};

struct paging_pager *paging_pager_create_and_init(FILE *file);
struct paging_pager *paging_pager_init(FILE *file);

void paging_pager_destroy(struct paging_pager *pager);

struct paging_write_result paging_write(struct paging_pager *pager,
                                        enum paging_type type, const void *data,
                                        size_t size);

struct paging_remove_result paging_remove(struct paging_pager *pager,
                                          struct paging_info info);

struct paging_read_result paging_read_first(const struct paging_pager *pager,
                                            enum paging_type type, void **data);
struct paging_read_result paging_read_next(const struct paging_pager *pager,
                                           struct paging_info info,
                                           void **data);

#endif // LOW_LEVEL_PROGRAMMING_LAB3_PAGING_H
