#ifndef PATH_MANAGEMENT_H
#define PATH_MANAGEMENT_H

extern char path[];
extern unsigned char path_length;
extern unsigned int left;

/**
 * Simplify the path by removing dead ends
 */
void simplify_path();

/**
 * Save the current path to EEPROM
 */
void savePath();

/**
 * Load a saved path from EEPROM
 */
void loadPath();

#endif // PATH_MANAGEMENT_H
