TTXAlwaysOnTop -- TeraTerm keeps a window always on top.

Feature:
  "Always on top" menu is added on TeraTerm control menu.
  If a user selects this menu, TeraTerm keeps a window always on top.
  If a user reselects the menu, this feature is canceled.

Description:
  SetWindowPos API specified with HWND_TOPMOST parameter is called againt TeraTErm window. TeraTerm internal implementation is not modified.

Bug:
  Not support TEK window.

