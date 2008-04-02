TTXAlwaysOnTop -- TeraTermのウィンドウを常に最前面に表示する。

機能:
  TeraTermのコントロールメニューに、“Always on top”を追加します。
  このメニュー項目を選択すると、TeraTermのウィンドウが常に最前面に表示される
  ようになります。
  再度選択すると、解除されます。

解説:
  TeraTermのウィンドウに対して、SetWindowPos()でHWND_TOPMOSTを指定している
  だけです。TeraTermの内部的な処理は一切変更していません。

バグ:
  TEKウィンドウには対応していません。

