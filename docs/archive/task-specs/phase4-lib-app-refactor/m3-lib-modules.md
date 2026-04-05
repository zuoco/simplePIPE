# M3 lib 模块化

## T63 为 lib/base 建立第一批模块接口单元

**目标**
为 `types`、`signal`、`log` 等提供模块接口单元。

**依赖**
- T58

**交付物**
- base 层 `.cppm` 接口单元
- 兼容头

**验收标准**
- `lib/base` 至少一组模块可被独立 import

## T64 为 lib/platform 建立 facade 模块

**目标**
收口 OCCT、VSG、VTK 的公开接口面。

**依赖**
- T59
- T60
- T63

**交付物**
- `pipecad.platform.occt`
- `pipecad.platform.vsg`
- `pipecad.platform.vtk`

**验收标准**
- apps 不直接依赖第三方头
- 第三方访问路径统一经过 `lib/platform`

## T65 为 lib/runtime 建立核心模块

**目标**
为 `document`、`graph`、`command`、`task`、`serialize` 建立运行时模块边界。

**依赖**
- T61
- T63

**交付物**
- runtime 模块接口单元

**验收标准**
- apps 可通过 import 或公开头访问 `lib/runtime`

## T66 为 lib/framework 建立框架模块

**目标**
完成 `application`、`workbench`、`scene` 的框架模块边界定义。

**依赖**
- T60
- T61
- T65

**交付物**
- framework 模块接口单元

**验收标准**
- `Application`、`Workbench`、`Scene` 具备稳定公开接口