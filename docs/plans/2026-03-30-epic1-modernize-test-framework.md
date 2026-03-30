# Epic 1: 現代化測試框架 (Google Test) 實作計畫

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** 引入 Google Test 取代自訂巨集測試框架，遷移所有 17 個既有測試，整合 JUnit XML 輸出到 CI。

**Architecture:** 使用 CMake FetchContent 引入 Google Test，保留既有測試邏輯但改用 GTest 巨集。根 CMakeLists.txt 和子專案的 test/CMakeLists.txt 都需修改。CI workflow 加入 JUnit XML 上傳。

**Tech Stack:** C++11, CMake 3.14+, Google Test, CTest, GitHub Actions

---

## Task 1: 引入 Google Test 到 CMake

**Files:**
- Create: `cmake/AddGoogleTest.cmake`
- Modify: `CMakeLists.txt:1-6` (提升 cmake_minimum_required 到 3.14)
- Modify: `synfig-core/test/CMakeLists.txt`

**Step 1: 建立 Google Test CMake 模組**

建立 `cmake/AddGoogleTest.cmake`：

```cmake
include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)
```

**Step 2: 修改根 CMakeLists.txt**

在 `CMakeLists.txt` 中：
- 將 `cmake_minimum_required(VERSION 3.6)` 改為 `cmake_minimum_required(VERSION 3.14)`
- 在 `if (${ENABLE_TESTS})` 區塊內加入 `include(AddGoogleTest)`

修改後：
```cmake
if (${ENABLE_TESTS})
    include(AddGoogleTest)
    enable_testing()
endif()
```

**Step 3: 驗證建置**

```bash
cd /Users/sam/project/synfig
mkdir -p build && cd build
cmake .. -DENABLE_TESTS=ON 2>&1 | tail -20
```

Expected: Google Test 被下載並成功配置，無 CMake 錯誤。

**Step 4: Commit**

```bash
git add cmake/AddGoogleTest.cmake CMakeLists.txt
git commit -m "feat: add Google Test via FetchContent"
```

---

## Task 2: 遷移 angle.cpp 測試（示範遷移模式）

**Files:**
- Create: `synfig-core/test/gtest_angle.cpp`
- Modify: `synfig-core/test/CMakeLists.txt`

**Step 1: 建立 GTest 版本的 angle 測試**

建立 `synfig-core/test/gtest_angle.cpp`：

```cpp
#include <gtest/gtest.h>
#include <synfig/angle.h>
#include <synfig/bezier.h>
#include <cmath>

using namespace synfig;

TEST(AngleTest, DistBetweenBiggerAndSmallerAngle) {
    float dist = Angle::deg(Angle::deg(330).dist(Angle::deg(30))).get();
    EXPECT_EQ(300, static_cast<int>(floor(dist + 0.5)));
}

TEST(AngleTest, DistBetweenSmallerAndBiggerAngle) {
    float dist = Angle::deg(Angle::deg(30).dist(Angle::deg(330))).get();
    EXPECT_EQ(-300, static_cast<int>(floor(dist + 0.5)));
}

TEST(AngleTest, DistBetweenPositiveAndNegativeAngle) {
    float dist = Angle::deg(Angle::deg(-30).dist(Angle::deg(30))).get();
    EXPECT_EQ(-60, static_cast<int>(floor(dist + 0.5)));
}

TEST(AngleTest, DistBetweenNegativeAndPositiveAngle) {
    float dist = Angle::deg(Angle::deg(30).dist(Angle::deg(-30))).get();
    EXPECT_EQ(60, static_cast<int>(floor(dist + 0.5)));
}

TEST(AngleTest, DistAlmostMinus180d) {
    float dist = Angle::deg(Angle::deg(181).dist(Angle::deg(0))).get();
    EXPECT_TRUE(dist > 0);
}

TEST(AngleTest, DistLittleLessThanMinus180dDoNotChangeSignal) {
    float dist = Angle::deg(Angle::deg(179).dist(Angle::deg(0))).get();
    EXPECT_TRUE(dist > 0);
}

TEST(AngleTest, DistForMultipleRotations1) {
    for (int i = -1000; i < 1000; i++) {
        float dist = Angle::deg(Angle::deg(195 + i + 360).dist(Angle::deg(20 + i - 360))).get();
        EXPECT_EQ(895, static_cast<int>(floor(dist + 0.5)));
    }
}

TEST(AngleTest, DistForMultipleRotations2) {
    for (int i = -1000; i < 1000; i++) {
        float dist = Angle::deg(Angle::deg(20 + i - 360).dist(Angle::deg(195 + i + 360))).get();
        EXPECT_EQ(-895, static_cast<int>(floor(dist + 0.5)));
    }
}

TEST(AngleTest, AffinComboAndHermite) {
    Angle a(Angle::deg(-2005));
    Angle b(Angle::deg(200));
    affine_combo<Angle> combo;
    hermite<Angle> hermie(a, b, b.dist(a), b.dist(a));
    // 確認不會崩潰
    for (float f = 0; f < 1.001; f += 0.1) {
        combo(a, b, f);
        hermie(f);
    }
}
```

**Step 2: 修改 synfig-core/test/CMakeLists.txt**

在檔案末尾加入：

```cmake
# Google Test based tests
if (TARGET gtest_main)
    add_executable(gtest_synfig_angle gtest_angle.cpp)
    target_link_libraries(gtest_synfig_angle PRIVATE libsynfig GTest::gtest_main)
    target_include_directories(gtest_synfig_angle PRIVATE ${PROJECT_SOURCE_DIR}/src)
    add_test(NAME gtest_synfig_angle COMMAND gtest_synfig_angle)
endif()
```

**Step 3: 建置並執行測試**

```bash
cd build
cmake --build . --target gtest_synfig_angle
ctest -R gtest_synfig_angle -V
```

Expected: 9 個測試全部 PASS。

**Step 4: Commit**

```bash
git add synfig-core/test/gtest_angle.cpp synfig-core/test/CMakeLists.txt
git commit -m "test: migrate angle tests to Google Test"
```

---

## Task 3: 遷移其餘 14 個 synfig-core 測試

對每個測試檔案，按照 Task 2 的模式建立 `gtest_*.cpp`：

**Files to create:**
- `synfig-core/test/gtest_benchmark.cpp`
- `synfig-core/test/gtest_bezier.cpp`
- `synfig-core/test/gtest_bline.cpp`
- `synfig-core/test/gtest_bone.cpp`
- `synfig-core/test/gtest_clock.cpp`
- `synfig-core/test/gtest_filesystem_path.cpp`
- `synfig-core/test/gtest_handle.cpp`
- `synfig-core/test/gtest_keyframe.cpp`
- `synfig-core/test/gtest_node.cpp`
- `synfig-core/test/gtest_pen.cpp`
- `synfig-core/test/gtest_reference_counter.cpp`
- `synfig-core/test/gtest_string.cpp`
- `synfig-core/test/gtest_surface_etl.cpp`
- `synfig-core/test/gtest_valuenode_maprange.cpp`

**遷移規則：**

| 舊巨集 | GTest 巨集 |
|--------|-----------|
| `ASSERT(x)` | `EXPECT_TRUE(x)` |
| `ASSERT_FALSE(x)` | `EXPECT_FALSE(x)` |
| `ASSERT_EQUAL(a, b)` | `EXPECT_EQ(a, b)` |
| `ASSERT_NOT_EQUAL(a, b)` | `EXPECT_NE(a, b)` |
| `ASSERT_APPROX_EQUAL(a, b)` | `EXPECT_NEAR(a, b, 1e-4)` 或自訂 matcher |
| `ASSERT_APPROX_EQUAL_MICRO(a, b)` | `EXPECT_NEAR(a, b, 1e-6)` |
| `ASSERT_EXCEPTION_THROWN(E, act)` | `EXPECT_THROW(act, E)` |
| `ASSERT_NO_EXCEPTION_THROWN(act)` | `EXPECT_NO_THROW(act)` |
| `TEST_SUITE_BEGIN()` / `TEST_SUITE_END()` | 移除，改用 `TEST()` 巨集 |
| `TEST_FUNCTION(fn)` | 移除，每個 fn 變成 `TEST(Suite, Name)` |

**Step 1: 逐一遷移每個測試檔案**

每個檔案：
1. 讀取原始 `.cpp`
2. 建立 `gtest_*.cpp`，替換巨集，移除 `main()` 和 `test_base.h`
3. 在 CMakeLists.txt 加入對應 target

**Step 2: 更新 synfig-core/test/CMakeLists.txt**

在 `if (TARGET gtest_main)` 區塊內，用迴圈簡化：

```cmake
# Google Test based tests
if (TARGET gtest_main)
    set(GTEST_SOURCES
        gtest_angle
        gtest_benchmark
        gtest_bezier
        gtest_bline
        gtest_bone
        gtest_clock
        gtest_filesystem_path
        gtest_handle
        gtest_keyframe
        gtest_node
        gtest_pen
        gtest_reference_counter
        gtest_string
        gtest_surface_etl
        gtest_valuenode_maprange
    )

    foreach(TEST_NAME ${GTEST_SOURCES})
        add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
        target_link_libraries(${TEST_NAME} PRIVATE libsynfig GTest::gtest_main)
        target_include_directories(${TEST_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endforeach()

    if (NOT WIN32)
        set_target_properties(${GTEST_SOURCES}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test
        )
    endif()
endif()
```

**Step 3: 建置並執行所有 GTest 測試**

```bash
cd build
cmake --build .
ctest -R gtest_ -V
```

Expected: 所有 15 個 gtest_ 測試 PASS。

**Step 4: Commit**

```bash
git add synfig-core/test/gtest_*.cpp synfig-core/test/CMakeLists.txt
git commit -m "test: migrate all synfig-core tests to Google Test"
```

---

## Task 4: 遷移 synfig-studio 2 個測試

**Files to create:**
- `synfig-studio/test/gtest_app_layerduplicate.cpp`
- `synfig-studio/test/gtest_smach.cpp`

**Modify:** `synfig-studio/test/CMakeLists.txt`

**Step 1: 讀取原始測試並建立 GTest 版本**

遵循 Task 3 的遷移規則。

**Step 2: 更新 synfig-studio/test/CMakeLists.txt**

加入：

```cmake
if (TARGET gtest_main)
    set(GTEST_STUDIO_SOURCES
        gtest_app_layerduplicate
        gtest_smach
    )

    foreach(TEST_NAME ${GTEST_STUDIO_SOURCES})
        add_executable(${TEST_NAME} ${TEST_NAME}.cpp)
        target_link_libraries(${TEST_NAME} PRIVATE synfigapp libsynfig GTest::gtest_main)
        target_include_directories(${TEST_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/src)
        add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
    endforeach()

    if (NOT WIN32)
        set_target_properties(${GTEST_STUDIO_SOURCES}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test
        )
    endif()
endif()
```

**Step 3: 建置並驗證**

```bash
cmake --build .
ctest -R gtest_ -V
```

Expected: 所有 17 個 gtest 測試 PASS。

**Step 4: Commit**

```bash
git add synfig-studio/test/gtest_*.cpp synfig-studio/test/CMakeLists.txt
git commit -m "test: migrate synfig-studio tests to Google Test"
```

---

## Task 5: 移除舊測試框架

**Files:**
- Modify: `synfig-core/test/CMakeLists.txt` (移除舊 target)
- Modify: `synfig-studio/test/CMakeLists.txt` (移除舊 target)
- Delete: `synfig-core/test/test_base.h`

**Step 1: 從 CMakeLists.txt 移除所有舊測試 target**

synfig-core/test/CMakeLists.txt：移除所有非 gtest_ 的 add_executable/add_test/set_target_properties。
synfig-studio/test/CMakeLists.txt：同上。

**Step 2: 刪除 test_base.h**

```bash
git rm synfig-core/test/test_base.h
```

**Step 3: 保留舊測試原始碼但標記為 legacy（可選）**

或直接刪除舊的 .cpp：
```bash
git rm synfig-core/test/angle.cpp synfig-core/test/benchmark.cpp ...
git rm synfig-studio/test/app_layerduplicate.cpp synfig-studio/test/smach.cpp
```

**Step 4: 確認建置和測試通過**

```bash
cmake --build .
ctest -V
```

Expected: 只有 gtest_ 測試，全部 PASS。

**Step 5: Commit**

```bash
git commit -m "refactor: remove legacy test framework, use Google Test exclusively"
```

---

## Task 6: 設定 JUnit XML 輸出與 GitHub Actions 整合

**Files:**
- Modify: `.github/workflows/synfig-ci.yml`

**Step 1: 確認 CTest JUnit 支援**

CMake 3.21+ 的 ctest 支援 `--output-junit`。若版本較低，用 Google Test 的 `--gtest_output=xml:` 參數。

在 CMakeLists.txt 的 gtest target 中加入 test property：

```cmake
set_tests_properties(${TEST_NAME} PROPERTIES
    ENVIRONMENT "GTEST_OUTPUT=xml:${CMAKE_BINARY_DIR}/test-results/${TEST_NAME}.xml"
)
```

**Step 2: 修改 GitHub Actions workflow**

在 `.github/workflows/synfig-ci.yml` 的測試步驟後加入：

```yaml
    - name: Run tests
      run: ctest --test-dir build -V --output-junit test-results.xml
      continue-on-error: true

    - name: Upload test results
      if: always()
      uses: actions/upload-artifact@v4
      with:
        name: test-results-${{ matrix.os }}
        path: build/test-results.xml
```

**Step 3: Commit**

```bash
git add .github/workflows/synfig-ci.yml synfig-core/test/CMakeLists.txt
git commit -m "ci: add JUnit XML test reporting to GitHub Actions"
```

---

## Task 7: 最終驗證與推送

**Step 1: 完整建置與測試**

```bash
cd build
cmake .. -DENABLE_TESTS=ON
cmake --build .
ctest -V
```

Expected: 所有 17 個 GTest 測試通過，無舊測試殘留。

**Step 2: 推送到 fork**

```bash
git push fork master
```

**Step 3: 更新 Jira tickets 狀態**

將 FRON-5296 (Epic) 和 FRON-5303~5306 (Stories) 標記為 Done。
