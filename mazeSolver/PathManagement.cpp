#include "PathManagement.h"
#include <Arduino.h>
#include "EEPROM.h"

void simplify_path() {
  // Only simplify the path if the second-to-last turn was a 'B'
  if (path_length < 3 || path[path_length-2] != 'B')
    return;

  int total_angle = 0;
  int i;

  if (left) {
    for (i = 1; i <= 3; i++) {
      switch (path[path_length - i]) {
        case 'R': total_angle += 90; break;
        case 'L': total_angle += 270; break;
        case 'B': total_angle += 180; break;
      }
    }
  } else {
    for (i = 1; i <= 3; i++) {
      switch (path[path_length - i]) {
        case 'L': total_angle += 90; break;
        case 'R': total_angle += 270; break;
        case 'B': total_angle += 180; break;
      }
    }
  }

  // Get the angle as a number between 0 and 360 degrees.
  total_angle = total_angle % 360;

  // Replace all of those turns with a single one.
  switch (total_angle) {
    case 0:   path[path_length - 3] = 'S'; break;
    case 90:  path[path_length - 3] = left == 0 ? 'L' : 'R'; break;
    case 180: path[path_length - 3] = 'B'; break;
    case 270: path[path_length - 3] = left == 0 ? 'R' : 'L'; break;
  }

  // The path is now two steps shorter.
  path_length -= 2;
}

void savePath() {
  EEPROM.write(0, path_length);
  for (int i = 1; i <= path_length; i++) {
    EEPROM.write(i, path[i-1]);
  }
}

void loadPath() {
  byte pathLen = EEPROM.read(0);
  path_length = char(pathLen);
  for (int i = 1; i <= path_length; i++) {
    byte readValue = EEPROM.read(i);
    if (readValue == 0) {
      break;
    }
    path[i-1] = char(readValue);
  }
}
