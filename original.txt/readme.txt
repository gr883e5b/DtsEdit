
*****************************************************************************
*
* DtsEdit.exe
*
*****************************************************************************
【概要】
・MP4(AVC)コンテナに対して、タイムコードの入出力を行います。
・対応している timecode format は、v1、v2になります。


【使い方】
Usage: DtsEdit <option> <mp4 file>

option list
        -tc <file name>       : 入力するTimeCodeファイルを指定します。
                                -tcが指定された場合は、TimeCodeを指定のMP4ファイルに埋め込みます。
                                指定されなかった場合は、MP4ファイルからTimeCodeを抽出します。
        -tv <1|2>             : 入出力するTimeCodeファイルのバージョンを指定します。デフォルトは2です。
        -s <time scale>       : timeScaleを指定します。
                                未指定の場合は、入力されたtimecode formatによって自動計算されます。
        -mlt <multiple>       : timeScaleの自動計算に使用します。
                                デフォルト値は4.0倍です。timeScaleを直接指定した場合は、内部の計算結果で上書きされます。
        -r <time rate>        : 最小timeRateを指定します。
                                未指定の場合は、入力されたMP4ファイルに従います。
        -no-dc                : 初期ディレイカットを無効にします。
        -df <count>           : 初期ディレイを付与する場合の、ディレイフレーム数を指定します。
                                未指定の場合は、入力されたMP4ファイルから自動計算します。
                                再生時間の短い動画では正確に取得できない場合があります。
        -o <output file>      : 出力ファイルを指定します。


  □TimeCodeを出力する。
    基本的にオプションの指定は不要です。
    timecode format v1 を出力したい場合は、オプションに「-tv 1」を指定します。

  □TimeCodeを入力する。
    TimeScale、TimeRateを自動判定するため、オプションを指定しなくても入力したTimeCodeにあわせて適切な値が設定されます。
    TimeCode入力はVFR化を前提にしていますので、TimeScaleは4倍精度で計算されます。
    精度を変更したい場合は、-mltオプションを利用します。（少数指定が可能です）
    目的のTimeScaleが明確な場合は、-sオプションを利用します。そのとき、TimeRateは入力元のファイルに従いますので注意が
    必要です。

    ※TimeRateについて。
      fpsを分数で表現した場合の分母にあたります。標準NTSCであれば29.97fpsですが、正確にはこのときのfpsは循環小数であり、
      30000/1001と表記します。
      この場合、TimeScaleは30000、TimeRateは1001となります。

      もし非NTSCソースを扱う場合は処理結果の表示を確認してください。
      例えば30fpsの動画の場合は、TimeRateは1でも構いません。（そのときのTimeScaleは120と自動計算されます。）
      ディレイフレームが3フレーム以上ある場合は、TimeRateが1では初期ディレイカットに失敗します。
      その場合はTimeRateを10に変更するなどしてください。

    ※倍精度について
      VFRを実現するにあたり、NTSCの表示タイミングを崩さないようにするためにTimeScaleをn倍します。
      例えば23.976fpsと29.97fpsの混合フレームレートであった場合、これを分数表現すると24000/1001、30000/1001となります。
      大きいほうのTimeScale 30000にあわせると、24000/1001 => 30000/1251.25 となってしまい、分子に端数が発生してしまいます。
      そこで、大きいほうのTimeScaleを4倍にし120000/5005、120000/4004と整数で表すことで、表示タイミングを崩さずにフレーム
      レートの混合を実現しています。
      さらに59.94fps(60000/1001)を混合する場合も、120000/2002と表現できるのでそのまま混合が可能です。
      （ただし、倍精度のデフォルト値は4倍になっていますので、上の様なTimeCodeを入力した場合は、240000/10010、240000/8008、
      240000/4004という計算結果になります。）
      TimeCodeがCFRになっている場合は、「-mlt 1」として処理しても問題ありませんが、初期ディレイカットを行うにあたり一定上の
      TimeScale分解能（TimeRateの大きさ）が必要です。

    ※ディレイフレーム数について
      言い換えると「前方参照フレームの連続数」になります。
      DtsEditでは、このフレーム数をCTS（Composition Time Stamp）の順序性をチェックすることで自動算出しています。
      十分な長さの動画ではない場合、自動算出がうまく動作しない場合があります。
      尚、この数値は初期ディレイカットを無効化した場合に使用します。
      処理結果に表示される「Delay Frame」の枚数が、入力元ファイルの本来の枚数と違う場合は、-dfオプションにて訂正してください。


【注意事項】
・パラメータの範囲チェックなどは行っていないので、かなり無茶な指定もできてしまいます。
・ご利用は計画的に。


【修正履歴】
    □20081009
      ・1fps〜120fpsが混在するような変態VFRでないかぎり、初期ディレイは平均fpsから算出しても大丈夫そう
        なので修正。ダメだった場合はエラーを表示するようにした。
        timecode format v2 を取り込むとき、TimeStampが整数に丸められていても、TimeRateから本来のTimeStamp
        を復元できるようにした。

    □20081008
      ・ -no-dc を使用し、精度を4倍未満でtimecodeを取り込んだ場合、付与するディレイが小さすぎて
        DTSの可変範囲を狭めてしまい、DTSがずれることにより実際のフレームの表示タイミングまでずれて
        しまう問題があったため修正。

        この修正により、付与するディレイは「ディレイフレーム数 * TimeRate * 精度」から、
        「ディレイフレーム数 * 最小fps」に変わりました。極端にfpsの低い部分があるTimeCodeを取り込むと、
        ディレイが大きくなるので注意してください。
        （※初期ディレイカットを使用した場合は、問題ありません。）

    □20080614
      ・GPACの最新化。

    □20080511
      ・トラック複製時に、入力元ファイルのedit boxがコピーされるので削除するように修正。

    □20080328
      ・初期ディレイカットを行わないときは、edtsを挿入して初期ディレイを打ち消すようにしてみた。
        でも、edtsに対応したsplitter、playerを知らない…  QuickTimeは対応しているのかな？
        手元で確認した限りだとVLCは対応してました。Gabestはダメ。Haaliは元々ディレイカットするので
        そもそも意味なし。

        一応mp4としては、初期ディレイカットがhackぽい対応で、edts埋め込みが規格通りの王道。
        しかし現状は、hackの方が互換性が高いという悲しい状況。
        なんで対応していないソフトがあるのかはお察し…

        なんと、Haali splitterはedtsを無視するのではなく、誤認して誤動作します。
        なんだか2年前とまるで状況が変わってないわけですね。というわけでedts対応は2年後にもう一回
        チャレンジします。
        このバージョンはお蔵入り。

    □20080327
      ・TimeScale、TimeRateの自動計算処理を実装。
        これに伴い、オプションを一部変更。

    □20080314
      ・いろいろ考えて、timecode format v1 のAssumeには平均ではなく最大fpsを出力する
        ようにしてみた。
        v1入力時のtimeScaleは、このAssumeから計算します。

    □20080313
      ・予想以上に、DTSRepairを timecode の入出力に使う人がいたようなので、急遽作成。
        DTSRepairは名前の通りDTSをNTSCタイミングに矯正するためのソフトですので、
        他のタイミングのMP4ファイルに使用すると、かなりの確率でダメにしてしまいます。

