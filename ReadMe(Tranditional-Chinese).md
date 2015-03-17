86Duino
---------

_VERSION_: Coding 208

這是 86Duino 硬體平台整合開發環境 (IDE) 的開源專案，相容於 Arduino 1.5.8。
已編譯完成的執行檔 (Windows/Linux/Mac) 可在 86Duino 網站下載頁面找到：
http://www.86duino.com/?page_id=8918&lang=TW

假如您在使用上有發現軟/硬體的 bug，或者想提交 patch，亦或對 86Duino
軟體功能有任何建議，請不吝至 86Duino 論壇發表：
http://www.86duino.com/?page_id=85&lang=TW

或在 GitHub 的 issue 頁面提出建議：https://github.com/roboard/86Duino/issues


#### 安裝方法 ####

86Duino 網站的新手上路專區，提供 Windows XP/7/8、Mac OS X、Linux
平台上的 86Duino IDE 詳細安裝步驟：http://www.86duino.com/?page_id=2844&lang=TW

另外，因為 86Duino IDE 相容於 Arduino 1.5.8，故您也可參考 Arduino
網站中的教學文件來安裝 IDE：http://arduino.cc/en/Guide/HomePage


#### 感謝 ####

86Duino 是基於 Arduino、RoBoard、FreeDOS、DJGPP、coreboot、SeaBIOS
所建立起來的開源專案。沒有前人打好的基礎，我們是不可能完成 86Duino 的，
因此我們要由衷感激所有對 86Duino 產生直接或間接貢獻的人們及開源專案。

86Duino 硬體開發由 Hao Wei Chiu 負責，軟韌體開發則由 Android Lin 及 Oswald Kuo 
負責。這幾位是來自台灣瞻營全電子 RoBoard 團隊的成員。

另外，感謝 OpenLab.Taipei 允許我們使用「口丁」做為 86Duino Coding 的中文名字。
我們選擇這個名字是有緣由的，欲瞭解前因後果，可參閱此網頁：
http://www.86duino.com/index.php?p=5746&lang=TW

台灣還有一些人們及單位影響或幫助了 86Duino 的發展，詳細列表請參照 
[credits.txt](https://github.com/roboard/86Duino/blob/master/credits.txt).


---------------------------------------

#### 關於 86Duino 開源硬體 ####

86Duino 開發板是一個採用 Vortex86EX SoC 的開源嵌入式硬體平台。Vortex86EX
是一顆高整合度的 x86 系統單晶片，於晶片內整合 PCI-E 及 xISA 匯流排、DDR3
記憶體控制器、GPIO、I2C、SPI、UART、CAN、SATA、USB 2.0、網路、類比輸入、
馬達及運動控制電路等多種適於工業控制應用的功能。由於完整的 I/O 功能，
以及體積小且功耗低的特性，Vortex86EX 成為 86Duino 主控核心的一個合適選擇。

86Duino 內建相容 Arudino 的韌體系統，此外，86Duino 也可執行 DOS、Windows、Linux 
以及大部份主流的 x86 即時作業系統。

可參考 86Duino 網站，以獲得更詳細的資訊：http://www.86duino.com/


---------------------------------------

#### 關於瞻營全電子 ####

瞻營全電子成立於 1989 年，始終專注於嵌入式電腦系統的研發生產，
研發領域涵蓋 x86 SoC、嵌入式系統、多媒體應用及智能機器人。
瞻營全產品現今廣泛使用於全球工業電腦製造商、智能電網、綠色節能電腦、工業自動化、
監控設備、廣告機及軍工規的嚴苛環境。

由於相信機器人是未來產業發展的一個新趨勢，
瞻營全多年前便規畫並投入大量人物力於機器人關鍵模組及技術研發上，
同時也積極參與了台灣學研單位 (如工研院、精密機械研發中心) 的合作開發計劃。
為支持機器人領域的開源發展，瞻營全有數個機器人相關的開源計劃正在籌備與進行，
包括人形與六足機器人、整合 LinuxCNC 的自動控制器、機器人專用控制板等。
