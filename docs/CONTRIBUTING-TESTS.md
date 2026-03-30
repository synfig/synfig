# 貢獻者測試指南

本文件說明為 Synfig 專案撰寫測試時應遵循的規範與最佳實踐。

## 測試命名規範

### C++ 測試

**檔案命名：** `gtest_<module>.cpp`

範例：
- `gtest_angle.cpp` — 角度運算模組測試
- `gtest_render_basic.cpp` — 基礎渲染測試
- `gtest_file_io_sif.cpp` — SIF 檔案 I/O 測試

**測試案例命名：** `TEST(SuiteName, TestName)` 或 `TEST_F(FixtureName, TestName)`

Suite 名稱使用 PascalCase，描述被測模組；Test 名稱描述預期行為：

```cpp
// 基本測試
TEST(AngleTest, DegreesToRadiansConversion) { ... }
TEST(BezierTest, EmptyCurveHasZeroLength) { ... }

// 使用 Fixture 的測試
TEST_F(RenderBasicTest, LoadsFixtureCanvas) { ... }
TEST_F(FileIOTest, HandlesCorruptedFileGracefully) { ... }
```

命名原則：
- Suite 名稱 = 被測模組 + `Test` 後綴（如 `AngleTest`、`GoldenImageTest`）
- Test 名稱應描述「預期行為」而非「測試動作」
- 使用正面語句：`IdenticalSurfacesHaveInfinitePSNR` 優於 `TestPSNRForIdenticalSurfaces`

### Python 測試

**檔案命名：** `test_<module>.py`

範例：
- `test_lottie_basic.py` — Lottie 匯出基礎驗證
- `test_lottie_structure.py` — Lottie 結構測試
- `test_svg_export.py` — SVG 匯出測試

**函式命名：** `def test_<behavior>():`

```python
def test_exporter_script_exists(exporter_dir):
    """驗證主要匯出腳本存在。"""
    ...

def test_canvas_module_exists(exporter_dir):
    """驗證 canvas.py 模組存在。"""
    ...
```

命名原則：
- 使用小寫加底線（snake_case）
- 函式名稱描述被驗證的行為
- 每個函式附帶 docstring 說明測試目的

## 測試夾具使用

### C++ 測試夾具

`.sif` 測試夾具檔案放在 `synfig-core/test/fixtures/` 目錄：

```
synfig-core/test/fixtures/
├── basic_shapes.sif         # 基本形狀圖層
├── animation_keyframes.sif  # 動畫關鍵幀
└── gradient.sif             # 漸層測試
```

CMake 建置時會自動將 fixtures 複製到建置目錄。在測試中使用 `test_utils.h` 提供的 `fixture_path()` 取得路徑：

```cpp
#include "test_utils.h"

TEST_F(FileIOTest, LoadsBasicShapesFixture) {
    std::string path = synfig::test::fixture_path("basic_shapes.sif");
    ASSERT_TRUE(synfig::test::file_exists(path));
    // 載入並驗證...
}
```

新增夾具檔案時：
1. 將 `.sif` 檔放入 `synfig-core/test/fixtures/`
2. 無需修改 CMakeLists.txt（使用 glob 自動複製）
3. 在 `fixtures/README.md` 中記錄該夾具的用途

### Python 測試夾具

使用 `conftest.py` 定義 pytest fixtures：

```python
# conftest.py
import os
import pytest

@pytest.fixture
def fixtures_dir():
    """回傳測試夾具目錄路徑。"""
    return os.path.join(os.path.dirname(__file__), "fixtures")

@pytest.fixture
def exporter_dir():
    """回傳 lottie-exporter 外掛目錄路徑。"""
    return os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
```

在測試函式的參數中宣告 fixture 名稱即可自動注入：

```python
def test_something(exporter_dir, fixtures_dir):
    # exporter_dir 和 fixtures_dir 由 conftest.py 自動提供
    ...
```

## 測試分類

### 單元測試

不需要 `synfig::Main` 初始化，測試純粹的邏輯運算：

```cpp
// 數學運算、資料結構、字串處理等
TEST(AngleTest, ...) { ... }
TEST(BezierTest, ...) { ... }
TEST(HandleTest, ...) { ... }
TEST(StringTest, ...) { ... }
TEST(FilesystemPathTest, ...) { ... }
```

這些測試直接使用 `TEST()` 巨集，執行速度快。

### 整合測試（需要 synfig::Main）

涉及以下功能的測試需要初始化 `synfig::Main`：

- Canvas 載入與儲存
- 圖層建立與操作
- 渲染管線
- 匯出功能
- ValueNode 計算

使用 `TEST_F()` 搭配 Fixture class：

```cpp
class MyIntegrationTest : public ::testing::Test {
protected:
    static std::unique_ptr<synfig::Main> main_instance;

    static void SetUpTestSuite() {
        if (!main_instance)
            main_instance = std::make_unique<synfig::Main>(".");
    }
};

std::unique_ptr<synfig::Main> MyIntegrationTest::main_instance;

TEST_F(MyIntegrationTest, CanLoadAndRender) { ... }
```

使用 `SetUpTestSuite()`（而非 `SetUp()`）確保 `synfig::Main` 只初始化一次。

### Studio 應用測試

`synfig-studio/test/` 中的測試連結 `synfigapp` 程式庫，可測試應用層邏輯：

```cpp
// 在 CMakeLists.txt 中連結 synfigapp
target_link_libraries(${TEST_NAME} PRIVATE synfigapp libsynfig GTest::gtest_main)
```

## Golden Image 測試指南

### 核心工具函式

`synfig-core/test/test_utils.h` 提供以下函式：

| 函式 | 說明 |
|------|------|
| `compute_psnr(a, b)` | 計算兩個 Surface 的 PSNR（峰值信噪比），單位 dB |
| `surfaces_match(a, b, threshold)` | 判斷兩個 Surface 是否視覺相似 |
| `fixture_path(filename)` | 取得測試夾具檔案路徑 |
| `file_exists(path)` | 檢查檔案是否存在 |

### PSNR 回傳值意義

| 回傳值 | 意義 |
|--------|------|
| `infinity` | 兩個 Surface 完全相同 |
| `> 40 dB` | 極為相似，肉眼無法分辨差異 |
| `30 ~ 40 dB` | 相似，可能有微小差異 |
| `20 ~ 30 dB` | 有可見差異 |
| `< 20 dB` | 明顯不同 |
| `0` | 尺寸不同或空白 Surface |

### PSNR 門檻建議

- **精確像素比對：** 使用 `std::isinf(psnr)` 確認完全一致
- **渲染正確性驗證：** 使用預設門檻 `30.0 dB`（`surfaces_match(a, b)`）
- **跨平台容差：** 考慮降至 `25.0 dB`（不同編譯器/平台可能有浮點差異）

```cpp
// 精確比對
EXPECT_TRUE(std::isinf(test::compute_psnr(expected, actual)));

// 標準比對（30 dB）
EXPECT_TRUE(test::surfaces_match(expected, actual));

// 寬鬆比對（跨平台）
EXPECT_TRUE(test::surfaces_match(expected, actual, 25.0));
```

### 撰寫 Golden Image 測試步驟

1. 建立或載入參考 Surface
2. 執行渲染或處理操作產生結果 Surface
3. 使用 `surfaces_match()` 比對兩者

```cpp
TEST_F(RenderTest, CircleLayerRendersCorrectly) {
    auto canvas = load_fixture("basic_shapes.sif");
    ASSERT_NE(canvas, nullptr);

    Surface result = render(canvas);
    Surface expected = load_reference("basic_shapes_expected.png");

    EXPECT_TRUE(test::surfaces_match(result, expected, 30.0))
        << "PSNR: " << test::compute_psnr(result, expected) << " dB";
}
```

## 提交前檢查清單

在提交包含測試的變更前，請確認：

- [ ] **測試通過：** `ctest --output-on-failure` 所有測試通過
- [ ] **命名正確：** 檔案名稱遵循 `gtest_<module>.cpp` / `test_<module>.py` 規範
- [ ] **CMake 已更新：** 新的 C++ 測試已加入對應的 `CMakeLists.txt`
- [ ] **夾具已提交：** 新增的 `.sif` 夾具檔案已加入 `fixtures/` 目錄
- [ ] **無硬編碼路徑：** 使用 `fixture_path()` 而非寫死的絕對路徑
- [ ] **Main 初始化正確：** 需要 `synfig::Main` 的測試使用 `SetUpTestSuite()`
- [ ] **Python 測試通過：** `pytest tests/ -v` 所有測試通過
- [ ] **覆蓋率未下降：** 新增程式碼有對應的測試覆蓋
- [ ] **測試獨立：** 每個測試案例可獨立執行，不依賴其他測試的執行順序
- [ ] **清理資源：** 測試產生的暫存檔案在結束後清理
