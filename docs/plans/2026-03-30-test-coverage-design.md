# Synfig 測試補齊設計文件

> 日期：2026-03-30
> 狀態：已核准
> Jira 專案：FRON

---

## 背景

Synfig Studio 目前有 17 個單元測試（synfig-core 15 個、synfig-studio 2 個），使用自訂巨集框架，缺乏現代測試實踐。本設計旨在系統性地補齊測試覆蓋率。

## 目標

- 引入 Google Test 現代化測試框架
- 建立測試基礎設施（夾具、golden image 比對）
- 補齊核心功能測試（檔案 I/O、渲染、匯出）
- 達成 80% 測試覆蓋率目標
- 完善 CI 品質門檻與測試文件

## 設計決策

### 砍掉的項目

| 原項目 | 理由 |
|--------|------|
| GUI/UI 測試 | C++ GTK 桌面應用的 UI 自動化測試工具不成熟，ROI 極低 |
| E2E 工作流程測試（獨立） | 併入渲染與匯出測試，用 CLI 渲染管線取代 UI 模擬 |
| CI 報告獨立 Epic | Google Test 原生支援 JUnit XML，併入框架遷移 |

---

## 階段規劃

### 第一階段：基礎建設

#### Epic 1：現代化測試框架
- **branch**: `test/modernize-framework`
- Story 1.1: 引入 Google Test 到 CMake 建置系統
- Story 1.2: 遷移 synfig-core 15 個既有測試到 Google Test
- Story 1.3: 遷移 synfig-studio 2 個既有測試到 Google Test
- Story 1.4: 設定 CTest + JUnit XML 輸出，整合到 GitHub Actions

#### Epic 2：測試基礎設施
- **branch**: `test/test-infrastructure`
- Story 2.1: 建立測試夾具目錄與範例 .sif/.sifz 檔案
- Story 2.2: 建立 golden image 比對工具（像素差異容許值）
- Story 2.3: 建立 pytest 環境用於 Python 匯出器測試

### 第二階段：核心功能測試

#### Epic 3：檔案 I/O 測試
- **branch**: `test/file-io`
- Story 3.1: 測試 .sif XML 解析與載入
- Story 3.2: 測試 .sifz 壓縮檔讀寫往返一致性
- Story 3.3: 測試損壞／格式錯誤檔案的錯誤處理
- Story 3.4: 測試自動恢復檔案的讀取

#### Epic 4：渲染與匯出測試
- **branch**: `test/render-export`
- Story 4.1: 基本形狀渲染正確性（圓形、矩形、星形 → golden image 比對）
- Story 4.2: 圖層混合模式渲染驗證
- Story 4.3: PNG/JPEG/SVG 匯入匯出往返測試
- Story 4.4: FFmpeg 影片匯出驗證（檔案格式與幀數正確性）

#### Epic 5：Python 匯出器測試
- **branch**: `test/python-exporters`
- Story 5.1: Lottie JSON 匯出結構與語意正確性（pytest）
- Story 5.2: SVG 匯出器輸出驗證
- Story 5.3: 整合到 CI pipeline

### 第三階段：品質保證

#### Epic 6：覆蓋率與 CI 品質門檻
- **branch**: `test/coverage-ci`
- Story 6.1: 整合 gcov/lcov 到 CMake
- Story 6.2: GitHub Actions 產出覆蓋率 HTML 報告
- Story 6.3: 設定覆蓋率門檻（初期 50%，目標 80%）

#### Epic 7：測試文件
- **branch**: `test/testing-docs`
- Story 7.1: 撰寫 TESTING.md（如何跑測試、如何寫新測試、框架說明）
- Story 7.2: 撰寫貢獻者測試指南（測試命名規範、夾具使用方式）

## 依賴關係

```
Stage 1: Epic 1 + Epic 2（可平行）
    ↓
Stage 2: Epic 3 + Epic 4 + Epic 5（可平行，依賴 Stage 1）
    ↓
Stage 3: Epic 6 + Epic 7（依賴 Stage 2）
```

## 總計

- 7 個 Epic、23 個 Story
- 7 個 git branch
- 3 個階段，有明確依賴順序
