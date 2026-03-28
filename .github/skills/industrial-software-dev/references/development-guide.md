# 工业软件开发环境配置与工具指南

## 目录
- [概览](#概览)
- [开发环境配置](#开发环境配置)
- [IDE与工具配置](#ide与工具配置)
- [项目脚手架](#项目脚手架)
- [调试与测试](#调试与测试)
- [部署与运维](#部署与运维)
- [最佳实践](#最佳实践)

## 概览
本指南面向使用Claude Code、Cursor等AI辅助开发工具的开发者，提供工业软件开发的环境配置、工具使用和最佳实践建议。

## 开发环境配置

### Python开发环境

#### 基础环境搭建
```bash
# 安装Python 3.9+
# 推荐使用pyenv管理Python版本
curl https://pyenv.run | bash
pyenv install 3.9.18
pyenv global 3.9.18

# 创建虚拟环境
python -m venv venv
source venv/bin/activate  # Linux/Mac
# 或
venv\Scripts\activate  # Windows

# 升级pip
pip install --upgrade pip
```

#### 依赖管理
```bash
# 安装依赖
pip install -r requirements.txt

# 开发依赖
pip install -r requirements-dev.txt

# 依赖锁定
pip freeze > requirements-lock.txt

# 依赖可视化
pip install pipdeptree
pipdeptree
```

#### 推荐开发依赖
```txt
# requirements-dev.txt
pytest>=7.4.0
pytest-cov>=4.1.0
pytest-asyncio>=0.21.0
black>=23.0.0
flake8>=6.0.0
mypy>=1.5.0
pre-commit>=3.5.0
```

### C++开发环境

#### 基础环境搭建 (Linux)
```bash
# 安装编译工具链
sudo apt-get update
sudo apt-get install build-essential cmake git

# 安装OpenCASCADE依赖
sudo apt-get install liboce-* oce-draw

# 或从源码编译OpenCASCADE
git clone https://github.com/Open-Cascade-SAS/OCCT.git
cd OCCT
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
```

#### Windows环境
```bash
# 使用vcpkg管理C++依赖
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.bat

# 安装OpenCASCADE
./vcpkg install opencascade:x64-windows

# 使用Visual Studio作为IDE
```

### Docker开发环境

#### Dockerfile示例
```dockerfile
# Python开发环境
FROM python:3.9-slim

# 安装系统依赖
RUN apt-get update && apt-get install -y \
    gcc \
    g++ \
    cmake \
    libfreetype6-dev \
    libxft-dev \
    && rm -rf /var/lib/apt/lists/*

# 设置工作目录
WORKDIR /app

# 复制依赖文件
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# 复制代码
COPY . .

# 暴露端口
EXPOSE 8000

# 启动命令
CMD ["uvicorn", "main:app", "--host", "0.0.0.0", "--port", "8000"]
```

#### docker-compose.yml
```yaml
version: '3.8'

services:
  app:
    build: .
    ports:
      - "8000:8000"
    volumes:
      - .:/app
    environment:
      - DATABASE_URL=postgresql://user:pass@db:5432/industrial
    depends_on:
      - db
      - redis
      - influxdb

  db:
    image: postgres:13
    environment:
      POSTGRES_USER: user
      POSTGRES_PASSWORD: pass
      POSTGRES_DB: industrial
    volumes:
      - postgres_data:/var/lib/postgresql/data

  redis:
    image: redis:7-alpine
    ports:
      - "6379:6379"

  influxdb:
    image: influxdb:2.7
    ports:
      - "8086:8086"
    environment:
      - DOCKER_INFLUXDB_INIT_MODE=setup
      - DOCKER_INFLUXDB_INIT_USERNAME=admin
      - DOCKER_INFLUXDB_INIT_PASSWORD=password
      - DOCKER_INFLUXDB_INIT_ORG=my-org
      - DOCKER_INFLUXDB_INIT_BUCKET=my-bucket
    volumes:
      - influxdb_data:/var/lib/influxdb2

volumes:
  postgres_data:
  influxdb_data:
```

#### 启动开发环境
```bash
# 构建并启动
docker-compose up -d

# 查看日志
docker-compose logs -f app

# 进入容器
docker-compose exec app bash

# 停止服务
docker-compose down

# 清理数据
docker-compose down -v
```

## IDE与工具配置

### Cursor 配置

#### .cursorrules — C++ 工业软件（OCCT + OSG + VTK）

在 C++ 工业软件项目根目录创建 `.cursorrules`：

```
# 工业软件 C++ 开发规范

## 项目技术栈
- 几何内核: OpenCASCADE (OCCT) 7.x C++ API
- 3D 渲染: OpenSceneGraph (OSG) 3.6.x
- 科学可视化: VTK 9.x
- 构建: CMake 3.20+ + vcpkg
- 标准: C++17
- GUI: Qt5/Qt6

## OCCT 编码规范
- 所有继承自 Standard_Transient 的对象必须使用 Handle<T> 管理，严禁裸指针或 std::shared_ptr
- 异常使用 Standard_Failure 及其子类，用 try { } catch (Standard_Failure& e) { } 捕获
- 几何精度比较使用 Precision::Confusion()（位置）、Precision::Angular()（角度）
- BRep 操作后必须检查 IsDone() 返回值
- 网格剖分使用 BRepMesh_IncrementalMesh，偏差值根据模型尺寸合理设置
- 多线程访问 OCCT 对象需加互斥锁（OCCT 核心非线程安全）

## OSG 编码规范
- 所有 osg::Referenced 子类必须使用 osg::ref_ptr<T> 管理，防止内存泄漏
- 场景图修改在渲染线程外需通过 UpdateCallback 或 OperationThread 提交
- 使用 osgUtil::Optimizer 对静态场景做批次合并和 StateSet 共享
- LOD 节点设置合理切换距离，大场景必须启用视锥裁剪
- 文件加载使用 osgDB::readNodeFile，格式插件自动注册

## VTK 编码规范
- 使用 vtkSmartPointer<T> 管理 VTK 对象生命周期
- Pipeline 修改后调用 Modified() + Update() 触发重算
- 大数据集使用 vtkStreamingDemandDrivenPipeline 流式处理
- 渲染窗口更新调用 Render()，交互器使用 vtkRenderWindowInteractor

## 通用 C++ 规范
- 类名 PascalCase，函数名 camelCase，成员变量前缀 m_，常量全大写加下划线
- 所有公开 API 写 Doxygen 注释（@brief/@param/@return/@throws）
- 使用 C++17 特性：std::optional、std::variant、if constexpr、结构化绑定
- 几何/渲染相关的耗时操作放入后台线程，通过信号/回调通知 UI
- 每个模块提供对应的单元测试（Google Test）

## 架构规范
- 三层严格分离：GeometryEngine（OCCT）/ RenderEngine（OSG或VTK）/ UILayer（Qt）
- 上层通过抽象接口访问底层，不直接依赖具体库类型
- 插件/模块通过接口注册，参考 FreeCAD Workbench 机制
- 文档对象支持 Undo/Redo，参考 OCAF（TDocStd）或自实现 Command 模式
```

#### .cursorrules — Python 工业软件（IoT/数据平台）

```
# 工业软件 Python 开发规范

## 项目技术栈
- Web 框架: FastAPI
- 工业协议: pymodbus / opcua-asyncio / paho-mqtt
- 时序数据库: InfluxDB
- 关系数据库: PostgreSQL + SQLAlchemy
- 缓存: Redis
- 任务队列: Celery
- 可视化: Dash / PyQt5
- CAD/几何: pythonocc-core / vtk / open3d

## 编码规范
- 所有函数必须有 Type Hints 标注
- 遵循 PEP 8，行长 ≤ 88（black 格式化）
- 每个模块/类/公开函数有 docstring（Google 风格）
- 异常处理要具体，严禁裸 except，至少捕获 Exception
- 敏感信息（密码/Token/密钥）只从环境变量读取，严禁硬编码

## 架构规范
- 分层：API Router → Service → Repository → Model
- 协议适配独立在 protocols/ 目录，通过统一接口对外
- 异步 IO 优先（async/await），避免在异步上下文中调用阻塞操作
- 配置统一通过 pydantic-settings 管理

## 工业协议规范
- Modbus 连接异常自动重连，超时和重试次数可配置
- OPC UA 连接使用证书认证（生产环境）
- MQTT 使用 QoS 1 或 2 保证消息可靠性
- 数据采集支持批量读取，减少通信次数

## 性能规范
- 批量写入时序数据（InfluxDB write_api BATCHING 模式）
- 热点数据（设备状态）缓存到 Redis，TTL 合理设置
- 高频采集任务使用 asyncio.gather 并发
- 长连接（Modbus/OPC UA）复用连接，不要每次重新建立
```

#### .cursorrules — 混合项目（C++ 核心 + Python 脚本）

```
# 混合工业软件开发规范（C++ + Python）

## 架构边界
- C++ 负责: 几何计算（OCCT）、3D 渲染（OSG/VTK）、性能关键路径
- Python 负责: 业务逻辑、配置管理、数据处理、脚本自动化、测试
- 绑定层: pybind11 暴露 C++ 核心功能给 Python

## pybind11 规范
- 暴露的函数提供详细的 docstring（Python 用户可见）
- 注意 GIL（全局解释器锁）：C++ 长时间计算用 py::gil_scoped_release 释放
- OCCT Handle<T> 在 Python 侧映射为智能指针，避免内存问题
- 提供 Python 类型的工厂函数，隐藏 C++ 构造细节

## 构建规范
- CMakeLists.txt 管理 C++ 构建，setup.py / pyproject.toml 管理 Python 包
- CI/CD 分别测试 C++ 单元测试（ctest）和 Python 测试（pytest）
```

### Claude Code 配置

#### CLAUDE.md（项目根目录）

在项目根目录创建 `CLAUDE.md`，Claude Code 会自动读取：

```markdown
# 项目说明（供 Claude Code 读取）

## 项目概述
工业 CAD 软件，基于 OpenCASCADE 几何内核 + OpenSceneGraph 渲染引擎 + Qt5 GUI

## 技术栈
- 几何内核: OCCT 7.8, pythonocc-core 7.7
- 渲染: OSG 3.6
- GUI: Qt 5.15
- 构建: CMake 3.25 + vcpkg
- 测试: Google Test (C++) + pytest (Python)

## 构建命令
```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release -j$(nproc)
ctest --test-dir build
```

## 常用提示词模板

### OCCT API 查询
查阅 OpenCASCADE 7.8 API，帮我实现 [功能]，要求使用 Handle<T> 管理内存，处理 Standard_Failure 异常

### OSG 场景查询
分析 OpenSceneGraph 源码中的 [功能模块]，帮我实现基于 NodeVisitor 的 [具体功能]

### VTK Pipeline 设计
使用 VTK 9.x Python API 实现 [可视化功能]，采用 Pipeline 架构，支持交互操作

### 参考开源源码
参考 FreeCAD 的 [src/Mod/Part] 模块源码实现思路，帮我设计 [类似功能] 的 C++ 实现
```

### GitHub 源码阅读工作流

#### 阅读 OCCT 源码
```bash
# 克隆源码
git clone https://github.com/Open-Cascade-SAS/OCCT.git
cd OCCT

# 查找特定 API 实现
grep -r "BRepAlgoAPI_Cut" src/ --include="*.cxx" -l

# 查看布尔运算实现
# 建议直接在 Cursor 中打开 src/BRepAlgoAPI/ 目录

# 运行官方示例
cd samples
cmake -B build && cmake --build build
```

**OCCT 模块快速定位表**：

| 需求 | 查看源码路径 |
|------|-------------|
| 基础几何（点/线/面） | `src/gp/`, `src/Geom/` |
| 拓扑数据结构 | `src/TopoDS/`, `src/BRep/` |
| 形状构建 | `src/BRepBuilderAPI/` |
| 布尔运算 | `src/BRepAlgoAPI/`, `src/BOPAlgo/` |
| 圆角/倒角 | `src/BRepFilletAPI/` |
| 网格剖分 | `src/BRepMesh/` |
| 可视化/交互 | `src/AIS/`, `src/V3d/` |
| STEP 读写 | `src/STEPControl/`, `src/STEPCAFControl/` |
| 文档管理（OCAF） | `src/TDocStd/`, `src/XCAFDoc/` |

#### 阅读 FreeCAD 源码
```bash
git clone https://github.com/FreeCAD/FreeCAD.git
cd FreeCAD
```

**FreeCAD 模块快速定位表**：

| 需求 | 查看源码路径 |
|------|-------------|
| 应用框架 | `src/App/` |
| GUI 框架 | `src/Gui/` |
| 插件/工作台机制 | `src/Gui/Workbench.*` |
| 文档/Undo/Redo | `src/App/Document.*` |
| 模型-视图分离 | `src/Gui/ViewProvider.*` |
| OCCT 集成封装 | `src/Mod/Part/App/TopoShape.*` |
| 参数化特征 | `src/Mod/PartDesign/App/Feature*.cpp` |
| Python 绑定 | `src/Mod/*/App/App*.cpp` |

#### 阅读 OSG 源码

**OSG 模块快速定位表**：

| 需求 | 查看源码路径 |
|------|-------------|
| 场景节点基类 | `include/osg/Node`, `src/osg/Node.cpp` |
| 几何体 | `include/osg/Geometry`, `src/osg/Geometry.cpp` |
| 渲染状态 | `include/osg/StateSet`, `src/osg/StateSet.cpp` |
| 相交测试（拾取）| `include/osgUtil/LineSegmentIntersector` |
| LOD 细节层次 | `include/osg/LOD` |
| 文件格式插件 | `src/osgPlugins/` |
| 官方示例 | `examples/osggeometry/`, `examples/osgpick/` |

#### 阅读 VTK 源码

**VTK 模块快速定位表**：

| 需求 | 查看源码路径 |
|------|-------------|
| 数据模型基类 | `Common/DataModel/vtkDataSet.h` |
| 多边形数据 | `Common/DataModel/vtkPolyData.h` |
| 非结构化网格 | `Common/DataModel/vtkUnstructuredGrid.h` |
| 渲染核心 | `Rendering/Core/vtkRenderer.h` |
| STL 读写 | `IO/Geometry/vtkSTLReader.h` |
| 标量颜色映射 | `Rendering/Core/vtkLookupTable.h` |
| 变形过滤器 | `Filters/General/vtkWarpVector.h` |
| 官方示例 | `Examples/Visualization/`, `Examples/IO/` |

### VSCode / Cursor 扩展推荐

```json
{
  "recommendations": [
    "ms-vscode.cpptools",
    "ms-vscode.cmake-tools",
    "twxs.cmake",
    "ms-python.python",
    "ms-python.pylance",
    "ms-python.black-formatter",
    "charliermarsh.ruff",
    "eamodio.gitlens",
    "mhutchie.git-graph",
    "streetsidesoftware.code-spell-checker",
    "gruntfuggly.todo-tree",
    "ms-azuretools.vscode-docker"
  ]
}
```


### VSCode配置

#### settings.json
```json
{
  "python.defaultInterpreterPath": "./venv/bin/python",
  "python.formatting.provider": "black",
  "python.linting.enabled": true,
  "python.linting.pylintEnabled": true,
  "python.linting.flake8Enabled": true,
  "python.linting.mypyEnabled": true,
  "python.testing.pytestEnabled": true,
  "python.testing.pytestArgs": ["tests/"],
  "files.exclude": {
    "**/__pycache__": true,
    "**/*.pyc": true,
    ".pytest_cache": true
  },
  "editor.formatOnSave": true,
  "editor.rulers": [88, 120],
  "files.trimTrailingWhitespace": true
}
```

#### .vscode/tasks.json
```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "Run Tests",
      "type": "shell",
      "command": "pytest",
      "group": {
        "kind": "test",
        "isDefault": true
      }
    },
    {
      "label": "Format Code",
      "type": "shell",
      "command": "black",
      "args": ["src/", "tests/"]
    },
    {
      "label": "Lint Code",
      "type": "shell",
      "command": "flake8",
      "args": ["src/", "tests/"]
    },
    {
      "label": "Start Dev Server",
      "type": "shell",
      "command": "uvicorn",
      "args": ["main:app", "--reload", "--host", "0.0.0.0", "--port", "8000"],
      "isBackground": true
    }
  ]
}
```

## 项目脚手架

### FastAPI项目模板

#### 项目结构
```
industrial-software/
├── src/
│   ├── __init__.py
│   ├── main.py                 # FastAPI应用入口
│   ├── config.py               # 配置管理
│   ├── models/
│   │   ├── __init__.py
│   │   ├── device.py           # 设备模型
│   │   └── data.py             # 数据模型
│   ├── protocols/
│   │   ├── __init__.py
│   │   ├── modbus.py           # Modbus适配器
│   │   ├── opcua.py            # OPC UA适配器
│   │   └── mqtt.py             # MQTT适配器
│   ├── services/
│   │   ├── __init__.py
│   │   ├── device_service.py   # 设备服务
│   │   ├── data_service.py     # 数据服务
│   │   └── alert_service.py    # 告警服务
│   ├── repositories/
│   │   ├── __init__.py
│   │   └── device_repository.py
│   └── utils/
│       ├── __init__.py
│       ├── logger.py           # 日志工具
│       └── cache.py            # 缓存工具
├── tests/
│   ├── __init__.py
│   ├── test_protocols/
│   ├── test_services/
│   └── conftest.py             # pytest配置
├── docker/
│   ├── Dockerfile
│   └── docker-compose.yml
├── scripts/
│   ├── init_db.py              # 数据库初始化
│   └── migrate.py              # 数据迁移
├── docs/
│   ├── api.md
│   └── architecture.md
├── .env.example                # 环境变量模板
├── .gitignore
├── requirements.txt
├── requirements-dev.txt
├── setup.py
├── pytest.ini
└── README.md
```

#### main.py
```python
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
from contextlib import asynccontextmanager
import uvicorn
import logging

from src.config import settings
from src.utils.logger import setup_logger

# 配置日志
logger = setup_logger(__name__, settings.LOG_LEVEL)

@asynccontextmanager
async def lifespan(app: FastAPI):
    # 启动事件
    logger.info("Starting application...")
    # 初始化数据库连接
    # 初始化协议适配器
    yield
    # 关闭事件
    logger.info("Shutting down application...")
    # 清理资源

# 创建FastAPI应用
app = FastAPI(
    title=settings.APP_NAME,
    description=settings.APP_DESCRIPTION,
    version=settings.APP_VERSION,
    lifespan=lifespan
)

# CORS配置
app.add_middleware(
    CORSMiddleware,
    allow_origins=settings.CORS_ORIGINS,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# 注册路由
from src.api import devices, data, alerts
app.include_router(devices.router, prefix="/api/v1/devices", tags=["devices"])
app.include_router(data.router, prefix="/api/v1/data", tags=["data"])
app.include_router(alerts.router, prefix="/api/v1/alerts", tags=["alerts"])

@app.get("/health")
async def health_check():
    return {"status": "healthy"}

if __name__ == "__main__":
    uvicorn.run(
        "main:app",
        host=settings.HOST,
        port=settings.PORT,
        reload=settings.DEBUG,
        log_level=settings.LOG_LEVEL.lower()
    )
```

#### config.py
```python
from pydantic_settings import BaseSettings
from typing import List
import os

class Settings(BaseSettings):
    # 应用配置
    APP_NAME: str = "Industrial Software"
    APP_VERSION: str = "1.0.0"
    APP_DESCRIPTION: str = "Industrial IoT Data Collection Platform"
    DEBUG: bool = False
    LOG_LEVEL: str = "INFO"

    # 服务器配置
    HOST: str = "0.0.0.0"
    PORT: int = 8000

    # CORS配置
    CORS_ORIGINS: List[str] = ["http://localhost:3000"]

    # 数据库配置
    DATABASE_URL: str = "postgresql://user:pass@localhost:5432/industrial"

    # InfluxDB配置
    INFLUXDB_URL: str = "http://localhost:8086"
    INFLUXDB_TOKEN: str = ""
    INFLUXDB_ORG: str = "my-org"
    INFLUXDB_BUCKET: str = "my-bucket"

    # Redis配置
    REDIS_URL: str = "redis://localhost:6379/0"

    # MQTT配置
    MQTT_BROKER: str = "localhost"
    MQTT_PORT: int = 1883
    MQTT_USERNAME: str = ""
    MQTT_PASSWORD: str = ""

    class Config:
        env_file = ".env"
        case_sensitive = True

settings = Settings()
```

#### .env.example
```env
# 应用配置
DEBUG=True
LOG_LEVEL=INFO

# 数据库
DATABASE_URL=postgresql://user:pass@localhost:5432/industrial

# InfluxDB
INFLUXDB_URL=http://localhost:8086
INFLUXDB_TOKEN=my-token
INFLUXDB_ORG=my-org
INFLUXDB_BUCKET=my-bucket

# Redis
REDIS_URL=redis://localhost:6379/0

# MQTT
MQTT_BROKER=localhost
MQTT_PORT=1883
MQTT_USERNAME=
MQTT_PASSWORD=
```

## 调试与测试

### 调试配置

#### VSCode launch.json
```json
{
  "version": "0.2.0",
  "configurations": [
    {
      "name": "Python: FastAPI",
      "type": "python",
      "request": "launch",
      "module": "uvicorn",
      "args": ["main:app", "--reload", "--host", "0.0.0.0", "--port", "8000"],
      "jinja": true,
      "justMyCode": false,
      "env": {
        "PYTHONPATH": "${workspaceFolder}"
      }
    },
    {
      "name": "Python: Current File",
      "type": "python",
      "request": "launch",
      "program": "${file}",
      "console": "integratedTerminal",
      "justMyCode": false
    },
    {
      "name": "Python: pytest",
      "type": "python",
      "request": "launch",
      "module": "pytest",
      "args": ["-v", "-s"],
      "justMyCode": false
    }
  ]
}
```

### 测试配置

#### pytest.ini
```ini
[pytest]
testpaths = tests
python_files = test_*.py
python_classes = Test*
python_functions = test_*
addopts =
    -v
    --strict-markers
    --cov=src
    --cov-report=html
    --cov-report=term-missing
markers =
    unit: Unit tests
    integration: Integration tests
    slow: Slow tests
```

#### conftest.py
```python
import pytest
import asyncio
from fastapi.testclient import TestClient
from src.main import app

@pytest.fixture
def client():
    """测试客户端"""
    return TestClient(app)

@pytest.fixture
def event_loop():
    """事件循环"""
    loop = asyncio.get_event_loop_policy().new_event_loop()
    yield loop
    loop.close()

@pytest.fixture
def sample_device():
    """示例设备数据"""
    return {
        "id": "dev001",
        "name": "Test Device",
        "type": "PLC",
        "protocol": "modbus",
        "connection": {
            "host": "192.168.1.100",
            "port": 502
        }
    }
```

#### 测试示例
```python
import pytest
from src.services.device_service import DeviceService

@pytest.mark.unit
class TestDeviceService:
    """设备服务测试"""

    def test_create_device(self, sample_device):
        """测试创建设备"""
        service = DeviceService()
        device = service.create_device(sample_device)
        assert device.id == "dev001"
        assert device.name == "Test Device"

    def test_get_device(self, client):
        """测试获取设备API"""
        response = client.get("/api/v1/devices/dev001")
        assert response.status_code == 200
        data = response.json()
        assert data["id"] == "dev001"

    @pytest.mark.integration
    def test_modbus_connection(self):
        """测试Modbus连接"""
        # 需要实际的Modbus设备或模拟器
        pass
```

## 部署与运维

### CI/CD配置

#### GitHub Actions (.github/workflows/ci.yml)
```yaml
name: CI

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  test:
    runs-on: ubuntu-latest

    services:
      postgres:
        image: postgres:13
        env:
          POSTGRES_USER: test
          POSTGRES_PASSWORD: test
          POSTGRES_DB: test
        options: >-
          --health-cmd pg_isready
          --health-interval 10s
          --health-timeout 5s
          --health-retries 5

    steps:
    - uses: actions/checkout@v3

    - name: Set up Python
      uses: actions/setup-python@v4
      with:
        python-version: '3.9'

    - name: Install dependencies
      run: |
        python -m pip install --upgrade pip
        pip install -r requirements.txt
        pip install -r requirements-dev.txt

    - name: Lint with flake8
      run: |
        flake8 src/ tests/

    - name: Format check with black
      run: |
        black --check src/ tests/

    - name: Type check with mypy
      run: |
        mypy src/

    - name: Run tests
      env:
        DATABASE_URL: postgresql://test:test@localhost:5432/test
      run: |
        pytest --cov=src --cov-report=xml

    - name: Upload coverage
      uses: codecov/codecov-action@v3
      with:
        file: ./coverage.xml
```

#### Docker镜像构建
```yaml
# .github/workflows/docker.yml
name: Docker

on:
  push:
    branches: [ main ]

jobs:
  build:
    runs-on: ubuntu-latest

    steps:
    - uses: actions/checkout@v3

    - name: Set up Docker Buildx
      uses: docker/setup-buildx-action@v2

    - name: Login to Docker Hub
      uses: docker/login-action@v2
      with:
        username: ${{ secrets.DOCKER_USERNAME }}
        password: ${{ secrets.DOCKER_PASSWORD }}

    - name: Build and push
      uses: docker/build-push-action@v4
      with:
        context: .
        push: true
        tags: |
          myuser/industrial-software:latest
          myuser/industrial-software:${{ github.sha }}
```

### 监控与日志

#### 日志配置 (logger.py)
```python
import logging
import sys
from logging.handlers import RotatingFileHandler
from pathlib import Path

def setup_logger(name: str, level: str = "INFO") -> logging.Logger:
    """配置日志"""

    logger = logging.getLogger(name)
    logger.setLevel(getattr(logging, level.upper()))

    # 控制台处理器
    console_handler = logging.StreamHandler(sys.stdout)
    console_handler.setLevel(logging.INFO)
    console_formatter = logging.Formatter(
        '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
    )
    console_handler.setFormatter(console_formatter)

    # 文件处理器
    log_dir = Path("logs")
    log_dir.mkdir(exist_ok=True)
    file_handler = RotatingFileHandler(
        log_dir / "app.log",
        maxBytes=10*1024*1024,  # 10MB
        backupCount=5
    )
    file_handler.setLevel(logging.DEBUG)
    file_formatter = logging.Formatter(
        '%(asctime)s - %(name)s - %(levelname)s - %(filename)s:%(lineno)d - %(message)s'
    )
    file_handler.setFormatter(file_formatter)

    logger.addHandler(console_handler)
    logger.addHandler(file_handler)

    return logger
```

#### Prometheus监控
```python
from prometheus_client import Counter, Histogram, Gauge, start_http_server

# 定义指标
request_count = Counter('http_requests_total', 'Total HTTP requests', ['method', 'endpoint'])
request_duration = Histogram('http_request_duration_seconds', 'HTTP request duration')
active_connections = Gauge('active_connections', 'Active connections')

# 启动Prometheus服务器
start_http_server(8001)

# 在FastAPI中间件中使用
@app.middleware("http")
async def metrics_middleware(request, call_next):
    request_count.labels(method=request.method, endpoint=request.url.path).inc()
    with request_duration.time():
        return await call_next(request)
```

## 最佳实践

### 代码规范

#### 类型注解
```python
from typing import List, Optional, Dict, Any
from pydantic import BaseModel

class Device(BaseModel):
    id: str
    name: str
    type: str
    protocol: str
    connection: Dict[str, Any]
    status: str = "offline"

async def create_device(device: Device) -> Device:
    """创建设备"""
    # 实现
    return device

async def get_devices(status: Optional[str] = None) -> List[Device]:
    """获取设备列表"""
    # 实现
    return []
```

#### 错误处理
```python
from fastapi import HTTPException, status
from src.exceptions import DeviceNotFoundError, ConnectionError

async def get_device(device_id: str) -> Device:
    """获取设备"""
    try:
        device = await device_service.get_device(device_id)
        if not device:
            raise DeviceNotFoundError(f"Device {device_id} not found")
        return device
    except ConnectionError as e:
        raise HTTPException(
            status_code=status.HTTP_503_SERVICE_UNAVAILABLE,
            detail=f"Connection error: {str(e)}"
        )
    except DeviceNotFoundError as e:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail=str(e)
        )
    except Exception as e:
        logger.error(f"Unexpected error: {e}")
        raise HTTPException(
            status_code=status.HTTP_500_INTERNAL_SERVER_ERROR,
            detail="Internal server error"
        )
```

### 性能优化

#### 异步IO
```python
import asyncio
from aiohttp import ClientSession

async def fetch_device_data(device_ids: List[str]) -> Dict[str, Any]:
    """并发获取多个设备数据"""
    async with ClientSession() as session:
        tasks = [
            fetch_single_device(session, device_id)
            for device_id in device_ids
        ]
        results = await asyncio.gather(*tasks, return_exceptions=True)
        return {device_id: result for device_id, result in zip(device_ids, results)}
```

#### 缓存策略
```python
from functools import lru_cache
from redis import asyncio as aioredis

class CacheService:
    def __init__(self, redis_url: str):
        self.redis = aioredis.from_url(redis_url)

    async def get(self, key: str) -> Optional[str]:
        """获取缓存"""
        return await self.redis.get(key)

    async def set(self, key: str, value: str, ttl: int = 3600):
        """设置缓存"""
        await self.redis.setex(key, ttl, value)

    async def delete_pattern(self, pattern: str):
        """删除匹配模式的所有缓存"""
        keys = await self.redis.keys(pattern)
        if keys:
            await self.redis.delete(*keys)
```

### 安全实践

#### 环境变量管理
```python
from pydantic_settings import BaseSettings
from functools import lru_cache

class Settings(BaseSettings):
    # 敏感信息通过环境变量配置
    DATABASE_PASSWORD: str
    API_SECRET_KEY: str

    class Config:
        env_file = ".env"

@lru_cache()
def get_settings() -> Settings:
    """获取配置（单例）"""
    return Settings()
```

#### 输入验证
```python
from pydantic import BaseModel, validator

class DeviceData(BaseModel):
    device_id: str
    temperature: float
    timestamp: int

    @validator('temperature')
    def validate_temperature(cls, v):
        if v < -50 or v > 150:
            raise ValueError('Temperature must be between -50 and 150')
        return v

    @validator('timestamp')
    def validate_timestamp(cls, v):
        import time
        if v > time.time():
            raise ValueError('Timestamp cannot be in the future')
        return v
```

## 参考资源
- FastAPI官方文档: https://fastapi.tiangolo.com/
- Python开发最佳实践: https://docs.python-guide.org/
- Docker最佳实践: https://docs.docker.com/develop/dev-best-practices/
- pytest文档: https://docs.pytest.org/
