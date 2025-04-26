# 网络游戏客户端

这是一个用C++实现的客户端，用于猜测服务器生成的随机数。

## 安装依赖

在 CentOS/RHEL 系统上：
```bash
sh scripts/build_deps.sh
```

## 编译方法

```bash
sh scripts/build.sh
```

## 运行方法

```bash
sh scripts/start.sh
```


## 程序说明

1. 程序会不断从服务器获取样本值（符合高斯分布的随机数）
2. 每次收集多个样本后计算平均值作为猜测值
3. 将猜测值提交给服务器
4. 所有的猜测结果会记录在 `game_client.log` 文件中

## 注意事项
- 程序每次提交猜测的间隔至少为0.9秒