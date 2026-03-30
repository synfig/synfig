# Synfig 測試指南

## 概述

Synfig 專案使用以下測試框架與工具：

- **Google Test (v1.14.0)** — C++ 單元測試與整合測試，透過 CMake FetchContent 自動下載
- **pytest** — Python 匯出器外掛測試
- **gcov / lcov** — 程式碼覆蓋率分析與報告產生
- **CTest** — CMake 內建的測試執行器，統一管理所有 C++ 測試

## 環境設定

### 建置並啟用測試

```bash
mkdir build && cd build
cmake -GNinja -DENABLE_TESTS=ON -DCMAKE_BUILD_TYPE=Release ..
ninja
```

`ENABLE_TESTS=ON` 會透過 `cmake/AddGoogleTest.cmake` 自動下載 Google Test v1.14.0，並編譯所有測試執行檔。

### Python 測試環境

```bash
pip3 install pytest
```

## 執行測試

### 執行所有 C++ 測試

```bash
cd build
ctest --output-on-failure
```

### 執行特定 C++ 測試

透過 CTest 名稱篩選：

```bash
ctest --output-on-failure -R gtest_render_basic
```

或直接執行測試二進位檔：

```bash
./bin/test/gtest_render_basic
```

Google Test 也支援用 `--gtest_filter` 執行單一測試案例：

```bash
./bin/test/gtest_golden_image --gtest_filter=GoldenImageTest.SimilarSurfacesHaveHighPSNR
```

### 執行 Python 測試

```bash
cd synfig-studio/plugins/lottie-exporter
pytest tests/ -v --tb=short
```

執行特定 Python 測試：

```bash
pytest tests/test_lottie_basic.py -v
pytest tests/test_lottie_basic.py::test_canvas_module_exists -v
```

## 測試結構

```
synfig/
├── synfig-core/test/           # 核心引擎 C++ 測試
│   ├── CMakeLists.txt          # 測試建置設定
│   ├── test_utils.h            # 測試工具（PSNR、golden image 比對）
│   ├── fixtures/               # 測試夾具檔案（.sif）
│   │   ├── basic_shapes.sif
│   │   ├── animation_keyframes.sif
│   │   └── gradient.sif
│   ├── gtest_angle.cpp         # 角度運算測試
│   ├── gtest_bezier.cpp        # 貝茲曲線測試
│   ├── gtest_render_basic.cpp  # 基礎渲染測試
│   ├── gtest_render_blend.cpp  # 混合模式渲染測試
│   ├── gtest_golden_image.cpp  # Golden image 比對測試
│   ├── gtest_file_io_sif.cpp   # SIF 檔案讀寫測試
│   ├── gtest_file_io_sifz.cpp  # SIFZ 壓縮檔讀寫測試
│   ├── gtest_export_format.cpp # 匯出格式測試
│   └── ...
├── synfig-studio/test/         # Studio 應用 C++ 測試
│   ├── CMakeLists.txt
│   ├── test_base.h             # 舊式測試巨集（相容用途）
│   ├── gtest_app_layerduplicate.cpp
│   └── gtest_smach.cpp
└── synfig-studio/plugins/lottie-exporter/tests/  # Python 匯出器測試
    ├── conftest.py             # pytest fixtures 定義
    ├── test_lottie_basic.py    # Lottie 匯出基礎測試
    ├── test_lottie_structure.py
    └── test_svg_export.py
```

## 撰寫新測試

### C++ 測試範例

使用 Google Test 的 `TEST` 巨集撰寫基本測試：

```cpp
#include <gtest/gtest.h>
#include <synfig/angle.h>

TEST(AngleTest, DegreesToRadians) {
    synfig::Angle::deg d(180.0);
    synfig::Angle::rad r(d);
    EXPECT_NEAR(r.get(), M_PI, 1e-6);
}

TEST(AngleTest, ZeroAngle) {
    synfig::Angle::deg d(0.0);
    EXPECT_EQ(d.get(), 0.0);
}
```

#### 需要 synfig::Main 初始化的測試

涉及 Canvas 載入、圖層操作或渲染的測試需要初始化 `synfig::Main`。使用 Test Fixture 模式：

```cpp
#include <gtest/gtest.h>
#include <synfig/main.h>
#include <synfig/canvas.h>
#include <synfig/loadcanvas.h>
#include "test_utils.h"

using namespace synfig;

class RenderTest : public ::testing::Test {
protected:
    static std::unique_ptr<Main> main_instance;

    static void SetUpTestSuite() {
        if (!main_instance)
            main_instance = std::make_unique<Main>(".");
    }

    Canvas::Handle load_fixture(const std::string& filename) {
        String errors, warnings;
        std::string path = test::fixture_path(filename);
        // 載入並回傳 Canvas...
    }
};

std::unique_ptr<Main> RenderTest::main_instance;

TEST_F(RenderTest, LoadsBasicShapes) {
    auto canvas = load_fixture("basic_shapes.sif");
    ASSERT_NE(canvas, nullptr);
}
```

#### 使用 test_utils.h 的 Golden Image 比對

`test_utils.h` 提供影像比對工具函式：

```cpp
#include <gtest/gtest.h>
#include "test_utils.h"
#include <synfig/surface.h>
#include <synfig/color.h>

using namespace synfig;

TEST(GoldenImageTest, SimilarSurfacesMatch) {
    Surface a(64, 64);
    Surface b(64, 64);
    a.fill(Color(1.0f, 0.0f, 0.0f, 1.0f));
    b.fill(Color(0.99f, 0.01f, 0.01f, 1.0f));

    // compute_psnr() 計算兩個 Surface 的峰值信噪比
    double psnr = test::compute_psnr(a, b);
    EXPECT_GT(psnr, 30.0);

    // surfaces_match() 是便捷函式，預設門檻 30 dB
    EXPECT_TRUE(test::surfaces_match(a, b));

    // 也可自訂門檻
    EXPECT_TRUE(test::surfaces_match(a, b, 25.0));
}
```

#### 將新測試加入建置系統

在 `synfig-core/test/CMakeLists.txt` 的 `GTEST_SOURCES` 清單中加入新測試（不含副檔名）：

```cmake
set(GTEST_SOURCES
    # ... 現有測試 ...
    gtest_your_new_test    # 加入此行
)
```

Studio 測試則編輯 `synfig-studio/test/CMakeLists.txt` 的 `GTEST_STUDIO_SOURCES`。

### Python 測試範例

```python
import os


def test_exporter_script_exists(exporter_dir):
    """驗證主要匯出腳本存在。"""
    assert os.path.isfile(os.path.join(exporter_dir, "lottie-exporter.py"))


def test_output_structure(exporter_dir):
    """驗證匯出器目錄結構完整。"""
    required_dirs = ["properties", "layers", "shapes", "common", "effects"]
    for dirname in required_dirs:
        assert os.path.isdir(os.path.join(exporter_dir, dirname))
```

`conftest.py` 中定義的 `exporter_dir` 和 `fixtures_dir` fixture 會自動注入。

## 程式碼覆蓋率

### 啟用覆蓋率並產生報告

```bash
mkdir build-coverage && cd build-coverage
cmake -GNinja \
    -DENABLE_TESTS=ON \
    -DENABLE_COVERAGE=ON \
    -DCMAKE_BUILD_TYPE=Debug ..
ninja
```

產生覆蓋率報告：

```bash
# 使用自訂 CMake target（自動執行測試並產生報告）
make coverage
```

或手動執行：

```bash
# 清除計數器
lcov --directory . --zerocounters

# 執行測試
ctest --output-on-failure

# 擷取覆蓋率資料
lcov --directory . --capture --output-file coverage.info

# 過濾測試程式碼與第三方程式庫
lcov --remove coverage.info \
    '*/test/*' '*/tests/*' '*/googletest/*' '*/usr/*' '*/opt/*' \
    --output-file coverage.info.cleaned

# 產生 HTML 報告
genhtml coverage.info.cleaned \
    --output-directory coverage-report \
    --title "Synfig Test Coverage"
```

報告位於 `build-coverage/coverage-report/index.html`。

### 注意事項

- 覆蓋率必須使用 `Debug` 建置類型，`Release` 最佳化可能導致不準確的結果
- 需要安裝 `lcov` 和 `genhtml`（macOS: `brew install lcov`，Ubuntu: `apt install lcov`）

## CI 整合

CI 工作流程定義於 `.github/workflows/synfig-ci.yml`，包含：

### JUnit XML 測試報告

透過環境變數 `GTEST_OUTPUT` 讓 Google Test 產生 JUnit XML 報告：

```yaml
env:
  GTEST_OUTPUT: xml:${{ github.workspace }}/test-results/
```

測試結果會上傳為 CI artifacts (`test-results-<os>`)。

### 覆蓋率報告

CI 在 Debug 模式下建置並執行測試，自動產生覆蓋率報告並上傳為 artifacts (`coverage-report-<os>`)。

### 覆蓋率門檻

CI 會檢查行覆蓋率是否達到最低門檻，未達標準時建置失敗。

### Python 測試

CI 自動安裝 pytest 並執行 Lottie 匯出器測試：

```yaml
- name: Run Python exporter tests
  run: |
    pip3 install pytest
    cd synfig-studio/plugins/lottie-exporter
    pytest tests/ -v --tb=short
```
