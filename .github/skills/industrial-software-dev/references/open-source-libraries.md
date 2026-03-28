# 工业软件开发常用开源库

## 目录

### 一、CAD 图形平台（集成）
- [FreeCAD](#freecad) — 开源参数化 3D CAD，基于 OCCT
- [OpenSCAD](#openscad) — 脚本式实体建模
- [QCAD](#qcad) — 开源 2D CAD
- [LibreCAD](#librecad) — 基于 QCAD 的 2D 绘图工具
- [BRL-CAD](#brl-cad) — 跨平台实体几何建模系统
- [JSketcher](#jsketcher) — Web 端参数化 3D 建模器
- [Macad3D](#macad3d) — C# + OCCT 的开源 3D 程序
- [BrepCAD / PythonOCC-CAD](#brepcad--pythonocc-cad) — Python 轻量级 CAD 框架

### 二、CAE
- [OpenFOAM](#openfoam) — CFD 流体仿真
- [FastCAE](#fastcae) — 国产开源 CAE 集成平台
- [deal.II](#dealii) — 有限元计算库
- [Gmsh](#gmsh) — 网格生成

### 三、CAM
- *(待补充)*

### 四、EDA
- [LibrePCB](#librepcb) — 开源 PCB 设计工具

### 五、几何造型器（内核）
- [OpenCASCADE (OCCT)](#opencascade-occt) — 世界知名 3D 建模内核
- [openNURBS](#opennurbs) — NURBS 几何库（.3dm 格式）
- [AMCAX（九韶内核）](#amcax九韶内核) — 国产自主研发 CAD/CAE/CAM 内核
- [LNLib](#lnlib) — C++ NURBS 算法库
- [Open3D](#open3d) — 点云与 3D 重建
- [CGAL](#cgal) — 计算几何算法库
- [libigl](#libigl) — 几何处理算法库

### 六、显示渲染器
- [OpenSceneGraph (OSG)](#openscenegraph-osg) — 工业级 3D 场景图引擎
- [VTK](#vtk-visualization-toolkit) — 科学可视化工具包
- [Mayo](#mayo) — 开源 3D CAD 查看器与转换器

### 七、几何约束求解器
- [SolveSpace](#solvespace) — 参数化 3D CAD 含约束求解器
- [PlaneGCS](#planegcs) — FreeCAD 2D 约束求解器
- [psketcher](#psketcher) — C++ 参数化草图约束

### 八、UI 界面
- [SARibbon](#saribbon) — Qt Ribbon 控件（Office 风格）
- [QtRibbonGUI](#qtribbongui) — Qt Ribbon 界面风格

### 九、空间数据索引
- [libspatialindex](#libspatialindex) — 高效 C++ 空间索引库

### 十、数据格式
- [IFC++（ifcplusplus）](#ifc-ifcplusplus) — C++ 开源 IFC 实现

### 十一、字体
- [FreeType](#freetype) — 高质量可移植字体引擎

### 十二、计算几何
- [CGAL](#cgal) — 计算几何算法库（同上）

### 其他
- [工业协议库](#工业协议库)
- [数据采集与处理](#数据采集与处理)
- [可视化与人机界面（HMI）](#可视化与人机界面)
- [数学计算与算法](#数学计算与算法)
- [典型技术栈组合](#典型技术栈组合)

---

## CAD/CAM/CAE 核心库

### OpenCASCADE (OCCT)

**GitHub**: [Open-Cascade-SAS/OCCT](https://github.com/Open-Cascade-SAS/OCCT)  
**官网**: https://dev.opencascade.org/  
**语言**: C++  
**许可证**: LGPL 2.1（商业动态链接免版权，需保留声明）  
**最新稳定版**: 7.8.x  

#### 核心模块架构

```
OCCT 模块体系
├── FoundationClasses    基础类（Handle、集合、字符串、异常）
├── ModelingData         几何数据（曲线、曲面、拓扑 BRep）
├── ModelingAlgorithms   建模算法（布尔运算、偏置、圆角）
├── Visualization        可视化（AIS 交互服务、3D 视图）
├── DataExchange         数据交换（STEP、IGES、STL、OBJ）
└── ApplicationFramework 应用框架（OCAF 文档管理）
```

#### 关键概念

| 概念 | 说明 |
|------|------|
| `Handle<T>` | OCCT 智能指针，管理继承自 `Standard_Transient` 的对象 |
| `TopoDS_Shape` | 所有拓扑形状的基类（点/边/面/体） |
| `BRep` | 边界表示法，OCCT 核心几何表示 |
| `gp_*` | 几何原语（gp_Pnt 点、gp_Vec 向量、gp_Trsf 变换） |
| `BRepBuilderAPI_*` | 形状构建器系列 API |
| `BRepAlgoAPI_*` | 布尔运算系列 API |
| `BRepMesh_IncrementalMesh` | 增量式网格剖分（形状 → 三角网格） |
| `OCAF (TDocStd)` | 文档管理框架，支持 Undo/Redo |

#### 源码目录结构（GitHub）

```
OCCT/
├── src/
│   ├── BRep/            # BRep 拓扑数据结构
│   ├── BRepAlgo/        # 布尔运算算法
│   ├── BRepBuilderAPI/  # 形状构建 API
│   ├── BRepMesh/        # 网格剖分
│   ├── gp/              # 几何原语
│   ├── Geom/            # 几何曲线曲面
│   ├── TopoDS/          # 拓扑数据结构
│   ├── TopExp/          # 拓扑遍历工具
│   ├── AIS/             # 交互式选择服务（可视化）
│   ├── V3d/             # 3D 视图
│   ├── STEPCAFControl/  # STEP 读写
│   └── TDocStd/         # OCAF 文档框架
├── samples/             # 官方示例（重点参考）
├── tests/               # 测试脚本
└── CMakeLists.txt
```

#### 集成示例 (C++)

**基础几何建模**：
```cpp
#include <deal.II/grid/tria.h>
#include <deal.II/grid/grid_generator.h>
#include <deal.II/dofs/dof_handler.h>
#include <deal.II/fe/fe_q.h>

using namespace dealii;

// 创建网格
Triangulation<3> triangulation;
GridGenerator::hyper_cube(triangulation, -1, 1);
triangulation.refine_global(3);  // 全局加密 3 次

// 设置有限元
FE_Q<3> fe(1);  // 一阶拉格朗日有限元
DoFHandler<3> dof_handler(triangulation);
dof_handler.distribute_dofs(fe);

std::cout << "节点数: " << dof_handler.n_dofs() << std::endl;
```

---

## 工业协议库

### pymodbus
**GitHub**: [pymodbus-dev/pymodbus](https://github.com/pymodbus-dev/pymodbus)  
**语言**: Python | **许可证**: BSD 3-Clause

```python
from pymodbus.client import ModbusTcpClient

client = ModbusTcpClient('192.168.1.100', port=502)
client.connect()

# 读取保持寄存器
result = client.read_holding_registers(address=0, count=10, slave=1)
if not result.isError():
    print(result.registers)

client.close()
```

### opcua-asyncio
**GitHub**: [FreeOpcUa/opcua-asyncio](https://github.com/FreeOpcUa/opcua-asyncio)  
**语言**: Python | **许可证**: LGPL 3.0

```python
import asyncio
from asyncua import Client

async def main():
    async with Client(url="opc.tcp://localhost:4840") as client:
        node = client.get_node("ns=2;s=Machine1.Temperature")
        value = await node.read_value()
        print(f"Temperature: {value}")

asyncio.run(main())
```

### paho-mqtt
**GitHub**: [eclipse/paho.mqtt.python](https://github.com/eclipse/paho.mqtt.python)  
**语言**: Python | **许可证**: Eclipse Public License

```python
import paho.mqtt.client as mqtt

def on_message(client, userdata, msg):
    print(f"{msg.topic}: {msg.payload.decode()}")

client = mqtt.Client()
client.on_message = on_message
client.connect("localhost", 1883)
client.subscribe("factory/line1/+")
client.loop_forever()
```

---

## 数据采集与处理

### InfluxDB（时序数据库）
```python
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS
from datetime import datetime

client = InfluxDBClient(url="http://localhost:8086", token="my-token", org="my-org")
write_api = client.write_api(write_options=SYNCHRONOUS)

point = Point("temperature") \
    .tag("device", "sensor1") \
    .field("value", 25.5) \
    .time(datetime.utcnow())
write_api.write(bucket="my-bucket", record=point)
```

### Apache Kafka（消息队列）
```python
from confluent_kafka import Producer, Consumer

producer = Producer({'bootstrap.servers': 'localhost:9092'})
producer.produce('device-data', key='device1', value='{"temp": 25}')
producer.flush()
```

---

## 可视化与人机界面

### PyQt5 / PySide6
**许可证**: PyQt5-GPL/Commercial | PySide6-LGPL/Commercial

```python
from PyQt5.QtWidgets import QApplication, QMainWindow
from PyQt5.QtCore import QTimer
import sys

class HMIWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Industrial HMI")
        self.setGeometry(100, 100, 1200, 800)

        self.timer = QTimer()
        self.timer.timeout.connect(self.update_data)
        self.timer.start(500)  # 500ms 刷新

    def update_data(self):
        # 从设备读取数据并更新界面
        pass

    app = QApplication(sys.argv)
    window = HMIWindow()
    window.show()
    sys.exit(app.exec_())
```

### Dash（Web 可视化）
```python
import dash
from dash import dcc, html
from dash.dependencies import Input, Output
import plotly.graph_objs as go

app = dash.Dash(__name__)
app.layout = html.Div([
    html.H1("Industrial Monitor"),
    dcc.Graph(id='live-graph'),
    dcc.Interval(id='graph-update', interval=1000)
])

@app.callback(
    Output('live-graph', 'figure'),
    [Input('graph-update', 'n_intervals')]
)
def update_graph(n):
    data = get_realtime_data()
    return {'data': [go.Scatter(x=data['t'], y=data['v'], mode='lines')]}

if __name__ == '__main__':
    app.run_server(debug=True)
```

---

## 数学计算与算法

### NumPy + SciPy
```python
import numpy as np
from scipy import signal, optimize

# 信号滤波（工业传感器数据去噪）
sampling_rate = 1000  # Hz
b, a = signal.butter(4, 50 / (sampling_rate / 2), btype='low')
filtered = signal.filtfilt(b, a, raw_sensor_data)

# 优化问题（PID 参数整定）
def pid_cost(params):
    kp, ki, kd = params
    # 仿真 PID 控制响应，返回误差
    return simulate_pid_error(kp, ki, kd)

result = optimize.minimize(pid_cost, [1.0, 0.1, 0.01], method='Nelder-Mead')
kp_opt, ki_opt, kd_opt = result.x
```

### scikit-learn（预测性维护）
```python
from sklearn.ensemble import IsolationForest
from sklearn.preprocessing import StandardScaler
import numpy as np

# 异常检测（设备故障预警）
scaler = StandardScaler()
X_scaled = scaler.fit_transform(sensor_features)

model = IsolationForest(contamination=0.05, random_state=42)
model.fit(X_scaled)

# 预测：-1 为异常，1 为正常
predictions = model.predict(X_scaled)
anomalies = np.where(predictions == -1)[0]
print(f"检测到 {len(anomalies)} 个异常点")
```

---

## 测试与仿真

### pytest
```python
import pytest
from unittest.mock import MagicMock

@pytest.fixture
def mock_modbus_client():
    client = MagicMock()
    client.read_holding_registers.return_value.registers = [100, 200, 300]
    return client

def test_device_data_collection(mock_modbus_client):
    collector = DataCollector(mock_modbus_client)
    data = collector.collect()
    assert data["temperature"] == 100
    assert data["pressure"] == 200
```

---

## 开发工具与框架

### FastAPI（工业物联网后端）
```python
from fastapi import FastAPI
from pydantic import BaseModel

app = FastAPI(title="Industrial IoT API")

class SensorData(BaseModel):
    device_id: str
    value: float
    timestamp: int

@app.post("/api/v1/data")
async def receive_data(data: SensorData):
    await process_and_store(data)
    return {"status": "success"}

@app.get("/api/v1/devices/{device_id}/status")
async def get_device_status(device_id: str):
    return await fetch_device_status(device_id)
```

---

## 典型技术栈组合

### CAD/CAM 系统
```yaml
几何内核:    OpenCASCADE (OCCT) — BRep 建模、布尔运算、STEP/IGES
CAD 平台参考: FreeCAD — 插件架构、参数化特征树
3D 渲染:    OpenSceneGraph (OSG) — 场景图、交互拾取
计算几何:   CGAL — 网格处理、碰撞检测
网格生成:   Gmsh — CAE 前处理
GUI:        Qt5/Qt6 — 跨平台界面
Python 绑定: pythonocc-core — OCCT Python API
构建:       CMake + vcpkg/Conan
```

### CAE 仿真后处理平台
```yaml
可视化:     VTK — 应力云图、流线、等值面
网格数据:   vtkUnstructuredGrid — 有限元网格
有限元计算: deal.II — 求解器
CFD:        OpenFOAM — 流体仿真
网格生成:   Gmsh — 前处理
Python:     vtk + numpy + scipy
GUI:        Qt + QVTKOpenGLNativeWidget
```

### 工业物联网 + 数字孪生
```yaml
数据采集:   pymodbus / opcua-asyncio / paho-mqtt
数据流:     Apache Kafka
时序存储:   InfluxDB / TimescaleDB
业务存储:   PostgreSQL
缓存:       Redis
后端框架:   FastAPI + Celery
3D 可视化:  VTK / Three.js
点云处理:   Open3D
Web HMI:    Dash / Grafana
部署:       Docker + Kubernetes
```

### 工业检测系统
```yaml
点云采集:   Open3D — 读取 3D 扫描数据
点云配准:   Open3D ICP — 扫描件与 CAD 对齐
几何参考:   OCCT — 读取 CAD 模型（STEP）
偏差分析:   Open3D + NumPy — 点到面距离计算
可视化:     Open3D / VTK — 偏差热图
报告生成:   matplotlib + reportlab
```

## 依赖管理

### Python 依赖 (requirements.txt)
```
# CAD/CAM
pythonocc-core==7.7.2

# 3D 可视化
vtk==9.3.0
open3d==0.18.0

# 网格处理
python-igl==2.5.1
gmsh==4.12.0

# 工业协议
pymodbus==3.5.2
asyncua==1.0.2
paho-mqtt==1.6.1

# 数据处理
numpy==1.26.2
scipy==1.11.4
pandas==2.1.3
scikit-learn==1.3.2

# 数据存储
influxdb-client==1.38.0
sqlalchemy==2.0.23
redis==5.0.1

# Web 框架
fastapi==0.104.1
uvicorn[standard]==0.24.0
dash==2.14.2
plotly==5.18.0

# GUI
PyQt5==5.15.10

# 测试
pytest==7.4.3
pytest-asyncio==0.21.1
pytest-cov==4.1.0

# 工具
python-dotenv==1.0.0
pydantic==2.5.0
```

### C++ vcpkg 依赖 (vcpkg.json)
```json
{
  "name": "industrial-cad-app",
  "version": "1.0.0",
  "dependencies": [
    "opencascade",
    "openscenegraph",
    "vtk",
    "cgal",
    "eigen3",
    "boost",
    "qt5",
    "pybind11",
    "gtest"
  ]
}
```

### C++ CMake 模板
```cmake
cmake_minimum_required(VERSION 3.20)
project(IndustrialCADApp VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# vcpkg 集成
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif()

# 查找依赖
find_package(OpenCASCADE REQUIRED)
find_package(OpenSceneGraph REQUIRED COMPONENTS osg osgDB osgViewer osgGA osgUtil)
find_package(VTK REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core Gui Widgets OpenGL)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/geometry/GeometryEngine.cpp
    src/rendering/SceneManager.cpp
    src/ui/MainWindow.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${OpenCASCADE_INCLUDE_DIR}
    ${OPENSCENEGRAPH_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    # OCCT
    TKernel TKMath TKBRep TKGeomBase TKGeomAlgo
    TKTopAlgo TKPrim TKBO TKSTEP TKMesh TKService
    TKV3d TKOpenGl TKAIS
    # OSG
    ${OPENSCENEGRAPH_LIBRARIES}
    # VTK
    ${VTK_LIBRARIES}
    # Qt
    Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL
)
```

## 参考资源

| 资源 | 链接 |
|------|------|
| OCCT 官方文档 | https://dev.opencascade.org/doc/overview/html/ |
| OCCT GitHub Examples | https://github.com/Open-Cascade-SAS/OCCT/tree/master/samples |
| pythonocc-core | https://github.com/tpaviot/pythonocc-core |
| OSG GitHub Examples | https://github.com/openscenegraph/OpenSceneGraph/tree/master/examples |
| OSG Recipes | https://github.com/xarray/osgRecipes |
| VTK Examples | https://kitware.github.io/vtk-examples/site/ |
| FreeCAD 源码 | https://github.com/FreeCAD/FreeCAD/tree/main/src |
| CGAL 文档 | https://doc.cgal.org/ |
| Open3D 文档 | http://www.open3d.org/docs/ |
| Gmsh 文档 | https://gmsh.info/doc/texinfo/gmsh.html |
| deal.II 教程 | https://www.dealii.org/current/doxygen/deal.II/Tutorial.html |

---

## 一、CAD 图形平台（集成）

### OpenSCAD

**GitHub**: [openscad/openscad](https://github.com/openscad/openscad)  
**语言**: C++  
**许可证**: GPL 2.0  
**官网**: https://openscad.org/

**定位**：脚本驱动的实体 3D CAD，专注 CAD 工程而非艺术建模。  
采用构造实体几何（CSG）和 2D 轮廓挤压两种建模技术，读取脚本文件渲染 3D 模型，天然支持版本控制和参数化设计。

**核心特点**：
- 无交互式建模，脚本即模型（文本版本控制友好）
- 支持 CSG 布尔运算、拉伸、旋转
- 读写 STL、OFF、DXF 格式
- 可配置参数驱动设计变体

**示例脚本**：
```scad
// 参数化支架
base_w = 60;
base_h = 40;
hole_r = 8;

difference() {
    // 基础板
    cube([base_w, base_h, 10]);
    // 挖孔
    translate([base_w/2, base_h/2, -1])
        cylinder(r=hole_r, h=12);
    // 四角螺丝孔
    for (x=[8, base_w-8], y=[8, base_h-8])
        translate([x, y, -1])
            cylinder(r=3, h=12);
}
```

**适用场景**：3D 打印零件设计、参数化机械零件、可脚本化的 CAD 自动化

---

### QCAD

**GitHub**: [qcad/qcad](https://github.com/qcad/qcad)  
**语言**: C++ / Qt  
**许可证**: GPL 3.0  
**官网**: https://qcad.org/

**定位**：专业开源 2D CAD，模块化可扩展架构，适合技术图纸、建筑平面、机械零件图。

**核心特点**：
- 完整的 2D CAD 工具集（直线、圆弧、样条、标注）
- 支持 DXF/DWG 格式
- 模块化插件架构
- 内置脚本引擎（ECMAScript）
- 跨平台：Windows / macOS / Linux

**二次开发示例（ECMAScript）**：
```javascript
// QCAD 脚本：批量绘制螺栓孔圆
function drawBoltHoles(cx, cy, pcd, count, holeR) {
    var angleStep = 2 * Math.PI / count;
    for (var i = 0; i < count; i++) {
        var angle = i * angleStep;
        var x = cx + pcd/2 * Math.cos(angle);
        var y = cy + pcd/2 * Math.sin(angle);
        var circle = new RCircleEntity(
            document,
            new RCircleData(new RVector(x, y), holeR)
        );
        op.addObject(circle);
    }
}
drawBoltHoles(0, 0, 100, 6, 5);
```

---

### LibreCAD

**GitHub**: [LibreCAD/LibreCAD](https://github.com/LibreCAD/LibreCAD)  
**语言**: C++ / Qt  
**许可证**: GPL 2.0  
**官网**: https://librecad.org/

**定位**：基于 QCAD 社区版的轻量级 2D CAD，完全免费无商业版限制。  
跨平台，适合教学、小型企业 2D 制图。

**核心特点**：
- 支持 DXF 格式（R12 / R2000）
- 图层管理、块/属性、尺寸标注
- 插件扩展支持
- 社区活跃，持续维护

---

### BRL-CAD

**GitHub**: [BRL-CAD/brlcad](https://github.com/BRL-CAD/brlcad)  
**语言**: C / C++  
**许可证**: LGPL 2.1 / BSD  

**定位**：美国军方支持的跨平台开源 3D 实体几何建模系统，拥有超过 40 年历史。

**核心特点**：
- CSG + BRep 混合几何表示
- 高性能光线追踪渲染器
- 网络分布式 SMP 并行计算
- 几何分析与图像信号处理工具
- 丰富的几何脚本接口（Tcl/Python）
- 支持导入 STEP、IGES、STL、OBJ 等格式

**适用场景**：军事/国防仿真、工程分析、弹道计算、核反应堆几何建模

---

### JSketcher

**GitHub**: [xibyte/jsketcher](https://github.com/xibyte/jsketcher)  
**语言**: TypeScript / WebAssembly  
**许可证**: MIT  

**定位**：纯 Web 端参数化 3D 建模器，使用 OpenCASCADE WASM 做实体建模，2D 约束求解器完全用 TypeScript 实现。

**核心特点**：
- 无需服务端，纯客户端 JS + WASM
- 几何约束求解器（同心、平行、相切、对称等）
- 特征历史（Feature History）参数化建模
- 导出 STL、DWG、SVG
- 尺寸库：符号名称参数，修改自动传播
- 基于 OpenCASCADE WASM 的布尔运算

**适用场景**：Web CAD 开发参考、在线 3D 打印设计工具、参数化草图引擎参考实现

---

### Macad3D

**GitHub**: [Macaad3d/Macad3D](https://github.com/Macaad3d/Macad3D)  
**语言**: C# / .NET  
**许可证**: MIT  

**定位**：基于 C# 和 OpenCASCADE 的免费开源 3D CAD 程序，展示了在 .NET 生态中集成 OCCT 的最佳实践。

**核心特点**：
- C# 绑定 OCCT（通过 C++/CLI 封装）
- 参数化建模，Undo/Redo 支持
- WPF 界面
- 适合 .NET / C# 技术栈的 CAD 项目参考

**参考价值**：C# 封装 OCCT 的完整示例，.NET CAD 开发的参考架构

---

### BrepCAD / PythonOCC-CAD

**GitHub**: [qunat/Pythonocc-CAD](https://github.com/qunat/Pythonocc-CAD)  
**语言**: Python  
**许可证**: MIT  

**定位**：基于 pythonocc-core 和 Qt Ribbon 风格界面的轻量级 Python CAD 框架，适合毕业设计和轻量级 CAD 原型开发。

```python
# pythonocc-core 基础使用示例
from OCC.Core.BRepBuilderAPI import BRepBuilderAPI_MakeBox
from OCC.Core.BRepAlgoAPI import BRepAlgoAPI_Fuse
from OCC.Display.SimpleGui import init_display

# 创建两个几何体并合并
box1 = BRepBuilderAPI_MakeBox(50, 30, 20).Shape()
box2 = BRepBuilderAPI_MakeBox(20, 60, 10).Shape()
fused = BRepAlgoAPI_Fuse(box1, box2).Shape()

display, start_display, _, _ = init_display()
display.DisplayShape(fused, update=True)
start_display()
```

---

## 二、CAE 扩展库

### FastCAE

**GitHub**: [DISOGitHub/FastCAE](https://github.com/DISOGitHub/FastCAE)  
**语言**: C++ / Python  
**许可证**: GPL 3.0  

**定位**：国产开源 CAE 软件集成开发平台，面向求解器开发者，定义规范数据接口，支持插件模式。

**核心特点**：
- 标准化 CAE 数据接口与框架
- 插件化：前处理 / 求解器 / 后处理均可独立插件集成
- 内置完整几何交互、网格划分（基于 Gmsh）、结果可视化（基于 VTK）
- 内置边界条件管理、材料库
- 支持集成自研求解器和第三方求解器（如 OpenFOAM）
- 跨平台：Windows / Linux

**架构参考**：
```
FastCAE 架构
├── 几何模块       基于 OCCT 的几何交互
├── 网格模块       基于 Gmsh 的网格生成
├── 求解器接口     标准化插件接口（动态库）
├── 后处理模块     基于 VTK 的结果可视化
└── 材料/边界库    内置工程材料数据库
```

**适用场景**：国产 CAE 平台开发、求解器集成、有限元前后处理

---

## 三、EDA

### LibrePCB

**GitHub**: [LibrePCB/LibrePCB](https://github.com/LibrePCB/LibrePCB)  
**语言**: C++ / Qt  
**许可证**: GPL 3.0  
**官网**: https://librepcb.org/

**定位**：免费开源 EDA 工具，用于原理图设计和 PCB 布局，支持 Linux / Windows / macOS。

**核心特点**：
- 统一的库管理系统（元件 / 封装 / 符号）
- 原理图编辑器 + PCB 布局编辑器
- 设计规则检查（DRC）
- 导出 Gerber、钻孔文件、BOM、Pick&Place
- 基于 Git 的版本管理友好文件格式
- 内置元件库（持续扩充中）

**适用场景**：PCB 设计、嵌入式硬件开发、电子工程教学

---

## 四、几何造型器（内核）扩展

### openNURBS

**GitHub**: [mcneel/opennurbs](https://github.com/mcneel/opennurbs)  
**语言**: C++  
**许可证**: MIT  

**定位**：Rhino（犀牛）背后的 NURBS 几何库，提供 `.3dm` 文件格式读写，是 CAD 间 NURBS 数据交换的重要工具。

**核心特点**：
- 400+ 软件采用 `.3dm` 格式交换 3D 模型
- NURBS 曲线/曲面评估工具
- 基本几何和 3D 视图操作工具
- 跨平台 C++ 实现，无外部依赖

**集成示例 (C++)**：
```cpp
#include "opennurbs.h"

// 读取 .3dm 文件
ONX_Model model;
ON_wString filename = L"model.3dm";
bool ok = model.Read(filename);

// 遍历几何对象
for (int i = 0; i < model.m_object_table.Count(); i++) {
    const ONX_Model_Object& obj = model.m_object_table[i];
    if (obj.m_object->ObjectType() == ON::curve_object) {
        const ON_Curve* curve = ON_Curve::Cast(obj.m_object);
        // 处理曲线...
    }
}

// 写入 .3dm 文件
model.Write(L"output.3dm");
```

**适用场景**：Rhino 模型数据读写、NURBS 数据交换、曲面评估计算

---

### AMCAX（九韶内核）

**下载/官网**: https://amcax.net/  
**语言**: C++  
**许可证**: 免费版（核心功能开放）+ 商业付费版  

**定位**：国产完全自主研发的 CAD/CAE/CAM 几何内核，从 0 到 1 自主知识产权，包含几何内核、几何约束求解器和 CAX 一体化计算引擎。

**核心特点**：
- 完全自主研发，不依赖 OCCT
- 多边形建模、参数化建模、自由曲面建模 API
- 几何约束求解器
- STEP/IGES 等标准格式读写
- 开源示例项目「九韶精灵」(AMCAX-Daemon) 展示完整使用方式
- 免费版逐步开放稳定测试功能

**参考价值**：国产替代 OCCT 的重要备选，国产化 CAD 项目优先评估

---

### LNLib

**GitHub**: [BIMCoderLiang/LNLib](https://github.com/BIMCoderLiang/LNLib)  
**语言**: C++  
**许可证**: MIT  

**定位**：基于《The NURBS Book（2nd Edition）》的 C++ NURBS 算法库，API 设计友好，适合学习和直接集成。

**核心特点**：
- 覆盖《The NURBS Book》主要算法
- NURBS 曲线/曲面创建、评估、插值、近似
- B-样条基函数计算
- 节点插入、细化、升阶算法
- Header-friendly，易于集成

**集成示例 (C++)**：
```cpp
#include "LNLib/LNLib.h"
using namespace LNLib;

// 创建 NURBS 曲线（圆弧）
std::vector<double> knots = {0,0,0,1,1,1};
std::vector<double> weights = {1.0, 0.707, 1.0};
std::vector<XYZ> controlPoints = {
    XYZ(1,0,0), XYZ(1,1,0), XYZ(0,1,0)
};

// 在参数 t=0.5 处评估曲线点
XYZ point = NurbsCurve::GetPointOnCurve(
    2, knots, weights, controlPoints, 0.5
);
```

**适用场景**：CAD 内核中的 NURBS 算法实现参考、曲面造型研究、学习《The NURBS Book》配套代码

---

## 五、显示渲染器扩展

### Mayo

**GitHub**: [fougue/mayo](https://github.com/fougue/mayo)  
**语言**: C++ / Qt  
**许可证**: BSD 3-Clause

**定位**：基于 OpenCASCADE + Qt 的开源 3D CAD 查看器和格式转换器，界面现代，支持多种工业格式。

**核心特点**：
- 支持格式：STEP、IGES、BREP、STL、OBJ、GLTF、PLY、AMF、3MF 等
- 批量格式转换（命令行模式）
- 现代 Qt 界面，支持深色主题
- 基于 OCCT AIS 的 3D 交互（旋转、缩放、拾取）
- 跨平台：Windows / Linux / macOS

**参考价值**：
- OCCT AIS 可视化的完整实现参考
- Qt + OCCT 集成的最佳实践参考
- 工业格式读写的实际使用示例

**核心代码参考**：
```cpp
// Mayo 的 OCCT AIS 显示封装（参考其源码）
// src/app/widget_occ_view.cpp — Qt + OCCT 视图集成
// src/io_occ/ — OCCT 各格式读写封装
// src/graphics/ — 图形显示管理封装
```

---

## 六、几何约束求解器

### SolveSpace

**GitHub**: [solvespace/solvespace](https://github.com/solvespace/solvespace)  
**语言**: C / C++  
**许可证**: GPL 3.0  
**官网**: https://solvespace.com/

**定位**：完整的参数化 3D CAD 程序，内置强大的几何约束求解器，支持 3D 零件建模、2D 草图、装配仿真。

**核心特点**：
- 几何约束求解器（点重合、平行、垂直、相切、对称、距离、角度等）
- 3D 零件：拉伸、扫掠、布尔运算
- 2D 草图：参数化截面设计，导出 DXF/PDF/SVG
- 机构仿真：Pin/Ball/Slide 关节的链接模拟
- 导出 STL、STEP 用于 CAM/3D 打印
- 约束求解器核心可独立复用（`solvespace/exposed/` API）

**约束求解器独立使用示例 (C)**：
```c
#include <slvs.h>

Slvs_System sys;
memset(&sys, 0, sizeof(sys));

// 定义工作平面
Slvs_hGroup g = 1;
Slvs_AddParam(&sys, SLVS_E_PARAM, g, 0.0);
// ... 添加实体和约束 ...

// 求解
Slvs_Solve(&sys, g);
if (sys.result == SLVS_RESULT_OKAY) {
    printf("约束求解成功，自由度: %d\n", sys.dof);
}
```

**适用场景**：参数化 CAD 中的草图约束求解、装配约束验证、机构运动仿真

---

### PlaneGCS

**GitHub**: [CadQuery/PlaneGCS](https://github.com/CadQuery/PlaneGCS)  
**语言**: C++  
**许可证**: LGPL 2.1  

**定位**：FreeCAD Sketcher 模块使用的 2D 几何约束求解器，从 FreeCAD 独立提取，可单独集成。

**核心特点**：
- 支持 2D 草图全套约束（重合、共线、平行、垂直、相切、等长、对称、角度、半径等）
- 基于牛顿-拉夫逊数值迭代求解
- 过约束检测与报告
- C++ 原生，无额外依赖

**集成示例**：
```cpp
#include "GCS.h"
using namespace GCS;

System sys;

// 定义点
double x1=0, y1=0, x2=1, y2=0;
Point p1(x1, y1), p2(x2, y2);

// 添加水平约束（两点 y 坐标相同）
sys.addConstraintEqual(&p1.y, &p2.y, 1);

// 添加距离约束
double dist = 50.0;
sys.addConstraintP2PDistance(p1, p2, &dist, 2);

// 求解
int dof = sys.solve();
printf("DOF: %d, x2=%.2f\n", dof, x2);
```

**适用场景**：CAD 草图模块的 2D 约束求解器集成

---

### psketcher

**GitHub**: [peizhan/psketcher](https://github.com/peizhan/psketcher)  
**语言**: C++  
**许可证**: BSD  

**定位**：C++ 参数化草图约束求解器，支持 2D/3D 草图约束，适合嵌入 CAD 应用。

---

## 七、UI 界面

### SARibbon

**GitHub**: [czyt1988/SARibbon](https://github.com/czyt1988/SARibbon)  
**语言**: C++ / Qt5+  
**许可证**: MIT  

**定位**：基于 Qt 的轻量级 Ribbon 控件（Office/CAD 风格 UI），最低要求 Qt5，支持 C++11。

**核心特点**：
- 完整的 Ribbon 界面（TabBar、Toolbar、快速访问栏）
- 多种按钮样式（大图标、小图标、文字+图标）
- 支持折叠/展开
- 自定义主题（亮色/暗色）
- 与 Qt Designer 集成
- 专为 CAD/工业软件设计

**集成示例 (C++)**：
```cpp
#include "SARibbon.h"

SARibbonMainWindow* mainWindow = new SARibbonMainWindow();
SARibbonBar* ribbon = mainWindow->ribbonBar();

// 添加选项卡
SARibbonCategory* homeTab = ribbon->addCategoryPage(tr("主页"));

// 添加面板
SARibbonPannel* sketchPanel = homeTab->addPannel(tr("草图"));

// 添加大图标按钮
QAction* newSketchAct = new QAction(QIcon(":/icons/sketch.svg"),
                                     tr("新建草图"), this);
sketchPanel->addLargeAction(newSketchAct);
```

**适用场景**：CAD 软件主界面、工业 HMI 软件、需要 Office 风格 Ribbon 的 Qt 应用

---

### QtRibbonGUI

**GitHub**: [liang1057/QtRibbonGUI](https://github.com/liang1057/QtRibbonGUI)  
**语言**: C++ / Qt  
**许可证**: MIT  

**定位**：Qt 实现的 Ribbon 界面风格控件，适合快速集成 Ribbon UI 到 Qt 项目。

---

## 八、空间数据索引

### libspatialindex

**GitHub**: [libspatialindex/libspatialindex](https://github.com/libspatialindex/libspatialindex)  
**语言**: C++  
**许可证**: MIT  

**定位**：高效 C++ 空间索引库，支持范围查询、点位置查询、最近邻查询（KNN），适合大规模空间数据检索。

**核心特点**：
- R*-tree 索引（线性/二次/星型分裂）
- MVR 树（多版本 R 树）
- TPR 树（时间参数化 R 树）
- KD-Tree 最近邻搜索
- STR 批量加载
- 支持内存和磁盘持久化
- 任意形状范围查询（通过通用几何接口）

**集成示例 (C++)**：
```cpp
#include <spatialindex/SpatialIndex.h>
using namespace SpatialIndex;

// 创建内存 R-tree 索引
IStorageManager* mem = StorageManager::createNewMemoryStorageManager();
ISpatialIndex* tree = RTree::createNewRTree(
    *mem, 0.7, 100, 100, 2,
    RTree::RV_RSTAR, indexIdentifier
);

// 插入边界框
double plow[2]  = {0.5, 0.5};
double phigh[2] = {1.5, 1.5};
Region r(plow, phigh, 2);
tree->insertData(0, nullptr, r, 1 /*id*/);

// 范围查询
double qlow[2]  = {0.0, 0.0};
double qhigh[2] = {2.0, 2.0};
Region query(qlow, qhigh, 2);
MyVisitor visitor;
tree->intersectsWithQuery(query, visitor);

delete tree;
delete mem;
```

**适用场景**：CAD 空间拾取加速、大规模点云空间检索、GIS 地理数据索引、碰撞检测预筛选

---

## 九、数据格式

### IFC++ (ifcplusplus)

**GitHub**: [ifcquery/ifcplusplus](https://github.com/ifcquery/ifcplusplus)  
**语言**: C++  
**许可证**: MIT  

**定位**：C++ 开源 IFC 实现，用于读写建筑信息模型（BIM）标准格式 IFC（STEP 格式），含简单的 IFC 查看器（Qt + OSG）。

**核心特点**：
- 完整的 IFC 2x3 / IFC 4 C++ 类模型
- 并行 STEP 解析器（多核 CPU 加速）
- 智能指针内存管理
- 读取属性集、材料、几何信息
- 附带 IFC 查看器：Qt + OpenSceneGraph
- MIT 许可，可用于商业项目

**集成示例 (C++)**：
```cpp
#include <ifcpp/reader/ReaderSTEP.h>
#include <ifcpp/model/BuildingModel.h>

// 读取 IFC 文件
shared_ptr<BuildingModel> model = make_shared<BuildingModel>();
shared_ptr<ReaderSTEP> reader = make_shared<ReaderSTEP>();
reader->loadModelFromFile(L"building.ifc", model);

// 遍历 IFC 实体
const auto& map = model->getMapIfcEntities();
for (auto& entry : map) {
    shared_ptr<BuildingEntity> entity = entry.second;
    // 处理 IFC 实体...
    auto ifcProduct = dynamic_pointer_cast<IfcProduct>(entity);
    if (ifcProduct) {
        // 获取几何表示
    }
}
```

**适用场景**：BIM 软件开发、建筑数字孪生、IFC 格式解析与转换、AEC 行业软件

---

## 十、字体

### FreeType

**官网**: https://freetype.org/  
**下载**: https://download.savannah.gnu.org/releases/freetype/  
**语言**: C  
**许可证**: FreeType License (BSD-style) / GPL 2.0  

**定位**：工业级高质量可移植字体引擎，提供统一接口访问多种字体格式，被 Qt、OpenGL 应用、VTK、OSG 等广泛集成。

**支持格式**：TrueType (.ttf)、OpenType (.otf)、Type1、CID、CFF、Windows FON/FNT、X11 PCF 等

**核心特点**：
- 高质量抗锯齿字体渲染
- 亚像素渲染（LCD 优化）
- 字形轮廓提取（可用于 3D 文字挤出）
- 支持 Unicode 和复杂文字排版
- 跨平台，无依赖

**集成示例 (C)**：
```c
#include <ft2build.h>
#include FT_FREETYPE_H

FT_Library library;
FT_Face face;

// 初始化 FreeType
FT_Init_FreeType(&library);
FT_New_Face(library, "arial.ttf", 0, &face);
FT_Set_Pixel_Sizes(face, 0, 48);  // 48px 字体大小

// 加载字形（字母 'A'）
FT_Load_Char(face, 'A', FT_LOAD_RENDER);
FT_Bitmap& bitmap = face->glyph->bitmap;
// bitmap.buffer 包含灰度像素数据，可上传到 GPU 纹理

FT_Done_Face(face);
FT_Done_FreeType(library);
```

**在 3D 应用中提取字形轮廓（用于挤出 3D 文字）**：
```cpp
// 使用 OCCT 将 FreeType 字形轮廓转换为 3D 形体
#include <Font_BRepTextBuilder.hxx>
#include <TCollection_AsciiString.hxx>

Font_BRepTextBuilder builder;
TopoDS_Shape textShape = builder.Perform(
    "Hello World",
    Font_FA_Regular,
    20.0f  // 字体大小
);
// textShape 可直接用于 OCCT 建模（拉伸为 3D 文字）
```

**适用场景**：3D 软件中的文字渲染、3D 文字模型生成（标牌/铭牌）、工业 HMI 字体显示

---

## 库选型速查表

| 需求场景 | 推荐库 | 备注 |
|---------|--------|------|
| CAD 几何内核（C++） | OpenCASCADE (OCCT) | 工业标准，LGPL |
| CAD 几何内核（国产替代） | AMCAX 九韶内核 | 自主知识产权 |
| NURBS 曲面建模 | openNURBS / LNLib | .3dm 格式 / 算法学习 |
| 3D CAD 集成平台参考 | FreeCAD | 插件架构参考 |
| Web 端 CAD | JSketcher | WASM + TS |
| .NET CAD 开发 | Macad3D | C# + OCCT |
| Python CAD 原型 | PythonOCC-CAD | 快速验证 |
| 脚本化实体建模 | OpenSCAD | 版本控制友好 |
| 2D CAD | QCAD / LibreCAD | 专业/轻量 |
| 国防/工程实体建模 | BRL-CAD | 光线追踪 |
| 3D 场景渲染 | OpenSceneGraph (OSG) | 工业级场景图 |
| 科学可视化 | VTK | CAE 后处理 |
| 3D CAD 查看器参考 | Mayo | OCCT+Qt 最佳实践 |
| CFD 仿真 | OpenFOAM | 工业标准 CFD |
| CAE 集成平台（国产） | FastCAE | 插件化 CAE 框架 |
| 有限元计算 | deal.II | 高阶 FEM |
| 网格生成 | Gmsh | CAE 前处理 |
| PCB 设计 | LibrePCB | 开源 EDA |
| 2D 草图约束求解 | PlaneGCS / SolveSpace | FreeCAD 内核 / 独立 CAD |
| Ribbon UI（Qt） | SARibbon | Office 风格 |
| 空间索引加速 | libspatialindex | R-tree / KNN |
| BIM/IFC 格式 | IFC++ | MIT 许可 |
| 字体渲染 | FreeType | 工业标准 |
| 点云处理 | Open3D | ICP / 重建 |
| 计算几何算法 | CGAL | 网格 / 布尔 / 凸包 |
| 几何处理 | libigl | 网格变形 / UV |
<BRepBuilderAPI_MakeBox.hxx>
#include <BRepBuilderAPI_MakeCylinder.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <StlAPI_Writer.hxx>

// 1. 创建基础形体
TopoDS_Shape box = BRepBuilderAPI_MakeBox(100.0, 60.0, 40.0).Shape();
TopoDS_Shape cyl = BRepBuilderAPI_MakeCylinder(
    gp_Ax2(gp_Pnt(50, 30, 0), gp_Dir(0, 0, 1)), 15.0, 50.0
).Shape();

// 2. 布尔差运算（挖孔）
BRepAlgoAPI_Cut cutter(box, cyl);
cutter.Build();
if (!cutter.IsDone()) {
    throw Standard_Failure::Raise("Boolean cut failed");
}
TopoDS_Shape result = cutter.Shape();

// 3. 网格剖分（用于渲染/导出）
BRepMesh_IncrementalMesh mesher(result, 0.1);  // 偏差 0.1mm
mesher.Perform();

// 4. 导出 STL
StlAPI_Writer writer;
writer.Write(result, "output.stl");
```

**拓扑遍历**：
```cpp
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <BRep_Tool.hxx>
#include <Geom_Surface.hxx>

// 遍历所有面
TopExp_Explorer explorer(shape, TopAbs_FACE);
for (; explorer.More(); explorer.Next()) {
    TopoDS_Face face = TopoDS::Face(explorer.Current());
    Handle(Geom_Surface) surf = BRep_Tool::Surface(face);
    // 获取面的参数范围
    Standard_Real u1, u2, v1, v2;
    surf->Bounds(u1, u2, v1, v2);
}

// 遍历所有边
TopExp_Explorer edgeExplorer(shape, TopAbs_EDGE);
for (; edgeExplorer.More(); edgeExplorer.Next()) {
    TopoDS_Edge edge = TopoDS::Edge(edgeExplorer.Current());
    Standard_Real first, last;
    Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);
}
```

**STEP 文件读写**：
```cpp
#include <STEPCAFControl_Reader.hxx>
#include <STEPCAFControl_Writer.hxx>
#include <TDocStd_Document.hxx>
#include <XCAFApp_Application.hxx>

// 读取 STEP
Handle(XCAFApp_Application) app = XCAFApp_Application::GetApplication();
Handle(TDocStd_Document) doc;
app->NewDocument("MDTV-XCAF", doc);

STEPCAFControl_Reader reader;
IFSelect_ReturnStatus status = reader.ReadFile("model.step");
if (status == IFSelect_RetDone) {
    reader.Transfer(doc);
}

// 写入 STEP
STEPCAFControl_Writer writer;
writer.Transfer(doc, STEPControl_AsIs);
writer.Write("output.step");
```

**Python 绑定 (pythonocc-core)**：
```python
# pip install pythonocc-core
from OCC.Core.BRepBuilderAPI import BRepBuilderAPI_MakeBox, BRepBuilderAPI_MakeCylinder
from OCC.Core.BRepAlgoAPI import BRepAlgoAPI_Cut
from OCC.Core.BRepMesh import BRepMesh_IncrementalMesh
from OCC.Core.gp import gp_Ax2, gp_Pnt, gp_Dir
from OCC.Display.SimpleGui import init_display

# 创建形体
box = BRepBuilderAPI_MakeBox(100, 60, 40).Shape()
ax2 = gp_Ax2(gp_Pnt(50, 30, 0), gp_Dir(0, 0, 1))
cyl = BRepBuilderAPI_MakeCylinder(ax2, 15, 50).Shape()

# 布尔差
cut = BRepAlgoAPI_Cut(box, cyl)
result = cut.Shape()

# 可视化
display, start_display, add_menu, add_function_to_menu = init_display()
display.DisplayShape(result, update=True)
display.FitAll()
start_display()
```

#### CMake 集成
```cmake
cmake_minimum_required(VERSION 3.20)
project(MyCADApp)

find_package(OpenCASCADE REQUIRED)

add_executable(MyCADApp main.cpp)

target_include_directories(MyCADApp PRIVATE ${OpenCASCADE_INCLUDE_DIR})
target_link_libraries(MyCADApp
    TKernel TKMath TKBRep TKGeomBase TKGeomAlgo
    TKTopAlgo TKPrim TKBO TKSTEP TKMesh TKService
    TKV3d TKOpenGl TKAIS
)
```

#### 参考资源
- 官方文档：https://dev.opencascade.org/doc/overview/html/
- Python OCC：https://github.com/tpaviot/pythonocc-core
- OCCT 论坛：https://dev.opencascade.org/forums
- 学习示例：`OCCT/samples/` 目录下的 `Tutorial`、`qt` 示例

---

### OpenSceneGraph (OSG)

**GitHub**: [openscenegraph/OpenSceneGraph](https://github.com/openscenegraph/OpenSceneGraph)  
**官网**: http://www.openscenegraph.com/  
**语言**: C++  
**许可证**: OpenSceneGraph Public License (LGPL 类似)  
**最新稳定版**: 3.6.x（活跃维护）

#### 核心架构

```
OSG 场景图体系
├── osg::Node            场景节点基类
│   ├── osg::Group       分组节点（可含多个子节点）
│   │   ├── osg::Transform  变换节点
│   │   │   └── osg::MatrixTransform  矩阵变换
│   │   ├── osg::Switch  开关节点
│   │   └── osg::LOD     细节层次节点
│   └── osg::Geode       几何节点（叶节点，含 Drawable）
├── osg::Drawable        可绘制对象
│   └── osg::Geometry    几何体（顶点/法线/纹理坐标）
├── osg::StateSet        渲染状态（材质/纹理/着色器）
├── osgViewer::Viewer    单视图查看器
└── osgGA::*             交互操作（轨迹球/飞行/驾驶）
```

#### 源码目录结构（GitHub）

```
OpenSceneGraph/
├── include/osg/         # 核心头文件
├── include/osgViewer/   # 视图管理
├── include/osgGA/       # 用户交互
├── include/osgDB/       # 数据库/文件 IO
├── include/osgUtil/     # 工具类（优化器/相交测试）
├── src/osg/             # 核心实现
├── src/osgPlugins/      # 文件格式插件（obj/fbx/ive...）
├── examples/            # 官方示例（重点参考）
│   ├── osggeometry/     # 几何创建
│   ├── osgintersection/ # 相交检测
│   ├── osgpick/         # 鼠标拾取
│   └── osganimation/    # 动画
└── CMakeLists.txt
```

#### 集成示例 (C++)

**基础场景创建**：
```cpp
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>

// 创建三角形几何体
osg::ref_ptr<osg::Geometry> createTriangle() {
    osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;

    // 顶点数组
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    vertices->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    vertices->push_back(osg::Vec3(1.0f, 0.0f, 0.0f));
    vertices->push_back(osg::Vec3(0.5f, 1.0f, 0.0f));
    geom->setVertexArray(vertices);

    // 法线数组
    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
    normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
    geom->setNormalArray(normals, osg::Array::BIND_OVERALL);

    // 颜色数组
    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
    geom->setColorArray(colors, osg::Array::BIND_OVERALL);

    // 绘制基元
    geom->addPrimitiveSet(new osg::DrawArrays(GL_TRIANGLES, 0, 3));
    return geom;
}

int main() {
    // 构建场景图
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable(createTriangle());

    osg::ref_ptr<osg::MatrixTransform> transform = new osg::MatrixTransform;
    transform->setMatrix(osg::Matrix::translate(0, 0, 0));
    transform->addChild(geode);

    // 创建查看器
    osgViewer::Viewer viewer;
    viewer.setSceneData(transform);
    viewer.setCameraManipulator(new osgGA::TrackballManipulator);
    return viewer.run();
}
```

**加载模型文件**：
```cpp
#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osgViewer/Viewer>

// 读取多种格式（obj/fbx/ive/osg）
osg::ref_ptr<osg::Node> model = osgDB::readNodeFile("model.obj");
if (!model) {
    OSG_WARN << "Failed to load model" << std::endl;
    return -1;
}

// 场景优化（合并 StateSet、网格合批）
osgUtil::Optimizer optimizer;
optimizer.optimize(model,
    osgUtil::Optimizer::FLATTEN_STATIC_TRANSFORMS |
    osgUtil::Optimizer::MERGE_GEOMETRY |
    osgUtil::Optimizer::SHARE_DUPLICATE_STATE
);

osgViewer::Viewer viewer;
viewer.setSceneData(model);
return viewer.run();
```

**鼠标拾取（Pick）**：
```cpp
#include <osgUtil/LineSegmentIntersector>
#include <osgViewer/Viewer>

class PickHandler : public osgGA::GUIEventHandler {
public:
    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) override {
        if (ea.getEventType() != osgGA::GUIEventAdapter::RELEASE) return false;
        if (ea.getButton() != osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON) return false;

        osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
        if (!view) return false;

        // 构建射线
        osg::ref_ptr<osgUtil::LineSegmentIntersector> picker =
            new osgUtil::LineSegmentIntersector(
                osgUtil::Intersector::WINDOW, ea.getX(), ea.getY());

        osgUtil::IntersectionVisitor iv(picker);
        view->getCamera()->accept(iv);

        if (picker->containsIntersections()) {
            auto& intersection = picker->getFirstIntersection();
            osg::Vec3d hitPoint = intersection.getWorldIntersectPoint();
            OSG_NOTICE << "Hit: " << hitPoint << std::endl;

            // 获取被选中的节点
            osg::NodePath& nodePath = intersection.nodePath;
            if (!nodePath.empty()) {
                osg::Node* selectedNode = nodePath.back();
                OSG_NOTICE << "Node: " << selectedNode->getName() << std::endl;
            }
        }
        return false;
    }
};
```

**NodeVisitor 遍历**：
```cpp
#include <osg/NodeVisitor>

class PrintVisitor : public osg::NodeVisitor {
public:
    PrintVisitor() : osg::NodeVisitor(TRAVERSE_ALL_CHILDREN), _depth(0) {}

    void apply(osg::Node& node) override {
        for (int i = 0; i < _depth; ++i) std::cout << "  ";
        std::cout << node.className() << " : " << node.getName() << "\n";
        _depth++;
        traverse(node);
        _depth--;
    }
private:
    int _depth;
};

// 使用
PrintVisitor visitor;
sceneRoot->accept(visitor);
```

#### CMake 集成
```cmake
find_package(OpenSceneGraph REQUIRED
    COMPONENTS osg osgDB osgUtil osgViewer osgGA osgText
)

target_include_directories(MyApp PRIVATE ${OPENSCENEGRAPH_INCLUDE_DIRS})
target_link_libraries(MyApp ${OPENSCENEGRAPH_LIBRARIES})
```

#### 参考资源
- OSG 官方示例：`OpenSceneGraph/examples/` 目录
- OSG 3.0 Cookbook：https://github.com/xarray/osgRecipes
- OpenSceneGraph 快速入门：http://trac.openscenegraph.org/projects/osg/wiki/Support/Tutorials

---

### VTK (Visualization Toolkit)

**GitHub**: [Kitware/VTK](https://github.com/Kitware/VTK)  
**官网**: https://vtk.org/  
**语言**: C++ / Python / Java  
**许可证**: Apache 2.0
**最新稳定版**: 9.3.x  

#### 核心架构

```
VTK Pipeline（流水线）体系
数据源 (Source)
    ↓
过滤器 (Filter)  →  过滤器  →  过滤器
    ↓
映射器 (Mapper)
    ↓
Actor（演员）
    ↓
Renderer（渲染器）
    ↓
RenderWindow（渲染窗口）
```

#### 源码目录结构（GitHub）

```
VTK/
├── Common/
│   ├── Core/          # 基础类（vtkObject、vtkSmartPointer）
│   ├── DataModel/     # 数据模型（vtkPolyData、vtkUnstructuredGrid）
│   └── Math/          # 数学工具
├── Filters/
│   ├── Core/          # 核心过滤器（三角化、法线计算）
│   ├── General/       # 通用过滤器（裁剪、轮廓）
│   └── Modeling/      # 建模过滤器
├── Rendering/
│   ├── Core/          # 渲染核心（Actor、Renderer、Camera）
│   └── OpenGL2/       # OpenGL 渲染后端
├── IO/
│   ├── Geometry/      # STL/OBJ/PLY 读写
│   ├── Legacy/        # VTK 格式读写
│   └── XML/           # VTU/VTP 格式读写
├── Examples/          # 官方示例
└── Wrapping/Python/   # Python 绑定
```

#### 集成示例

**CAE 应力云图可视化**：
```python
import vtk
import numpy as np

# 1. 创建非结构化网格（模拟有限元网格）
points = vtk.vtkPoints()
for i, (x, y, z) in enumerate([(0,0,0),(1,0,0),(1,1,0),(0,1,0),(0.5,0.5,1)]):
    points.InsertNextPoint(x, y, z)

# 创建四面体单元
tet = vtk.vtkTetra()
tet.GetPointIds().SetId(0, 0)
tet.GetPointIds().SetId(1, 1)
tet.GetPointIds().SetId(2, 2)
tet.GetPointIds().SetId(3, 4)

cells = vtk.vtkCellArray()
cells.InsertNextCell(tet)

grid = vtk.vtkUnstructuredGrid()
grid.SetPoints(points)
grid.InsertNextCell(tet.GetCellType(), tet.GetPointIds())

# 2. 添加应力标量数据
stress = vtk.vtkFloatArray()
stress.SetName("VonMisesStress")
for val in [0.0, 50.0, 100.0, 75.0, 200.0]:
    stress.InsertNextValue(val)
grid.GetPointData().SetScalars(stress)

# 3. 设置颜色映射
lut = vtk.vtkLookupTable()
lut.SetNumberOfColors(256)
lut.SetHueRange(0.667, 0.0)  # 蓝→红
lut.Build()

# 4. 创建映射器和 Actor
mapper = vtk.vtkDataSetMapper()
mapper.SetInputData(grid)
mapper.SetLookupTable(lut)
mapper.SetScalarRange(0, 200)

actor = vtk.vtkActor()
actor.SetMapper(mapper)

# 5. 颜色条
scalarBar = vtk.vtkScalarBarActor()
scalarBar.SetLookupTable(lut)
scalarBar.SetTitle("Von Mises Stress (MPa)")
scalarBar.SetNumberOfLabels(5)

# 6. 渲染
renderer = vtk.vtkRenderer()
renderer.AddActor(actor)
renderer.AddActor(scalarBar)
renderer.SetBackground(0.1, 0.1, 0.2)

renderWindow = vtk.vtkRenderWindow()
renderWindow.AddRenderer(renderer)
renderWindow.SetSize(800, 600)

interactor = vtk.vtkRenderWindowInteractor()
interactor.SetRenderWindow(renderWindow)
renderWindow.Render()
interactor.Start()
```

**变形动画**：
```python
import vtk

# 读取网格
reader = vtk.vtkXMLUnstructuredGridReader()
reader.SetFileName("mesh.vtu")
reader.Update()
grid = reader.GetOutput()

# 使用 WarpVector 实现变形
warp = vtk.vtkWarpVector()
warp.SetInputData(grid)
warp.SetInputArrayToProcess(0, 0, 0,
    vtk.vtkDataObject.FIELD_ASSOCIATION_POINTS, "Displacement")
warp.SetScaleFactor(10.0)  # 放大系数
warp.Update()

mapper = vtk.vtkDataSetMapper()
mapper.SetInputConnection(warp.GetOutputPort())

actor = vtk.vtkActor()
actor.SetMapper(mapper)

# 动画回调
class AnimationCallback:
    def __init__(self, warp, renderWindow):
        self.warp = warp
        self.renderWindow = renderWindow
        self.scale = 0.0
        self.direction = 1.0

    def execute(self, caller, event):
        self.scale += 0.5 * self.direction
        if self.scale > 20 or self.scale < 0:
            self.direction *= -1
        self.warp.SetScaleFactor(self.scale)
        self.warp.Update()
        self.renderWindow.Render()
```

**STL 文件读写**：
```python
import vtk

# 读取 STL
reader = vtk.vtkSTLReader()
reader.SetFileName("model.stl")
reader.Update()

# 网格处理：计算法线
normals = vtk.vtkPolyDataNormals()
normals.SetInputConnection(reader.GetOutputPort())
normals.ComputePointNormalsOn()
normals.ComputeCellNormalsOn()
normals.Update()

# 写入 STL
writer = vtk.vtkSTLWriter()
writer.SetInputConnection(normals.GetOutputPort())
writer.SetFileName("output.stl")
writer.SetFileTypeToBinary()
writer.Write()
```

**Qt + VTK 集成 (C++)**：
```cpp
#include <QVTKOpenGLNativeWidget.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>

// Qt Widget 中嵌入 VTK
QVTKOpenGLNativeWidget* vtkWidget = new QVTKOpenGLNativeWidget(parent);
auto renderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
vtkWidget->setRenderWindow(renderWindow);

auto renderer = vtkSmartPointer<vtkRenderer>::New();
renderWindow->AddRenderer(renderer);

auto sphere = vtkSmartPointer<vtkSphereSource>::New();
sphere->SetRadius(5.0);

auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
mapper->SetInputConnection(sphere->GetOutputPort());

auto actor = vtkSmartPointer<vtkActor>::New();
actor->SetMapper(mapper);
renderer->AddActor(actor);
renderer->ResetCamera();
```

#### 参考资源
- VTK 示例库：https://kitware.github.io/vtk-examples/site/
- VTK 用户指南：https://vtk.org/documentation/
- Python VTK：`pip install vtk`

---

### FreeCAD

**GitHub**: [FreeCAD/FreeCAD](https://github.com/FreeCAD/FreeCAD)  
**官网**: https://www.freecad.org/  
**语言**: C++ / Python  
**许可证**: LGPL 2.1  
**最新稳定版**: 1.0.x  

#### 架构参考价值

FreeCAD 是最值得参考的开源 CAD 平台，其架构设计对工业软件开发有极高参考价值：

```
FreeCAD 架构
├── App 层（应用逻辑）
│   ├── Application      应用程序管理
│   ├── Document         文档管理（Undo/Redo）
│   ├── DocumentObject   文档对象基类
│   └── GeoFeature       几何特征基类
├── Gui 层（图形界面）
│   ├── Application      GUI 应用管理
│   ├── Document         GUI 文档
│   ├── ViewProvider     视图提供者（模型-视图分离）
│   └── Workbench        工作台机制（插件架构核心）
├── Mod 层（功能模块/工作台）
│   ├── Part             OCCT 几何建模
│   ├── PartDesign       参数化建模
│   ├── Sketcher         草图
│   ├── FEM              有限元分析
│   └── Path             CAM 路径
└── Base 层（基础设施）
    ├── PyObjectBase     Python 绑定基类
    ├── Persistence      序列化
    └── Exception        异常体系
```

#### 关键设计模式（供参考）

**Workbench 插件机制**：
```python
# FreeCAD 的工作台注册机制（参考用于设计自己的插件系统）
import FreeCADGui

class MyWorkbench(Workbench):
    MenuText = "My Workbench"
    ToolTip = "My custom workbench"
    Icon = "my_icon.svg"

    def Initialize(self):
        """工作台初始化，注册命令"""
        import MyCommands
        self.list = ["MyCommand1", "MyCommand2"]
        self.appendToolbar("My Tools", self.list)
        self.appendMenu("My Menu", self.list)

    def Activated(self):
        """工作台激活时调用"""
        pass

    def Deactivated(self):
        """工作台停用时调用"""
        pass

FreeCADGui.addWorkbench(MyWorkbench())
```

**ViewProvider 模型-视图分离**：
```python
# FreeCAD ViewProvider 模式（参考实现模型-视图分离）
import FreeCAD, FreeCADGui
from pivy import coin

class MyFeature(FreeCAD.DocumentObject):
    """数据模型层 - 只处理数据，不涉及显示"""
    def __init__(self, obj):
        obj.addProperty("App::PropertyFloat", "Length", "Dimensions")
        obj.addProperty("App::PropertyFloat", "Width", "Dimensions")
        obj.Proxy = self

    def execute(self, obj):
        """重新计算形状"""
        import Part
        shape = Part.makeBox(obj.Length, obj.Width, 10)
        obj.Shape = shape

class MyFeatureViewProvider:
    """视图层 - 只处理显示，不涉及业务逻辑"""
    def __init__(self, vobj):
        vobj.Proxy = self

    def getIcon(self):
        return ":/icons/my_icon.svg"

    def onChanged(self, vobj, prop):
        """视图属性变化时调用"""
        pass
```

**Python API 脚本化建模**：
```python
import FreeCAD as App
import Part

# 创建新文档
doc = App.newDocument("MyPart")

# 参数化建模
def create_bracket(length, width, height, hole_radius):
    """创建一个带孔的支架"""
    # 基础板
    base = Part.makeBox(length, width, height)

    # 挖孔
    hole = Part.makeCylinder(hole_radius, height * 2,
                              App.Vector(length/2, width/2, -height/2),
                              App.Vector(0, 0, 1))
    bracket = base.cut(hole)
    return bracket

bracket = create_bracket(100, 60, 10, 8)
Part.show(bracket)

# 导出 STEP
doc.ActiveObject.Shape.exportStep("bracket.step")

# 导出 STL
doc.ActiveObject.Shape.exportStl("bracket.stl")
```

#### 源码中值得参考的模块

| 模块路径 | 参考价值 |
|---------|----------|
| `src/App/Document.cpp` | 文档管理、Undo/Redo 实现 |
| `src/Gui/Workbench.cpp` | 插件/工作台注册机制 |
| `src/Gui/ViewProvider.cpp` | 模型-视图分离模式 |
| `src/Mod/Part/App/PartFeature.cpp` | OCCT 与 FreeCAD 文档对象集成 |
| `src/Mod/PartDesign/` | 参数化特征树实现 |
| `src/Mod/FEM/` | 有限元前后处理集成 |

---

### CGAL

**GitHub**: [CGAL/cgal](https://github.com/CGAL/cgal)  
**官网**: https://www.cgal.org/  
**语言**: C++ (Header-only)  
**许可证**: GPL 3.0 / LGPL 3.0（部分包）  

#### 核心能力

```
CGAL 功能包
├── 凸包算法（2D/3D Convex Hull）
├── 三角剖分（Delaunay Triangulation）
├── 网格生成（Surface/Volume Mesh Generation）
├── 布尔运算（Polygon Mesh Processing - PMP）
├── 空间搜索（KD-Tree、AABB Tree）
├── 曲面重建（Poisson Surface Reconstruction）
└── 计算几何（Minkowski Sum、Voronoi Diagram）
```

#### 集成示例 (C++)

**AABB 树加速碰撞检测**：
```cpp
#include <CGAL/Simple_cartesian.h>
#include <CGAL/AABB_tree.h>
#include <CGAL/AABB_traits.h>
#include <CGAL/AABB_triangle_primitive.h>

typedef CGAL::Simple_cartesian<double> K;
typedef K::Triangle_3 Triangle;
typedef K::Ray_3 Ray;
typedef K::Point_3 Point;
typedef std::list<Triangle>::iterator Iterator;
typedef CGAL::AABB_triangle_primitive<K, Iterator> Primitive;
typedef CGAL::AABB_traits<K, Primitive> Traits;
typedef CGAL::AABB_tree<Traits> Tree;

std::list<Triangle> triangles; // 填入三角形网格

// 构建 AABB 树（空间加速结构）
Tree tree(triangles.begin(), triangles.end());
tree.accelerate_distance_queries();

// 射线相交检测
Ray ray(Point(0, 0, 0), Point(1, 1, 1));
bool intersected = tree.do_intersect(ray);

// 最近点查询
Point query(5, 5, 5);
Point closest = tree.closest_point(query);
```

**网格布尔运算（PMP）**：
```cpp
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/corefinement.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Surface_mesh<K::Point_3> Mesh;
namespace PMP = CGAL::Polygon_mesh_processing;

Mesh mesh1, mesh2, result;
// ... 填入网格数据 ...

// 网格布尔差运算
PMP::corefine_and_compute_difference(mesh1, mesh2, result);
```

---

### libigl

**GitHub**: [libigl/libigl](https://github.com/libigl/libigl)  
**语言**: C++ (Header-only) / Python  
**许可证**: MPL 2.0  

#### 核心能力
```
libigl 功能
├── 网格处理（法线计算、平滑、简化）
├── 参数化（UV 展开）
├── 变形（ARAP、Biharmonic）
├── 网格分析（测地距离、曲率）
└── 可视化（内置 OpenGL 查看器）
```

**Python 示例**：
```python
import igl
import numpy as np

# 读取网格
v, f = igl.read_triangle_mesh("model.obj")

# 计算顶点法线
n = igl.per_vertex_normals(v, f)

# 计算测地距离
vs = np.array([0])  # 源点
vt = np.array([100])  # 目标点
d = igl.exact_geodesic(v, f, vs, vt)

# 网格平滑（Laplacian 平滑）
l = igl.cotmatrix(v, f)
m = igl.massmatrix(v, f, igl.MASSMATRIX_TYPE_VORONOI)
```

---

### Open3D

**GitHub**: [isl-org/Open3D](https://github.com/isl-org/Open3D)  
**官网**: http://www.open3d.org/  
**语言**: C++ / Python  
**许可证**: MIT  

#### 核心能力
```
Open3D 功能
├── 点云处理（读写、可视化、预处理）
├── 配准（ICP、RANSAC、FPFH 特征）
├── 3D 重建（TSDF、PointNet）
├── 网格处理（泊松重建、简化）
└── 深度学习集成（Open3D-ML）
```

**工业质检示例（点云 vs CAD 对比）**：
```python
import open3d as o3d
import numpy as np

# 1. 加载扫描点云和参考 CAD 网格
scan_pcd = o3d.io.read_point_cloud("scan.ply")
ref_mesh = o3d.io.read_triangle_mesh("reference.stl")

# 2. 点云预处理
scan_pcd = scan_pcd.voxel_down_sample(voxel_size=0.5)  # 体素下采样
scan_pcd.estimate_normals()

# 3. 粗配准（FPFH 特征 + RANSAC）
fpfh = o3d.pipelines.registration.compute_fpfh_feature(
    scan_pcd,
    o3d.geometry.KDTreeSearchParamHybrid(radius=5, max_nn=100)
)

ref_pcd = ref_mesh.sample_points_poisson_disk(number_of_points=50000)
ref_fpfh = o3d.pipelines.registration.compute_fpfh_feature(
    ref_pcd,
    o3d.geometry.KDTreeSearchParamHybrid(radius=5, max_nn=100)
)

result_ransac = o3d.pipelines.registration.registration_ransac_based_on_feature_matching(
    scan_pcd, ref_pcd, fpfh, ref_fpfh, True, 1.5
)

# 4. 精配准（ICP）
result_icp = o3d.pipelines.registration.registration_icp(
    scan_pcd, ref_pcd, 0.2,
    result_ransac.transformation,
    o3d.pipelines.registration.TransformationEstimationPointToPlane()
)

scan_pcd.transform(result_icp.transformation)

# 5. 计算偏差
distances = np.asarray(scan_pcd.compute_point_cloud_distance(ref_pcd))
print(f"最大偏差: {distances.max():.3f} mm")
print(f"均方根偏差: {np.sqrt(np.mean(distances**2)):.3f} mm")

# 6. 偏差可视化（热图）
colors = plt.get_cmap("jet")(distances / distances.max())[:, :3]
scan_pcd.colors = o3d.utility.Vector3dVector(colors)
o3d.visualization.draw_geometries([scan_pcd])
```

---

### Gmsh

**GitHub**: [gmsh-org/gmsh](https://github.com/gmsh-org/gmsh)  
**官网**: https://gmsh.info/  
**语言**: C++ / Python / Julia  
**许可证**: GPL 2.0+  

#### 核心能力
网格生成（1D/2D/3D）、CAD 几何内核（OCC 集成）、有限元前处理

```python
import gmsh

gmsh.initialize()
gmsh.model.add("bracket")

# 创建几何体
gmsh.model.occ.addBox(0, 0, 0, 100, 60, 10)
gmsh.model.occ.synchronize()

# 设置网格尺寸
gmsh.model.mesh.setSize(gmsh.model.getEntities(0), 5.0)

# 生成网格
gmsh.model.mesh.generate(3)
gmsh.model.mesh.optimize("Netgen")

# 导出
gmsh.write("bracket.msh")
gmsh.write("bracket.vtk")

gmsh.finalize()
```

---

### OpenFOAM

**GitHub**: [OpenFOAM/OpenFOAM-dev](https://github.com/OpenFOAM/OpenFOAM-dev)  
**官网**: https://openfoam.org/  
**语言**: C++  
**许可证**: GPL 3.0  

#### 核心能力
CFD 流体仿真、传热、多相流、湍流模型

**Python 后处理（使用 PyFoam）**：
```python
from PyFoam.RunDictionary.ParsedParameterFile import ParsedParameterFile
from PyFoam.Execution.BasicRunner import BasicRunner

# 修改仿真参数
controlDict = ParsedParameterFile("system/controlDict")
controlDict["endTime"] = 10.0
controlDict["writeInterval"] = 0.5
controlDict.writeFile()

# 读取结果
import numpy as np
import os

def read_field(case_dir, time, field_name):
    """读取 OpenFOAM 场数据"""
    field_file = os.path.join(case_dir, str(time), field_name)
    # 解析 OpenFOAM 场文件格式
    ...
```

---

### deal.II

**GitHub**: [dealii/dealii](https://github.com/dealii/dealii)  
**官网**: https://www.dealii.org/  
**语言**: C++  
**许可证**: LGPL 2.1  

#### 核心能力
有限元计算库、自适应网格加密、高阶有限元、并行计算

```cpp
#include 