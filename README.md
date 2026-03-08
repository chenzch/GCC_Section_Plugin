# GCC Section Plugin

一个用于 ARM Cortex-M/MCU 开发场景的 GCC 插件，通过 `#pragma GCC section` 语法控制函数和变量的链接段。

## 功能特性

- 支持四种段类型：`text`、`data`、`bss`、`rodata`
- 支持自定义段名（如 `.sec1`、`.data1`）
- 支持恢复默认段（使用 `default` 关键字）
- 支持 `-ffunction-sections` 和 `-fdata-sections` 时的名称追加

## 使用方法

### 编译插件

```bash
make
```

### 安装插件

```bash
sudo make install    # 安装到 GCC 插件目录
```

### 测试

```bash
make test
```

### 清理

```bash
make clean
```

## 语法说明

```c
#pragma GCC section text "<section_name>"
#pragma GCC section data "<section_name>"
#pragma GCC section bss "<section_name>"
#pragma GCC section rodata "<section_name>"

// 恢复默认段
#pragma GCC section text
#pragma GCC section data
```

### 参数说明

| 参数 | 说明 |
|------|------|
| `text` | 代码段 |
| `data` | 初始化数据段 |
| `bss` | 未初始化数据段 |
| `rodata` | 只读数据段 |
| `default` | 恢复默认段名 |

### 示例

```c
#pragma GCC section text ".sec1"
#pragma GCC section data ".data1"
#pragma GCC section bss ".bss1"
#pragma GCC section rodata ".rodata1"

int fun1(int a, int b) {
    const int base1 = 3;
    return base1 + a - b;
}

static int ga;          // -> .bss1.ga
static int gb = 4;      // -> .data1.gb
static const int gc = 3; // -> .rodata1.gc

// 恢复默认段
#pragma GCC section text
// ...
```

## 编译参数

插件支持以下 GCC 编译选项的配合使用：

- `-ffunction-sections` - 每个函数单独一个段
- `-fdata-sections` - 每个变量单独一个段
- `-flto` - 链接时优化
- `-Wl,--gc-sections` - 移除未使用段

## 环境要求

- GCC ARM Embedded Toolchain (`arm-none-eabi-gcc`)
- Linux/macOS (需要 `sudo` 权限安装插件)

## 许可证

GPL 兼容
