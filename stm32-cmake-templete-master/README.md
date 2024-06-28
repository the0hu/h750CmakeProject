# 基于CMake实现自动工程管理的STM32工程样例

本仓库基于[stm32cubemx-vscode-cmake](https://github.com/Duanyll/stm32cubemx-vscode-cmake)改进完成，基于个人使用习惯进行了部分改动，并完善了使用样例。

> 本文提供了一种在 VSCode 上基于 CMake 开发 STM32CubeMX 项目的方案，配置了 Clangd 以获得更好的静态检查，并使用 Ninja 加快编译速度。本文的 CMake 配置文件能从 STM32CubeMX 生成的 Makefile 中读取编译参数，能自动同步 CubeMX 中的更改，也能在 CubeMX 重新生成项目时保留自定义选项。本文中的配置文件理论上适用于 Windows, Linux 和 macOS. 由于芯片相关信息是从 Makefile 中读取的, 本文的配置文件理论上适用于所有 STM32CubeMX 支持的 MCU。

## 使用说明

详细的使用可参考上方的原仓库链接，此处仅说明本样例新增的变化。

- 更改文件目录结构：

    应用逻辑与硬件外设分离，所有的硬件部分初始化可在`mcu/Core/Src/main.c`中进行，所有的应用逻辑都写在`app/src`中。

- 增加了编译优化等级选项：

    根据`CMake`提供的不同编译选项自动选择对应的优化等级
    |BUILD_TYPE     |Opt Level  |
    |---            |---        |
    |Debug          |-O0        |
    |Release        |-Ofast     |
    |RelWithDebInfo |-Ofast -g  |
    |MinSizeRel     |-Os        |

- 修改源文件读取方式

    修改了原本使用`RegExp`来获取`Makefile`中的.c/.h文件目录的方式
    
    可在`mcu/Core`中自由添加自己的外设驱动而无需修改`Makefile`

## 测试条件

1. 测试平台：Windows 11
2. 样例MCU：STM32H750VBT6
3. ST-Link版本：V2.J44.M29

## 补充

如有任何改进的提议，均可提出`Issue`或是`Pull request`
