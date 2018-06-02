/* dir.h */

#ifndef DIR_H_FILE
#define DIR_H_FILE

char **dir_list_files(const char *dir, const char *ext);
void dir_free_files(char **file_list);

char *get_path_filename(char *path);

#endif /* DIR_H_FILE */
