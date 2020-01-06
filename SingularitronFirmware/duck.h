#ifndef DUCK_H
#define DUCK_H

//#include <string.h>

#define notACommand 0
#define commandOk 1
#define changedDefaultDelay 2
#define missingToken 3
#define mustRepeat 4
#define badToken 5
#define mustDelay 6

#define endOfLine 0x00

struct DuckResponse {
  byte response;
  int otherInformation;

  DuckResponse(byte response) : response(response), otherInformation(0) {}
  DuckResponse(byte response, int otherInformation) : response(response), otherInformation(otherInformation) {}
};

int tokenize(char * tokenBuffer, int tokenBufferLength, const char * line, int lineLength, int startingPoint, char delimiter) {
  for (int i = 0; i < tokenBufferLength; i++) tokenBuffer[i] = 0;

  int tokenLength = 0;

  if (lineLength - startingPoint <= 0) {
    debug_println("End of line, no token");
    return -1;
  }

  for (int i = 0; i < min(tokenBufferLength, lineLength - startingPoint); i++) {
    tokenLength++;

    if (line[startingPoint + i] == delimiter) {
      if (tokenLength != tokenBufferLength - 1) tokenBuffer[i + 1] = 0x00;
      break;
    }
    else tokenBuffer[i] = line[startingPoint + i];
  }

  debug_print("Token: ");
  debug_println(tokenBuffer);
  return tokenLength;
}

int tokenize(char * tokenBuffer, int tokenBufferLength, const char * line, int lineLength, int startingPoint) {
  return tokenize(tokenBuffer, tokenBufferLength, line, lineLength, startingPoint, ' ');
}

int keycodeOf(char * token) {
  if (token[1] == 0x00) { // One char token = just press the key
    char target = token[0]; // For convenience

    if (target == '0') return KEY_0; // This one's out of order
    else if (target >= '1' && target <= '9') return KEY_1 + (target - '1');
    else if (target >= 'a' && target <= 'z') return KEY_A + (target - 'a');
    else if (target >= 'A' && target <= 'Z') return KEY_A + (target - 'A');
    else return 0;
    // TODO Add other chars
  }

  if (0 == strcmp(token, "BREAK") || (0 == strcmp(token, "PAUSE"))) return KEY_PAUSE;
  else if (0 == strcmp(token, "F1")) return KEY_F1;
  else if (0 == strcmp(token, "F2")) return KEY_F2;
  else if (0 == strcmp(token, "F3")) return KEY_F3;
  else if (0 == strcmp(token, "F4")) return KEY_F4;
  else if (0 == strcmp(token, "F5")) return KEY_F5;
  else if (0 == strcmp(token, "F6")) return KEY_F6;
  else if (0 == strcmp(token, "F7")) return KEY_F7;
  else if (0 == strcmp(token, "F8")) return KEY_F8;
  else if (0 == strcmp(token, "F9")) return KEY_F9;
  else if (0 == strcmp(token, "F10")) return KEY_F10;
  else if (0 == strcmp(token, "F11")) return KEY_F11;
  else if (0 == strcmp(token, "F12")) return KEY_F12;
  else if (0 == strcmp(token, "ESC") || 0 == strcmp(token, "ESCAPE")) return KEY_ESC;
  else if (0 == strcmp(token, "DOWN") || 0 == strcmp(token, "DOWNARROW")) return KEY_DOWN;
  else if (0 == strcmp(token, "UP") || 0 == strcmp(token, "UPARROW")) return KEY_UP;
  else if (0 == strcmp(token, "LEFT") || 0 == strcmp(token, "LEFTARROW")) return KEY_LEFT;
  else if (0 == strcmp(token, "RIGHT") || 0 == strcmp(token, "RIGHTARROW")) return KEY_RIGHT;
  else if (0 == strcmp(token, "CAPSLOCK")) return KEY_CAPS_LOCK;
  else if (0 == strcmp(token, "DELETE")) return KEY_DELETE;
  else if (0 == strcmp(token, "END")) return KEY_END;
  else if (0 == strcmp(token, "HOME")) return KEY_HOME;
  else if (0 == strcmp(token, "INSERT")) return KEY_INSERT;
  else if (0 == strcmp(token, "NUMLOCK")) return KEY_NUM_LOCK;
  else if (0 == strcmp(token, "PAGEUP")) return KEY_PAGE_UP;
  else if (0 == strcmp(token, "PAGEDOWN")) return KEY_PAGE_DOWN;
  else if (0 == strcmp(token, "PRINTSCREEN")) return KEY_PRINTSCREEN;
  else if (0 == strcmp(token, "SCROLLOCK")) return KEY_SCROLL_LOCK;
  else if (0 == strcmp(token, "SPACE")) return KEY_SPACE;
  else if (0 == strcmp(token, "TAB")) return KEY_TAB;
  else if (0 == strcmp(token, "ENTER") || 0 == strcmp(token, "RETURN")) return KEY_ENTER;
  // TODO Add other keys, including modifiers and media shit

  return 0;
}

DuckResponse duck(const char * line, unsigned int length) {
  const int tokenBufferLength = 21;
  static char token[tokenBufferLength] = {0};
  int tokenLength = 0;

  for (int i = 0; i < tokenBufferLength; i++) token[i] = 0x00;

  tokenLength = tokenize(token, tokenBufferLength, line, length, 0, ' ');

  if (tokenLength < 1) return DuckResponse(notACommand);

  int index = tokenLength;

  if (0 == strcmp(token, "REM")) return DuckResponse(commandOk);

  if (0 == strcmp(token, "STRING")) {
    debug_println("Writing string");
    for (int i = index; i < length; i++) {
      debug_println(line[i]);
      Keyboard.print(line[i]);
      delay(5);
    }
    return DuckResponse(commandOk);
  }

  if (0 == strcmp(token, "DELAY")) {
    if (tokenize(token, tokenBufferLength, line, length, index, endOfLine) < 1) return DuckResponse(missingToken);
    if (atoi(token) == 0) return DuckResponse(badToken);

    return DuckResponse(mustDelay, atoi(token));
  }

  if (0 == strcmp(token, "DEFAULTDELAY") || 0 == strcmp(token, "DEFAULT_DELAY")) {
    if (tokenize(token, tokenBufferLength, line, length, index, endOfLine) < 1) return DuckResponse(missingToken);
//    if (atoi(token) == 0) return DuckResponse(badToken);
    // Problem: If parsing fails, we can get a 0, but 0 is actually a real default delay value

    return DuckResponse(changedDefaultDelay, atoi(token));
  }

  if (0 == strcmp(token, "REPEAT")) {
    if (tokenize(token, tokenBufferLength, line, length, index, endOfLine) < 1) return DuckResponse(missingToken);
    if (atoi(token) == 0) return DuckResponse(badToken);

    return DuckResponse(mustRepeat, atoi(token));
  }

  byte modifiers = 0;

  if (0 == strcmp(token, "ALT")) modifiers = MODIFIERKEY_ALT;
  else if (0 == strcmp(token, "ALT-SHIFT")) modifiers = MODIFIERKEY_ALT | MODIFIERKEY_SHIFT;
  else if (0 == strcmp(token, "CTRL") || 0 == strcmp(token, "CONTROL")) modifiers = MODIFIERKEY_CTRL;
  else if (0 == strcmp(token, "CTRL-ALT")) modifiers = MODIFIERKEY_ALT | MODIFIERKEY_CTRL;
  else if (0 == strcmp(token, "CTRL-SHIFT")) modifiers = MODIFIERKEY_CTRL | MODIFIERKEY_SHIFT;
  else if (0 == strcmp(token, "ALT-SHIFT")) modifiers = MODIFIERKEY_ALT | MODIFIERKEY_SHIFT;
  else if (0 == strcmp(token, "COMMAND") || 0 == strcmp(token, "GUI") || 0 == strcmp(token, "WINDOWS")) modifiers = MODIFIERKEY_GUI;
  else if (0 == strcmp(token, "SHIFT")) modifiers = MODIFIERKEY_SHIFT;
  else {
    int keycode = keycodeOf(token);
    if (keycode == 0) return DuckResponse(notACommand);
    else {
      Keyboard.press(keycode);
      delay(5); // TODO Make sure this works
      Keyboard.release(keycode);
      return DuckResponse(commandOk);
    }
  }

  int keycode = 0;

  if (tokenize(token, tokenBufferLength, line, length, index, endOfLine) >= 1) {
    keycode = keycodeOf(token);
  }

  debug_print("Mods: ");
  debug_print(modifiers, HEX);
  debug_print(" Key: ");
  debug_print(keycode, HEX);

  Keyboard.set_modifier(modifiers);
  Keyboard.send_now();

  delay(5);

  if (keycode != 0) {
    Keyboard.set_key1(keycode);
    Keyboard.send_now();
    delay(5);
  }

  Keyboard.set_modifier(0);
  Keyboard.set_key1(0);
  Keyboard.send_now();

  return DuckResponse(commandOk);
}

#endif
