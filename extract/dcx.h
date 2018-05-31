/* dcx.h */

#ifndef DCX_H_FILE
#define DCX_H_FILE

void *dcx_read_file(const char *filename, size_t *p_out_size);
void *dcx_read_mem(const void *data, size_t size, size_t *p_out_size);

#endif /* DCX_H_FILE */
