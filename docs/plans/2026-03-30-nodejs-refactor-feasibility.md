# Synfig 重構為 Node.js 核心：技術可行性評估報告

> 日期：2026-03-30
> 狀態：評估報告
> 類型：架構決策紀錄 (ADR)

---

## 1. 現有架構摘要

### 1.1 程式碼規模

| 元件 | 檔案數 | 程式碼行數 | 語言 |
|------|--------|-----------|------|
| synfig-core/src | 680 | 162,094 | C++ |
| synfig-studio/src (GUI) | 598 | 154,382 | C++ |
| ETL (模板庫) | — | ~918+ | C++ |
| modules (23 個) | — | 35,442 | C++ |
| rendering 子系統 | — | 43,820 | C++ |
| valuenodes (152 類) | — | 25,237 | C++ |
| Python plugins | 83 | 26,398 | Python |
| **C++ 總計** | **~1,278** | **~316,476** | |

### 1.2 核心子系統

```
synfig-core/
  src/synfig/
    rendering/          # 渲染引擎 (43,820 行)
      software/         # CPU 軟體渲染器 (含 blur IIR 20,513 行)
      opengl/           # OpenGL 渲染器 (實驗性)
      renderqueue.h     # 多執行緒任務佇列 (std::thread)
    layers/             # 20+ 圖層類型
    valuenodes/         # 152 個 ValueNode 類型 (動畫參數)
    color/              # 顏色混合 (30+ blend mode，2,575 行)
    blur/               # 模糊演算法
    loadcanvas.cpp      # XML 解析 (3,640 行)
    savecanvas.cpp      # XML 序列化 (1,103 行)
    canvas.cpp          # Canvas 資料結構 (1,587 行)
    layer.cpp           # Layer 基底類 (1,051 行)

synfig-studio/
  src/gui/              # GTK3/gtkmm GUI (104,805 行)
  src/synfigapp/        # 應用邏輯層 (undo/redo, actions)
  src/brushlib/         # 筆刷引擎
  src/player/           # 播放器

modules/ (23 個)
  mod_ffmpeg            # FFmpeg 影片匯出
  mod_png/mod_jpeg      # 圖片 I/O
  mod_libavcodec        # 直接 libav 編碼
  lyr_freetype          # FreeType 文字渲染
  mod_svg               # SVG 匯入
  mod_geometry           # 幾何圖形 (6 layer types)
  ...等
```

### 1.3 關鍵外部依賴

| 依賴 | 用途 | Node.js 對等方案 |
|------|------|-----------------|
| GTK3/gtkmm3 | 桌面 GUI | Electron, Tauri, 或 Web UI |
| sigc++ | 信號/插槽系統 | EventEmitter (原生) |
| glibmm/giomm | 檔案系統、事件迴圈 | Node.js fs, events (原生) |
| libxml++ | XML 解析 | xml2js, fast-xml-parser |
| libpng/libjpeg | 圖片 I/O | sharp, jimp |
| FFTW3 | 快速傅立葉轉換 | fft.js (純 JS，效能差距大) |
| FreeType2/HarfBuzz | 文字排版渲染 | opentype.js (功能有限) |
| libavcodec/FFmpeg | 影片編解碼 | fluent-ffmpeg (CLI 包裝) |
| OpenEXR | HDR 圖片 | 無成熟方案 |
| ImageMagick/Magick++ | 圖片處理 | sharp (部分) |
| ZLIB | 壓縮 (.sifz) | zlib (原生模組) |
| Pango/Fontconfig | 字型系統 | 無直接對等 |
| JACK | 音訊同步 | 無對等方案 |

### 1.4 效能關鍵路徑

1. **多執行緒渲染佇列**：`RenderQueue` 使用 `std::thread` 產生 `hardware_concurrency()` 個工作執行緒，透過 mutex + condition_variable 調度渲染任務
2. **像素級運算**：blur IIR 係數計算 (20,513 行純數學)、resample (577 行)、mesh deformation (526 行)、contour rasterization (350 行)
3. **顏色混合**：30+ blend mode 的每像素浮點運算
4. **Bezier/曲線數學**：ETL 模板庫中的 hermite、bezier 曲線計算

---

## 2. 可行性分析

### 2.1 技術可行性：各子系統評估

| 子系統 | 可行性 | 難度 | 說明 |
|--------|--------|------|------|
| XML I/O (load/save canvas) | 可行 | 中 | Node.js XML 處理成熟，但需 1:1 相容現有 .sif 格式 |
| Canvas/Layer 資料模型 | 可行 | 中 | 純資料結構，可移植 |
| ValueNode 系統 (152 類) | 可行 | 高 | 工作量巨大，每個需逐一移植並驗證 |
| 軟體渲染器 | 技術上可行但不實際 | 極高 | 像素級運算在 JS 中慢 10-100x |
| 顏色混合 (30+ modes) | 可行但慢 | 高 | 浮點密集運算，JS 效能不足 |
| Blur/FFT 處理 | 不實際 | 極高 | FFTW3 無對等 JS 方案 |
| 多執行緒渲染 | 受限 | 極高 | Node.js Worker Threads 可用但開銷大 |
| GTK GUI | 需完全重寫 | 極高 | 改用 Electron/Web 等於重做 UI |
| 影片匯出 | 可行 | 低 | 已透過 CLI 呼叫 FFmpeg |
| 圖片 I/O | 可行 | 低 | sharp 等方案成熟 |
| 文字渲染 | 受限 | 高 | FreeType+HarfBuzz+Pango 組合無 JS 對等 |
| 音訊同步 (JACK) | 不可行 | — | 無 Node.js JACK 綁定 |

### 2.2 效能差距估算

基於 JavaScript vs C++ 的已知基準測試差距：

| 運算類型 | 預估效能比 (C++ = 1x) | 影響範圍 |
|----------|----------------------|---------|
| 像素迴圈 (純浮點) | JS 約 10-50x 慢 | 渲染核心 |
| 矩陣/向量運算 | JS 約 5-20x 慢 | 變形、投影 |
| 記憶體分配 (Surface) | JS 約 3-10x 慢 | 圖層合成 |
| XML 解析 | JS 約 1-3x 慢 | 檔案開啟 |
| 檔案 I/O | 接近 (1-2x) | 匯入匯出 |

**以一個 1920x1080、20 個圖層的動畫場景為例：**
- C++ 渲染一幀：~50-200ms
- 純 JS 預估：~1-10 秒/幀
- 即時預覽 (24fps) 所需：<42ms/幀 → **Node.js 無法達成**

### 2.3 WebAssembly (WASM) 作為加速方案

可將 C++ 渲染核心編譯為 WASM 模組在 Node.js 中執行：
- **優點**：保留 C++ 效能 (約原生 70-90%)，可在瀏覽器/Node.js 共用
- **缺點**：WASM 無法直接存取 OS 執行緒 (SharedArrayBuffer 有限制)、無法直接呼叫系統 API
- **結論**：若走此路線，等於維護兩套程式碼 (WASM 核心 + JS 膠合層)，複雜度不減反增

---

## 3. 三種方案比較

### 方案 A：完全重寫為 Node.js（不建議）

將 synfig-core 和 synfig-studio 完全用 TypeScript/Node.js 重寫。

| 面向 | 評估 |
|------|------|
| 工作量 | ~316,000 行 C++ → 預估 18-36 人月 (2-4 位工程師全職 1 年) |
| 渲染效能 | 下降 10-50x，無法即時預覽 |
| 功能對等 | 152 個 ValueNode、30+ blend mode、23 個模組需逐一移植 |
| 依賴替代 | FFTW、FreeType+HarfBuzz+Pango、JACK 無 JS 對等方案 |
| 社群衝擊 | 現有 C++ 貢獻者全部無法參與 |
| 優點 | 統一技術棧、降低新貢獻者門檻、npm 生態系 |
| 風險 | 極高，可能導致專案停擺 2+ 年 |

**結論：技術上部分可行，但效能代價和工作量使其不切實際。**

### 方案 B：混合架構 — C++ 核心 + Node.js/Electron 前端（有條件建議）

保留 synfig-core (C++ 渲染引擎) 作為共享函式庫，用 Node.js/Electron 重寫 GUI 前端。

| 面向 | 評估 |
|------|------|
| 工作量 | GUI 重寫 ~154,000 行 → 預估 12-18 人月 |
| 渲染效能 | 保持 C++ 原生效能 |
| 前端優勢 | 現代 Web UI、跨平台一致性、HTML/CSS/React 生態系 |
| 整合方式 | N-API addon 或 child_process 呼叫 synfig CLI |
| 依賴 | 仍需 C++ 編譯環境 |
| 優點 | 現代化 UI、保留核心效能、漸進式遷移可行 |
| 風險 | N-API 綁定複雜、GTK→Web 的 UI 模式差異大 |

**前置條件：** synfig-core 需先重構出清晰的 C API/共享函式庫介面。目前 synfig-core 與 synfig-studio 透過 synfigapp 層高度耦合，需先解耦。

### 方案 C：維持 C++ 核心 + 擴展 Node.js 工具鏈（建議）

不重構核心，而是在周邊建設 Node.js 工具：

| 面向 | 評估 |
|------|------|
| 工作量 | 3-6 人月 |
| 範圍 | 建置工具、CI/CD、Plugin SDK、Web 預覽器 |
| 核心不變 | C++ 渲染效能、現有功能完整保留 |
| Node.js 用途 | 建置腳本、測試工具、Plugin manager、.sif/.lottie 轉換器、Web preview |
| 優點 | 風險最低、漸進式、不影響現有貢獻者 |
| 缺點 | 未解決 GTK3 老化問題 |

---

## 4. 風險矩陣

| 風險 | 方案 A | 方案 B | 方案 C |
|------|--------|--------|--------|
| 專案停擺 | 極高 | 中 | 低 |
| 效能退化 | 確定發生 | 無 | 無 |
| 功能缺失 | 高 (JACK, OpenEXR, 文字渲染) | 低 | 無 |
| 社群分裂 | 高 | 中 | 低 |
| 維護複雜度增加 | 低 (單語言) | 高 (雙語言) | 中 |
| 招募困難度 | 低 (JS 開發者多) | 中 | 中 |

---

## 5. 業界先例參考

| 專案 | 原架構 | 重構結果 |
|------|--------|---------|
| Figma | C++ (Skia) → WebAssembly + React | 成功，但核心渲染仍為 C++/WASM |
| Blender | C/C++ | 從未考慮離開 C++，Python 僅用於腳本 |
| GIMP | C/GTK | 仍為 C，考慮 GTK4 但未換語言 |
| Krita | C++/Qt | 仍為 C++，Python 用於腳本外掛 |
| Inkscape | C++/GTK | 仍為 C++，GTK3→GTK4 遷移中 |
| VS Code | Electron + TS | 成功，但核心無像素級運算需求 |
| Atom → VS Code | 證明 Electron 可行於**編輯器**，不適用於**渲染密集型** |

**關鍵觀察：** 所有成功的 2D/3D 圖形應用 (Figma, Blender, Krita, GIMP) 都保留 C/C++ 作為渲染核心。即使 Figma 使用 WebAssembly，核心仍是 C++ 編譯而來。

---

## 6. 總結與建議

### 6.1 核心結論

**將 Synfig 核心重構為純 Node.js：技術上不可行。**

理由：
1. **效能差距致命**：像素級渲染運算 JS 慢 10-50x，無法達成即時預覽的基本要求
2. **關鍵依賴無替代**：FFTW3、FreeType+HarfBuzz+Pango、JACK 在 Node.js 生態系無成熟對等方案
3. **工作量不合理**：316,000+ 行 C++ 程式碼需 2-4 人年完整移植，且 Synfig 近年活躍貢獻者僅個位數
4. **零業界先例**：無任何 2D 動畫/圖形編輯器成功從 C++ 遷移到 Node.js

### 6.2 建議路徑

**短期（推薦方案 C）：** 保持 C++ 核心，在周邊擴展 Node.js 工具鏈
- Node.js 建置腳本與 CI 工具
- .sif 解析器 (TypeScript) 用於 Web 預覽
- Lottie 匯出器改用 TypeScript 重寫 (取代目前 Python)
- Plugin SDK (Node.js)

**中期（評估方案 B）：** 若決定現代化 GUI
- 將 synfig-core 解耦為獨立共享函式庫
- 建立 C API / N-API 綁定
- 以 Electron + React/Canvas 重寫 GUI
- 渲染仍透過 C++ 核心執行

**長期備選：** 若要走 Web 路線
- 將 C++ 核心編譯為 WebAssembly
- 在瀏覽器中執行 (類似 Figma 模式)
- 需大幅投資，但技術上最有前景

### 6.3 不建議事項

- 不建議將渲染引擎改用 JavaScript/TypeScript
- 不建議在 Node.js 中重新實作 152 個 ValueNode
- 不建議放棄 C++ 多執行緒渲染架構
- 不建議在沒有先解耦 synfig-core 的情況下嘗試任何混合架構

---

## 附錄 A：Node.js 可立即採用的領域

即使不重構核心，以下領域適合引入 Node.js：

| 領域 | 具體方案 | 價值 |
|------|---------|------|
| 建置系統 | 用 Node.js 腳本取代部分 shell script | 跨平台一致性 |
| .sif 解析器 | TypeScript 實作 .sif XML 讀取 | Web 預覽、工具開發 |
| Lottie 匯出 | TypeScript 重寫現有 Python 匯出器 | 效能提升、型別安全 |
| CI/CD 工具 | GitHub Actions + Node.js 腳本 | 已在進行中 |
| 文件網站 | 靜態網站生成器 (Astro, Next.js) | 社群導入 |
| Plugin Manager | npm-like 外掛管理 | 降低外掛開發門檻 |

## 附錄 B：資料來源

- 程式碼統計：直接分析 synfig/synfig repository (commit f12b9cb97)
- 效能基準：基於公開的 JavaScript vs C++ benchmark 資料 (The Benchmarks Game, V8 performance papers)
- 業界案例：各專案官方技術部落格與架構文件
